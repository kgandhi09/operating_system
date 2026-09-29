#!/usr/bin/env bash
# Build the desktop stack (KDE Plasma, started on demand with jk-gui) into
# build/<arch>/dyn, next to the network stack, from the sources in
# userspace/desktop (configs/desktop/sources.list).
#
# Everything is compiled with jk_os's own GCC 16 and its glibc
# (build/<arch>/toolchain as the sysroot), so nothing from the build host's
# headers or libraries gets in.
#
# Phase A: graphics (libdrm, Mesa, Vulkan), Wayland, input (libinput,
#          xkbcommon, seatd), fonts, and the X11 libraries for Xwayland.
# Phase B: Qt 6.
# Phase C: KDE Frameworks.
# Phase D: KDE Plasma, elogind, PAM, UPower.
# Phase E: Konsole, Dolphin.
#
# A package is rebuilt only when its source tree or its build options change.
source "$(dirname "$0")/common.sh"
# A host LD_LIBRARY_PATH (e.g. one with /usr/lib/x86_64-linux-gnu) beats the
# RUNPATH of the tools built here, so host Qt tools would load the host's Qt.
unset LD_LIBRARY_PATH
need make meson ninja cmake pkg-config python3 wayland-scanner glslangValidator

TC="$OUT_DIR/toolchain"
[[ -x "$TC/usr/bin/gcc" ]] || die "toolchain not built yet (run: make toolchain)"
[[ -x "$OUT_DIR/dyn/usr/sbin/NetworkManager" ]] || die "network stack not built yet (run: make network)"
SRC="$ROOT_DIR/userspace/desktop"
[[ -d "$SRC" ]] || die "no desktop sources (run: scripts/update-desktop-sources.sh)"

# Compiler wrappers: GCC 16 with the jk_os sysroot. On x86_64 that is the
# toolchain's own GCC; for aarch64 the cross compiler build-toolchain.sh made.
# Meson from userspace/desktop (some packages need a newer one than the host's).
[[ -f "$SRC/meson/meson.py" ]] || die "no userspace/desktop/meson (run: scripts/update-desktop-sources.sh meson)"
MESON="python3 $SRC/meson/meson.py"

TRIPLE="$ARCH-linux-gnu"
WRAP="$OUT_DIR/desktop/bin"
mkdir -p "$WRAP"
if [[ "$ARCH" == "$HOST_ARCH" ]]; then
    for t in gcc g++; do
        printf '#!/bin/sh\nexec %s --sysroot=%s "$@"\n' "$TC/usr/bin/$t" "$TC" > "$WRAP/$TRIPLE-$t"
    done
    for t in ar ranlib strip nm objcopy readelf; do ln -sf "$TC/usr/bin/$t" "$WRAP/$TRIPLE-$t"; done
    # The toolchain's programs (llvm-config, ...) run here; they need its libraries.
    TC_RUN="env LD_LIBRARY_PATH=$TC/usr/lib"
else
    CROSS="$OUT_DIR/cross/bin"
    [[ -x "$CROSS/$TRIPLE-g++" ]] || die "no cross compiler at build/$ARCH/cross (run: make toolchain)"
    for t in gcc g++ ar ranlib strip nm objcopy readelf; do ln -sf "$CROSS/$TRIPLE-$t" "$WRAP/$TRIPLE-$t"; done
    TC_RUN=""
fi
chmod +x "$WRAP"/*
DYN_WORK=desktop DYN_CC="$WRAP/$TRIPLE-gcc" DYN_CXX="$WRAP/$TRIPLE-g++" DYN_AR="$WRAP/$TRIPLE-ar" \
    DYN_RANLIB="$WRAP/$TRIPLE-ranlib" DYN_STRIP="$WRAP/$TRIPLE-strip"
source "$(dirname "$0")/lib-dyn.sh"
export LDFLAGS="$LDFLAGS -Wl,-rpath-link,$TC/usr/lib"

# Tools that run on the build host while other packages build (the Wayland
# protocol scanner, of the same version as the libraries), built with the
# host's compiler into build/<arch>/desktop/host. Meson finds them through
# PKG_CONFIG_PATH_FOR_BUILD.
HOSTDIR="$OUT_DIR/desktop/host"
export PKG_CONFIG_PATH_FOR_BUILD="$HOSTDIR/lib/pkgconfig:$HOSTDIR/share/pkgconfig"
# The build host's own LLVM and Clang: the toolchain built for it
# (build/<host arch>/toolchain). Its programs need its libraries.
HOST_TC="$ROOT_DIR/build/$HOST_ARCH/toolchain"
HOST_ENV=(env -u CC -u CXX -u AR -u RANLIB -u STRIP -u CPPFLAGS -u LDFLAGS -u PKG_CONFIG_SYSROOT_DIR
          -u PKG_CONFIG_LIBDIR "PKG_CONFIG_PATH=$HOSTDIR/lib/pkgconfig:$HOSTDIR/share/pkgconfig"
          "LD_LIBRARY_PATH=$HOSTDIR/lib:$HOST_TC/usr/lib")
hostmeson() {   # hostmeson <src> <out> <meson options...>
    local src="$1" out="$2"; shift 2
    printf '[binaries]\nllvm-config = [%s]\n' "'$HOST_TC/usr/bin/llvm-config'" > "$out.native.ini"
    "${HOST_ENV[@]}" $MESON setup "$out" "$src" --native-file "$out.native.ini" \
        --prefix="$HOSTDIR" --libdir=lib --buildtype=release "$@"
    "${HOST_ENV[@]}" ninja -C "$out" -j"$JOBS"
    "${HOST_ENV[@]}" $MESON install -C "$out" --no-rebuild
}
hostcmake() {   # hostcmake <src> <out> <cmake options...>
    local src="$1" out="$2"; shift 2
    "${HOST_ENV[@]}" cmake -G Ninja -S "$src" -B "$out" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$HOSTDIR" -DCMAKE_INSTALL_LIBDIR=lib \
        -DCMAKE_C_COMPILER="$(command -v gcc)" -DCMAKE_CXX_COMPILER="$(command -v g++)" \
        "-DCMAKE_PREFIX_PATH=$HOSTDIR;$HOST_TC/usr" -DBUILD_TESTING=OFF "$@"
    "${HOST_ENV[@]}" ninja -C "$out" -j"$JOBS"
    "${HOST_ENV[@]}" ninja -C "$out" -j"$JOBS" install
}
# host_wrap <program>...: $HOSTDIR/wbin/<program>, running it with its libraries
# (the target builds find these on $PATH).
host_wrap() {
    mkdir -p "$HOSTDIR/wbin"
    local p
    for p in "$@"; do
        printf '#!/bin/sh\nLD_LIBRARY_PATH=%s exec %s "$@"\n' "$HOSTDIR/lib:$HOST_TC/usr/lib" \
            "$HOSTDIR/bin/$p" > "$HOSTDIR/wbin/$p"
        chmod +x "$HOSTDIR/wbin/$p"
    done
}
export PATH="$HOSTDIR/wbin:$PATH"

# cmakepkg <src> <out> <cmake options...>
# $DYN/usr/include is searched like /usr/include would be (<libmount/libmount.h>
# with only .../include/libmount from pkg-config).
cmakepkg() {
    local src="$1" out="$2"; shift 2
    cmake -G Ninja -S "$src" -B "$out" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" \
        -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR="$ARCH" -DCMAKE_SYSROOT="$TC" \
        "-DCMAKE_FIND_ROOT_PATH=$DYN" -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY -DBUILD_TESTING=OFF \
        -DCMAKE_C_STANDARD_INCLUDE_DIRECTORIES="$DYN/usr/include" \
        -DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES="$DYN/usr/include" "$@"
    ninja -C "$out" -j"$JOBS"
    DESTDIR="$DYN" ninja -C "$out" -j"$JOBS" install
}

# pkg <name> <function> <options...>: build userspace/desktop/<name>.
pkg() { local name="$1"; shift; step "$name" "$SRC/$name" "$@"; }

# libxcb and the xcb utilities read xcb-proto's XML and Python module
# through pkg-config variables that sysroot handling doesn't rewrite.
xcb_env() {
    export XCBPROTO_XCBINCLUDEDIR="$DYN/usr/share/xcb"
    export XCBPROTO_XCBPYTHONDIR="$(echo "$DYN"/usr/lib/python3*/site-packages)"
    export PYTHONPATH="$XCBPROTO_XCBPYTHONDIR"
}
b_libxcb() { xcb_env; autotools "$@" --disable-devel-docs --without-doxygen; }

