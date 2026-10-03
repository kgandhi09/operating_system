#!/usr/bin/env bash
# Package the kernel for a device whose bootloader loads an Android boot
# image (DEVICE_BOOT=android-bootimg), in build/<arch>/<device>-<kernel>/bootimg:
#
#   boot.img     the kernel (jk_os's initramfs is built into it), the device
#                tree (DEVICE_DTB) and the command line (DEVICE_CMDLINE), in
#                the stock boot image's format (DEVICE_MKBOOTIMG_ARGS)
#   vbmeta.img   Android Verified Boot off, so an unlocked bootloader starts
#                a kernel Samsung didn't sign
#   dtbo.img     no overlays, so the bootloader applies none of Samsung's to
#                jk_os's device tree
#
# and, for Samsung's Odin, the three in out/jk_os-<ver>-<name>-<arch>.tar.md5,
# to flash in the AP slot.
source "$(dirname "$0")/common.sh"
need python3 tar md5sum cpio

AND="$ROOT_DIR/scripts/android"
[[ "$DEVICE_BOOT" == android-bootimg ]] || die "device $JK_DEVICE doesn't boot from an Android boot image"
[[ -n "$DEVICE_MKBOOTIMG_ARGS" ]] \
    || die "DEVICE_MKBOOTIMG_ARGS is empty in targets/devices/$JK_CATEGORY/$JK_DEVICE/device.env: copy the stock boot.img's from scripts/inspect-stock.sh"
kernel="$KERNEL_OUT/$KIMAGE"
dtb="$DTB_OUT/$DEVICE_DTB"
[[ -f "$kernel" ]] || die "kernel not built yet (run: make kernel)"
[[ -f "$dtb" ]] || die "device tree not built yet (run: make kernel)"

read -ra args <<< "$DEVICE_MKBOOTIMG_ARGS"
hv=0
for (( i = 0; i < ${#args[@]}; i++ )); do
    [[ "${args[i]}" == --header_version ]] && hv=${args[i + 1]}
done

rm -rf "$BOOTIMG_DIR"
mkdir -p "$BOOTIMG_DIR" "$IMAGE_DIR"
cd "$BOOTIMG_DIR"

# Header v0/v1 have no place for a device tree: it follows the kernel
# (Image.gz-dtb). v2 has its own section. v3/v4 move it to vendor_boot.
case "$hv" in
    0|1) cat "$kernel" "$dtb" > Image.gz-dtb; kargs=(--kernel Image.gz-dtb) ;;
    2)   kargs=(--kernel "$kernel" --dtb "$dtb") ;;
    *)   die "boot image header v$hv (vendor_boot) isn't supported yet" ;;
esac

# The ramdisk is an empty archive: the initramfs is in the kernel, and some
# bootloaders reject a boot image without one.
cpio -o -H newc --quiet < /dev/null > ramdisk.cpio

python3 "$AND/mkbootimg.py" "${kargs[@]}" --ramdisk ramdisk.cpio \
    --cmdline "$DEVICE_CMDLINE" "${args[@]}" --output boot.img

# Flag 2: verification disabled.
python3 "$AND/avbtool.py" make_vbmeta_image --flags 2 --padding_size 4096 --output vbmeta.img

# A DTBO table with no entries (dt_table_header: magic, total and header
# size, entry size, entry count, entries offset, page size, version).
python3 -c 'import struct, sys; sys.stdout.buffer.write(struct.pack(">8I", 0xd7b7ab1e, 32, 32, 32, 0, 32, 4096, 0))' > dtbo.img

# Odin: a ustar tar of images named after their partitions, with its MD5
# appended as "<md5>  <name>.tar".
tarname="$(basename "$BOOT_TAR" .md5)"
tar -H ustar -cf "$tarname" boot.img vbmeta.img dtbo.img
md5sum -t "$tarname" >> "$tarname"
mv "$tarname" "$BOOT_TAR"

log "boot.img: $BOOTIMG_DIR/boot.img ($(du -h boot.img | cut -f1), header v$hv)"
log "Odin (AP): $BOOT_TAR"
