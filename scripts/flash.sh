#!/usr/bin/env bash
# Write the jk_os ISO to a USB stick / SD card / disk.
#   scripts/flash.sh /dev/sdX            (uses the ISO for $ARCH)
#   scripts/flash.sh /dev/sdX image.iso
# EVERYTHING on the target device is destroyed. Refuses to touch the disk
# holding the running system, mounted devices, and partitions.
source "$(dirname "$0")/common.sh"
# A device with an Android bootloader is flashed over USB in Download mode.
[[ "$DEVICE_BOOT" == android-* ]] && exec "$ROOT_DIR/scripts/flash-heimdall.sh"
need lsblk findmnt dd

dev="${1:-}"
image="${2:-$ISO}"
[[ -n "$dev" ]] || die "usage: $0 /dev/<disk> [image.iso]   (see: lsblk -d -o NAME,SIZE,MODEL,TRAN)"
[[ -f "$image" ]] || die "image not found: $image (run: make iso)"
[[ -b "$dev" ]] || die "$dev is not a block device"

dev="$(readlink -f "$dev")"
[[ "$(lsblk -dno TYPE "$dev")" == disk ]] || die "$dev is not a whole disk (pass /dev/sdX, not /dev/sdX1)"

# Refuse the disk backing / or /boot, and anything currently mounted.
for mnt in / /boot /boot/efi; do
    src="$(findmnt -no SOURCE "$mnt" 2>/dev/null)" || continue
    # -s walks from the mount source up through LVM/LUKS/partitions to the disk.
    if lsblk -nspo NAME "$src" 2>/dev/null | sed 's/^[^/]*//' | grep -qxF "$dev"; then
        die "$dev holds $mnt of the running system"
    fi
done
if lsblk -no MOUNTPOINTS "$dev" 2>/dev/null | grep -q .; then
    lsblk -o NAME,SIZE,MOUNTPOINTS "$dev"
    die "$dev has mounted partitions; unmount them first"
fi

img_size=$(stat -c %s "$image")
dev_size=$(lsblk -bdno SIZE "$dev")
(( img_size <= dev_size )) || die "image ($img_size bytes) is larger than $dev ($dev_size bytes)"

echo
lsblk -do NAME,SIZE,MODEL,TRAN,RM "$dev"
[[ "$(lsblk -dno RM "$dev" | tr -d ' ')" == 1 ]] || warn "$dev is NOT a removable device"
echo
echo "About to write $(basename "$image") to $dev. ALL DATA ON $dev WILL BE LOST."
read -r -p "Type the device path ($dev) to continue: " answer
[[ "$answer" == "$dev" ]] || die "aborted"

sudo=()
[[ $EUID -eq 0 ]] || { need sudo; sudo=(sudo); }
log "writing $image -> $dev"
"${sudo[@]}" dd if="$image" of="$dev" bs=4M conv=fsync oflag=direct status=progress
sync
log "done — $dev is ready to boot"
