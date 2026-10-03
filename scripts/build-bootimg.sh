#!/usr/bin/env bash
# Package the kernel for a device whose bootloader loads an Android boot
# image (DEVICE_BOOT=android-bootimg), in build/<arch>/<device>-<kernel>/bootimg:
#
#   boot.img     the kernel (jk_os's initramfs is built into it), the device
#                tree (DEVICE_DTB) and the command line (DEVICE_CMDLINE), in
#                the stock boot image's format (DEVICE_MKBOOTIMG_ARGS)
#   vbmeta.img   Android Verified Boot off, so an unlocked bootloader starts
#                a kernel Samsung didn't sign
#   dtbo.img     the device's own overlays (targets/devices/.../dtbo/*.dts),
#                or none: either way none of Samsung's get applied to
#                jk_os's device tree
#   vendor_boot.img  (header v3/v4) the device tree, which moved out of
#                boot.img in those versions
#
# With DEVICE_BOOT=android-uboot the bootloader doesn't start Linux itself:
#   boot.img     U-Boot (build-uboot.sh), as the kernel
#   vendor_boot.img  the device tree the bootloader accepts (DEVICE_ABL_DTB),
#                and as its ramdisk a FIT image with jk_os's kernel and device
#                tree, which U-Boot finds through the bootloader and boots
#
# and, for Samsung's Odin, all of them in out/jk_os-<ver>-<name>-<arch>.tar.md5,
# to flash in the AP slot.
source "$(dirname "$0")/common.sh"
need python3 tar md5sum cpio
[[ -z "$DEVICE_AVB_FOOTERS" ]] || need openssl

AND="$ROOT_DIR/scripts/android"
AVB="$AND/avbtool.py"
[[ "$DEVICE_BOOT" == android-bootimg || "$DEVICE_BOOT" == android-uboot ]] \
    || die "device $JK_DEVICE doesn't boot from an Android boot image"
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

# The ramdisks are empty archives: the initramfs is in the kernel, and some
# bootloaders reject a boot image without one.
cpio -o -H newc --quiet < /dev/null > ramdisk.cpio

# Header v0/v1 have no place for a device tree: it follows the kernel
# (Image.gz-dtb). v2 has its own section. v3/v4 move it, with the load
# addresses, into vendor_boot.img.
images=(boot.img vbmeta.img dtbo.img)
if [[ "$DEVICE_BOOT" == android-uboot ]]; then
    (( hv >= 3 )) || die "android-uboot needs boot image header v3 or later (vendor_boot), not v$hv"
    [[ -f "$UBOOT_OUT/u-boot.bin" ]] || die "U-Boot not built yet (run: make uboot)"
    abl_dtb=$(compgen -G "$ROOT_DIR/$DEVICE_ABL_DTB" | head -n1 || true)
    [[ -n "$DEVICE_ABL_DTB" && -f "$abl_dtb" ]] \
        || die "no DEVICE_ABL_DTB ($DEVICE_ABL_DTB): read the stock firmware first (scripts/inspect-stock.sh)"
    # kernel_noload: U-Boot unpacks the kernel wherever it has room and
    # moves it where arm64 wants it. (mkimage -f auto would list such a
    # kernel as a "loadable", which bootm doesn't start: hence a .its.)
    cat > jk_os.its <<ITS
/dts-v1/;
/ {
	description = "$OS_NAME $OS_VERSION ($JK_NAME)";
	#address-cells = <1>;

	images {
		kernel {
			description = "Linux $(cat "$KERNEL_OUT/include/config/kernel.release")";
			data = /incbin/("$kernel");
			type = "kernel_noload";
			arch = "arm64";
			os = "linux";
			compression = "gzip";
			load = <0>;
			entry = <0>;
			hash { algo = "crc32"; };
		};
		fdt {
			description = "$(basename "$DEVICE_DTB")";
			data = /incbin/("$dtb");
			type = "flat_dt";
			arch = "arm64";
			compression = "none";
			hash { algo = "crc32"; };
		};
	};

	configurations {
		default = "jk_os";
		jk_os {
			kernel = "kernel";
			fdt = "fdt";
		};
	};
};
ITS
    DTC="$KERNEL_OUT/scripts/dtc/dtc" "$UBOOT_OUT/tools/mkimage" -q -f jk_os.its jk_os.itb
    kargs=(--kernel "$UBOOT_OUT/u-boot.bin" --vendor_boot vendor_boot.img --vendor_ramdisk jk_os.itb
           --dtb "$abl_dtb")
    images+=(vendor_boot.img)
