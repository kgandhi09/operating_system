#!/usr/bin/env bash
# Build the network stack: NetworkManager (nmcli) and what it needs at run
# time, the D-Bus system bus, udev (eudev), GLib and wpa_supplicant for Wi-Fi.
#
# Unlike the rest of jk_os these are shared libraries and dynamically linked
# programs (NetworkManager loads its device plugins with dlopen), so the
# glibc runtime from the toolchain is shipped with them. Every package
# installs with prefix /usr into build/<arch>/dyn, which build-rootfs.sh
# copies into the image (without headers, static libraries and docs).
#
# A package is rebuilt only when its source tree or its build options change.
source "$(dirname "$0")/common.sh"
need make meson ninja pkg-config python3 gperf autoreconf "${CROSS_COMPILE}gcc"

for t in "${NET_TREES[@]}"; do
    src="${t}_SRC"
    [[ -d "${!src}" ]] || die "no source at ${!src#"$ROOT_DIR"/} (see ${t}_TREE in versions.env)"
done

HOST_TRIPLE="$ARCH-linux-gnu"
NET_OUT="$OUT_DIR/net"      # build directories
DYN="$OUT_DIR/dyn"          # install root (DESTDIR); the image's /usr comes from here
mkdir -p "$NET_OUT" "$DYN"

export CC="${CROSS_COMPILE}gcc" CXX="${CROSS_COMPILE}g++" AR="${CROSS_COMPILE}ar"
export RANLIB="${CROSS_COMPILE}ranlib" STRIP="${CROSS_COMPILE}strip"
# Find (and link) only what was built here, never the host's libraries.
export PKG_CONFIG_SYSROOT_DIR="$DYN" PKG_CONFIG_PATH=
export PKG_CONFIG_LIBDIR="$DYN/usr/lib/pkgconfig:$DYN/usr/share/pkgconfig"
export CPPFLAGS="-I$DYN/usr/include"
export LDFLAGS="-L$DYN/usr/lib -Wl,-rpath-link,$DYN/usr/lib"
AUTOCONF_ARGS=(--host="$HOST_TRIPLE" --prefix=/usr --libdir=/usr/lib --sysconfdir=/etc
               --localstatedir=/var --disable-static)

case "$ARCH" in
    x86_64)  MESON_CPU=x86_64;  OPENSSL_TARGET=linux-x86_64 ;;
    aarch64) MESON_CPU=aarch64; OPENSSL_TARGET=linux-aarch64 ;;
esac

# Meson cross file: target compiler, and pkg-config inside $DYN.
CROSS_FILE="$NET_OUT/meson-cross.ini"
cat > "$CROSS_FILE.new" <<EOF
[binaries]
c = '$CC'
cpp = '$CXX'
ar = '$AR'
strip = '$STRIP'
pkg-config = 'pkg-config'

[properties]
sys_root = '$DYN'
pkg_config_libdir = ['$DYN/usr/lib/pkgconfig', '$DYN/usr/share/pkgconfig']

[built-in options]
c_args = ['-I$DYN/usr/include']
c_link_args = ['-L$DYN/usr/lib', '-Wl,-rpath-link,$DYN/usr/lib']

[host_machine]
system = 'linux'
cpu_family = '$MESON_CPU'
cpu = '$MESON_CPU'
endian = 'little'
EOF
cmp -s "$CROSS_FILE.new" "$CROSS_FILE" && rm "$CROSS_FILE.new" || mv "$CROSS_FILE.new" "$CROSS_FILE"

# Build machine side: GLib's code generators are Python scripts, so the ones
# built into $DYN run on the host too (NetworkManager needs them).
NATIVE_FILE="$NET_OUT/meson-native.ini"
cat > "$NATIVE_FILE" <<EOF
[binaries]
glib-mkenums = ['python3', '$DYN/usr/bin/glib-mkenums']
glib-genmarshal = ['python3', '$DYN/usr/bin/glib-genmarshal']
gdbus-codegen = ['env', 'PYTHONPATH=$DYN/usr/share/glib-2.0', 'python3', '$DYN/usr/bin/gdbus-codegen']
EOF