# xorgpkg: X.org packages ship either autotools or (newer ones) meson only.
xorgpkg() {
    if [[ -x "$1/configure" ]]; then autotools "$@"; else mesonpkg "$1" "$2"; fi
}

b_zstd() { mesonpkg "$1/build/meson" "$2" -Dbin_programs=true -Dbin_contrib=false -Dzlib=disabled \
    -Dlzma=disabled -Dlz4=disabled; }

b_dejavu() {
    install -d "$DYN/usr/share/fonts/dejavu"
    install -m 0644 "$1"/ttf/*.ttf "$DYN/usr/share/fonts/dejavu/"
    install -d "$DYN/etc/fonts/conf.d"
    cp "$1"/fontconfig/*.conf "$DYN/etc/fonts/conf.d/" 2>/dev/null || true
}

# Mesa's GPU drivers: every laptop and desktop GPU family on x86_64; on
# aarch64 the common ARM GPUs too. llvmpipe is the software fallback.
case "$ARCH" in
    x86_64)
        MESA_GALLIUM=iris,crocus,radeonsi,nouveau,virgl,llvmpipe,softpipe,zink
        MESA_VULKAN=intel,amd,swrast ;;
    aarch64)
        MESA_GALLIUM=panfrost,freedreno,v3d,vc4,etnaviv,lima,nouveau,radeonsi,virgl,llvmpipe,softpipe,zink
        MESA_VULKAN=broadcom,freedreno,panfrost,amd,swrast ;;
esac
b_mesa() {
    # LLVM (radeonsi, llvmpipe) from the toolchain; its llvm-config can only
    # run on an x86_64 host, so for aarch64 Mesa finds it through CMake.
    local llvm=(-Dllvm=enabled -Dshared-llvm=enabled)
    if [[ -n "$TC_RUN" ]]; then
        printf '[binaries]\nllvm-config = [%s]\n' \
            "'env', 'LD_LIBRARY_PATH=$TC/usr/lib', '$TC/usr/bin/llvm-config'" > "$2.llvm.ini"
    else
        printf '[properties]\ncmake_prefix_path = [%s]\n' "'$TC/usr'" > "$2.llvm.ini"
    fi
    env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_LIBDIR \
    $MESON setup "$2" "$1" --cross-file "$CROSS_FILE" --cross-file "$2.llvm.ini" \
        --native-file "$NATIVE_FILE" --prefix=/usr --libdir=lib --sysconfdir=/etc \
        --buildtype=release --wrap-mode=nodownload \
        -Dplatforms=x11,wayland -Dgallium-drivers="$MESA_GALLIUM" -Dvulkan-drivers="$MESA_VULKAN" \
        -Degl=enabled -Dgbm=enabled -Dgles1=disabled -Dgles2=enabled -Dglx=dri -Dopengl=true \
        -Dshared-glapi=enabled "${llvm[@]}" -Dvalgrind=disabled -Dlibunwind=disabled \
        -Dlmsensors=disabled -Dbuild-tests=false -Dgallium-va=disabled -Dgallium-rusticl=false \
        -Dzstd=enabled -Dexpat=enabled -Dvideo-codecs= -Dintel-rt=disabled -Dcpp_rtti=false \
        -Dmesa-clc=system -Dprecomp-compiler=system
    ninja -C "$2" -j"$JOBS"
    DESTDIR="$DYN" $MESON install -C "$2" --no-rebuild
}

# ---------------------------------------------------------------- Phase A
pkg libpciaccess     mesonpkg -Dzlib=disabled
pkg libdrm           mesonpkg -Dtests=false -Dman-pages=disabled -Dvalgrind=disabled \
                         -Dcairo-tests=disabled -Dudev=true
pkg zstd             b_zstd
pkg libxml2          mesonpkg -Dpython=disabled -Dicu=disabled -Dhistory=disabled -Dreadline=disabled
step host-wayland-scanner "$SRC/wayland" hostmeson -Dlibraries=false -Ddocumentation=false \
                         -Dtests=false -Ddtd_validation=false
pkg wayland          mesonpkg -Ddocumentation=false -Dtests=false -Ddtd_validation=false
pkg wayland-protocols mesonpkg -Dtests=false
pkg xorgproto        mesonpkg
pkg xcb-proto        autotools
pkg libXau           autotools
pkg libXdmcp         autotools --disable-docs
pkg libxcb           b_libxcb
pkg xtrans           autotools --disable-docs
for x in libX11 libXext libXfixes libXrender libXrandr libXi libXcursor libXdamage \
         libXcomposite libXtst libXinerama libXxf86vm libxkbfile libfontenc libxshmfence libICE libSM; do
    pkg "$x" xorgpkg --disable-docs --disable-specs --without-xmlto --without-fop \
        --enable-malloc0returnsnull=no
done
pkg libxcvt          mesonpkg
for x in xcb-util xcb-util-wm xcb-util-image xcb-util-keysyms xcb-util-renderutil xcb-util-cursor; do
    pkg "$x" xorgpkg
done
pkg xkeyboard-config mesonpkg -Dxorg-rules-symlinks=true
pkg libxkbcommon     mesonpkg -Denable-docs=false -Denable-x11=true -Denable-wayland=true \
                         -Denable-xkbregistry=true -Denable-tools=true \
                         -Dxkb-config-root=/usr/share/X11/xkb -Dx-locale-root=/usr/share/X11/locale
pkg libevdev         mesonpkg -Dtests=disabled -Ddocumentation=disabled
pkg mtdev            autotools
pkg libinput         mesonpkg -Dlibwacom=false -Ddebug-gui=false -Dtests=false -Ddocumentation=false \
                         -Dudev-dir=/usr/lib/udev
pkg seatd            mesonpkg -Dlibseat-seatd=enabled -Dlibseat-builtin=enabled -Dlibseat-logind=disabled \
                         -Dserver=enabled -Dexamples=disabled -Dman-pages=disabled
pkg pixman           mesonpkg -Dtests=disabled -Ddemos=disabled -Dgtk=disabled -Dlibpng=disabled
pkg libpng           autotools
pkg libjpeg-turbo    cmakepkg -DENABLE_STATIC=OFF -DWITH_SIMD=OFF -DWITH_TURBOJPEG=ON
pkg freetype         mesonpkg -Dharfbuzz=disabled -Dpng=enabled -Dzlib=enabled -Dbzip2=disabled \
                         -Dbrotli=disabled
pkg libXfont2        xorgpkg --disable-devel-docs
pkg fontconfig       mesonpkg -Ddoc=disabled -Dtests=disabled -Dtools=enabled -Dcache-build=disabled
pkg harfbuzz         mesonpkg -Dtests=disabled -Ddocs=disabled -Dfreetype=enabled -Dglib=enabled \
                         -Dicu=disabled -Dcairo=disabled -Dintrospection=disabled
pkg fribidi          mesonpkg -Ddocs=false -Dtests=false
pkg dejavu-fonts     b_dejavu
pkg vulkan-headers   cmakepkg
pkg vulkan-loader    cmakepkg -DBUILD_WSI_XCB_SUPPORT=ON -DBUILD_WSI_XLIB_SUPPORT=ON \
                         -DBUILD_WSI_WAYLAND_SUPPORT=ON -DUPDATE_DEPS=OFF
# mesa_clc and vtn_bindgen2 for the build host (Intel's and Panfrost's
# precompiled kernels): SPIR-V tools, the LLVM-to-SPIR-V translator,
# libclc, then Mesa with only those tools.
hostcmake_libclc() {   # hostcmake_libclc <src> <out>: the spirv64 OpenCL library for mesa_clc
    # libclc is compiled with Clang (it checks the C compiler for Clang's flags).
    # Clang 23 no longer takes the old spirv64-mesa3d- triple, so it is built
    # as spirv64-- and installed under the name (and pkg-config file) Mesa
    # looks for.
    hostcmake "$1/libclc" "$2" -DLLVM_DIR="$HOST_TC/usr/lib/cmake/llvm" \
        -DCMAKE_C_COMPILER="$HOST_TC/usr/bin/clang" -DCMAKE_CXX_COMPILER="$HOST_TC/usr/bin/clang++" \
        -DLLVM_DEFAULT_TARGET_TRIPLE=spirv64-- -DLLVM_SPIRV="$HOSTDIR/bin/llvm-spirv"
    install -Dm644 "$2/spirv64--/libclc.spv" "$HOSTDIR/share/clc/spirv64-mesa3d-.spv"
    install -d "$HOSTDIR/share/pkgconfig"
    printf 'prefix=%s\nlibexecdir=${prefix}/share/clc\n\nName: libclc\nDescription: OpenCL builtins\nVersion: %s\n' \
        "$HOSTDIR" "$(tree_version "$1")" > "$HOSTDIR/share/pkgconfig/libclc.pc"
}
b_host_mesa_clc() {
    hostmeson "$1" "$2" -Dgallium-drivers= -Dvulkan-drivers= -Dplatforms= -Dglx=disabled \
        -Degl=disabled -Dgbm=disabled -Dopengl=false -Dgles1=disabled -Dgles2=disabled \
        -Dllvm=enabled -Dshared-llvm=enabled -Dcpp_rtti=false -Dmesa-clc=enabled \
        -Dinstall-mesa-clc=true -Dprecomp-compiler=enabled -Dinstall-precomp-compiler=true \
        -Dbuild-tests=false -Dvalgrind=disabled -Dlibunwind=disabled -Dlmsensors=disabled -Dzstd=disabled
    host_wrap mesa_clc vtn_bindgen2
}
step host-spirv-headers "$SRC/spirv-headers" hostcmake
step host-spirv-tools "$SRC/spirv-tools" hostcmake -DSPIRV-Headers_SOURCE_DIR="$SRC/spirv-headers" \
    -DSPIRV_SKIP_TESTS=ON -DSPIRV_WERROR=OFF
step host-spirv-llvm-translator "$SRC/spirv-llvm-translator" hostcmake \
    -DLLVM_DIR="$HOST_TC/usr/lib/cmake/llvm" -DLLVM_EXTERNAL_SPIRV_HEADERS_SOURCE_DIR="$SRC/spirv-headers" \
    -DLLVM_SPIRV_INCLUDE_TESTS=OFF
step host-libclc "$LLVM_SRC" hostcmake_libclc
step host-mesa-clc "$SRC/mesa" b_host_mesa_clc
# libelf only (radeonsi reads the ELF code objects LLVM makes for AMD GPUs).
b_libelf() {
    (cd "$2" && "$1/configure" "${AUTOCONF_ARGS[@]}" --disable-debuginfod --disable-libdebuginfod \
        --without-bzlib --without-lzma --without-zstd --disable-nls)
    make -C "$2/lib" -j"$JOBS"
    make -C "$2/libelf" -j"$JOBS"
    make -C "$2/libelf" install DESTDIR="$DYN"
    make -C "$2/config" install DESTDIR="$DYN"   # libelf.pc
}
pkg elfutils         b_libelf
pkg mesa             b_mesa
# libepoxy compiles against Mesa's EGL and GL headers.
pkg libepoxy         mesonpkg -Dtests=false -Degl=yes -Dglx=yes -Dx11=true
# Xwayland compiles in the DRI driver directory from dri.pc, where pkg-config's
# sysroot handling puts $DYN in front; the generated headers get /usr back.
b_xwayland() {
    local src="$1" out="$2"; shift 2
    env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_LIBDIR \
    $MESON setup "$out" "$src" --cross-file "$CROSS_FILE" --native-file "$NATIVE_FILE" \
        --prefix=/usr --libdir=lib --sysconfdir=/etc --localstatedir=/var \
        --buildtype=release --wrap-mode=nodownload -Ddefault_library=shared "$@"
    find "$out" -maxdepth 2 -name '*.h' -exec sed -i "s|$DYN/usr/|/usr/|g" {} +
    ! grep -rl "$DYN" --include='*.h' "$out" || die "xwayland: build paths left in its headers"
    ninja -C "$out" -j"$JOBS"
    DESTDIR="$DYN" $MESON install -C "$out" --no-rebuild
}
pkg xwayland         b_xwayland -Dglamor=true -Dxvfb=false -Dxwayland_ei=false -Dlibdecor=false \
                         -Ddocs=false -Dsha1=libcrypto -Dsecure-rpc=false -Dxkb_dir=/usr/share/X11/xkb \
                         -Dxkb_output_dir=/var/lib/xkb -Dxkb_bin_dir=/usr/bin -Dsystemd_notify=false

# ---------------------------------------------------------------- Phase B
# Qt 6. Its build tools (moc, rcc, qmlcachegen, qsb, qtwaylandscanner, ...)
# have to run on the build host, so a small host Qt is built first
# (build/<arch>/desktop/host/qt, no windowing) and the target Qt uses it
# through QT_HOST_PATH.
QT_HOST="$HOSTDIR/qt"
hostqt() {      # hostqt <src> <out> <cmake options...>
    local src="$1" out="$2"; shift 2
    "${HOST_ENV[@]}" cmake -G Ninja -S "$src" -B "$out" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$QT_HOST" -DCMAKE_PREFIX_PATH="$QT_HOST" \
        -DCMAKE_C_COMPILER="$(command -v gcc)" -DCMAKE_CXX_COMPILER="$(command -v g++)" \
        -DQT_BUILD_EXAMPLES=OFF -DQT_BUILD_TESTS=OFF "$@"
    "${HOST_ENV[@]}" ninja -C "$out" -j"$JOBS"
    "${HOST_ENV[@]}" ninja -C "$out" -j"$JOBS" install
}
qtpkg() {       # qtpkg <src> <out> <cmake options...>: a target Qt module
    cmakepkg "$@" -DQT_HOST_PATH="$QT_HOST" -DQT_BUILD_EXAMPLES=OFF -DQT_BUILD_TESTS=OFF \
        -DQT_GENERATE_SBOM=OFF -DCMAKE_PREFIX_PATH="$DYN/usr"
}
# Where the target Qt goes (the other modules take it from qtbase): its
# plugins, QML modules and internal tools under /usr/lib/qt6, as distributions
# do, instead of /usr/plugins, /usr/qml, ...
QT_DIRS=(-DINSTALL_ARCHDATADIR=lib/qt6 -DINSTALL_PLUGINSDIR=lib/qt6/plugins -DINSTALL_QMLDIR=lib/qt6/qml
         -DINSTALL_LIBEXECDIR=lib/qt6/libexec -DINSTALL_MKSPECSDIR=lib/qt6/mkspecs
         -DINSTALL_DESCRIPTIONSDIR=lib/qt6/modules -DINSTALL_DATADIR=share/qt6
         -DINSTALL_DOCDIR=share/doc/qt6 -DINSTALL_INCLUDEDIR=include/qt6)

step host-qtbase "$SRC/qtbase" hostqt -DFEATURE_gui=ON -DFEATURE_widgets=OFF -DFEATURE_xcb=OFF \
    -DFEATURE_opengl=OFF -DFEATURE_vulkan=OFF -DFEATURE_egl=OFF -DFEATURE_fontconfig=OFF \
    -DFEATURE_freetype=OFF -DFEATURE_harfbuzz=OFF -DFEATURE_icu=OFF -DFEATURE_glib=OFF \
    -DFEATURE_sql=OFF -DFEATURE_printsupport=OFF -DFEATURE_openssl=OFF -DFEATURE_libinput=OFF \
    -DFEATURE_xkbcommon=OFF -DFEATURE_dbus=ON -DINPUT_dbus=runtime -DFEATURE_system_zlib=OFF \
    -DFEATURE_system_pcre2=OFF -DFEATURE_system_png=OFF -DFEATURE_system_jpeg=OFF
step host-qtshadertools "$SRC/qtshadertools" hostqt
step host-qtdeclarative "$SRC/qtdeclarative" hostqt
step host-qtwayland "$SRC/qtwayland" hostqt -DFEATURE_wayland_client=OFF -DFEATURE_wayland_server=OFF
step host-qttools "$SRC/qttools" hostqt -DFEATURE_assistant=OFF -DFEATURE_designer=OFF \
    -DFEATURE_qdoc=OFF -DFEATURE_clang=OFF -DFEATURE_distancefieldgenerator=OFF \
    -DFEATURE_pixeltool=OFF -DFEATURE_qtdiag=OFF -DFEATURE_qtplugininfo=OFF -DFEATURE_qdbus=OFF

# PCRE2: Qt needs the 16-bit library, the system one is 8-bit only, so Qt uses its own copy.
pkg qtbase           qtpkg "${QT_DIRS[@]}" -DFEATURE_xcb=ON -DFEATURE_xcb_xlib=ON -DFEATURE_opengl=ON \
                         -DINPUT_opengl=desktop -DFEATURE_egl=ON -DFEATURE_vulkan=ON \
                         -DFEATURE_eglfs=OFF -DFEATURE_linuxfb=OFF -DFEATURE_kms=OFF \
                         -DFEATURE_icu=OFF -DFEATURE_glib=ON -DFEATURE_libinput=ON -DFEATURE_xkbcommon=ON \
                         -DFEATURE_openssl_linked=ON -DFEATURE_dbus_linked=ON -DFEATURE_sql_sqlite=ON \
                         -DFEATURE_system_sqlite=OFF -DFEATURE_cups=OFF -DFEATURE_gtk3=OFF \
                         -DFEATURE_system_zlib=ON -DFEATURE_system_png=ON -DFEATURE_system_jpeg=ON \
                         -DFEATURE_system_freetype=ON -DFEATURE_system_harfbuzz=ON -DFEATURE_system_pcre2=OFF \
                         -DFEATURE_fontconfig=ON -DFEATURE_zstd=ON -DFEATURE_brotli=OFF
pkg qtshadertools    qtpkg
pkg qtdeclarative    qtpkg
pkg qtsvg            qtpkg
pkg qtwayland        qtpkg
pkg qt5compat        qtpkg
pkg qtimageformats   qtpkg
pkg qtpositioning    qtpkg
pkg qtsensors        qtpkg
pkg qtlocation       qtpkg
# QtMultimedia without media backends for now (QtSpeech needs the module;
# audio comes with PipeWire later).
pkg qtmultimedia     qtpkg -DFEATURE_ffmpeg=OFF -DFEATURE_gstreamer=OFF -DFEATURE_pulseaudio=OFF \
                         -DFEATURE_pipewire=OFF -DFEATURE_alsa=OFF -DFEATURE_vaapi=OFF \
                         -DFEATURE_spatialaudio_quick3d=OFF
# The text-to-speech API KTextEditor needs; no speech engine behind it yet.
pkg qtspeech         qtpkg -DFEATURE_flite=OFF -DFEATURE_speechd=OFF
# qttools on the target only for QtUiTools (KWin's scripting) and its helpers.
pkg qttools          qtpkg -DFEATURE_assistant=OFF -DFEATURE_designer=OFF -DFEATURE_linguist=OFF \
                         -DFEATURE_qdoc=OFF -DFEATURE_clang=OFF -DFEATURE_distancefieldgenerator=OFF \
                         -DFEATURE_pixeltool=OFF -DFEATURE_qtdiag=OFF -DFEATURE_qtplugininfo=OFF \
                         -DFEATURE_qdbus=ON -DFEATURE_qtattributionsscanner=OFF

# ---------------------------------------------------------------- Phase C
# KDE Frameworks. The code generators some of them run at build time
# (kconfig_compiler, ...) come from a host build of those frameworks, next to
# the host Qt; the target builds find them through KF6_HOST_TOOLING.
KF_OPTS=(-DBUILD_QCH=OFF -DBUILD_PYTHON_BINDINGS=OFF -DKDE_INSTALL_USE_QT_SYS_PATHS=OFF
         -DKDE_INSTALL_QTPLUGINDIR=lib/qt6/plugins -DKDE_INSTALL_QMLDIR=lib/qt6/qml
         -DKF6_HOST_TOOLING="$QT_HOST/lib/cmake" -DBUILD_DESIGNERPLUGIN=OFF)
kfpkg() { qtpkg "$@" "${KF_OPTS[@]}"; }

step host-extra-cmake-modules "$SRC/extra-cmake-modules" hostqt -DBUILD_DOC=OFF -DBUILD_TESTING=OFF
hostkf() { hostqt "$@" -DKDE_INSTALL_LIBDIR=lib -DBUILD_TESTING=OFF -DBUILD_QCH=OFF \
    -DBUILD_PYTHON_BINDINGS=OFF -DBUILD_DESIGNERPLUGIN=OFF \
    -DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES="$HOSTDIR/include"; }
step host-kconfig "$SRC/kconfig" hostkf -DKCONFIG_USE_GUI=OFF -DKCONFIG_USE_QML=OFF -DUSE_DBUS=OFF
# kpackagetool6, which installs Plasma's packages (applets, themes, ...).
host_libmount() {
    (cd "$2" && "${HOST_ENV[@]}" "$1/configure" --prefix="$HOSTDIR" --libdir="$HOSTDIR/lib" \
        --disable-static --disable-all-programs --enable-libmount --enable-libblkid --disable-nls \
        --disable-asciidoc --disable-poman --disable-bash-completion --without-python \
        --without-systemd --without-udev --without-ncursesw --without-tinfo --without-readline)
    "${HOST_ENV[@]}" make -C "$2" -j"$JOBS"
    "${HOST_ENV[@]}" make -C "$2" install
}
step host-libmount "$UTIL_LINUX_SRC" host_libmount
step host-kcoreaddons "$SRC/kcoreaddons" hostkf -DKCOREADDONS_USE_QML=OFF -DUSE_DBUS=OFF
step host-ki18n "$SRC/ki18n" hostkf -DBUILD_WITH_QML=OFF
step host-karchive "$SRC/karchive" hostkf -DWITH_BZIP2=OFF -DWITH_LIBLZMA=OFF -DWITH_OPENSSL=OFF \
    -DWITH_LIBZSTD=OFF
step host-kpackage "$SRC/kpackage" hostkf
# kcmdesktopfilegenerator (System Settings modules' .desktop files).
step host-kcmutils "$SRC/kcmutils" hostkf -DTOOLS_ONLY=ON

# libmount and libblkid (KCoreAddons, Solid, elogind), shared, from the util-linux in the repo.
step libmount "$UTIL_LINUX_SRC" autotools --disable-all-programs --enable-libmount --enable-libblkid \
    --disable-nls --disable-asciidoc --disable-poman --disable-bash-completion --without-python \
    --without-systemd --without-udev --without-ncursesw --without-tinfo --without-readline
pkg xz               autotools --disable-doc --disable-nls --disable-xz --disable-xzdec \
                         --disable-lzmadec --disable-lzmainfo --disable-scripts
# Event sounds (KNotifications, KWin).
pkg libtool          autotools --enable-ltdl-install
pkg qrencode         cmakepkg -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DWITH_TOOLS=NO -DBUILD_SHARED_LIBS=ON
pkg libogg           autotools
pkg libvorbis        autotools
pkg libcanberra      autotools --disable-oss --disable-pulse --disable-alsa --disable-gstreamer \
                         --disable-gtk --disable-gtk3 --disable-tdb --disable-lynx --enable-null
# ModemManager (mobile broadband), for ModemManagerQt; AT-command modems only
# (no MBIM/QMI libraries yet).
pkg ModemManager     mesonpkg -Dmbim=false -Dqmi=false -Dqrtr=false -Dpolkit=no -Dintrospection=false \
                         -Dvapi=false -Dman=false -Dgtk_doc=false -Dbash_completion=false -Dtests=false \
                         -Dexamples=false -Dsystemdsystemunitdir=no -Dsystemd_suspend_resume=false \
                         -Dsystemd_journal=false -Dudevdir=/usr/lib/udev
# breeze-icons' two generators (dark icon variants, the icon alias list).
b_host_breeze_tools() {
    "${HOST_ENV[@]}" cmake -G Ninja -S "$1" -B "$2" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH="$QT_HOST" -DCMAKE_C_COMPILER="$(command -v gcc)" \
        -DCMAKE_CXX_COMPILER="$(command -v g++)" -DBUILD_TESTING=OFF -DBINARY_ICONS_RESOURCE=OFF
    "${HOST_ENV[@]}" ninja -C "$2" -j"$JOBS" generate-symbolic-dark qrcAlias
    install -m 0755 "$2/bin/generate-symbolic-dark" "$2/bin/qrcAlias" "$QT_HOST/bin/"
}
step host-breeze-icons-tools "$SRC/breeze-icons" b_host_breeze_tools

pkg extra-cmake-modules       cmakepkg -DBUILD_DOC=OFF
pkg plasma-wayland-protocols  cmakepkg -DCMAKE_PREFIX_PATH="$DYN/usr"
# In dependency order.
for kf in kconfig kcoreaddons ki18n kwidgetsaddons kguiaddons kitemmodels kitemviews kcodecs \
          kdbusaddons kwindowsystem karchive solid sonnet threadweaver kidletime kplotting \
          syntax-highlighting kholidays kcolorscheme breeze-icons kirigami kquickcharts \
          kunitconversion attica kcrash kauth kcompletion kconfigwidgets kglobalaccel \
          knotifications kiconthemes kjobwidgets kservice kpackage kxmlgui kbookmarks \
          kwallet ktextwidgets kio kded kdeclarative ksvg kcmutils knewstuff kparts kpty kdesu \
          kstatusnotifieritem krunner knotifyconfig frameworkintegration qqc2-desktop-style \
          networkmanager-qt modemmanager-qt kuserfeedback prison kimageformats ktexteditor kirigami-addons; do
    case "$kf" in
        # Only the KWallet client library: the wallet daemons need GPGME and
        # libgcrypt, and there is nothing to keep in a wallet here.
        kwallet)     pkg kwallet kfpkg -DBUILD_KWALLETD=OFF -DBUILD_KSECRETD=OFF \
                         -DBUILD_KWALLET_QUERY=OFF ;;
        # The icons as files; no compiled-in copy.
        breeze-icons) pkg breeze-icons kfpkg -DBINARY_ICONS_RESOURCE=OFF \
                          -DGENERATE_SYMBOLIC_DARK="$QT_HOST/bin/generate-symbolic-dark" \
                          -DQRC_ALIAS="$QT_HOST/bin/qrcAlias" ;;
        # gzip, xz and zstd archives; bzip2 left out.
        karchive)    pkg karchive kfpkg -DWITH_BZIP2=OFF ;;
        # No spell-check dictionaries (Hunspell, ...) in jk_os yet.
        sonnet)      pkg sonnet kfpkg -DSONNET_NO_BACKENDS=ON ;;
        ktextwidgets) pkg ktextwidgets kfpkg -DWITH_TEXT_TO_SPEECH=OFF ;;
        # No utmp logging of terminal sessions.
        kpty)        pkg kpty kfpkg -DCMAKE_DISABLE_FIND_PACKAGE_UTEMPTER=ON ;;
        # QR codes only: no Data Matrix, no barcode scanning.
        prison)      pkg prison kfpkg -DWITH_DMTX=OFF -DWITH_ZXING=OFF -DWITH_MULTIMEDIA=OFF ;;
        *)           pkg "$kf" kfpkg ;;
    esac
done

# ---------------------------------------------------------------- Phase D
# KDE Plasma and what it needs besides the frameworks.

# Boost: only its headers (kactivitymanagerd).
b_boost_headers() { rm -rf "$DYN/usr/include/boost"; cp -a "$1/boost" "$DYN/usr/include/"; }
# hwdata: the PCI/USB/monitor vendor ID tables.
b_hwdata() {
    copy_tree "$1" "$2"
    (cd "$2" && ./configure --prefix=/usr --datadir=/usr/share --libdir=/usr/lib)
    make -C "$2" install DESTDIR="$DYN"
}
# libcap (for elogind), shared.
b_libcap() {
    copy_tree "$1" "$2"
    make -C "$2/libcap" -j"$JOBS" CC="$CC" AR="$AR" RANLIB="$RANLIB" BUILD_CC=gcc \
        prefix=/usr lib=lib GOLANG=no PAM_CAP=no SHARED=yes
    make -C "$2/libcap" install CC="$CC" AR="$AR" RANLIB="$RANLIB" BUILD_CC=gcc \
        prefix=/usr lib=lib GOLANG=no PAM_CAP=no DESTDIR="$DYN" RAISE_SETFCAP=no
    rm -f "$DYN/usr/lib/libcap.a" "$DYN/usr/lib/libpsx.a"
}
# linux-pam: only for the session helper and the screen locker. Logins on
# the consoles keep using shadow's login without PAM.
b_pam() {
    mesonpkg "$1" "$2" -Ddocs=disabled -Dexamples=false -Dxtests=false -Dnis=disabled \
        -Daudit=disabled -Deconf=disabled -Dselinux=disabled -Dlogind=disabled \
        -Delogind=disabled -Dopenssl=disabled -Di18n=disabled -Dpam_lastlog=disabled
}
# The D-Bus directories are given: elogind takes them from dbus-1.pc, where
# pkg-config's sysroot handling would put $DYN in front of them.
b_elogind() {
    mesonpkg "$1" "$2" -Dmode=release -Dman=disabled -Dhtml=disabled -Dtests=false \
        -Dselinux=disabled -Dsmack=false -Dacl=enabled -Daudit=disabled -Dpolkit=disabled \
        -Dpam=enabled -Dpamlibdir=/usr/lib/security -Dcgroup-controller=elogind \
        -Dhalt-path=/sbin/poweroff -Dpoweroff-path=/sbin/poweroff -Dreboot-path=/sbin/reboot \
        -Dnologin-path=/sbin/nologin -Ddefault-kill-user-processes=true \
        -Ddbuspolicydir=/usr/share/dbus-1/system.d \
        -Ddbussystemservicedir=/usr/share/dbus-1/system-services
}

# lm-sensors: libsensors (temperatures and fans in System Monitor) and `sensors`.
b_lm_sensors() {
    copy_tree "$1" "$2"
    local mk=(-C "$2" CC="$CC" AR="$AR" PREFIX=/usr LIBDIR=/usr/lib ETCDIR=/etc MANDIR=/usr/share/man
              BUILD_STATIC_LIB=0 PROG_EXTRA=)
    make "${mk[@]}" -j"$JOBS" all
    make "${mk[@]}" install DESTDIR="$DYN"
}
pkg lm-sensors       b_lm_sensors
# ICU: its data is compiled by ICU's own tools, so a build for the host first.
b_icu() {
    mkdir -p "$2/host" "$2/target"
    (cd "$2/host" && "${HOST_ENV[@]}" "$1/source/configure" --disable-tests --disable-samples \
        --disable-extras)
    "${HOST_ENV[@]}" make -C "$2/host" -j"$JOBS"
    (cd "$2/target" && "$1/source/configure" "${AUTOCONF_ARGS[@]}" --with-cross-build="$2/host" \
        --disable-tests --disable-samples --disable-extras)
    make -C "$2/target" -j"$JOBS"
    make -C "$2/target" install DESTDIR="$DYN"
}
pkg icu              b_icu
pkg libXft           xorgpkg
pkg xkbcomp          xorgpkg
# The MIME database is compiled by update-mime-database: the one just built,
# run here when the build host can (same arch); otherwise S17mime compiles
# it on the first boot.
b_shared_mime_info() {
    mesonpkg "$1" "$2" -Dupdate-mimedb=false -Dbuild-tools=true -Dbuild-tests=false \
        -Dbuild-translations=false -Dbuild-spec=false
    if [[ "$ARCH" == "$HOST_ARCH" ]]; then
        "$TC/usr/lib/ld-linux-x86-64.so.2" --library-path "$DYN/usr/lib:$TC/usr/lib" \
            "$DYN/usr/bin/update-mime-database" "$DYN/usr/share/mime"
    fi
}
pkg shared-mime-info b_shared_mime_info
pkg boost            b_boost_headers
pkg qcoro            qtpkg -DUSE_QT_VERSION=6 -DQCORO_BUILD_EXAMPLES=OFF -DQCORO_WITH_QTWEBSOCKETS=OFF
# Password storage API (plasma-nm); no libsecret, so it uses KWallet's D-Bus API.
pkg qtkeychain       qtpkg -DBUILD_WITH_QT6=ON -DLIBSECRET_SUPPORT=OFF -DBUILD_TRANSLATIONS=OFF
pkg lcms2            autotools
pkg hwdata           b_hwdata
pkg libdisplay-info  mesonpkg
pkg libei            mesonpkg -Dtests=disabled -Dliboeffis=disabled -Ddocumentation=[]
# libcrypt with yescrypt (the hashes in /etc/shadow), for PAM's pam_unix.
step libxcrypt "$LIBXCRYPT_SRC" autotools --enable-hashes=strong,glibc --enable-obsolete-api=no \
    --disable-failure-tokens --disable-werror
pkg linux-pam        b_pam
step libcap "$ROOT_DIR/userspace/libcap" b_libcap
pkg attr             autotools --disable-nls
pkg acl              autotools --disable-nls
pkg elogind          b_elogind
# jk-session (src/jk-session): opens the elogind session jk-gui's desktop runs in.
b_jk_session() {
    $CC $CPPFLAGS ${CFLAGS:-} -O2 -Wall -Wextra -o "$2/jk-session" "$1/jk-session.c" $LDFLAGS -lpam
    install -D -m 4755 "$2/jk-session" "$DYN/usr/lib/jk_os/jk-session"
}
# (Rebuilt when the source changes: its checksum is part of the options.)
step jk-session "$ROOT_DIR/src/jk-session" b_jk_session \
    "$(cat "$ROOT_DIR"/src/jk-session/* | sha256sum | cut -c1-16)"
pkg libgudev         mesonpkg -Dintrospection=disabled -Dvapi=disabled -Dtests=disabled
pkg upower           mesonpkg -Dintrospection=disabled -Dgtk-doc=false -Dman=false \
                         -Didevice=disabled -Dsystemdsystemunitdir=no -Dudevrulesdir=/usr/lib/udev/rules.d \
                         -Dudevhwdbdir=/usr/lib/udev/hwdb.d -Dos_backend=linux

# plasma-desktop compiles in the keyboard layouts' directory from pkg-config
# and checks it under CMAKE_SYSROOT (the toolchain, not $DYN). Built from a
# copy whose check looks in $DYN; freedesktop sysroot rules keep the path
# itself /usr/share/X11/xkb.
b_plasma_desktop() {
    local src="$1" out="$2"; shift 2
    mkdir -p "$out/src"; copy_tree "$src" "$out/src"
    sed -i 's|"${CMAKE_SYSROOT}/${XKBDIR}"|"'"$DYN"'/${XKBDIR}"|' "$out/src/ConfigureChecks.cmake"
    PKG_CONFIG_FDO_SYSROOT_RULES=1 kfpkg "$out/src" "$out/build" "$@"
}
b_no_users_kcm() {
    rm -f "$DYN/usr/lib/qt6/plugins/plasma/kcms/systemsettings/kcm_users.so" \
          "$DYN/usr/share/applications/kcm_users.desktop" "$DYN"/usr/share/locale/*/LC_MESSAGES/kcm_users.mo
}
for p in kdecoration layer-shell-qt kwayland plasma-activities plasma-activities-stats \
         kactivitymanagerd libplasma plasma5support kglobalacceld knighttime libkscreen \
         libksysguard kscreenlocker breeze kwin plasma-integration plasma-workspace milou \
         plasma-desktop systemsettings kscreen powerdevil plasma-nm kde-cli-tools \
         qqc2-breeze-style ocean-sound-theme; do
    case "$p" in
        breeze)             pkg breeze kfpkg -DBUILD_QT5=OFF -DBUILD_QT6=ON ;;
        plasma-nm)          pkg plasma-nm kfpkg -DBUILD_OPENCONNECT=OFF ;;
        # No KDocTools: no handbooks.
        kde-cli-tools)      pkg kde-cli-tools kfpkg -DBUILD_DOC=OFF ;;
        plasma-integration) pkg plasma-integration kfpkg -DBUILD_QT5=OFF -DBUILD_QT6=ON ;;
        # Wayland session only (X11 apps still run, through Xwayland); locales
        # come with glibc, nothing to generate.
        plasma-workspace)   pkg plasma-workspace kfpkg -DWITH_X11_SESSION=OFF -DGLIBC_LOCALE_GEN=OFF
                            # No user management in the desktop: users are managed as
                            # root on the console (jk_os's accounts), so the Users
                            # page of System Settings is removed.
                            step no-users-kcm "$SRC/plasma-workspace" b_no_users_kcm ;;
        # Tablets and the X11 input drivers are left out; mouse and touchpad
        # settings are KWin's (Wayland).
        plasma-desktop)     pkg plasma-desktop b_plasma_desktop -DBUILD_KCM_TABLET=OFF -DBUILD_KCM_MOUSE_X11=OFF \
                                -DBUILD_KCM_TOUCHPAD_X11=OFF -DBUILD_DOC=OFF ;;
        *)                  pkg "$p" kfpkg ;;
    esac
