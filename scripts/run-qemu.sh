#!/usr/bin/env bash
# Boot the jk_os ISO in QEMU with the console on this terminal.
#   UEFI=1   (x86_64) boot through OVMF instead of legacy BIOS
#   MEM=2G   guest memory (default 1G)
#   AARCH64_EFI=/path/QEMU_EFI.fd   (aarch64) firmware, if not in a standard place
#   GUI=1    also open a window with a virtual GPU, keyboard and mouse, for the
#            desktop (log in on tty2 there and run jk-gui; MEM=4G or more)
#   DISPLAY_OPT=vnc=:1   how QEMU shows that screen (default: gtk)
# Quit QEMU with Ctrl-A then X.
source "$(dirname "$0")/common.sh"

[[ -f "$ISO" ]] || die "no ISO at $ISO (run: make iso)"
MEM="${MEM:-1G}"

# first_file <candidates...>: print the first path that exists.
first_file() {
    local f
    for f in "$@"; do [[ -f "$f" ]] && { echo "$f"; return 0; }; done
    return 1
}

# The console stays on this terminal (the serial line); with GUI=1 a window
# shows the virtual screen (tty1, tty2 and the desktop) too.
if [[ "${GUI:-0}" == 1 ]]; then
    screen=(-device virtio-vga -device qemu-xhci -device usb-kbd -device usb-tablet
            -display "${DISPLAY_OPT:-gtk}" -serial mon:stdio)
    [[ "$ARCH" == aarch64 ]] && screen[1]=virtio-gpu-pci
else
    screen=(-nographic)
fi

accel=(-cpu max)
# Emulated arm64 (TCG): with -cpu max the kernel doesn't get past the
# firmware in a reasonable time; a plain Cortex-A72 boots in seconds.
[[ "$ARCH" == aarch64 ]] && accel=(-cpu cortex-a72)
if [[ "$ARCH" == "$HOST_ARCH" && -w /dev/kvm ]]; then
    accel=(-enable-kvm -cpu host)
fi

case "$ARCH" in
x86_64)
    need qemu-system-x86_64
    fw=()
    if [[ "${UEFI:-0}" == 1 ]]; then
        code="$(first_file /usr/share/OVMF/OVMF_CODE_4M.fd /usr/share/OVMF/OVMF_CODE.fd \
                /usr/share/edk2/ovmf/OVMF_CODE.fd /usr/share/edk2/x64/OVMF_CODE.4m.fd \
                /usr/share/qemu/edk2-x86_64-code.fd)" || die "OVMF firmware not found (install ovmf)"
        vars_src="$(first_file "${code/CODE/VARS}" /usr/share/OVMF/OVMF_VARS_4M.fd /usr/share/OVMF/OVMF_VARS.fd)" \
            || die "OVMF variable store not found"
        vars="$OUT_DIR/ovmf-vars.fd"
        [[ -f "$vars" ]] || cp "$vars_src" "$vars"
        fw=(-machine q35
            -drive "if=pflash,format=raw,readonly=on,file=$code"
            -drive "if=pflash,format=raw,file=$vars")
    fi
    exec qemu-system-x86_64 "${accel[@]}" "${fw[@]}" -m "$MEM" -smp 2 \
        -cdrom "$ISO" -boot d \
        -nic user,model=virtio-net-pci \
        "${screen[@]}"
    ;;
aarch64)
    need qemu-system-aarch64
    fw=()
    if bios="$(first_file "${AARCH64_EFI:-}" /usr/share/qemu-efi-aarch64/QEMU_EFI.fd /usr/share/edk2/aarch64/QEMU_EFI.fd)"; then
        fw=(-bios "$bios")
    elif code="$(first_file /usr/share/AAVMF/AAVMF_CODE.fd /usr/share/qemu/edk2-aarch64-code.fd \
                 /usr/share/edk2/aarch64/QEMU_EFI-pflash.raw)"; then
        fw=(-drive "if=pflash,format=raw,readonly=on,file=$code")
    else
        die "aarch64 UEFI firmware not found (install qemu-efi-aarch64)"
    fi
    # The ISO goes in as a (USB-stick-like) disk, not a CD: its EFI system
    # partition holds the whole kernel, too big for an El Torito boot image
    # (32 MiB at most), so UEFI only finds it through the GPT.
    exec qemu-system-aarch64 -machine virt,gic-version=max "${accel[@]}" "${fw[@]}" -m "$MEM" -smp 2 \
        -drive "if=virtio,format=raw,readonly=on,file=$ISO" \
        -nic user,model=virtio-net-pci \
        "${screen[@]}"
    ;;
esac
