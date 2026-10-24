#!/usr/bin/env bash
# Build the build machine's own tools: everything the build runs on this
# machine beyond the floor (scripts/lib-host.sh), from the sources in this
# repo, into build/host/<host arch>/install, so that nothing comes from a
# package manager (docs/self-hosting.md). Every preset and every arch built on
# this machine shares them; common.sh puts install/bin first on PATH.
#
#   Python (with Mesa's modules), Perl, pkg-config, ninja, CMake, meson
#   (a wrapper for userspace/desktop/meson), m4, bison, flex, gperf,
#   autoconf, automake, libtool, gettext, libxslt, bc, cpio
#   OpenSSL, libelf, zlib, bzip2, xz, zstd, lz4 (the kernel's and U-Boot's
#   own programs link them)
#   squashfs-tools, xorriso, mtools, dosfstools, dtc, kmod, file
#   GLib's and gdk-pixbuf's generators, glslang
#   rsync, curl, jq, unzip, telnet, gpgv, heimdall
#
# First the GNU basics (coreutils, sed, grep, gawk, findutils, diffutils,
# patch, tar, gzip): after them the build runs these, whatever the build
# machine's own are (BusyBox's on a jk_os PC). Built with the floor's compiler. Each is rebuilt only when its source tree
# or its options change (stamps in build/host/<host arch>); the work
# directories can go once it's built (make PRESET=<preset> prune).
source "$(dirname "$0")/common.sh"

H="$HOST_WORK"
mkdir -p "$H" "$HOSTDIR"
host_lock

# The floor's compiler, and only what is built here: the build machine's own
# development files (/usr/lib/pkgconfig, ...) are never looked for.
unset CC CXX CPP AR RANLIB STRIP CFLAGS CXXFLAGS CPPFLAGS LDFLAGS PKG_CONFIG_SYSROOT_DIR CROSS_COMPILE
export CC=gcc CXX=g++
export CPPFLAGS="-I$HOSTDIR/include"
# (RPATH, not RUNPATH: searched before any LD_LIBRARY_PATH.)
export LDFLAGS="-L$HOSTDIR/lib -Wl,-rpath,$HOSTDIR/lib -Wl,--disable-new-dtags"
export PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR="$HOSTDIR/lib/pkgconfig:$HOSTDIR/share/pkgconfig"
export ACLOCAL_PATH="$HOSTDIR/share/aclocal"
HOST_TRIPLE="$HOST_ARCH-linux-gnu"
TC_SRC="$ROOT_DIR/userspace/toolchain"
NET_SRC="$ROOT_DIR/userspace/network"
DESK_SRC="$ROOT_DIR/userspace/desktop"
HT_SRC="$ROOT_DIR/userspace/hosttools"
[[ -d "$HT_SRC" ]] || die "no userspace/hosttools (run: scripts/update-desktop-sources.sh --component hosttools)"

# step <name> <src> <function> <options...>: run "<function> <src> <build dir>
# <options...>" unless the same options were already built from this tree
# (as build-network.sh's step, in build/host/<host arch>).
step() {
    local name="$1" src="$2" fn="$3"; shift 3
    local out="$H/$name" stamp="$H/$name.done" want rc=0
    [[ -d "$src" ]] || die "no source for $name at ${src#"$ROOT_DIR"/}"
    want="$(tree_version "$src") $*"
    if [[ -f "$stamp" && "$stamp" -nt "$src" && "$(cat "$stamp")" == "$want" ]]; then
        return
    fi
    log "building host tool $name $(tree_version "$src") ($HOST_ARCH)"
    rm -rf "$out"; mkdir -p "$out"
    set +e
    ( set -e; "$fn" "$src" "$out" "$@" ) >"$out.log" 2>&1
    rc=$?
    set -e
    if (( rc != 0 )); then
        tail -n 30 "$out.log" >&2
        die "host tool $name failed, see $out.log"
    fi
    echo "$want" > "$stamp"
}

