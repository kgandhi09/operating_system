#!/usr/bin/env bash
# Assemble the root filesystem tree in build/<arch>/rootfs.
# The kernel build packs this tree into its built-in initramfs, together with
# configs/initramfs.list (device nodes, which need no root privileges there).
source "$(dirname "$0")/common.sh"

[[ -x "$BUSYBOX_OUT/busybox" ]] || die "busybox not built yet (run: make busybox)"

log "assembling rootfs ($ARCH)"
rm -rf "$ROOTFS_DIR"
mkdir -p "$ROOTFS_DIR"

make -C "$BUSYBOX_SRC" O="$BUSYBOX_OUT" ARCH="$KARCH" CROSS_COMPILE="$CROSS_COMPILE" \
    CONFIG_PREFIX="$ROOTFS_DIR" install >/dev/null
rm -f "$ROOTFS_DIR/linuxrc"

cd "$ROOTFS_DIR"
mkdir -p dev proc sys run tmp mnt root home var/log etc
ln -s ../run var/run
ln -s bin/busybox init   # the kernel runs /init from an initramfs
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
chmod 1777 tmp

log "rootfs: $(du -sh . | cut -f1) in $ROOTFS_DIR"
