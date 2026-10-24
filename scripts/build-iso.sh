#!/usr/bin/env bash
# Package the kernel (which contains the rootfs) into a bootable hybrid ISO.
# The ISO can be booted from CD/DVD or written raw to a USB stick (flash.sh).
#
#   x86_64:  GRUB via grub-mkrescue -> BIOS and UEFI: GRUB built here from
#            userspace/hosttools/grub, its tools for this machine and its
#            pc and efi platforms with jk_os's x86_64 compiler.
#   aarch64: no bootloader. The arm64 kernel is an EFI application, so it goes
#            straight into the EFI system partition as EFI/BOOT/BOOTAA64.EFI.
source "$(dirname "$0")/common.sh"
need xorriso

kernel="$KERNEL_OUT/$KIMAGE"
[[ -f "$kernel" ]] || die "kernel not built yet (run: $MAKE_CMD kernel)"
[[ -f "$SQUASHFS_IMG" ]] || die "OS image not built yet (run: $MAKE_CMD rootfs)"

rm -rf "$ISO_DIR"
mkdir -p "$ISO_DIR/boot" "$IMAGE_DIR"
rm -f "$ISO"
# The OS image the kernel's initramfs looks for on the medium (label JK_OS).
cp "$SQUASHFS_IMG" "$ISO_DIR/jk_os.squashfs"

case "$ARCH" in
x86_64)
    need mformat
    # GRUB for BIOS (i386-pc) and UEFI (x86_64-efi), into one prefix: its
    # programs (grub-mkrescue) run here, its platforms' modules are built by
    # jk_os's x86_64 compiler. Rebuilt when its tree or this changes.
    # The BIOS kernel is linked at 0x9000 with -Ttext: GRUB picks --image-base
    # when the linker has it (binutils 2.44+), which puts the ELF headers in
    # front of .text and grub-mkrescue rejects the image (start 0x9074).
    GRUB_SRC="$ROOT_DIR/userspace/hosttools/grub"
    GRUB="$OUT_DIR/grub"
    [[ -d "$GRUB_SRC" ]] || die "no userspace/hosttools/grub (run: scripts/update-desktop-sources.sh --component hosttools grub)"
    want="$(tree_version "$GRUB_SRC") $(${CROSS_COMPILE}gcc -dumpversion) ttext mkfont"
    if [[ "$(cat "$GRUB/.done" 2>/dev/null)" != "$want" ]]; then
        rm -rf "$GRUB"
        for p in pc:i386 efi:x86_64; do
            log "building GRUB $(tree_version "$GRUB_SRC") (${p#*:}-${p%%:*})"
            mkdir -p "$GRUB/build-${p%%:*}"
            (cd "$GRUB/build-${p%%:*}" && "$GRUB_SRC/configure" --prefix="$GRUB/install" \
                --target="${p#*:}" --with-platform="${p%%:*}" --disable-nls --disable-werror \
                --enable-grub-mkfont --disable-grub-themes --disable-device-mapper \
                --disable-libzfs --disable-grub-mount --disable-efiemu \
                TARGET_CC="${CROSS_COMPILE}gcc" TARGET_OBJCOPY="${CROSS_COMPILE}objcopy" \
                TARGET_STRIP="${CROSS_COMPILE}strip" TARGET_NM="${CROSS_COMPILE}nm" \
                TARGET_RANLIB="${CROSS_COMPILE}ranlib" CC=gcc \
                > "$GRUB/build-${p%%:*}.log" 2>&1 \
             && make -j"$JOBS" TARGET_IMG_BASE_LDOPT=-Wl,-Ttext >> "$GRUB/build-${p%%:*}.log" 2>&1 \
             && make install >> "$GRUB/build-${p%%:*}.log" 2>&1) \
                || { tail -n 30 "$GRUB/build-${p%%:*}.log" >&2; die "GRUB failed, see $GRUB/build-${p%%:*}.log"; }
        done
        # grub-mkrescue draws the ISO's label (Mac boot picker) with
        # unicode.pf2, made here from DejaVu Sans (no GNU Unifont in the repo).
        "$GRUB/install/bin/grub-mkfont" -o "$GRUB/install/share/grub/unicode.pf2" \
            "$ROOT_DIR/userspace/desktop/dejavu-fonts/ttf/DejaVuSans.ttf"
        echo "$want" > "$GRUB/.done"
    fi
    platforms=(i386-pc x86_64-efi)

    cp "$kernel" "$ISO_DIR/boot/vmlinuz"
    mkdir -p "$ISO_DIR/boot/grub"
    sed -e "s/@OS_NAME@/$OS_NAME/g" -e "s/@OS_VERSION@/$OS_VERSION/g" \
        "$ROOT_DIR/boot/grub.cfg" > "$ISO_DIR/boot/grub/grub.cfg"

    log "building x86_64 ISO (GRUB: ${platforms[*]})"
    # No fonts in the image: boot/grub.cfg uses the text console.
    "$GRUB/install/bin/grub-mkrescue" --fonts= -o "$ISO" "$ISO_DIR" -- -volid JK_OS
    ;;
aarch64)
    need mkfs.fat mmd mcopy
    cp "$kernel" "$ISO_DIR/boot/Image"

    # EFI system partition image: kernel size + 8 MiB of slack, in KiB.
    esp="$TARGET_OUT/efiboot.img"
    size_kb=$(( ($(stat -c %s "$kernel") / 1024) + 8192 ))
    rm -f "$esp"
    mkfs.fat -C -n JK_OS_EFI "$esp" "$size_kb" >/dev/null
    mmd   -i "$esp" ::/EFI ::/EFI/BOOT
    mcopy -i "$esp" "$kernel" ::/EFI/BOOT/BOOTAA64.EFI

    log "building aarch64 ISO (EFI stub, no bootloader)"
    # The ESP is appended as GPT partition 2, which is how UEFI boots the ISO
    # written to a USB stick or disk. It is also the El Torito EFI image, but
    # an El Torito image can be at most 32 MiB and the kernel is larger, so
    # firmware generally can't boot this ISO from a CD/DVD.
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
