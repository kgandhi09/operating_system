#!/usr/bin/env bash
# Flash an Android-bootloader Samsung device (DEVICE_BOOT=android-bootimg)
# from Linux with Heimdall, Odin's open-source counterpart: the boot.img,
# vendor_boot.img (header v3/v4), vbmeta.img and dtbo.img that make bootimg
# made. make flash runs this for such a device.
#
#   scripts/flash-heimdall.sh            flash, then the device reboots
#   scripts/flash-heimdall.sh --pit      only show the device's partitions
#
# The device must be in Download mode: power it off, then hold Vol Up +
# Vol Down while plugging in the USB cable, and press Vol Up to continue.
# Each image goes to the partition the device's own partition table (PIT)
# names for that file (boot.img -> boot, ...); nothing else is written.
source "$(dirname "$0")/common.sh"
need heimdall

[[ "$DEVICE_BOOT" == android-* ]] || die "$JK_DEVICE isn't flashed with Heimdall (see make flash)"

heimdall detect >/dev/null 2>&1 \
    || die "no Samsung device in Download mode on USB (power off, hold Vol Up + Vol Down, plug in the cable, press Vol Up)"

# Read the PIT and leave the device in Download mode for the flash session
# (--no-reboot here, --resume there).
pit="$BOOTIMG_DIR/device-pit.txt"
mkdir -p "$BOOTIMG_DIR"
heimdall print-pit --no-reboot > "$pit" 2>&1 || { cat "$pit" >&2; die "could not read the device's partition table"; }
if [[ "${1:-}" == --pit ]]; then
    grep -E 'Partition Name|Flash Filename|Partition Block Count' "$pit"
    log "the device stays in Download mode: hold Vol Down + Power to leave it"
    exit 0
fi
[[ $# -eq 0 ]] || die "usage: $0 [--pit]"

# partition_for <file>: the PIT's partition whose flash filename is <file>.
partition_for() {
    awk -v f="$1" '
        /Partition Name:/ { sub(/.*Partition Name: */, ""); name = $0 }
        /Flash Filename:/ { sub(/.*Flash Filename: */, ""); if (tolower($0) == tolower(f)) { print name; exit } }
    ' "$pit"
}

args=()
[[ -f "$BOOTIMG_DIR/boot.img" ]] || die "no $BOOTIMG_DIR/boot.img (run: make bootimg)"
for f in boot.img vendor_boot.img vbmeta.img dtbo.img; do
    [[ -f "$BOOTIMG_DIR/$f" ]] || continue
    p=$(partition_for "$f")
    [[ -n "$p" ]] || die "the device's partition table has no partition for $f (see $pit)"
    args+=("--$p" "$BOOTIMG_DIR/$f")
    echo "  $f -> partition $p"
done

echo
echo "This replaces the partitions above on the device in"
echo "Download mode with jk_os's ($JK_NAME). The bootloader must be unlocked."
read -r -p "Type 'flash' to continue: " reply
[[ "$reply" == flash ]] || die "aborted"

# Some bootloaders reboot before confirming the reboot request: Heimdall
# then fails although every image went in. Only that is not an error.
out=$(heimdall flash --resume "${args[@]}" 2>&1 | tee /dev/stderr) && status=0 || status=$?
if (( status )); then
    if grep -q 'Failed to receive reboot confirmation' <<< "$out" \
        && (( $(grep -c 'upload successful' <<< "$out") == ${#args[@]} / 2 )); then
        warn "the device didn't confirm the reboot, but every image was written"
    else
        die "flashing failed (see above)"
    fi
fi
log "flashed: the device reboots into jk_os"
