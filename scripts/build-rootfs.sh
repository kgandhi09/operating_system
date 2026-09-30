#!/usr/bin/env bash
# Assemble the root filesystem tree in build/<arch>/rootfs, and pack it into
# the OS image build/<arch>/jk_os.squashfs. At boot the kernel's small
# initramfs (build-initramfs.sh) mounts that image read-only, with a writable
# layer on top (RAM on the live medium, /data on an installed system).
source "$(dirname "$0")/common.sh"

[[ -x "$BUSYBOX_OUT/busybox" ]] || die "busybox not built yet (run: make busybox)"
[[ -f "$BINARIES_DIR/sources.lock" ]] || die "binaries not fetched yet (run: make binaries)"
[[ -x "$TOOLS_OUT/sbin/sfdisk" ]] || die "disk tools not built yet (run: make tools)"
DYN="$OUT_DIR/dyn"
[[ -x "$DYN/usr/sbin/NetworkManager" ]] || die "network stack not built yet (run: make network)"
TC="$OUT_DIR/toolchain"
[[ -x "$TC/usr/bin/gcc" && -x "$TC/usr/bin/clang" ]] || die "toolchain not built yet (run: make toolchain)"
[[ -x "$DYN/usr/bin/startplasma-wayland" ]] || die "desktop not built yet (run: make desktop)"

log "assembling rootfs ($ARCH)"
rm -rf "$ROOTFS_DIR"
mkdir -p "$ROOTFS_DIR"

# Same flags as build-busybox.sh, or make would rebuild BusyBox without them.
make -C "$BUSYBOX_SRC" O="$BUSYBOX_OUT" ARCH="$KARCH" CROSS_COMPILE="$CROSS_COMPILE" \
    EXTRA_CFLAGS="${NOWARN_CFLAGS:-}" \
    HOSTCFLAGS="-Wall -Wstrict-prototypes -O2 -fomit-frame-pointer ${NOWARN_CFLAGS:-}" \
    CONFIG_PREFIX="$ROOTFS_DIR" install >/dev/null
rm -f "$ROOTFS_DIR/linuxrc"

cd "$ROOTFS_DIR"
mkdir -p dev proc sys run tmp mnt root home var/log etc boot data
ln -s ../run var/run
# util-linux / e2fsprogs / shadow replace BusyBox's more limited applets
# (fdisk, mke2fs, ...). Drop the applet links first: cp would follow them and
# overwrite the busybox binary itself.
for f in "$TOOLS_OUT/sbin/"* "$TOOLS_OUT/bin/"*; do
    rm -f {bin,sbin,usr/bin,usr/sbin}/"$(basename "$f")"