done

# ---------------------------------------------------------------- Phase E
# Applications: the terminal and the file manager.
# File metadata, without the optional extractors (taglib, exiv2, ffmpeg, ...).
pkg kfilemetadata    kfpkg
pkg konsole          kfpkg -DBUILD_DOC=OFF -DWITH_LIBSSH=OFF
pkg dolphin          kfpkg -DBUILD_DOC=OFF

# ---------------------------------------------------------------- Firefox
# Mozilla's own release build, on the GTK 3 libraries built here.

# GTK's build runs GLib and gdk-pixbuf tools (resource and schema compilers).
# The ones built here run on the build host when it is the same arch; a
# cross build would need them built for the host first.
[[ "$ARCH" == "$HOST_ARCH" ]] || die "Firefox's GTK 3 needs GLib tools for the build host; cross builds of it are not set up yet"
dyn_tool() {    # dyn_tool <program>...: run $DYN's program on the build host
    mkdir -p "$HOSTDIR/wbin"
    local p
    for p in "$@"; do
        printf '#!/bin/sh\nexec %s --library-path %s %s "$@"\n' "$TC/usr/lib/ld-linux-x86-64.so.2" \
            "$DYN/usr/lib:$TC/usr/lib" "$DYN/usr/bin/$p" > "$HOSTDIR/wbin/$p"
        chmod +x "$HOSTDIR/wbin/$p"
    done
}
dyn_tool glib-compile-resources glib-compile-schemas

