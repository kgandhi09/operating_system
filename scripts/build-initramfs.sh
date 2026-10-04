#!/usr/bin/env bash
# Assemble the small initramfs built into the kernel
# (build/<arch>/<device>-<kernel>/initramfs):
# BusyBox, e2fsck, the early-boot /init (initramfs/init), and the firmware
# network drivers load before the real root exists. /init mounts the OS image
# (jk_os.squashfs) and switches into it; see initramfs/init.
source "$(dirname "$0")/common.sh"

[[ -x "$BUSYBOX_OUT/busybox" ]] || die "busybox not built yet (run: make busybox)"
[[ -x "$TOOLS_OUT/sbin/e2fsck" ]] || die "disk tools not built yet (run: make tools)"

log "assembling initramfs ($ARCH)"
rm -rf "$INITRAMFS_DIR"
mkdir -p "$INITRAMFS_DIR"/{bin,sbin,dev,proc,sys,run,newroot}
cd "$INITRAMFS_DIR"

install -m 0755 "$BUSYBOX_OUT/busybox" bin/busybox
for a in sh mount umount mkdir sleep cat echo sed head grep mv rm ls ln chmod \
         blkid findfs switch_root setsid cttyhack mountpoint dmesg; do
    ln -s busybox "bin/$a"
done
install -m 0755 "$TOOLS_OUT/sbin/e2fsck" sbin/e2fsck
ln -s e2fsck sbin/fsck.ext4
# Disk tools a device's initramfs needs besides (DEVICE_INITRAMFS_TOOLS),
# e.g. mke2fs for a tablet installed from its initramfs.
for t in $DEVICE_INITRAMFS_TOOLS; do
    [[ -x "$TOOLS_OUT/sbin/$t" ]] || die "no $t in the disk tools (build/<arch>/tools/sbin)"
    install -m 0755 "$TOOLS_OUT/sbin/$t" "sbin/$t"
done
[[ " $DEVICE_INITRAMFS_TOOLS " == *" mke2fs "* ]] && ln -sf mke2fs sbin/mkfs.ext4
install -m 0755 "$ROOT_DIR/initramfs/init" init

# Drivers built into the kernel (Wi-Fi, some Ethernet) ask for firmware while
# the kernel starts, before jk_os.squashfs is mounted. The image has the same
# files for devices plugged in later.
# A device can leave them out (DEVICE_INITRAMFS_FIRMWARE=no) when its boot
# image has no room for them and its drivers don't need them that early.
FW_SRC="$ROOT_DIR/userspace/firmware"
if [[ "$DEVICE_INITRAMFS_FIRMWARE" == no ]]; then
    log "no firmware in the initramfs for $JK_DEVICE (the OS image still has it)"
elif [[ -f "$FW_SRC/$ARCH.files" ]]; then
    while IFS= read -r f; do
        [[ -f "$FW_SRC/$f" ]] || die "userspace/firmware/$f missing (run scripts/update-firmware.sh)"
        mkdir -p "lib/firmware/$(dirname "$f")"
        cp "$FW_SRC/$f" "lib/firmware/$f"
    done < "$FW_SRC/$ARCH.files"
else
    warn "no userspace/firmware/$ARCH.files: Wi-Fi and some Ethernet chips will lack firmware"
fi

# Firmware from the device's stock vendor partition for its drivers that
# start before the system is mounted (DEVICE_STOCK_FIRMWARE_EARLY).
# shellcheck disable=SC2086
copy_stock_firmware lib/firmware $DEVICE_STOCK_FIRMWARE_EARLY

# The device category's and device's own files (targets/devices/.../initramfs),
# e.g. /etc/jk/early hooks /init runs.
for d in "$CATEGORY_DIR" "$DEVICE_DIR"; do
    [[ -d "$d/initramfs" ]] && cp -a "$d/initramfs/." .
done

chmod -R go-w .
log "initramfs: $(du -sh . | cut -f1) in $INITRAMFS_DIR"
