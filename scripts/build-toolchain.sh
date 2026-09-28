#!/usr/bin/env bash
# Build the C/C++ toolchain that ships in the OS image, into
# build/<arch>/toolchain (prefix /usr; build-rootfs.sh copies it in):
#
#   GCC (C, C++, libstdc++) and binutils, Clang + lld + clangd/clang-format/
#   clang-tidy (LLVM), CMake, Ninja, GDB, the glibc headers and libraries
#   (from the toolchain that built jk_os) and the Linux headers (from
#   jk_os's kernel). C++20 (gnu++20) is the default standard of both
#   compilers (GCC 16's own default; Clang gets it from /etc/clang).
#
# The tree doubles as the sysroot the target libraries are built against.
# For aarch64 (built on x86_64) a GCC cross compiler is built first
# (build/aarch64/cross), and everything else is cross-built with it.
#
# A package is rebuilt only when its source tree or its build options change.
source "$(dirname "$0")/common.sh"
need make cmake ninja python3 "${CROSS_COMPILE}gcc"

for t in "${TC_TREES[@]}"; do
    src="${t}_SRC"
    [[ -d "${!src}" ]] || die "no source at ${!src#"$ROOT_DIR"/} (see ${t}_TREE in versions.env)"
done

TRIPLE="$ARCH-linux-gnu"                       # what jk_os's compilers target
BUILD_TRIPLE="$(normalize_arch "$(uname -m)")-linux-gnu"
TC="$OUT_DIR/toolchain"                        # install root = target sysroot
TC_OUT="$OUT_DIR/tc"                           # build directories
DEPS="$OUT_DIR/tc-deps"                        # GMP/MPFR/MPC for the target, static
DYN="$OUT_DIR/dyn"                             # the network stack (readline, expat, OpenSSL)
[[ -d "$DYN/usr/include" ]] || die "network stack not built yet (run: make network)"
mkdir -p "$TC" "$TC_OUT" "$DEPS"

# Build this toolchain with: the host compiler for x86_64 on x86_64, the GCC
# cross compiler built below for aarch64.
if [[ "$ARCH" == "$HOST_ARCH" ]]; then
    TCC=gcc TCXX=g++
    CROSS=""
else
    CROSS="$OUT_DIR/cross"
    TCC="$CROSS/bin/$TRIPLE-gcc" TCXX="$CROSS/bin/$TRIPLE-g++"
    export PATH="$CROSS/bin:$PATH"
fi
unset CROSS_COMPILE CC CXX CFLAGS CXXFLAGS CPPFLAGS LDFLAGS

# step <name> <src> <function> <options...>: as in build-network.sh.
step() {
    local name="$1" src="$2" fn="$3"; shift 3
    local out="$TC_OUT/$name" stamp="$TC_OUT/$name.done" want rc=0
    want="$(tree_version "$src") $*"
    if [[ -f "$stamp" && "$stamp" -nt "$src" && "$(cat "$stamp")" == "$want" ]]; then
        return
    fi
    log "building $name $(tree_version "$src") ($ARCH) — see $out.log"
    # An interrupted build with the same options goes on where it stopped
    # (LLVM takes hours); different options start over.
    if [[ "$(cat "$out.options" 2>/dev/null)" != "$want" ]]; then
        rm -rf "$out"
        echo "$want" > "$out.options"
    fi
    mkdir -p "$out"
    set +e
    ( set -e; "$fn" "$src" "$out" "$@" ) >"$out.log" 2>&1
    rc=$?
    set -e
    if (( rc != 0 )); then
        tail -n 30 "$out.log" >&2
        die "$name failed, see $out.log"
    fi
    echo "$want" > "$stamp"
}

# ------------------------------------------------------------ glibc, Linux