b_gdk_pixbuf() {
    mesonpkg "$@"
    dyn_tool gdk-pixbuf-pixdata gdk-pixbuf-csource
}
# GTK's settings schemas, compiled (GTK aborts without them, e.g. in the
# file chooser).
b_gtk3() {
    mesonpkg "$@"
    glib-compile-schemas "$DYN/usr/share/glib-2.0/schemas"
}
b_firefox() {
    rm -rf "$DYN/usr/lib/firefox"
    cp -a "$1" "$DYN/usr/lib/firefox"
    rm -f "$DYN/usr/lib/firefox/.jk_os-version"
    ln -sf ../lib/firefox/firefox "$DYN/usr/bin/firefox"
    # Updates come with the OS image, not from Firefox itself.
    install -d "$DYN/usr/lib/firefox/distribution"
    cat > "$DYN/usr/lib/firefox/distribution/policies.json" <<'JSON'
{
  "policies": {
    "DisableAppUpdate": true,
    "DontCheckDefaultBrowser": true
  }
}
JSON
    local s
    for s in 16 32 48 64 128; do
        install -Dm644 "$1/browser/chrome/icons/default/default$s.png" \
            "$DYN/usr/share/icons/hicolor/${s}x$s/apps/firefox.png"
    done
    install -Dm644 /dev/stdin "$DYN/usr/share/applications/firefox.desktop" <<'DESKTOP'
[Desktop Entry]
Type=Application
Name=Firefox
GenericName=Web Browser
Comment=Browse the web
Exec=firefox %u
Icon=firefox
Terminal=false
Categories=Network;WebBrowser;
MimeType=text/html;text/xml;application/xhtml+xml;x-scheme-handler/http;x-scheme-handler/https;
StartupWMClass=firefox
Actions=new-window;new-private-window;

[Desktop Action new-window]
Name=New Window
Exec=firefox --new-window %u

[Desktop Action new-private-window]
Name=New Private Window
Exec=firefox --private-window %u
DESKTOP
}

