#!/usr/bin/env bash
# Build U-Boot for a device whose own bootloader can't start mainline Linux
# (DEVICE_BOOT=android-uboot), out of tree in build/<arch>/<device>-<kernel>/
# u-boot. The device's bootloader starts U-Boot as if it were a kernel; U-Boot
# then starts jk_os's kernel with jk_os's device tree (build-bootimg.sh packs
# both). Nothing to do for other devices.
#
# U-Boot is Qualcomm's generic build (qcom_defconfig) with its phone settings
# (qcom-phone.config: messages on the display, a boot menu) and the device's
# own (targets/devices/<category>/<device>/uboot.config), and runs with the
# device's mainline tree (DEVICE_DTB) appended as its own device tree. Its
# environment is the device's uboot.env, with @CMDLINE@ replaced by
# DEVICE_CMDLINE.
source "$(dirname "$0")/common.sh"

[[ "$DEVICE_BOOT" == android-uboot ]] || exit 0
need make bison flex "${CROSS_COMPILE}gcc"
UBOOT_SRC="$ROOT_DIR/$UBOOT_TREE"
[[ -f "$UBOOT_SRC/Makefile" ]] || die "no U-Boot source at $UBOOT_TREE (scripts/update-source.sh u-boot <version>)"
dtb="$DTB_OUT/$DEVICE_DTB"
[[ -f "$dtb" ]] || die "device tree not built yet (run: make kernel)"
[[ -f "$DEVICE_DIR/uboot.env" ]] || die "no targets/devices/$JK_CATEGORY/$JK_DEVICE/uboot.env"

mkdir -p "$UBOOT_OUT"
u_make() { make -C "$UBOOT_SRC" O="$UBOOT_OUT" ARCH=arm CROSS_COMPILE="$CROSS_COMPILE" "$@"; }

# The environment, and a config fragment pointing U-Boot at it (an absolute
# path: U-Boot builds out of tree and the source stays untouched).
sed "s|@CMDLINE@|$DEVICE_CMDLINE|" "$DEVICE_DIR/uboot.env" > "$UBOOT_OUT/jk_os.env"
{
    echo "CONFIG_ENV_DEFAULT_ENV_TEXT_FILE=\"$UBOOT_OUT/jk_os.env\""
    [[ -f "$DEVICE_DIR/uboot.config" ]] && cat "$DEVICE_DIR/uboot.config"
} > "$UBOOT_OUT/jk_os.config"

# qcom_defconfig and qcom-phone.config come from the tree, the last fragment
# from the device. Reconfigured when that fragment changes (the stamp is a
# copy of the one last applied).
stamp="$UBOOT_OUT/.jk_os-configured"
if [[ ! -f "$UBOOT_OUT/.config" ]] || ! cmp -s "$UBOOT_OUT/jk_os.config" "$stamp"; then
    log "configuring U-Boot $(tree_version "$UBOOT_SRC") for $JK_DEVICE"
    u_make qcom_defconfig qcom-phone.config >/dev/null
    "$UBOOT_SRC/scripts/kconfig/merge_config.sh" -m -O "$UBOOT_OUT" "$UBOOT_OUT/.config" "$UBOOT_OUT/jk_os.config" >/dev/null
    u_make olddefconfig >/dev/null
    while IFS= read -r line; do
        grep -qxF "$line" "$UBOOT_OUT/.config" || warn "U-Boot option not applied: $line"
    done < <(grep -E '^CONFIG_[A-Za-z0-9_]+=' "$UBOOT_OUT/jk_os.config")
    cp "$UBOOT_OUT/jk_os.config" "$stamp"
fi

log "building U-Boot ($JK_DEVICE) with $JOBS jobs"
u_make -j"$JOBS" EXT_DTB="$dtb" u-boot.bin >/dev/null
log "U-Boot: $UBOOT_OUT/u-boot.bin ($(du -h "$UBOOT_OUT/u-boot.bin" | cut -f1)), with $DEVICE_DTB"
