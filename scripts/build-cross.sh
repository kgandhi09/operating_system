#!/usr/bin/env bash
# Build jk_os's cross compiler for the preset's arch, and the C library it
# builds against, from the sources in this repo: every later step compiles
# the target's code with these (CROSS_COMPILE, common.sh), for this arch as
# for the other, so nothing from the build machine's compiler or C library
# gets into the image (docs/self-hosting.md).
#
#   build/<arch>/cross      binutils and GCC (C, C++) for <arch>-linux-gnu,
#                           running on the build machine
#   build/<arch>/toolchain  their sysroot: the Linux headers and glibc (and
#                           later the whole toolchain the image carries,
#                           build-toolchain.sh); the C.UTF-8 locale
#
# The usual bootstrap: binutils, a first GCC (C only, no C library yet), the
# Linux headers, glibc built with that GCC, then the full GCC against glibc.
# Each piece is rebuilt only when its source tree or its options change.
source "$(dirname "$0")/common.sh"
need make gawk bison python3 "gcc"

for t in GCC BINUTILS GLIBC GMP MPFR MPC; do
    src="${t}_SRC"
    [[ -d "${!src}" ]] || die "no source at ${!src#"$ROOT_DIR"/} (see ${t}_TREE in versions.env)"
done

TC="$OUT_DIR/toolchain"            # the sysroot
CROSS="$CROSS_DIR"
XB="$OUT_DIR/xbuild"               # build directories
mkdir -p "$TC" "$CROSS" "$XB"
export PATH="$CROSS/bin:$PATH"
# The build machine's own compiler, for the programs that run here.
unset CC CXX CPP AR RANLIB STRIP CFLAGS CXXFLAGS CPPFLAGS LDFLAGS CROSS_COMPILE