pkg cairo            mesonpkg -Dtests=disabled -Dxlib=enabled -Dxcb=enabled -Dfreetype=enabled \
                         -Dfontconfig=enabled -Dpng=enabled -Dglib=enabled -Dzlib=enabled -Dlzo=disabled \
                         -Dspectre=disabled -Dsymbol-lookup=disabled -Dgtk2-utils=disabled -Dgtk_doc=false
# Pango calls FcFreeTypeQueryAll() without including its header
# (fontconfig/fcfreetype.h), which GCC 16 rejects: every file gets it.
b_pango() {
    local src="$1" out="$2"; shift 2
    mkdir -p "$out.inc"
    printf '#include <fontconfig/fontconfig.h>\n#include <fontconfig/fcfreetype.h>\n' > "$out.inc/fcft.h"
    mesonpkg "$src" "$out" "-Dc_args=['-I$DYN/usr/include','-I$DYN/usr/include/freetype2','-include','$out.inc/fcft.h']" "$@"
}
pkg pango            b_pango \
                         -Dintrospection=disabled -Dbuild-testsuite=false -Dbuild-examples=false \
                         -Dgtk_doc=false -Ddocumentation=false -Dman-pages=false -Dcairo=enabled \
                         -Dxft=enabled -Dfreetype=enabled -Dfontconfig=enabled -Dlibthai=disabled -Dsysprof=disabled