else
case "$hv" in
    0|1) cat "$kernel" "$dtb" > Image.gz-dtb; kargs=(--kernel Image.gz-dtb) ;;
    2)   kargs=(--kernel "$kernel" --dtb "$dtb") ;;
    3|4) kargs=(--kernel "$kernel" --vendor_boot vendor_boot.img --vendor_ramdisk ramdisk.cpio --dtb "$dtb")
         images+=(vendor_boot.img) ;;
    *)   die "boot image header v$hv isn't supported" ;;
esac
fi

python3 "$AND/mkbootimg.py" "${kargs[@]}" --ramdisk ramdisk.cpio \
    --cmdline "$DEVICE_CMDLINE" "${args[@]}" --output boot.img

# Samsung's marker after the image data (stock: page-aligned, right after it).
[[ "$DEVICE_BOOTIMG_SEANDROID" == yes ]] && printf 'SEANDROIDENFORCE' >> boot.img

# Flag 2: verification disabled.
python3 "$AVB" make_vbmeta_image --flags 2 --padding_size 4096 --output vbmeta.img

# The device's overlays, built with the kernel's dtc; without any, a DTBO
# table with no entries (dt_table_header: magic, total and header size,
# entry size, entry count, entries offset, page size, version).
overlays=()
for src in "$DEVICE_DIR"/dtbo/*.dts; do
    [[ -f "$src" ]] || continue
    o="$(basename "$src" .dts).dtbo"
    # -a 8: each overlay's size a multiple of 8, so the next one in the table
    # starts aligned. A bootloader reading a misaligned tree rejects it.
    "$KERNEL_OUT/scripts/dtc/dtc" -q -I dts -O dtb -a 8 -o "$o" "$src"
    overlays+=("$o")
done
if (( ${#overlays[@]} )); then
    python3 "$AND/mkdtboimg.py" create dtbo.img --page_size=4096 "${overlays[@]}"
else
    python3 -c 'import struct, sys; sys.stdout.buffer.write(struct.pack(">8I", 0xd7b7ab1e, 32, 32, 32, 0, 32, 4096, 0))' > dtbo.img
fi

# Partitions whose stock image carries its own signed AVB footer (vbmeta
# chains to them) get one too: Samsung's Download mode checks for it as it
# flashes. The key is jk_os's own, made once per build tree; the bootloader
# doesn't check it, verification being off in vbmeta.img.
if [[ -n "$DEVICE_AVB_FOOTERS" ]]; then
    key="$OUT_DIR/avb-key.pem"
    [[ -f "$key" ]] || openssl genrsa -out "$key" 4096 2>/dev/null
    for spec in $DEVICE_AVB_FOOTERS; do          # <partition>:<size in bytes>
        part=${spec%%:*}
        python3 "$AVB" add_hash_footer --image "$part.img" --partition_name "$part" \
            --partition_size "${spec#*:}" --key "$key" --algorithm SHA256_RSA4096
    done
fi

# Odin: a ustar tar of images named after their partitions, with its MD5
# appended as "<md5>  <name>.tar".
tarname="$(basename "$BOOT_TAR" .md5)"
tar -H ustar -cf "$tarname" "${images[@]}"
md5sum -t "$tarname" >> "$tarname"
mv "$tarname" "$BOOT_TAR"

log "boot.img: $BOOTIMG_DIR/boot.img ($(du -h boot.img | cut -f1), header v$hv${abl_dtb:+, U-Boot})"
[[ -z "${abl_dtb:-}" ]] || log "vendor_boot.img: $(du -h vendor_boot.img | cut -f1), jk_os's kernel in jk_os.itb"
log "Odin (AP): $BOOT_TAR"