# step <name> <src> <function> <options...>: as in build-toolchain.sh.
step() {
    local name="$1" src="$2" fn="$3"; shift 3
    local out="$XB/$name" stamp="$XB/$name.done" want rc=0
    want="$(tree_version "$src") $*"
    if [[ -f "$stamp" && "$stamp" -nt "$src" && "$(cat "$stamp")" == "$want" ]]; then
        return
    fi
    log "building $name $(tree_version "$src") ($ARCH) — see $out.log"
    rm -rf "$out"; mkdir -p "$out"
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

# GMP, MPFR and MPC for the build machine (GCC's own arithmetic), static.
b_numlibs() {
    local d="$CROSS/deps"
    mkdir -p "$2"/{gmp,mpfr,mpc}
    # GMP 6.3's configure tests fail under GCC 15's default C23.
    (cd "$2/gmp" && CFLAGS="-O2 -std=gnu17" "$GMP_SRC/configure" --prefix="$d" \
        --disable-shared --enable-static --with-pic && make -j"$JOBS" && make install)
    (cd "$2/mpfr" && "$MPFR_SRC/configure" --prefix="$d" --disable-shared --enable-static \
        --with-pic --with-gmp="$d" && make -j"$JOBS" && make install)
    (cd "$2/mpc" && "$MPC_SRC/configure" --prefix="$d" --disable-shared --enable-static \
        --with-pic --with-gmp="$d" --with-mpfr="$d" && make -j"$JOBS" && make install)
}

b_binutils() {
    (cd "$2" && "$BINUTILS_SRC/configure" --target="$TRIPLE" --prefix="$CROSS" \
        --with-sysroot="$TC" --disable-nls --disable-werror --disable-multilib --disable-gdb \
        --disable-gprofng --disable-gold --enable-deterministic-archives)
    make -C "$2" -j"$JOBS" MAKEINFO=true
    make -C "$2" install MAKEINFO=true
}

GCC_COMMON=(--target="$TRIPLE" --prefix="$CROSS" --with-sysroot="$TC" --disable-multilib
            --disable-nls --disable-bootstrap --with-gmp="$CROSS/deps" --with-mpfr="$CROSS/deps"
            --with-mpc="$CROSS/deps" --with-native-system-header-dir=/usr/include)
# The first GCC: C only, and without a C library to build against.
b_gcc1() {
    (cd "$2" && "$GCC_SRC/configure" "${GCC_COMMON[@]}" --enable-languages=c --with-newlib \
        --without-headers --disable-shared --disable-threads --disable-libatomic --disable-libgomp \
        --disable-libquadmath --disable-libssp --disable-libvtv --disable-libstdcxx \
        --disable-libsanitizer --disable-libitm)
    make -C "$2" -j"$JOBS" all-gcc all-target-libgcc MAKEINFO=true
    make -C "$2" install-gcc install-target-libgcc MAKEINFO=true
}

b_linux_headers() {
    make -C "$KERNEL_TREE_SRC" O="$2" ARCH="$KARCH" INSTALL_HDR_PATH="$TC/usr" headers_install
}

# glibc: everything in /usr/lib (one library directory, as the rest of the
# image), the dynamic loader there too (/lib and /lib64 lead there).
GLIBC_KERNEL=5.4   # the oldest kernel it supports: jk_os runs its own, newer
b_glibc() {
    printf 'slibdir=/usr/lib\nrtlddir=/usr/lib\nsbindir=/usr/bin\nrootsbindir=/usr/bin\n' > "$2/configparms"
    (cd "$2" && CC="$TRIPLE-gcc" CXX=false AR="$TRIPLE-ar" RANLIB="$TRIPLE-ranlib" \
        "$GLIBC_SRC/configure" --prefix=/usr --libdir=/usr/lib --libexecdir=/usr/lib \
        --host="$TRIPLE" --build="$BUILD_TRIPLE" --with-headers="$TC/usr/include" \
        --enable-kernel="$GLIBC_KERNEL" --disable-werror --disable-nscd --without-selinux \
        --enable-stack-protector=strong --enable-bind-now --disable-profile --disable-crypt \
        libc_cv_slibdir=/usr/lib)
    make -C "$2" -j"$JOBS"
    make -C "$2" install DESTDIR="$TC"
    # GCC (and programs built on jk_os) look in lib64 too; the loader's path
    # in every program is /lib64/... (x86_64) or /lib/... (aarch64).
    ln -sfn lib "$TC/usr/lib64"
    ln -sfn usr/lib "$TC/lib"
    ln -sfn usr/lib "$TC/lib64"
}

# The full GCC (C, C++), against glibc.
b_gcc() {
    (cd "$2" && "$GCC_SRC/configure" "${GCC_COMMON[@]}" --enable-languages=c,c++ --enable-shared \
        --enable-threads=posix --enable-__cxa_atexit --enable-clocale=gnu --disable-libsanitizer \
        --disable-libssp --enable-default-pie --enable-default-ssp --enable-linker-build-id)
    make -C "$2" -j"$JOBS" MAKEINFO=true
    make -C "$2" install MAKEINFO=true
    # One library directory in the sysroot too (libstdc++ and friends are in
    # $CROSS/$TRIPLE/lib*, used from there).
}

# C.UTF-8, compiled from glibc's own locale source: by the new glibc's
# localedef when it runs here (same arch), else by the build machine's.
b_locale() {
    local localedef=(localedef)
    if [[ "$ARCH" == "$HOST_ARCH" ]]; then
        localedef=("$TC/usr/lib/$LDSO" --library-path "$TC/usr/lib" "$TC/usr/bin/localedef")
    fi
    command -v "${localedef[0]}" >/dev/null || die "no localedef on the build machine (glibc's)"
    mkdir -p "$TC/usr/lib/locale"
    I18NPATH="$GLIBC_SRC/localedata" "${localedef[@]}" --no-archive -c -i C -f UTF-8 \
        "$TC/usr/lib/locale/C.UTF-8" || [[ -d "$TC/usr/lib/locale/C.UTF-8" ]]
    # glibc looks a locale up by its normalised name too.
    ln -sfn C.UTF-8 "$TC/usr/lib/locale/C.utf8"
    touch "$2/ok"
}

step cross-numlibs  "$GMP_SRC"          b_numlibs
step cross-binutils "$BINUTILS_SRC"     b_binutils "$TRIPLE"
step cross-gcc1     "$GCC_SRC"          b_gcc1 "$TRIPLE"
step linux-headers  "$KERNEL_TREE_SRC"  b_linux_headers "$KARCH"
step glibc          "$GLIBC_SRC"        b_glibc "$TRIPLE kernel>=$GLIBC_KERNEL"
step cross-gcc      "$GCC_SRC"          b_gcc "$TRIPLE"
step locale         "$GLIBC_SRC"        b_locale C.UTF-8

log "cross compiler ($TRIPLE): $CROSS; glibc $(tree_version "$GLIBC_SRC") in $TC"