# The image loaders built into the library (no loader cache to generate).
pkg gdk-pixbuf       b_gdk_pixbuf -Dpng=enabled -Djpeg=enabled -Dtiff=disabled -Dgif=enabled \
                         -Dglycin=disabled -Dothers=enabled -Dbuiltin_loaders=all -Dintrospection=disabled \
                         -Dgtk_doc=false -Ddocumentation=false -Dman=false -Dtests=false -Dinstalled_tests=false
# The accessibility bus and ATK; its D-Bus directories are given (from
# dbus-1.pc they would carry $DYN).
pkg at-spi2-core     mesonpkg -Dintrospection=disabled -Ddocs=false -Duse_systemd=false -Dx11=enabled \
                         -Ddbus_daemon=/usr/bin/dbus-daemon -Ddbus_services_dir=/usr/share/dbus-1/services \
                         -Dsystemd_user_dir=/usr/lib/systemd/user -Dgtk2_atk_adaptor=false
pkg gtk3             b_gtk3 -Dx11_backend=true -Dwayland_backend=true -Dbroadway_backend=false \
                         -Dintrospection=false -Dgtk_doc=false -Dman=false -Dtests=false -Dinstalled_tests=false \
                         -Dexamples=false -Ddemos=false -Dcolord=no -Dcloudproviders=false -Dtracker3=false \
                         -Dprint_backends=file -Dprofiler=false
pkg "firefox-$ARCH"  b_firefox

log "desktop stack ($ARCH): $(du -sh "$DYN" | cut -f1) in $DYN"
