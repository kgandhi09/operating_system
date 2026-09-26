#!/usr/bin/env bash
# Assemble the root filesystem tree in build/<arch>/rootfs.
# The kernel build packs this tree into its built-in initramfs, together with
# configs/initramfs.list (device nodes, which need no root privileges there).
source "$(dirname "$0")/common.sh"

[[ -x "$BUSYBOX_OUT/busybox" ]] || die "busybox not built yet (run: make busybox)"
[[ -f "$BINARIES_DIR/sources.lock" ]] || die "binaries not fetched yet (run: make binaries)"
[[ -x "$TOOLS_OUT/sbin/sfdisk" ]] || die "disk tools not built yet (run: make tools)"

log "assembling rootfs ($ARCH)"
rm -rf "$ROOTFS_DIR"
mkdir -p "$ROOTFS_DIR"

make -C "$BUSYBOX_SRC" O="$BUSYBOX_OUT" ARCH="$KARCH" CROSS_COMPILE="$CROSS_COMPILE" \
    CONFIG_PREFIX="$ROOTFS_DIR" install >/dev/null
rm -f "$ROOTFS_DIR/linuxrc"

cd "$ROOTFS_DIR"
mkdir -p dev proc sys run tmp mnt root home var/log etc boot data
ln -s ../run var/run
ln -s bin/busybox init   # the kernel runs /init from an initramfs
# util-linux / e2fsprogs replace BusyBox's more limited applets (fdisk,
# mke2fs, ...). Drop the applet links first: cp would follow them and
# overwrite the busybox binary itself.
for f in "$TOOLS_OUT/sbin/"*; do
    rm -f {bin,sbin,usr/bin,usr/sbin}/"$(basename "$f")"
done
cp -a "$TOOLS_OUT/sbin/." sbin/
# /usr/local/bin comes first in PATH, so these win over BusyBox applets.
mkdir -p usr/local/bin
if compgen -G "$BINARIES_DIR/bin/*" >/dev/null; then
    cp -a "$BINARIES_DIR/bin/." usr/local/bin/
fi
cp -a "$ROOT_DIR/rootfs/." "$ROOTFS_DIR/"

echo "$OS_HOSTNAME" > etc/hostname
printf '127.0.1.1\t%s\n' "$OS_HOSTNAME" >> etc/hosts

cat > etc/os-release <<OSR
NAME="$OS_NAME"
ID=$OS_NAME
VERSION="$OS_VERSION"
VERSION_ID=$OS_VERSION
PRETTY_NAME="$OS_NAME $OS_VERSION ($ARCH)"
OSR

printf '\n%s %s (%s) \\n \\l\n\n' "$OS_NAME" "$OS_VERSION" "$ARCH" > etc/issue

chmod -R go-w .
chmod 0700 root
chmod 0600 etc/shadow
chmod 1777 tmp

log "rootfs: $(du -sh . | cut -f1) in $ROOTFS_DIR"