copy_src() { rm -rf "$2"; mkdir -p "$2"; cp -a "$1/." "$2/"; }
autotools() {   # autotools <src> <out> <configure args...>
    local src="$1" out="$2"; shift 2
    (cd "$out" && "$src/configure" --prefix="$HOSTDIR" --libdir="$HOSTDIR/lib" --disable-dependency-tracking "$@")
    make -C "$out" -j"$JOBS"
    make -C "$out" install
}
# autotools_in: the same, built inside a copy of the tree (projects whose
# build doesn't work out of tree).
autotools_in() { local src="$1" out="$2"; shift 2; copy_src "$src" "$out"; autotools "$out" "$out" "$@"; }
mesonpkg() {    # mesonpkg <src> <out> <meson options...>
    local src="$1" out="$2"; shift 2
    meson setup "$out" "$src" --prefix="$HOSTDIR" --libdir=lib --buildtype=release \
        --wrap-mode=nodownload -Ddefault_library=shared "$@"
    ninja -C "$out" -j"$JOBS"
    meson install -C "$out" --no-rebuild
}
cmakepkg() {    # cmakepkg <src> <out> <cmake options...>
    local src="$1" out="$2"; shift 2
    cmake -G Ninja -S "$src" -B "$out" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOSTDIR" \
        -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_PREFIX_PATH="$HOSTDIR" -DCMAKE_INSTALL_RPATH="$HOSTDIR/lib" \
        -DCMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH=OFF -DBUILD_TESTING=OFF "$@"
    ninja -C "$out" -j"$JOBS"
    ninja -C "$out" install
}

# ---------------------------------------------------------------- the GNU basics
# FORCE_UNSAFE_CONFIGURE: coreutils' and tar's configure refuse root, which a
# jk_os build may be.
export FORCE_UNSAFE_CONFIGURE=1
step coreutils "$HT_SRC/coreutils" autotools --disable-nls --enable-no-install-program=kill,uptime \
    --without-selinux --disable-acl --disable-xattr --without-openssl --without-libgmp
step sed       "$HT_SRC/sed"       autotools --disable-nls --disable-acl --without-selinux
step grep      "$HT_SRC/grep"      autotools --disable-nls --disable-perl-regexp
step gawk      "$HT_SRC/gawk"      autotools --disable-nls --without-readline \
    --without-mpfr
step findutils "$HT_SRC/findutils" autotools --disable-nls --without-selinux
step diffutils "$HT_SRC/diffutils" autotools --disable-nls
step patch     "$HT_SRC/patch"     autotools --disable-xattr
step tar       "$HT_SRC/tar"       autotools --disable-nls --disable-acl --without-selinux --without-xattrs
step gzip      "$HT_SRC/gzip"      autotools
hash -r

# ---------------------------------------------------------------- the basics
# A few small commands, from BusyBox: bc (the kernel's build), cpio (boot
# images), telnet (tablet-install.sh), unzip (fetch-binaries.sh).
BB_APPLETS="bc cpio telnet unzip"
b_busybox() {
    make -C "$1" O="$2" allnoconfig >/dev/null
    local a
    for a in LONG_OPTS $(tr 'a-z' 'A-Z' <<< "$BB_APPLETS") FEATURE_CPIO_O FEATURE_UNZIP_CDF \
             FEATURE_UNZIP_BZIP2 FEATURE_UNZIP_LZMA FEATURE_UNZIP_XZ FEATURE_SEAMLESS_GZ; do
        sed -i "s/^# CONFIG_$a is not set\$/CONFIG_$a=y/" "$2/.config"
    done
    # (yes ends on SIGPIPE once oldconfig has its answers.)
    { yes "" || true; } | make -C "$1" O="$2" oldconfig >/dev/null
    make -C "$1" O="$2" -j"$JOBS" EXTRA_CFLAGS="${NOWARN_CFLAGS:-}" busybox
    install -d "$HOSTDIR/libexec/busybox" "$HOSTDIR/bin"
    install -m 0755 "$2/busybox" "$HOSTDIR/libexec/busybox/busybox"
    for a in $BB_APPLETS; do ln -sf ../libexec/busybox/busybox "$HOSTDIR/bin/$a"; done
}
step busybox "$ROOT_DIR/$BUSYBOX_TREE" b_busybox "$BB_APPLETS"

