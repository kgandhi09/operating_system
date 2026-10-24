#!/usr/bin/env bash
# Boot the jk_os ISO in QEMU with the console on this terminal.
#   UEFI=1   (x86_64) boot through OVMF instead of legacy BIOS
#   MEM=2G   guest memory (default 1G)
#   AARCH64_EFI=/path/QEMU_EFI.fd   (aarch64) another firmware than the one built here
#   GUI=1    also open a window with a virtual GPU, keyboard and mouse, for the
#            desktop (log in on a console there and run jk-gui; MEM=4G or more)
#   DISPLAY_OPT=vnc=:1   how QEMU shows that screen (default: VNC on :1, port
#                        5901: QEMU is built without a window toolkit)
#   DISK=path.qcow2      a hard disk to install on (default build/<arch>/disk.qcow2,
#                        created empty at DISK_SIZE, 32G, on first use); DISK=none: no disk
#   BOOT=disk            boot the installed system without attaching the ISO
# Quit QEMU with Ctrl-A then X.
source "$(dirname "$0")/common.sh"

[[ -f "$ISO" ]] || die "no ISO at $ISO (run: $MAKE_CMD iso)"
MEM="${MEM:-1G}"


# The console stays on this terminal (the serial line); with GUI=1 a window
# shows the virtual screen (tty1, tty2 and the desktop) too.
if [[ "${GUI:-0}" == 1 ]]; then
    log "consoles in the window: Alt+F1 / Alt+F2 on a text console, Ctrl+Alt+F1 / F2 in the desktop"
    log "if the host keeps those keys: Ctrl-A C here, then 'sendkey ctrl-alt-f2' (Ctrl-A C again to come back)"
    screen=(-device virtio-vga -device qemu-xhci -device usb-kbd -device usb-tablet
            -display "${DISPLAY_OPT:-vnc=:1}" -serial mon:stdio)
    [[ "$ARCH" == aarch64 ]] && screen[1]=virtio-gpu-pci
else
    screen=(-nographic)
fi

# The disk jk-install installs on. As on a PC, the firmware tries the disk
# first and falls back to the installer medium while the disk is empty: after
# installing, the reboot starts the installed system (the initramfs still
# offers the installer for a few seconds while the ISO stays attached).
# BOOT=disk leaves the ISO out. The installed system boots through UEFI, so
# with a disk x86_64 boots through UEFI too.
disk=() iso=()
DISK="${DISK:-$TARGET_OUT/disk.qcow2}"
if [[ "$DISK" != none ]]; then
    if [[ ! -f "$DISK" ]]; then
        need qemu-img
        qemu-img create -q -f qcow2 "$DISK" "${DISK_SIZE:-32G}"
        log "created an empty install disk: $DISK (${DISK_SIZE:-32G})"
    fi
    disk=(-drive "if=none,id=disk,format=qcow2,file=$DISK"
          -device "virtio-blk-pci,drive=disk,bootindex=0")
    UEFI=1
fi
if [[ "${BOOT:-iso}" != disk ]]; then
    if [[ "$ARCH" == x86_64 ]]; then
        if [[ "${UEFI:-0}" == 1 ]]; then
            iso=(-drive "if=none,id=iso,media=cdrom,readonly=on,file=$ISO"
                 -device "ide-cd,drive=iso,bootindex=1")
        else
            iso=(-cdrom "$ISO" -boot d)
        fi
    else
        # The ISO goes in as a (USB-stick-like) disk, not a CD: its EFI system
        # partition holds the whole kernel, too big for an El Torito boot image
        # (32 MiB at most), so UEFI only finds it through the GPT.
        iso=(-drive "if=none,id=iso,format=raw,readonly=on,file=$ISO"
             -device "virtio-blk-pci,drive=iso,bootindex=1")
    fi
elif [[ "$DISK" == none ]]; then
    die "BOOT=disk needs a DISK"
fi