done
cp -a "$TOOLS_OUT/sbin/." sbin/
cp -a "$TOOLS_OUT/bin/." bin/   # passwd, su, ... keep their setuid bit
# The network stack (build-network.sh: NetworkManager, D-Bus, udev, GLib,
# wpa_supplicant) and the desktop (build-desktop.sh: KDE Plasma, Qt, Mesa,
# elogind). Only what runs: no headers, static libraries, docs, translations
# or build tools. BusyBox keeps clear/reset/tput and friends. GLib's settings
# schemas stay compiled only (gschemas.compiled: GTK aborts without it).
(cd "$DYN" && tar -cf - \
    --exclude=./usr/include --exclude=./usr/lib/pkgconfig --exclude=./usr/share/pkgconfig \
    --exclude=./usr/lib/cmake --exclude=./usr/lib/glib-2.0 --exclude=./usr/lib/dbus-1.0 \
    --exclude=./usr/lib/engines-3 --exclude=./usr/lib/ossl-modules \
    --exclude='*.a' --exclude='*.la' \
    --exclude=./usr/share/man --exclude=./usr/share/doc --exclude=./usr/share/info \
    --exclude=./usr/share/locale --exclude=./usr/share/aclocal --exclude=./usr/share/gdb \
    --exclude=./usr/share/bash-completion --exclude=./usr/share/gettext \
    --exclude=./usr/share/terminfo \
    --exclude=./usr/share/glib-2.0/codegen --exclude=./usr/share/glib-2.0/dtds \
    --exclude=./usr/share/glib-2.0/gdb --exclude=./usr/share/glib-2.0/valgrind \
    --exclude='./usr/share/glib-2.0/schemas/*.xml' --exclude=./usr/share/glib-2.0/schemas/gschema.dtd \
    --exclude=./etc/NetworkManager/dnsmasq.d --exclude=./etc/NetworkManager/dnsmasq-shared.d \
    --exclude=./usr/libexec/gio-launch-desktop --exclude=./usr/libexec/nm-initrd-generator \
    --exclude=./usr/libexec/dbus-daemon-launch-helper \
    --exclude=./usr/lib/qt6/mkspecs --exclude=./usr/lib/qt6/metatypes --exclude=./usr/lib/qt6/modules \
    --exclude=./usr/lib/icu --exclude=./usr/lib/systemd --exclude='./usr/lib/python3*' \
    --exclude=./usr/share/ECM --exclude=./usr/share/wayland-protocols \
    --exclude=./usr/share/plasma-wayland-protocols --exclude=./usr/share/xcb \
    --exclude=./usr/share/libtool --exclude=./usr/share/qt6/modules --exclude=./usr/share/qt6/sbom \
    $(for b in glib-compile-resources glib-compile-schemas glib-genmarshal glib-gettextize \
               glib-mkenums gdbus-codegen gtester gtester-report gi-compile-repository \
               gi-decompile-typelib gi-inspect-typelib gresource gobject-query gio-querymodules \
               gapplication pcre2-config pcre2grep pcre2test xmlwf ncursesw6-config dbus-launch \
               dbus-test-tool dbus-cleanup-sockets \
               qmake qmake6 qtpaths qtpaths6 target_qt.conf qt-cmake qt-cmake-create \
               qt-configure-module icu-config libtool libtoolize libtool-next-version \
               libpng-config libpng16-config xml2-config wayland-scanner genbrk gencfu curl-config \
               gencnval gendict genrb makeconv pkgdata icuexportdata derb \
               clear reset tput tset tabs captoinfo infocmp infotocap tic toe; do
          echo "--exclude=./usr/bin/$b"; done) .) | tar -xf - -C "$ROOTFS_DIR"
# The C/C++ toolchain (build-toolchain.sh): GCC, Clang/LLVM, binutils,
# CMake, Ninja, GDB, glibc and Linux headers. Docs and translations stay out.
(cd "$TC" && tar -cf - --exclude=./lib --exclude=./lib64 \
    --exclude=./usr/share/doc --exclude=./usr/share/info --exclude=./usr/share/man \
    --exclude=./usr/share/locale --exclude=./usr/share/gdb/python .) | tar -xf - -C "$ROOTFS_DIR"

# Terminal descriptions for the consoles and common terminals only.
for t in d/dumb l/linux v/vt100 v/vt102 v/vt220 x/xterm x/xterm-256color x/xterm-color \
         s/screen s/screen-256color t/tmux t/tmux-256color a/ansi; do
    [[ -e "$DYN/usr/share/terminfo/$t" ]] || continue
    mkdir -p "usr/share/terminfo/${t%/*}"
    cp -L "$DYN/usr/share/terminfo/$t" "usr/share/terminfo/$t"
done
find usr/bin usr/sbin usr/libexec usr/lib -type f \( -perm -u+x -o -name '*.so*' \) \
    -exec "${CROSS_COMPILE}strip" --strip-unneeded {} + 2>/dev/null || true

