#!/usr/bin/env bash
# Build targets: which device jk_os is built for. Sourced by common.sh (needs
# ROOT_DIR, die and normalize_arch from there), don't run it.
#
# A target is four choices, saved by `make config` (scripts/configure.sh) in
# build.conf, plus a free name:
#
#   JK_CATEGORY  targets/devices/<category>/            category.env
#   JK_DEVICE    targets/devices/<category>/<device>/   device.env
#   JK_KERNEL    targets/kernels/<kernel>/              kernel.env
#   JK_ARCH      x86_64 | aarch64
#   JK_NAME      your name for this build (output file name), e.g. asus-laptop
#
# device.env fields:
#   DEVICE_DESC, DEVICE_STATUS   shown by make config
#   DEVICE_ARCHS, DEVICE_KERNELS what the device can be built with
#   DEVICE_BOOT                  boot format (BOOT_FORMATS below)
#   DEVICE_DTB                   its device tree, as built from the kernel
#                                profile's dts/ (e.g. qcom/<board>.dtb)
#   DEVICE_CMDLINE               kernel command line in the boot image
#   DEVICE_INITRAMFS_FIRMWARE    no: leave the network firmware out of the
#                                initramfs (default: in it)
#   DEVICE_INITRAMFS_TOOLS       more disk tools in the initramfs (from
#                                build/<arch>/tools/sbin), e.g. mke2fs;
#                                added to the category's
#                                CATEGORY_INITRAMFS_TOOLS (category.env)
#   DEVICE_KERNEL_IMAGE          android-bootimg: Image (uncompressed) or
#                                Image.gz (default)
#   DEVICE_ABL_DTB               android-uboot: the device tree the device's
#                                bootloader is given (it starts U-Boot with
#                                it), a path (glob) under the repo
#   DEVICE_STOCK                 the stock firmware as scripts/inspect-stock.sh
#                                read it, a directory (glob) under the repo
#   DEVICE_STOCK_FIRMWARE        files from the stock vendor partition's
#                                /firmware (DEVICE_STOCK/vendor/firmware) for
#                                the image's /lib/firmware, as <name> (globs
#                                allowed) or <name>:<path there> (not
#                                redistributed: each build takes them from the
#                                stock firmware)
#   DEVICE_STOCK_FIRMWARE_EARLY  the same, in the initramfs too: for drivers
#                                that start before the system is mounted
#   DEVICE_BOOTIMG_SEANDROID     yes: end boot.img with Samsung's
#                                "SEANDROIDENFORCE" marker
#   DEVICE_AVB_FOOTERS           android-bootimg: images that need a signed
#                                AVB footer, as <partition>:<partition size>
#   DEVICE_MKBOOTIMG_ARGS        android-bootimg: header version, page size,
#                                base, offsets, os version and patch level,
#                                as scripts/inspect-stock.sh prints them
#
# Each profile directory may also hold:
#   dts/            (kernel) device trees that aren't in the kernel tree,
#                   built against it by build-dtbs.sh
#   patches/        (kernel) patches for the kernel tree, applied to a copy
#                   of it (prepare-kernel.sh)
#   kernel.config   a kernel config fragment, merged after configs/kernel/
#                   common.config and <arch>.config, in the order kernel,
#                   category, device
#   rootfs/         files copied over the root filesystem (category, device)
#   initramfs/      files copied into the initramfs (category, device); its
#                   etc/jk/early/* are sourced by /init before it looks for
#                   the system
#
# Device names are unique across categories: build/<arch>/<device>-<kernel>
# holds what is built for one device and kernel.