# The UEFI firmware for the guest: EDK2's (OVMF for x86_64, ArmVirtQemu for
# aarch64), built here once from userspace/hosttools/edk2 with jk_os's cross
# compiler for the preset's arch, into build/<arch>/edk2.
EDK2_SRC="$ROOT_DIR/userspace/hosttools/edk2"
EDK2="$OUT_DIR/edk2"
edk2_firmware() {
    local want
    want="$(tree_version "$EDK2_SRC") $("${CROSS_COMPILE}gcc" -dumpversion 2>/dev/null)"
    [[ "$(cat "$EDK2/.done" 2>/dev/null)" == "$want" ]] && return
    [[ -d "$EDK2_SRC" ]] || die "no userspace/hosttools/edk2 (run: scripts/update-desktop-sources.sh --component hosttools edk2)"
    [[ -x "${CROSS_COMPILE}gcc" ]] || die "no cross compiler for $ARCH (run: $MAKE_CMD cross)"
    need nasm iasl python3
    log "building the UEFI firmware for $ARCH (EDK2 $(tree_version "$EDK2_SRC")), once — see $EDK2.log"
    rm -rf "$EDK2"; mkdir -p "$EDK2"
    cp -a "$EDK2_SRC/." "$EDK2/src/"
    (
        set +eu   # edksetup.sh isn't written for either
        cd "$EDK2/src" || exit 1
        export WORKSPACE="$EDK2/src" PYTHON_COMMAND=python3
        # BaseTools' programs run here: the host tools' libuuid.
        export C_INCLUDE_PATH="$HOSTDIR/include" LIBRARY_PATH="$HOSTDIR/lib" LD_LIBRARY_PATH="$HOSTDIR/lib"
        make -C BaseTools -j"$JOBS" CC=gcc CXX=g++ || exit 1
        . ./edksetup.sh BaseTools || exit 1
        case "$ARCH" in
            x86_64)
                export GCC_BIN="$CROSS_COMPILE"   # (tools_def: GCC_X64_PREFIX)
                build -a X64 -t GCC -b RELEASE -n "$JOBS" -p OvmfPkg/OvmfPkgX64.dsc \
                    -D NETWORK_HTTP_BOOT_ENABLE=FALSE -D NETWORK_TLS_ENABLE=FALSE || exit 1
                cp Build/OvmfX64/RELEASE_GCC/FV/OVMF_CODE.fd Build/OvmfX64/RELEASE_GCC/FV/OVMF_VARS.fd "$EDK2/" ;;
            aarch64)
                export GCC_AARCH64_PREFIX="$CROSS_COMPILE"
                build -a AARCH64 -t GCC -b RELEASE -n "$JOBS" -p ArmVirtPkg/ArmVirtQemu.dsc \
                    -D NETWORK_HTTP_BOOT_ENABLE=FALSE -D NETWORK_TLS_ENABLE=FALSE || exit 1
                cp Build/ArmVirtQemu-AARCH64/RELEASE_GCC/FV/QEMU_EFI.fd "$EDK2/" ;;
        esac
    ) > "$EDK2.log" 2>&1 || { tail -n 30 "$EDK2.log" >&2; die "the UEFI firmware failed, see $EDK2.log"; }
    rm -rf "$EDK2/src"
    echo "$want" > "$EDK2/.done"
}

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
        edk2_firmware
        code="$EDK2/OVMF_CODE.fd" vars_src="$EDK2/OVMF_VARS.fd"
        vars="$TARGET_OUT/ovmf-vars.fd"
        [[ -f "$vars" ]] || cp "$vars_src" "$vars"
        fw=(-machine q35
            -drive "if=pflash,format=raw,readonly=on,file=$code"
            -drive "if=pflash,format=raw,file=$vars")
    fi
    exec qemu-system-x86_64 "${accel[@]}" "${fw[@]}" -m "$MEM" -smp 2 \
        "${iso[@]}" "${disk[@]}" \
        -nic user,model=virtio-net-pci \
        "${screen[@]}"
    ;;
aarch64)
    need qemu-system-aarch64
    fw=()
    if [[ -n "${AARCH64_EFI:-}" ]]; then
        [[ -f "$AARCH64_EFI" ]] || die "no $AARCH64_EFI"
        fw=(-bios "$AARCH64_EFI")
    else
        edk2_firmware
        fw=(-bios "$EDK2/QEMU_EFI.fd")
    fi
    exec qemu-system-aarch64 -machine virt,gic-version=max "${accel[@]}" "${fw[@]}" -m "$MEM" -smp 2 \
        "${iso[@]}" "${disk[@]}" \
        -nic user,model=virtio-net-pci \
        "${screen[@]}"
    ;;
esac
