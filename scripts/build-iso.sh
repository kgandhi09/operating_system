#!/usr/bin/env bash
# Package the kernel (which contains the rootfs) into a bootable hybrid ISO.
# The ISO can be booted from CD/DVD or written raw to a USB stick (flash.sh).
#
#   x86_64:  GRUB via grub-mkrescue -> BIOS and/or UEFI, whichever GRUB
#            platforms are installed (grub-pc-bin, grub-efi-amd64-bin).
#   aarch64: no bootloader. The arm64 kernel is an EFI application, so it goes
#            straight into the EFI system partition as EFI/BOOT/BOOTAA64.EFI.
source "$(dirname "$0")/common.sh"
need xorriso

kernel="$KERNEL_OUT/$KIMAGE"
[[ -f "$kernel" ]] || die "kernel not built yet (run: make kernel)"

rm -rf "$ISO_DIR"
mkdir -p "$ISO_DIR/boot" "$IMAGE_DIR"
rm -f "$ISO"

case "$ARCH" in
x86_64)
    need grub-mkrescue mformat
    platforms=()
    for p in i386-pc x86_64-efi; do
        [[ -d "/usr/lib/grub/$p" ]] && platforms+=("$p")
    done
    (( ${#platforms[@]} )) || die "no GRUB platform files found (install grub-pc-bin and/or grub-efi-amd64-bin)"
    [[ " ${platforms[*]} " == *" i386-pc "* ]]    || warn "grub-pc-bin missing: ISO will not boot on legacy BIOS"
    [[ " ${platforms[*]} " == *" x86_64-efi "* ]] || warn "grub-efi-amd64-bin missing: ISO will not boot on UEFI"

    cp "$kernel" "$ISO_DIR/boot/vmlinuz"
    mkdir -p "$ISO_DIR/boot/grub"
    sed -e "s/@OS_NAME@/$OS_NAME/g" -e "s/@OS_VERSION@/$OS_VERSION/g" \
        "$ROOT_DIR/boot/grub.cfg" > "$ISO_DIR/boot/grub/grub.cfg"

    log "building x86_64 ISO (GRUB: ${platforms[*]})"
    grub-mkrescue -o "$ISO" "$ISO_DIR" -- -volid JK_OS
    ;;
aarch64)
    need mkfs.fat mmd mcopy
    cp "$kernel" "$ISO_DIR/boot/Image"

    # EFI system partition image: kernel size + 8 MiB of slack, in KiB.
    esp="$OUT_DIR/efiboot.img"
    size_kb=$(( ($(stat -c %s "$kernel") / 1024) + 8192 ))
    rm -f "$esp"
    mkfs.fat -C -n JK_OS_EFI "$esp" "$size_kb" >/dev/null
    mmd   -i "$esp" ::/EFI ::/EFI/BOOT
    mcopy -i "$esp" "$kernel" ::/EFI/BOOT/BOOTAA64.EFI

    log "building aarch64 ISO (EFI stub, no bootloader)"
    # The ESP is appended as GPT partition 2 and doubles as the El Torito EFI
    # boot image, so the same file boots as optical media and as a USB disk.
    xorriso -as mkisofs \
        -iso-level 3 -full-iso9660-filenames -joliet -rational-rock \
        -volid JK_OS \
        -appended_part_as_gpt \
        -append_partition 2 C12A7328-F81F-11D2-BA4B-00A0C93EC93B "$esp" \
        -e --interval:appended_partition_2:all:: -no-emul-boot \
        -output "$ISO" "$ISO_DIR" 2>/dev/null
    ;;
esac

log "ISO: $ISO ($(du -h "$ISO" | cut -f1))"
