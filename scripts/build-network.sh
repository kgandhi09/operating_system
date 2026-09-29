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

DYN_WORK=net
source "$(dirname "$0")/lib-dyn.sh"

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