step m4      "$TC_SRC/m4"      autotools
step bison   "$TC_SRC/bison"   autotools --disable-nls
step flex    "$TC_SRC/flex"    autotools --disable-nls
step gperf   "$HT_SRC/gperf"   autotools
step pkgconf "$DESK_SRC/pkgconf" autotools --with-system-libdir=/usr/lib --with-system-includedir=/usr/include \
    --with-pkg-config-dir="$HOSTDIR/lib/pkgconfig:$HOSTDIR/share/pkgconfig"
ln -sf pkgconf "$HOSTDIR/bin/pkg-config"

# Compression: libraries (Python's modules, squashfs-tools, kmod, the
# kernel's tools) and their commands.
b_zlib() { (cd "$2" && "$1/configure" --prefix="$HOSTDIR" --libdir="$HOSTDIR/lib"); make -C "$2" -j"$JOBS"; make -C "$2" install; }
b_bzip2() {
    copy_src "$1" "$2"
    make -C "$2" -f Makefile-libbz2_so -j"$JOBS" CC=gcc
    make -C "$2" -j"$JOBS" CC=gcc bzip2 bzip2recover
    install -Dm755 "$2/bzip2" "$HOSTDIR/bin/bzip2"
    install -Dm644 "$2/bzlib.h" "$HOSTDIR/include/bzlib.h"
    install -Dm755 "$2"/libbz2.so.1.0.* "$HOSTDIR/lib/"
    ln -sf "$(basename "$2"/libbz2.so.1.0.*)" "$HOSTDIR/lib/libbz2.so.1.0"
    ln -sf libbz2.so.1.0 "$HOSTDIR/lib/libbz2.so"
}
b_zstd() { mesonpkg "$1/build/meson" "$2" -Dbin_programs=true -Dbin_contrib=false -Dzlib=disabled \
    -Dlzma=disabled -Dlz4=disabled; }
b_lz4() {
    copy_src "$1" "$2"
    make -C "$2" -j"$JOBS" CC=gcc PREFIX="$HOSTDIR" lib lz4
    make -C "$2" PREFIX="$HOSTDIR" LIBDIR="$HOSTDIR/lib" install
}
step zlib  "$NET_SRC/zlib"  b_zlib
step bzip2 "$TC_SRC/bzip2"  b_bzip2
step xz    "$DESK_SRC/xz"   autotools --disable-nls --disable-doc
step zstd  "$DESK_SRC/zstd" b_zstd
step lz4   "$HT_SRC/lz4"    b_lz4
# FreeType: GRUB's grub-mkfont (build-iso.sh), for the font grub-mkrescue
# draws the ISO's label with.
step freetype "$DESK_SRC/freetype" mesonpkg -Dbrotli=disabled -Dbzip2=disabled -Dharfbuzz=disabled \
    -Dpng=disabled -Dzlib=internal -Dtests=disabled

# ---------------------------------------------------------------- libraries
step libffi  "$NET_SRC/libffi"  autotools --disable-docs --disable-multi-os-directory
step expat   "$NET_SRC/expat"   autotools --without-docbook --without-examples --without-tests
step pcre2   "$NET_SRC/pcre2"   autotools --enable-pcre2-16 --disable-static
step ncurses "$NET_SRC/ncurses" autotools --with-shared --without-debug --without-ada --without-manpages \
    --without-tests --enable-widec --enable-pc-files --with-pkg-config-libdir="$HOSTDIR/lib/pkgconfig" \
    --with-termlib=no --disable-db-install
