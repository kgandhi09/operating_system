#!/usr/bin/env bash
# Build the device's tree (DEVICE_DTB) into build/<arch>/<device>-<kernel>/
# dtbs. Its source is the kernel profile's dts/ (targets/kernels/<kernel>/
# dts/<vendor>/<board>.dts) or, for a board mainline already has, the kernel
# tree's arch/<arch>/boot/dts. Either way it is built the way kbuild builds
# the kernel's own trees (cpp, then the kernel's dtc), against the kernel
# tree's .dtsi files and headers, so the kernel tree itself stays untouched.
# build-kernel.sh runs this after the kernel.
source "$(dirname "$0")/common.sh"

[[ -n "$DEVICE_DTB" ]] || { log "device $JK_DEVICE has no device tree to build"; exit 0; }
[[ -f "$KERNEL_OUT/.config" ]] || die "kernel not configured yet (run: make kernel)"
need "${CROSS_COMPILE}gcc"

name=${DEVICE_DTB%.dtb}                    # qcom/<board>
kdts="$KERNEL_SRC/arch/$KARCH/boot/dts"
src=
for d in "$KERNEL_DIR/dts" "$kdts"; do
    [[ -f "$d/$name.dts" ]] && { src="$d/$name.dts"; break; }
done
[[ -n "$src" ]] || die "no $name.dts in targets/kernels/$JK_KERNEL/dts or the kernel tree"

# The kernel's own dtc, built from the kernel tree.
make -C "$KERNEL_SRC" O="$KERNEL_OUT" ARCH="$KARCH" CROSS_COMPILE="$CROSS_COMPILE" scripts_dtc >/dev/null </dev/null
dtc="$KERNEL_OUT/scripts/dtc/dtc"

out="$DTB_OUT/$DEVICE_DTB"
mkdir -p "$(dirname "$out")"
vendor_dir=$(dirname "$name")
# As kbuild's cmd_dtc (scripts/Makefile.dtbs, its default W=0 warnings): the .dts's own directory first (cpp does that for
# "quoted" includes), then the kernel tree's same vendor directory for its
# .dtsi files, and the dt-bindings headers through include-prefixes.
"${CROSS_COMPILE}gcc" -E -nostdinc -undef -D__DTS__ -x assembler-with-cpp \
    -I "$(dirname "$src")" -I "$kdts/$vendor_dir" -I "$kdts" \
    -I "$KERNEL_SRC/scripts/dtc/include-prefixes" \
    -o "$out.pre" "$src"
# -@ keeps the labels (__symbols__), as kbuild does for trees overlays apply
# to: a bootloader applying dtbo overlays may need them.
"$dtc" -O dtb -o "$out" -b 0 -@ -a 8 \
    -i "$(dirname "$src")" -i "$kdts/$vendor_dir" -i "$KERNEL_SRC/scripts/dtc/include-prefixes" \
    -Wno-unique_unit_address -Wno-unit_address_vs_reg -Wno-avoid_unnecessary_addr_size \
    -Wno-alias_paths -Wno-interrupt_map -Wno-simple_bus_reg \
    "$out.pre"
rm -f "$out.pre"
log "device tree: $out ($(du -h "$out" | cut -f1), from ${src#"$ROOT_DIR"/})"

# A device with stock firmware read (DEVICE_STOCK, scripts/inspect-stock.sh):
# the tree must reserve every memory region stock's does, its base trees with
# Samsung's dtbo overlays applied (a region the firmware owns reset the
# Galaxy Tab S7 FE whenever Linux used it).
stock=$(compgen -G "$ROOT_DIR/$DEVICE_STOCK" | head -n1 || true)
if [[ -n "$DEVICE_STOCK" && -d "$stock/dtb" && -d "$stock/dtbo" ]]; then
    need python3 fdtoverlay
    # A captured bootloader tree can contain reservations that are absent
    # from both vendor_boot and dtbo (Samsung's UH regions are one case).
    bootloader_check=()
    [[ ! -f "$stock/abl.dtb" ]] || bootloader_check=(--bootloader "$stock/abl.dtb")
    PATH="$(dirname "$dtc"):$PATH" python3 "$ROOT_DIR/scripts/check-reserved-memory.py" \
        "$out" "$stock"/dtb/*.dtb -- "$stock"/dtbo/*.dtb "${bootloader_check[@]}" \
        || die "$DEVICE_DTB doesn't reserve all of stock's memory (above)"
fi