TARGETS_DIR="$ROOT_DIR/targets"
BUILD_CONF="${BUILD_CONF:-$ROOT_DIR/build.conf}"
[[ "$BUILD_CONF" == /* ]] || BUILD_CONF="$PWD/$BUILD_CONF"

# Boot formats the image steps know how to make (DEVICE_BOOT):
#   efi-iso          hybrid ISO: GRUB (x86_64) or the EFI stub (aarch64)
#   android-bootimg  Android boot.img + vbmeta.img + dtbo.img, and an Odin tar
#                    for Samsung devices (build-bootimg.sh)
#   android-uboot    the same, but the bootloader starts U-Boot (build-uboot.sh),
#                    which starts jk_os's kernel from a FIT image: for
#                    bootloaders that refuse a mainline device tree
BOOT_FORMATS="efi-iso android-bootimg android-uboot"

in_list() { [[ " $2 " == *" $1 "* ]]; }   # in_list <word> <space-separated list>

target_categories() {
    local d
    for d in "$TARGETS_DIR"/devices/*/category.env; do
        [[ -f "$d" ]] && basename "$(dirname "$d")"
    done
}
target_devices() {   # target_devices <category>
    local d
    for d in "$TARGETS_DIR/devices/$1"/*/device.env; do
        [[ -f "$d" ]] && basename "$(dirname "$d")"
    done
}

# load_category / load_device / load_kernel: read a profile into CATEGORY_*,
# DEVICE_* or KERNEL_*, clearing what a previous one set. Fail if missing.
load_category() {
    CATEGORY_DESC= CATEGORY_INITRAMFS_TOOLS=
    CATEGORY_DIR="$TARGETS_DIR/devices/$1"
    [[ -f "$CATEGORY_DIR/category.env" ]] || return 1
    # shellcheck source=/dev/null
    source "$CATEGORY_DIR/category.env"
}
load_device() {   # load_device <category> <device>
    DEVICE_DESC= DEVICE_ARCHS= DEVICE_KERNELS= DEVICE_BOOT= DEVICE_STATUS=
    DEVICE_DTB= DEVICE_CMDLINE= DEVICE_MKBOOTIMG_ARGS= DEVICE_INITRAMFS_FIRMWARE=
    DEVICE_AVB_FOOTERS= DEVICE_BOOTIMG_SEANDROID= DEVICE_KERNEL_IMAGE=
    DEVICE_ABL_DTB= DEVICE_INITRAMFS_TOOLS= DEVICE_STOCK= DEVICE_STOCK_FIRMWARE= DEVICE_STOCK_FIRMWARE_EARLY=
    DEVICE_DIR="$TARGETS_DIR/devices/$1/$2"
    [[ -f "$DEVICE_DIR/device.env" ]] || return 1
    # shellcheck source=/dev/null
    source "$DEVICE_DIR/device.env"
}
load_kernel() {
    KERNEL_DESC= KERNEL_TREE= KERNEL_ARCHS=
    KERNEL_DIR="$TARGETS_DIR/kernels/$1"
    [[ -f "$KERNEL_DIR/kernel.env" ]] || return 1
    # shellcheck source=/dev/null
    source "$KERNEL_DIR/kernel.env"
}

# target_archs: the archs both the loaded device and kernel support.
target_archs() {
    local a out=()
    for a in $DEVICE_ARCHS; do
        in_list "$a" "$KERNEL_ARCHS" && out+=("$a")
    done
    echo "${out[*]}"
}

valid_name() { [[ "$1" =~ ^[a-z0-9][a-z0-9._-]*$ ]]; }

# check_target: load the profiles JK_* name and check they fit together.
# Prints what is wrong and fails.
check_target() {
    load_category "$JK_CATEGORY" || { echo "unknown device category '$JK_CATEGORY'"; return 1; }
    load_device "$JK_CATEGORY" "$JK_DEVICE" || { echo "no device '$JK_DEVICE' in category '$JK_CATEGORY'"; return 1; }
    load_kernel "$JK_KERNEL" || { echo "unknown kernel '$JK_KERNEL'"; return 1; }
    in_list "$JK_KERNEL" "$DEVICE_KERNELS" \
        || { echo "device '$JK_DEVICE' doesn't use kernel '$JK_KERNEL' (it uses: $DEVICE_KERNELS)"; return 1; }
    in_list "$JK_ARCH" "$(target_archs)" \
        || { echo "'$JK_DEVICE' with kernel '$JK_KERNEL' can't be built for $JK_ARCH (only: $(target_archs))"; return 1; }
    valid_name "$JK_NAME" \
        || { echo "name '$JK_NAME': use lowercase letters, digits, '.', '_' and '-'"; return 1; }
}

# load_target: set JK_* from build.conf, or without one the generic PC for
# $ARCH (default: the host arch), as `make ARCH=...` always built. An ARCH
# given next to a build.conf must agree with it.
load_target() {
    local want_arch="${ARCH:-}" err
    JK_CATEGORY= JK_DEVICE= JK_KERNEL= JK_ARCH= JK_NAME=
    if [[ -f "$BUILD_CONF" ]]; then
        # shellcheck source=/dev/null
        source "$BUILD_CONF"
        if [[ -n "$want_arch" && "$(normalize_arch "$want_arch")" != "$JK_ARCH" ]]; then
            die "ARCH=$want_arch, but $BUILD_CONF builds for $JK_ARCH (change the target with: make config)"
        fi
    else
        JK_CATEGORY=pc JK_DEVICE=generic JK_KERNEL=mainline JK_NAME=generic
        JK_ARCH="$(normalize_arch "${want_arch:-$(uname -m)}")"
    fi
    err=$(check_target) || die "$BUILD_CONF: $err (run: make config)"
    check_target   # again, in this shell, for the profile variables
}

# print_target: one line per choice, for `make showconfig` and configure.sh.
print_target() {
    printf '  category  %s\n  device    %s\n  kernel    %s (%s)\n  arch      %s\n  name      %s\n' \
        "$JK_CATEGORY" "$JK_DEVICE" "$JK_KERNEL" "$KERNEL_TREE" "$JK_ARCH" "$JK_NAME"
}