step readline "$NET_SRC/readline" autotools --with-curses --disable-static
step sqlite  "$TC_SRC/sqlite"   autotools --disable-static
b_openssl() {
    (cd "$2" && "$1/Configure" --prefix="$HOSTDIR" --libdir=lib --openssldir="$HOSTDIR/ssl" \
        shared no-tests no-docs no-module "-Wl,-rpath,$HOSTDIR/lib")
    make -C "$2" -j"$JOBS"
    make -C "$2" install_sw install_ssldirs
}
step openssl "$NET_SRC/openssl" b_openssl
# libelf (objtool and the kernel's BPF tools link it); its own program set
# isn't needed.
b_libelf() {
    (cd "$2" && "$1/configure" --prefix="$HOSTDIR" --libdir="$HOSTDIR/lib" --disable-debuginfod \
        --disable-libdebuginfod --without-bzlib --without-lzma --without-zstd --disable-nls)
    make -C "$2/lib" -j"$JOBS"
    make -C "$2/libelf" -j"$JOBS"
    make -C "$2/libelf" install
    make -C "$2/config" install
}
step elfutils "$DESK_SRC/elfutils" b_libelf

# ---------------------------------------------------------------- Perl, Python
b_perl() {
    copy_src "$1" "$2"
    (cd "$2" && sh Configure -des -Dprefix="$HOSTDIR" -Dcc=gcc -Duseshrplib -Dusethreads \
        -Dman1dir=none -Dman3dir=none -Ui_db -Ui_gdbm -Ui_ndbm -Uinstallusrbinperl \
        -Dlddlflags="-shared $LDFLAGS" -Dldflags="$LDFLAGS" -Dccflags="$CPPFLAGS")
    make -C "$2" -j"$JOBS"
    make -C "$2" install
}
step perl "$TC_SRC/perl" b_perl