# step <name> <src> <function> <options...>: run "<function> <src> <build dir>
# <options...>" unless the same options were already built from this tree.
step() {
    local name="$1" src="$2" fn="$3"; shift 3
    local out="$NET_OUT/$name" stamp="$NET_OUT/$name.done" want
    want="$(tree_version "$src") $*"
    if [[ -f "$stamp" && "$stamp" -nt "$src" && "$(cat "$stamp")" == "$want" ]]; then
        return
    fi
    log "building $name $(tree_version "$src") ($ARCH)"
    rm -rf "$out"; mkdir -p "$out"
    # A subshell with errexit, outside any "if": the first failing command stops it.
    local rc=0
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

# copy_tree: packages that build only inside their source tree.
copy_tree() { cp -a "$1/." "$2/"; }

autotools() {   # autotools <src> <out> <configure args...>
    local src="$1" out="$2"; shift 2
    (cd "$out" && "$src/configure" "${AUTOCONF_ARGS[@]}" "$@")
    make -C "$out" -j"$JOBS"
    make -C "$out" install DESTDIR="$DYN"
}

mesonpkg() {    # mesonpkg <src> <out> <meson options...>
    local src="$1" out="$2"; shift 2
    meson setup "$out" "$src" --cross-file "$CROSS_FILE" --native-file "$NATIVE_FILE" \
        --prefix=/usr --libdir=lib --sysconfdir=/etc --localstatedir=/var \
        --buildtype=release --wrap-mode=nodownload -Ddefault_library=shared "$@"
    ninja -C "$out"
    DESTDIR="$DYN" meson install -C "$out" --no-rebuild
}

b_zlib() {
    copy_tree "$1" "$2"
    (cd "$2" && CHOST="$HOST_TRIPLE" ./configure --prefix=/usr --libdir=/usr/lib --shared)
    make -C "$2" -j"$JOBS"
    make -C "$2" install DESTDIR="$DYN"
}

b_libndp() {    # from git: generate configure first
    copy_tree "$1" "$2"
    (cd "$2" && ./autogen.sh && ./configure "${AUTOCONF_ARGS[@]}")
    make -C "$2" -j"$JOBS"
    make -C "$2" install DESTDIR="$DYN"
}

b_eudev() {     # from git: generate configure first
    copy_tree "$1" "$2"
    (cd "$2" && autoreconf -fi && ./configure "${AUTOCONF_ARGS[@]}" \
        --exec-prefix=/usr --sbindir=/usr/sbin --with-rootprefix=/usr --with-rootlibdir=/usr/lib \
        --disable-blkid --disable-selinux --disable-kmod --disable-manpages --disable-hwdb \
        --disable-mtd --disable-rule-generator)
    make -C "$2" -j"$JOBS"
    make -C "$2" install DESTDIR="$DYN"
}

b_openssl() {
    # OpenSSL would put $CROSS_COMPILE in front of the already prefixed $CC.
    unset CROSS_COMPILE
    (cd "$2" && "$1/Configure" "$OPENSSL_TARGET" --prefix=/usr --libdir=lib --openssldir=/etc/ssl \
        shared no-tests no-docs no-apps no-legacy no-engine no-module no-ssl3 \
        CC="$CC" AR="$AR" RANLIB="$RANLIB")
    make -C "$2" -j"$JOBS"
    make -C "$2" install_sw DESTDIR="$DYN"
}

b_wpa() {
    copy_tree "$1" "$2"
    local w="$2/wpa_supplicant"
    # The Makefile tests "ifdef CONFIG_X", so an option is off only when its
    # line is gone (not "=n").
    grep -vE '^(CONFIG_DRIVER_WEXT|CONFIG_P2P|CONFIG_WPS|CONFIG_READLINE|CONFIG_WIFI_DISPLAY)=' \
        "$w/defconfig" > "$w/.config"
    cat >> "$w/.config" <<'EOF'
# jk_os: nl80211 drivers, D-Bus control for NetworkManager, WPA2/WPA3
# (SAE, OWE) and enterprise methods through OpenSSL, access-point mode for
# hotspots. No readline, WPS or P2P (NetworkManager is the user interface).
CONFIG_DRIVER_NL80211=y
CONFIG_LIBNL32=y
CONFIG_CTRL_IFACE=y
CONFIG_CTRL_IFACE_DBUS_NEW=y
CONFIG_CTRL_IFACE_DBUS_INTRO=y
CONFIG_TLS=openssl
CONFIG_SAE=y
CONFIG_OWE=y
CONFIG_SUITEB192=y
CONFIG_AP=y
CONFIG_DEBUG_SYSLOG=y
CONFIG_BGSCAN_SIMPLE=y
CONFIG_MATCH_IFACE=y
EOF
    make -C "$w" -j"$JOBS" CC="$CC" PKG_CONFIG=pkg-config \
        EXTRA_CFLAGS="$CPPFLAGS" LDFLAGS="$LDFLAGS" BINDIR=/usr/sbin wpa_supplicant wpa_cli wpa_passphrase
    install -Dm755 "$w/wpa_supplicant" "$DYN/usr/sbin/wpa_supplicant"
    install -Dm755 "$w/wpa_cli" "$DYN/usr/sbin/wpa_cli"
    install -Dm755 "$w/wpa_passphrase" "$DYN/usr/sbin/wpa_passphrase"
    install -Dm644 "$w/dbus/dbus-wpa_supplicant.conf" "$DYN/usr/share/dbus-1/system.d/wpa_supplicant.conf"
    # D-Bus starts it when NetworkManager first asks for it.
    install -d "$DYN/usr/share/dbus-1/system-services"
    sed -e 's|@BINDIR@|/usr/sbin|' "$w/dbus/fi.w1.wpa_supplicant1.service.in" \
        | sed '/^SystemdService/d' > "$DYN/usr/share/dbus-1/system-services/fi.w1.wpa_supplicant1.service"
}

b_readline() {
    (cd "$2" && bash_cv_termcap_lib=libncursesw bash_cv_wcwidth_broken=no \
        bash_cv_func_sigsetjmp=present bash_cv_must_reinstall_sighandlers=no \
        "$1/configure" "${AUTOCONF_ARGS[@]}" --with-curses --disable-install-examples)
    make -C "$2" -j"$JOBS" SHLIB_LIBS=-lncursesw
    make -C "$2" install DESTDIR="$DYN" SHLIB_LIBS=-lncursesw
}

step zlib   "$ZLIB_SRC"   b_zlib
# libuuid (for NetworkManager) as a shared library; the static tools have their own.
step libuuid "$UTIL_LINUX_SRC" autotools --disable-all-programs --enable-libuuid \
    --disable-nls --disable-asciidoc --disable-poman --disable-bash-completion --without-python \
    --without-systemd --without-udev --without-ncursesw --without-tinfo --without-readline
step libffi "$LIBFFI_SRC" autotools --disable-docs --disable-multi-os-directory
step pcre2  "$PCRE2_SRC"  autotools --enable-pcre2-8 --disable-pcre2grep-libz --disable-pcre2grep-libbz2
step glib   "$GLIB_SRC"   mesonpkg -Dtests=false -Dinstalled_tests=false -Dintrospection=disabled \
    -Dlibmount=disabled -Dselinux=disabled -Dxattr=false -Dman-pages=disabled -Ddocumentation=false \
    -Dnls=disabled -Dsysprof=disabled -Dlibelf=disabled -Dglib_debug=disabled
step expat  "$EXPAT_SRC"  autotools --without-docbook --without-examples --without-tests
step dbus   "$DBUS_SRC"   mesonpkg -Dsystemd=disabled -Dx11_autolaunch=disabled -Dselinux=disabled \
    -Dapparmor=disabled -Dlibaudit=disabled -Dmodular_tests=disabled -Dinstalled_tests=false \
    -Ddoxygen_docs=disabled -Dducktype_docs=disabled -Dxml_docs=disabled -Dqt_help=disabled \
    -Druntime_dir=/run -Dsystem_pid_file=/run/dbus/pid -Dsystem_socket=/run/dbus/system_bus_socket \
    -Ddbus_user=messagebus -Dmessage_bus=true -Dtools=true -Duser_session=false \
    -Dinotify=enabled -Depoll=enabled -Dtraditional_activation=true
step eudev  "$EUDEV_SRC"  b_eudev
step libndp "$LIBNDP_SRC" b_libndp
step libnl  "$LIBNL_SRC"  autotools --disable-cli --disable-debug
step openssl "$OPENSSL_SRC" b_openssl
step wpa_supplicant "$WPA_SUPPLICANT_SRC" b_wpa
step ncurses "$NCURSES_SRC" autotools --with-shared --without-normal --without-debug \
    --without-cxx --without-cxx-binding --without-ada --without-manpages --without-tests \
    --enable-widec --enable-pc-files --with-pkg-config-libdir=/usr/lib/pkgconfig \
    --with-default-terminfo-dir=/usr/share/terminfo --disable-stripping --with-build-cc=gcc
# readline links ncursesw for the terminal; the cache values are what a
# cross build can't test by running programs.
step readline "$READLINE_SRC" b_readline
step NetworkManager "$NETWORKMANAGER_SRC" mesonpkg \
    -Dsystemdsystemunitdir=no -Dsystemdsystemgeneratordir=no -Dsystemd_journal=false \
    -Dsession_tracking=no -Dsession_tracking_consolekit=false -Dsuspend_resume=consolekit \
    -Dpolkit=false -Dconfig_auth_polkit_default=root-only -Dmodify_system=false \
    -Dselinux=false -Dlibaudit=no -Dwext=false -Dwifi=true -Diwd=false \
    -Dconfig_wifi_backend_default=wpa_supplicant -Dppp=false -Dmodem_manager=false -Dofono=false \
    -Dconcheck=false -Dteamdctl=false -Dovs=false -Dnmcli=true -Dnmtui=false -Dnm_cloud_setup=false \
    -Dbluez5_dun=false -Debpf=false -Dnbft=false -Dclat=false -Difcfg_rh=false -Difupdown=false \
    -Dresolvconf=no -Dnetconfig=no -Ddhcpcd=no -Dconfig_dhcp_default=internal \
    -Dconfig_dns_rc_manager_default=file -Dconfig_logging_backend_default=syslog \
    -Diptables=/usr/sbin/iptables -Dnft=/usr/sbin/nft -Ddnsmasq=/usr/sbin/dnsmasq -Dmodprobe=/sbin/modprobe \
    -Dudev_dir=/usr/lib/udev -Ddbus_conf_dir=/usr/share/dbus-1/system.d -Druntime_dir=/run/NetworkManager \
    -Dintrospection=false -Dvapi=false -Ddocs=false -Dman=false -Dtests=no -Dfirewalld_zone=false \
    -Dlibpsl=false -Dcrypto=null -Dqt=false -Dreadline=libreadline -Dmore_asserts=no \
    -Dld_gc=false

log "network stack ($ARCH): $(du -sh "$DYN" | cut -f1) in $DYN"