# The glibc runtime those programs link against, from the cross toolchain:
# every library some ELF file needs and the image doesn't have yet.
elf_needs() {
    find . -xdev -type f \( -perm -u+x -o -name '*.so*' \) -print0 \
        | xargs -0 "${CROSS_COMPILE}readelf" -d 2>/dev/null \
        | sed -n 's/.*(NEEDED).*\[\(.*\)\]/\1/p' | sort -u
}
while :; do
    added=0
    for lib in $(elf_needs); do
        [[ -e "usr/lib/$lib" ]] && continue
        # A package's private library, found through its RUNPATH (elogind's).
        [[ -n "$(find usr/lib -mindepth 2 -name "$lib" -print -quit)" ]] && continue
        from="$("${CROSS_COMPILE}gcc" -print-file-name="$lib")"
        [[ "$from" == /* && -f "$from" ]] || die "no $lib in the toolchain (needed by the network stack)"
        cp -L "$from" "usr/lib/$lib"
        added=1
    done
    (( added )) || break
done
# pthread_cancel/exit unwinding makes glibc dlopen libgcc_s.
[[ -e usr/lib/libgcc_s.so.1 ]] || cp -L "$("${CROSS_COMPILE}gcc" -print-file-name=libgcc_s.so.1)" usr/lib/
# glibc builds in only the plain C locale; C.UTF-8 is data it loads from
# /usr/lib/locale (without it, LANG=C.UTF-8 falls back to ASCII and perl
# warns). The build host's copy (libc-bin) is the toolchain's glibc release.
[[ -d /usr/lib/locale/C.utf8 ]] || die "no C.UTF-8 locale on the build host (/usr/lib/locale/C.utf8, from libc-bin)"
mkdir -p usr/lib/locale
cp -R /usr/lib/locale/C.utf8 usr/lib/locale/
# The dynamic loader where the programs look for it (/lib64/ld-linux-x86-64.so.2,
# /lib/ld-linux-aarch64.so.1): /lib and /lib64 point at /usr/lib.
ln -s usr/lib lib
ln -s usr/lib lib64
interp="$("${CROSS_COMPILE}readelf" -l usr/sbin/NetworkManager | sed -n 's/.*interpreter: \(.*\)\]/\1/p')"
[[ -e ".$interp" ]] || die "dynamic loader $interp missing from the image"

# Firmware for this arch's network chips (userspace/firmware/<arch>.files,
# see scripts/update-firmware.sh), zstd-compressed as the kernel loads it.
FW_SRC="$ROOT_DIR/userspace/firmware"
if [[ -f "$FW_SRC/$ARCH.files" ]]; then
    while IFS= read -r f; do
        [[ -f "$FW_SRC/$f" ]] || die "userspace/firmware/$f missing (run scripts/update-firmware.sh)"
        mkdir -p "usr/lib/firmware/$(dirname "$f")"
        cp "$FW_SRC/$f" "usr/lib/firmware/$f"
    done < "$FW_SRC/$ARCH.files"
else
    warn "no userspace/firmware/$ARCH.files: Wi-Fi and some Ethernet chips will lack firmware"
fi
# Firmware shared by many chips (<arch>.linked: NVIDIA's GSP firmware), links kept.
if [[ -f "$FW_SRC/$ARCH.linked" ]]; then
    while IFS= read -r f; do
        [[ -n "$f" ]] || continue
        [[ -e "$FW_SRC/$f" || -L "$FW_SRC/$f" ]] || die "userspace/firmware/$f missing (run scripts/update-firmware.sh)"
        mkdir -p "usr/lib/firmware/$(dirname "$f")"
        cp -P "$FW_SRC/$f" "usr/lib/firmware/$f"
    done < "$FW_SRC/$ARCH.linked"
fi

# The kernel's loadable modules (nouveau) and NVIDIA's driver
# (build-nvidia.sh): modules, libraries, tools and GSP firmware.
if [[ -d "$MODULES_OUT/lib/modules" ]]; then
    mkdir -p usr/lib/modules
    cp -a "$MODULES_OUT/lib/modules/." usr/lib/modules/
fi
if [[ -d "$OUT_DIR/nvidia" ]]; then
    cp -a "$OUT_DIR/nvidia/." .
fi
for d in usr/lib/modules/*/; do
    [[ -d "$d" ]] && depmod -b "$ROOTFS_DIR" "$(basename "$d")"
done

# /usr/local/bin comes first in PATH, so these win over BusyBox applets.
mkdir -p usr/local/bin
if compgen -G "$BINARIES_DIR/bin/*" >/dev/null; then
    cp -a "$BINARIES_DIR/bin/." usr/local/bin/
fi
cp -a "$ROOT_DIR/rootfs/." "$ROOTFS_DIR/"
# The Debian base system apt-setup unpacks into /data/apt when apt is
# chosen (scripts/update-debian-rootfs.sh).
DEB_SRC="$ROOT_DIR/userspace/debian/$ARCH"
[[ -f "$DEB_SRC/rootfs.tar.gz" ]] || die "no userspace/debian/$ARCH/rootfs.tar.gz (run: scripts/update-debian-rootfs.sh $ARCH)"
mkdir -p usr/share/jk_os/debian
cp "$DEB_SRC/rootfs.tar.gz" "$DEB_SRC/pin" usr/share/jk_os/debian/

echo "$OS_HOSTNAME" > etc/hostname
printf '127.0.1.1\t%s\n' "$OS_HOSTNAME" >> etc/hosts

cat > etc/os-release <<OSR
NAME="$OS_NAME"
ID=$OS_NAME
VERSION="$OS_VERSION"
VERSION_ID=$OS_VERSION
PRETTY_NAME="$OS_NAME $OS_VERSION ($ARCH)"
LOGO=jk-os
OSR

# Before login: the banner (getty reads backslashes as escapes such as \n,
# so the art's own are doubled), then name, version, hostname and console.
{
    echo
    sed 's/\\/\\\\/g' usr/share/jk_os/banner
    printf '\n  %s %s (%s) - \\n - \\l\n\n' "$OS_NAME" "$OS_VERSION" "$ARCH"
} > etc/issue
# After login: a short welcome (shadow's login shows it; ~/.hushlogin silences it).
cat > etc/motd <<MOTD

  Welcome to $OS_NAME $OS_VERSION, the J.K. Robotics Pvt. Ltd. OS.

    sudo <command>     run a command as root (administrators)
    nmcli              network: wired, Wi-Fi, IPv6
    gcc / clang        C and C++ (C++20), with cmake, ninja and gdb
    git / ssh / curl   version control, remote login, downloads
    sudo apt install   Debian packages, kept in /data/apt (if chosen at install)
    jk-dev             the dev session (a full-screen terminal), on tty1
    jk-gui             the desktop (KDE Plasma), on tty2 (Ctrl+Alt+F2)
                       tty3 (Ctrl+Alt+F3) is a plain text console

MOTD

chmod -R go-w .
chmod 0700 root
chmod 0600 etc/shadow etc/gshadow
chmod 0440 etc/sudoers
chmod 0750 etc/sudoers.d
chmod 0440 etc/sudoers.d/*
chmod 1777 tmp
# setuid helpers the strip above rewrote (strip drops the setuid bit).
chmod 4755 usr/lib/jk_os/jk-session

log "rootfs: $(du -sh . | cut -f1) in $ROOTFS_DIR"

# Every file belongs to root in the image (-all-root), as the builder's
# files would otherwise keep the builder's uid.
need mksquashfs
rm -f "$SQUASHFS_IMG"
mksquashfs "$ROOTFS_DIR" "$SQUASHFS_IMG" -all-root -noappend -comp zstd -Xcompression-level 15 \
    -b 256K -quiet -no-progress >/dev/null
log "image: $SQUASHFS_IMG ($(du -h "$SQUASHFS_IMG" | cut -f1))"