b_python() {
    copy_src "$1" "$2"
    (cd "$2" && ./configure --prefix="$HOSTDIR" --enable-shared --with-ensurepip=install \
        --disable-test-modules --with-dbmliborder= --with-openssl="$HOSTDIR" \
        LDFLAGS="$LDFLAGS" CPPFLAGS="$CPPFLAGS")
    make -C "$2" -j"$JOBS"
    make -C "$2" install
}
step python "$TC_SRC/python" b_python ensurepip
# Mesa's build scripts' modules, all pure Python: their package directories
# go into site-packages as they are (no pip, no network).
PY_MODULES="markupsafe:src/markupsafe mako:mako pyyaml:lib/yaml ply:src/ply pycparser:pycparser packaging:src/packaging jinja2:src/jinja2"
b_pymodules() {
    local site m name dir src
    site=$("$HOSTDIR/bin/python3" -c 'import sysconfig; print(sysconfig.get_path("purelib"))')
    for m in $PY_MODULES; do
        name=${m%%:*} dir=${m#*:}
        src="$HT_SRC/$name/$dir"
        [[ -d "$src" ]] || src="$HT_SRC/$name/${dir##*/}"
        [[ -d "$src" ]] || die "$name: no ${dir##*/} package in userspace/hosttools/$name"
        rm -rf "${site:?}/${dir##*/}"
        cp -a "$src" "$site/"
    done
    "$HOSTDIR/bin/python3" -c 'import mako, yaml, ply, pycparser, packaging, markupsafe, jinja2'
}
step pymodules "$HT_SRC/mako" b_pymodules "$PY_MODULES $(cat "$HT_SRC"/{markupsafe,pyyaml,ply,pycparser,packaging,jinja2}/.jk_os-version | paste -sd,)"
# setuptools and wheel, installed by pip (no network, no build isolation:
# each builds with what is already here, flit_core first) with their
# metadata: QEMU's configure checks their versions.
b_pybuild() {
    local m
    for m in flit_core setuptools wheel; do
        copy_src "$HT_SRC/$m" "$2/$m"
        "$HOSTDIR/bin/python3" -m pip install --no-index --no-build-isolation --no-deps \
            --disable-pip-version-check --root-user-action=ignore "$2/$m"
    done
}
step pybuild "$HT_SRC/setuptools" b_pybuild "$(cat "$HT_SRC"/{flit_core,setuptools,wheel}/.jk_os-version | paste -sd,)"

# ---------------------------------------------------------------- build systems
b_ninja() {
    copy_src "$1" "$2"
    (cd "$2" && "$HOSTDIR/bin/python3" configure.py --bootstrap)
    install -Dm755 "$2/ninja" "$HOSTDIR/bin/ninja"
}
step ninja "$TC_SRC/ninja" b_ninja
b_cmake() {
    (cd "$2" && "$1/bootstrap" --prefix="$HOSTDIR" --parallel="$JOBS" --no-qt-gui -- \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_CursesDialog=OFF \
        -DCMAKE_USE_OPENSSL=ON -DOPENSSL_ROOT_DIR="$HOSTDIR" -DCMAKE_INSTALL_RPATH="$HOSTDIR/lib")
    make -C "$2" -j"$JOBS"
    make -C "$2" install
}
step cmake "$TC_SRC/cmake" b_cmake
# meson: the one in userspace/desktop, run by this Python.
[[ -f "$DESK_SRC/meson/meson.py" ]] || die "no userspace/desktop/meson"
printf '#!/bin/sh\nexec %s %s "$@"\n' "$HOSTDIR/bin/python3" "$DESK_SRC/meson/meson.py" > "$HOSTDIR/bin/meson"
chmod +x "$HOSTDIR/bin/meson"

step autoconf "$TC_SRC/autoconf" autotools
step automake "$TC_SRC/automake" autotools
step libtool  "$DESK_SRC/libtool" autotools
step gettext  "$HT_SRC/gettext"  autotools --disable-java --disable-csharp --disable-openmp --without-emacs \
    --without-git --disable-libasprintf --disable-acl --disable-curses --without-included-gettext \
    --without-selinux
step libxml2  "$DESK_SRC/libxml2" mesonpkg -Dpython=disabled -Dicu=disabled -Dhistory=disabled \
    -Dreadline=disabled
step libxslt  "$HT_SRC/libxslt"  autotools --without-python --without-crypto --disable-static
# lxml, for breeze-icons' 24px icon generator: against this libxml2 and
# libxslt (their xml2-config and xslt-config), with the rpath above.
b_lxml() {
    copy_src "$1" "$2"
    "$HOSTDIR/bin/python3" -m pip install --no-index --no-build-isolation --no-deps \
        --disable-pip-version-check --root-user-action=ignore "$2"
    "$HOSTDIR/bin/python3" -c 'import lxml.etree'
}
step lxml     "$HT_SRC/lxml"     b_lxml

# ---------------------------------------------------------------- generators
# GLib's (glib-compile-resources, glib-compile-schemas, glib-mkenums,
# glib-genmarshal, gdbus-codegen) and gdk-pixbuf's (gdk-pixbuf-pixdata,
# gdk-pixbuf-csource): a cross build runs these, not the target's.
step glib "$NET_SRC/glib" mesonpkg -Dtests=false -Dintrospection=disabled -Dnls=disabled \
    -Dselinux=disabled -Dlibmount=disabled -Dxattr=false -Dman-pages=disabled -Ddocumentation=false \
    -Dsysprof=disabled
step libpng "$DESK_SRC/libpng" autotools --disable-static --disable-tests --disable-tools
step gdk-pixbuf "$DESK_SRC/gdk-pixbuf" mesonpkg -Dpng=enabled -Djpeg=disabled -Dtiff=disabled \
    -Dgif=disabled -Dothers=disabled -Dbuiltin_loaders=png -Dglycin=disabled -Dintrospection=disabled \
    -Dgtk_doc=false -Ddocumentation=false -Dman=false -Dtests=false -Dinstalled_tests=false \
    -Dgio_sniffing=false -Dthumbnailer=disabled
# wayland-scanner, of the same version as the Wayland libraries built for the
# target (the protocols' code generator).
step wayland-scanner "$DESK_SRC/wayland" mesonpkg -Dlibraries=false -Ddocumentation=false \
    -Dtests=false -Ddtd_validation=false
# glslangValidator: Mesa compiles its Vulkan drivers' GLSL with it.
step glslang "$HT_SRC/glslang" cmakepkg -DENABLE_OPT=OFF -DGLSLANG_TESTS=OFF -DENABLE_HLSL=OFF \
    -DALLOW_EXTERNAL_SPIRV_TOOLS=OFF -DBUILD_SHARED_LIBS=OFF

# ---------------------------------------------------------------- images, devices
b_squashfs() {
    copy_src "$1" "$2"
    # (install builds again: the same options.)
    local opts=(CC=gcc XZ_SUPPORT=1 LZO_SUPPORT=0 LZ4_SUPPORT=1 ZSTD_SUPPORT=1 GZIP_SUPPORT=1
                EXTRA_CFLAGS="$CPPFLAGS" EXTRA_LDFLAGS="$LDFLAGS")
    make -C "$2/squashfs-tools" -j"$JOBS" "${opts[@]}"
    make -C "$2/squashfs-tools" "${opts[@]}" INSTALL_PREFIX="$HOSTDIR" INSTALL_MANPAGES_DIR= install
}
step squashfs-tools "$HT_SRC/squashfs-tools" b_squashfs
step xorriso    "$HT_SRC/xorriso"    autotools --disable-libreadline --disable-libedit --disable-libacl \
    --disable-xattr --disable-launch-frontend
step mtools     "$HT_SRC/mtools"     autotools --without-x
step dosfstools "$HT_SRC/dosfstools" autotools --enable-compat-symlinks --without-udev
b_dtc() {
    copy_src "$1" "$2"
    # (dtc needs no library from here: its own CPPFLAGS stay.)
    make -C "$2" -j"$JOBS" CC=gcc NO_PYTHON=1 NO_YAML=1 NO_VALGRIND=1 PREFIX="$HOSTDIR"
    # The programs and their libfdt; its headers in a directory of their own
    # ($HOSTDIR/include/libfdt, for QEMU): the kernel's and U-Boot's own
    # libfdt copies must not pick them up from $HOSTDIR/include (their
    # HOSTCFLAGS) instead of their own.
    make -C "$2" NO_PYTHON=1 NO_YAML=1 NO_VALGRIND=1 PREFIX="$HOSTDIR" LIBDIR="$HOSTDIR/lib" \
        install-bin install-lib
    rm -f "$HOSTDIR"/include/{fdt.h,libfdt.h,libfdt_env.h}
    install -Dm644 -t "$HOSTDIR/include/libfdt" "$2"/libfdt/{fdt.h,libfdt.h,libfdt_env.h}
}
step dtc  "$HT_SRC/dtc"  b_dtc private-headers
step kmod "$HT_SRC/kmod" mesonpkg -Dzstd=enabled -Dxz=enabled -Dzlib=enabled -Dopenssl=disabled \
    -Dmanpages=false -Ddocs=false -Dbashcompletiondir=no -Dfishcompletiondir=no -Dzshcompletiondir=no
step file "$HT_SRC/file" autotools --disable-static --disable-libseccomp

# ---------------------------------------------------------------- release, flashing, upkeep
step curl "$NET_SRC/curl" autotools --with-openssl="$HOSTDIR" --without-libpsl --disable-static \
    --with-ca-bundle="$HOSTDIR/ssl/cert.pem" --without-brotli --without-nghttp2 --without-libidn2 \
    --disable-ldap --disable-docs
# The CA certificates curl and Python check servers with.
install -Dm644 "$NET_SRC/ca-certificates/cacert.pem" "$HOSTDIR/ssl/cert.pem" 2>/dev/null \
    || install -Dm644 "$(ls "$NET_SRC"/ca-certificates/*.pem | head -n1)" "$HOSTDIR/ssl/cert.pem"
step jq    "$HT_SRC/jq"    autotools --with-oniguruma=builtin --disable-docs --disable-valgrind
step rsync "$HT_SRC/rsync" autotools --disable-md2man --disable-xxhash --disable-lz4 --disable-zstd \
    --disable-openssl --disable-idn --with-included-popt --with-included-zlib=no
step libusb "$HT_SRC/libusb" autotools --disable-udev --disable-static
# heimdall: no install rule of its own.
b_heimdall() {
    cmake -G Ninja -S "$1" -B "$2" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$HOSTDIR" \
        -DCMAKE_BUILD_RPATH="$HOSTDIR/lib" -DDISABLE_FRONTEND=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    ninja -C "$2" -j"$JOBS"
    install -Dm755 "$2/bin/heimdall" "$HOSTDIR/bin/heimdall"
}
step heimdall "$HT_SRC/heimdall" b_heimdall
# gpgv: GnuPG's signature checker (update-source.sh), with the libraries
# GnuPG's build needs.
step libgpg-error "$HT_SRC/libgpg-error" autotools --disable-nls --disable-doc --disable-tests
step libgcrypt    "$HT_SRC/libgcrypt"    autotools --disable-doc --with-libgpg-error-prefix="$HOSTDIR"
step libassuan    "$HT_SRC/libassuan"    autotools --with-libgpg-error-prefix="$HOSTDIR"
step libksba      "$HT_SRC/libksba"      autotools --with-libgpg-error-prefix="$HOSTDIR"
step npth         "$HT_SRC/npth"         autotools
step gnupg        "$HT_SRC/gnupg"        autotools --disable-doc --disable-gpgsm --disable-scdaemon \
    --disable-dirmngr --disable-keyboxd --disable-tpm2d --disable-wks-tools --disable-gpgtar \
    --disable-ldap --disable-sqlite --disable-ntbtls --disable-gnutls --disable-card-support \
    --disable-ccid-driver --without-readline --with-libgpg-error-prefix="$HOSTDIR"

# ---------------------------------------------------------------- util-linux, e2fsprogs, ssh
# lsblk and findmnt (flash.sh), libuuid (EDK2's tools); debugfs
# (inspect-stock.sh reads Android's ext4 images with it); the ssh client
# (publish.sh).
b_util_linux() {
    (cd "$2" && "$1/configure" --prefix="$HOSTDIR" --libdir="$HOSTDIR/lib" --disable-all-programs \
        --enable-libuuid --enable-libblkid --enable-libmount --enable-libsmartcols --enable-lsblk \
        enable_findmnt=yes --disable-nls --disable-static --without-python --without-systemd \
        --without-udev --without-ncursesw --without-tinfo --without-readline --without-cap-ng \
        --without-libmagic --without-econf --without-selinux --without-audit --disable-asciidoc \
        --disable-poman --disable-bash-completion --disable-liblastlog2 --disable-makeinstall-chown \
        --disable-makeinstall-setuid)
    make -C "$2" -j"$JOBS"
    make -C "$2" install
}
step util-linux "$ROOT_DIR/$UTIL_LINUX_TREE" b_util_linux
b_debugfs() {
    (cd "$2" && "$1/configure" --prefix="$HOSTDIR" --libdir="$HOSTDIR/lib" --disable-nls \
        --disable-fuse2fs --disable-uuidd --enable-libuuid --enable-libblkid --disable-defrag)
    make -C "$2" -j"$JOBS"
    install -Dm755 "$2/debugfs/debugfs" "$HOSTDIR/bin/debugfs"
}
step debugfs "$ROOT_DIR/$E2FSPROGS_TREE" b_debugfs
b_openssh() {
    copy_src "$1" "$2"
    (cd "$2" && ./configure --prefix="$HOSTDIR" --sysconfdir="$HOSTDIR/etc/ssh" \
        --with-ssl-dir="$HOSTDIR" --with-zlib="$HOSTDIR" --without-pam --without-selinux \
        --without-kerberos5 --without-libedit --without-security-key-builtin --disable-lastlog \
        --with-privsep-path="$HOSTDIR/var/empty")
    make -C "$2" -j"$JOBS" ssh scp ssh-keygen
    install -Dm755 "$2/ssh" "$HOSTDIR/bin/ssh"
    install -m755 "$2/scp" "$2/ssh-keygen" "$HOSTDIR/bin/"
}
step openssh "$NET_SRC/openssh" b_openssh

# ---------------------------------------------------------------- make run
# QEMU (x86_64 and aarch64 systems) for make run, with user networking
# (libslirp) and the screen over VNC (GUI=1). The UEFI firmware it boots is
# built per arch by run-qemu.sh; nasm and iasl are for that.
step pixman   "$DESK_SRC/pixman"  mesonpkg -Dtests=disabled -Ddemos=disabled -Dgtk=disabled -Dlibpng=disabled
step libslirp "$HT_SRC/libslirp"  mesonpkg
b_qemu() {
    (cd "$2" && "$1/configure" --prefix="$HOSTDIR" --target-list=x86_64-softmmu,aarch64-softmmu \
        --python="$HOSTDIR/bin/python3" --extra-cflags="$CPPFLAGS -I$HOSTDIR/include/libfdt" \
        --extra-ldflags="$LDFLAGS" \
        --disable-docs --disable-werror --enable-slirp --enable-fdt=system \
        --disable-gtk --disable-sdl --disable-opengl --disable-spice --disable-virglrenderer \
        --disable-libusb --disable-usb-redir --disable-smartcard --disable-capstone --disable-curl \
        --disable-gnutls --disable-nettle --disable-gcrypt --enable-vnc --disable-vnc-jpeg \
        --disable-vnc-sasl --disable-xkbcommon --disable-libudev --disable-guest-agent --disable-libdw \
        --enable-tools --disable-download)
    make -C "$2" -j"$JOBS"
    make -C "$2" install
}
step qemu   "$HT_SRC/qemu"   b_qemu
step nasm   "$HT_SRC/nasm"   autotools
b_acpica() {
    copy_src "$1" "$2"
    make -C "$2" -j"$JOBS" iasl CC=gcc
    install -Dm755 "$2/generate/unix/bin/iasl" "$HOSTDIR/bin/iasl"
}
step acpica "$HT_SRC/acpica" b_acpica

# ---------------------------------------------------------------- LLVM
# The build machine's LLVM and Clang, of the toolchain's version and built the
# same way (the same targets, one shared libLLVM), into its own prefix
# (build/host/<arch>/llvm, not on PATH): Mesa's build asks its llvm-config
# about the target's LLVM (a cross-built llvm-config doesn't run here), and
# Mesa's OpenCL kernels are compiled with its Clang (build-desktop.sh).
b_llvm() {
    cmake -G Ninja -S "$1/llvm" -B "$2" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOST_LLVM" \
        -DCMAKE_INSTALL_RPATH="$HOST_LLVM/lib" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
        -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD="X86;AArch64;ARM;RISCV;AMDGPU" \
        -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_LINK_LLVM_DYLIB=ON -DCLANG_LINK_CLANG_DYLIB=ON \
        -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_INCLUDE_DOCS=OFF -DCLANG_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_DOCS=OFF \
        -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_LIBXML2=OFF \
        -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_LIBPFM=OFF -DLLVM_ENABLE_BINDINGS=OFF \
        -DCLANG_ENABLE_ARCMT=OFF -DLLVM_PARALLEL_LINK_JOBS=2 -DLLVM_APPEND_VC_REV=OFF
    ninja -C "$2" -j"$JOBS"
    ninja -C "$2" -j"$JOBS" install
}
step llvm-build "$TC_SRC/llvm-project" b_llvm   # (its build directory; installed in llvm/)

log "host tools ($HOST_ARCH): $(du -sh "$HOSTDIR" | cut -f1) in $HOSTDIR"