# The glibc the image runs on (build-rootfs.sh copies its runtime from the
# same toolchain): headers, crt files, static and shared libraries, taken
# from the Debian/Ubuntu packages of that toolchain and flattened into
# /usr/include and /usr/lib.
glibc_packages() {
    if [[ "$ARCH" == "$HOST_ARCH" ]]; then echo libc6 libc6-dev
    else echo "libc6-$(case $ARCH in aarch64) echo arm64 ;; x86_64) echo amd64 ;; esac)-cross" \
              "libc6-dev-$(case $ARCH in aarch64) echo arm64 ;; x86_64) echo amd64 ;; esac)-cross"; fi
}
b_glibc() {
    command -v dpkg >/dev/null || die "the toolchain build takes glibc from Debian/Ubuntu packages (dpkg)"
    local pkg f rel
    for pkg in $(glibc_packages); do
        dpkg -L "$pkg" >/dev/null || die "package $pkg not installed (run: make deps)"
        dpkg -L "$pkg" | while IFS= read -r f; do
            [[ -f "$f" || -L "$f" ]] || continue
            case "$f" in
                */gconv/*|*/audit/*|/usr/share/*|/etc/*|*/bin/*|*/sbin/*) continue ;;
                /usr/include/*)                   rel="usr/include/${f#/usr/include/}"
                                                  rel="${rel/usr\/include\/$TRIPLE\//usr/include/}" ;;
                /usr/lib/$TRIPLE/*)               rel="usr/lib/${f#/usr/lib/$TRIPLE/}" ;;
                /usr/$TRIPLE/include/*)           rel="usr/include/${f#/usr/$TRIPLE/include/}" ;;
                /usr/$TRIPLE/lib/*|/usr/$TRIPLE/lib64/*) rel="usr/lib/${f#/usr/$TRIPLE/lib*/}" ;;
                *) continue ;;
            esac
            mkdir -p "$TC/$(dirname "$rel")"
            if [[ -L "$f" && "$(readlink "$f")" == /* ]]; then
                ln -sfn "$(basename "$(readlink "$f")")" "$TC/$rel"   # absolute link: same directory now
            else
                cp -P "$f" "$TC/$rel"
            fi
        done
    done
    # Linker scripts (libc.so, libm.so) name the libraries by their old paths.
    for f in "$TC"/usr/lib/lib*.so; do
        [[ -f "$f" && ! -L "$f" ]] && grep -q '^GROUP' "$f" && sed -i -E 's|/[^ ()]*/||g' "$f"
    done
    # GCC looks in lib64 on both arches; the loader is at /lib64 or /lib.
    ln -sfn lib "$TC/usr/lib64"
    ln -sfn usr/lib "$TC/lib"
    ln -sfn usr/lib "$TC/lib64"
    touch "$2/ok"
}

b_linux_headers() {
    make -C "$KERNEL_SRC" O="$2" ARCH="$KARCH" INSTALL_HDR_PATH="$TC/usr" headers_install
}

# ------------------------------------------------------------ GMP, MPFR, MPC

# numlibs <prefix> <host triple> <cc>: GMP, MPFR and MPC as static libraries.
numlibs() {
    local prefix="$1" host="$2" cc="$3" out="$4"
    mkdir -p "$out"/{gmp,mpfr,mpc}
    # GMP 6.3's configure tests fail under GCC 15's default C23.
    (cd "$out/gmp" && CC="$cc" CFLAGS="-O2 -std=gnu17" "$GMP_SRC/configure" --host="$host" --prefix="$prefix" \
        --disable-shared --enable-static --with-pic && make -j"$JOBS" && make install)
    (cd "$out/mpfr" && CC="$cc" "$MPFR_SRC/configure" --host="$host" --prefix="$prefix" \
        --disable-shared --enable-static --with-pic --with-gmp="$prefix" && make -j"$JOBS" && make install)
    (cd "$out/mpc" && CC="$cc" "$MPC_SRC/configure" --host="$host" --prefix="$prefix" \
        --disable-shared --enable-static --with-pic --with-gmp="$prefix" --with-mpfr="$prefix" \
        && make -j"$JOBS" && make install)
}
b_numlibs_target() { numlibs "$DEPS" "$TRIPLE" "$TCC" "$2"; }
b_numlibs_build()  { numlibs "$CROSS/deps" "$BUILD_TRIPLE" gcc "$2"; }

# ------------------------------------------------------------ cross (aarch64)

b_cross_binutils() {
    (cd "$2" && "$BINUTILS_SRC/configure" --target="$TRIPLE" --prefix="$CROSS" \
        --with-sysroot="$TC" --disable-nls --disable-werror --disable-multilib --disable-gdb \
        --disable-gprofng --disable-gold)
    make -C "$2" -j"$JOBS"
    make -C "$2" install
}
b_cross_gcc() {
    (cd "$2" && "$GCC_SRC/configure" --target="$TRIPLE" --prefix="$CROSS" --with-sysroot="$TC" \
        --enable-languages=c,c++ --disable-multilib --disable-nls --disable-bootstrap \
        --disable-libsanitizer --disable-libssp --enable-threads=posix --enable-__cxa_atexit \
        --with-gmp="$CROSS/deps" --with-mpfr="$CROSS/deps" --with-mpc="$CROSS/deps")
    make -C "$2" -j"$JOBS"
    make -C "$2" install
}

# ------------------------------------------------------------ the toolchain

TARGET_ARGS=(--build="$BUILD_TRIPLE" --host="$TRIPLE" --target="$TRIPLE" --prefix=/usr
             --libdir=/usr/lib --libexecdir=/usr/lib --disable-nls --disable-werror)

b_binutils() {
    (cd "$2" && CC="$TCC" CXX="$TCXX" "$BINUTILS_SRC/configure" "${TARGET_ARGS[@]}" \
        --with-sysroot=/ --disable-multilib --disable-gdb --disable-gdbserver --disable-sim \
        --disable-gprofng --disable-gold --enable-plugins --enable-deterministic-archives \
        --enable-default-hash-style=gnu --with-build-sysroot="$TC")
    make -C "$2" -j"$JOBS"
    make -C "$2" install DESTDIR="$TC"
}

b_gcc() {
    # build != host for aarch64: the target libraries are built by the cross
    # compiler above ($TRIPLE-gcc on $PATH); for x86_64 by the new compiler.
    # The host's binutils have to be named too, or the plugin checks find no nm.
    # libcc1 (GDB's "compile" command plugin) is left out: its configure can't
    # check a cross-built host (it needs an objdump it never looks for).
    local host_tools=() extra=()
    if [[ -n "$CROSS" ]]; then
        host_tools=(AR="$TRIPLE-ar" NM="$TRIPLE-nm" RANLIB="$TRIPLE-ranlib"
                    OBJDUMP="$TRIPLE-objdump" STRIP="$TRIPLE-strip")
        extra=(--disable-libcc1)
    fi
    (cd "$2" && env CC="$TCC" CXX="$TCXX" "${host_tools[@]}" "$GCC_SRC/configure" "${TARGET_ARGS[@]}" \
        --with-build-sysroot="$TC" --with-native-system-header-dir=/usr/include \
        --enable-languages=c,c++ --disable-multilib --disable-multiarch --disable-bootstrap \
        --enable-shared --enable-threads=posix --enable-__cxa_atexit --enable-clocale=gnu \
        --enable-default-pie --enable-default-ssp --enable-linker-build-id --enable-lto \
        --enable-plugin --with-gmp="$DEPS" --with-mpfr="$DEPS" --with-mpc="$DEPS" \
        --with-pkgversion="$OS_NAME $OS_VERSION" "${extra[@]}")
    make -C "$2" -j"$JOBS"
    make -C "$2" install DESTDIR="$TC"
    # One library directory: what GCC put in lib64 goes to lib (lib64 links there).
    if [[ -d "$TC/usr/lib64" && ! -L "$TC/usr/lib64" ]]; then
        cp -a "$TC/usr/lib64/." "$TC/usr/lib/"; rm -rf "$TC/usr/lib64"; ln -s lib "$TC/usr/lib64"
    fi
    ln -sf gcc "$TC/usr/bin/cc"
    ln -sf g++ "$TC/usr/bin/c++"
    # C++20 (gnu++20) is GCC 16's own default; check it stays so.
    grep -q 'set_std_cxx20 (/\*ISO\*/false);' "$GCC_SRC/gcc/c-family/c-opts.cc" \
        || echo "note: this GCC's default C++ standard is not C++20" >&2
}

# CMake settings for building a target program with this toolchain.
cmake_target_args() {
    echo -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib \
         -DCMAKE_C_COMPILER="$TCC" -DCMAKE_CXX_COMPILER="$TCXX"
    if [[ -n "$CROSS" ]]; then
        echo -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR="$ARCH" \
             "-DCMAKE_FIND_ROOT_PATH=$TC;$DYN" -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
             -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
             -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY
    fi
}

b_llvm() {
    local targets="X86;AArch64;ARM;RISCV"
    # LLVM_APPEND_VC_REV=OFF: the source sits inside this repo, and LLVM would
    # otherwise put the repo's git URL and commit into "clang --version".
    # shellcheck disable=SC2046 # word-split the option list
    cmake -G Ninja -S "$1/llvm" -B "$2" $(cmake_target_args) \
        -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra" -DLLVM_TARGETS_TO_BUILD="$targets" \
        -DLLVM_HOST_TRIPLE="$TRIPLE" -DLLVM_DEFAULT_TARGET_TRIPLE="$TRIPLE" \
        -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_LINK_LLVM_DYLIB=ON -DCLANG_LINK_CLANG_DYLIB=ON \
        -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_INCLUDE_DOCS=OFF -DCLANG_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_DOCS=OFF \
        -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_LIBXML2=OFF \
        -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_LIBPFM=OFF -DLLVM_ENABLE_BINDINGS=OFF \
        -DLLVM_INSTALL_UTILS=OFF -DCLANG_DEFAULT_CXX_STDLIB=libstdc++ \
        -DCLANG_CONFIG_FILE_SYSTEM_DIR=/etc/clang -DCLANG_ENABLE_ARCMT=OFF \
        -DCLANG_ENABLE_STATIC_ANALYZER=ON -DLLVM_PARALLEL_LINK_JOBS=2 \
        -DLLVM_APPEND_VC_REV=OFF -DCLANG_VENDOR="$OS_NAME"
    ninja -C "$2" -j"$JOBS"
    DESTDIR="$TC" ninja -C "$2" -j"$JOBS" install
}

b_cmake() {
    # shellcheck disable=SC2046
    cmake -G Ninja -S "$1" -B "$2" $(cmake_target_args) \
        -DBUILD_TESTING=OFF -DBUILD_CursesDialog=OFF -DCMAKE_USE_OPENSSL=ON \
        -DOPENSSL_ROOT_DIR="$DYN/usr" -DCMAKE_DOC_DIR=share/doc/cmake
    ninja -C "$2" -j"$JOBS"
    DESTDIR="$TC" ninja -C "$2" -j"$JOBS" install
}

b_ninja() {
    # shellcheck disable=SC2046
    cmake -G Ninja -S "$1" -B "$2" $(cmake_target_args) -DBUILD_TESTING=OFF
    ninja -C "$2" -j"$JOBS"
    DESTDIR="$TC" ninja -C "$2" -j"$JOBS" install
}

b_gdb() {
    # Include paths go in CFLAGS/CXXFLAGS: the sub-configures don't get CPPFLAGS.
    (cd "$2" && CC="$TCC" CXX="$TCXX" CFLAGS="-O2 -I$DYN/usr/include" CXXFLAGS="-O2 -I$DYN/usr/include" \
        LDFLAGS="-L$DYN/usr/lib -Wl,-rpath-link,$DYN/usr/lib" "$GDB_SRC/configure" "${TARGET_ARGS[@]}" \
        --with-gmp="$DEPS" --with-mpfr="$DEPS" --with-system-readline --with-expat \
        --with-libexpat-prefix="$DYN/usr" --without-python --without-guile --without-debuginfod \
        --without-babeltrace --without-xxhash --without-lzma --without-zstd --disable-sim \
        --disable-gprofng --disable-source-highlight --with-curses)
    make -C "$2" -j"$JOBS"
    make -C "$2" install DESTDIR="$TC"
}

step glibc          "$KERNEL_SRC"   b_glibc "$(glibc_packages)"
step linux-headers  "$KERNEL_SRC"   b_linux_headers
if [[ -n "$CROSS" ]]; then
    step cross-numlibs  "$GMP_SRC"      b_numlibs_build
    step cross-binutils "$BINUTILS_SRC" b_cross_binutils
    step cross-gcc      "$GCC_SRC"      b_cross_gcc
fi
step numlibs  "$GMP_SRC"      b_numlibs_target
step binutils "$BINUTILS_SRC" b_binutils
step gcc      "$GCC_SRC"      b_gcc
step ninja    "$NINJA_SRC"    b_ninja
step cmake    "$CMAKE_SRC"    b_cmake
step gdb      "$GDB_SRC"      b_gdb
step llvm     "$LLVM_SRC"     b_llvm

# Clang's C++ standard, like GCC's (config file in CLANG_CONFIG_FILE_SYSTEM_DIR).
mkdir -p "$TC/etc/clang"
printf '# Read by clang++ on every run; a -std= on the command line wins.\n-std=gnu++20\n' \
    > "$TC/etc/clang/clang++.cfg"
log "toolchain ($ARCH): $(du -sh "$TC" | cut -f1) in $TC"
