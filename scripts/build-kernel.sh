#!/usr/bin/env bash
# Configure and build the Linux kernel for $ARCH with the rootfs built in.
source "$(dirname "$0")/common.sh"
need make flex bison bc "${CROSS_COMPILE}gcc"

[[ -f "$KERNEL_SRC/Makefile" ]] || die "no kernel source at $KERNEL_TREE (see KERNEL_TREE_$ARCH in versions.env)"

[[ -d "$ROOTFS_DIR" ]] || die "rootfs not assembled yet (run: make rootfs)"

frags=("$ROOT_DIR/configs/kernel/common.config" "$ROOT_DIR/configs/kernel/$ARCH.config")
mkdir -p "$KERNEL_OUT"
k_make() { make -C "$KERNEL_SRC" O="$KERNEL_OUT" ARCH="$KARCH" CROSS_COMPILE="$CROSS_COMPILE" "$@"; }

# The stamp is written only after a complete configure, so an interrupted
# one is redone rather than leaving a half-merged .config behind.
stamp="$KERNEL_OUT/.jk_os-configured"
reconfigure=0
[[ -f "$stamp" && -f "$KERNEL_OUT/.config" ]] || reconfigure=1
for f in "${frags[@]}"; do
    [[ "$f" -nt "$stamp" ]] && reconfigure=1
done

if (( reconfigure )); then
    log "configuring linux $(tree_version "$KERNEL_SRC") from $KERNEL_TREE ($ARCH)"
    k_make defconfig >/dev/null
    (cd "$KERNEL_OUT" && "$KERNEL_SRC/scripts/kconfig/merge_config.sh" -m -O "$KERNEL_OUT" .config "${frags[@]}" >/dev/null)
    touch "$stamp"
fi

# Paths depend on where the repo is checked out, so set them on every build.
# ROOT_UID/GID map the builder's files to root:root inside the initramfs.
"$KERNEL_SRC/scripts/config" --file "$KERNEL_OUT/.config" \
    --set-str INITRAMFS_SOURCE "$ROOTFS_DIR $ROOT_DIR/configs/initramfs.list" \
    --set-val INITRAMFS_ROOT_UID "$(id -u)" \
    --set-val INITRAMFS_ROOT_GID "$(id -g)"
k_make olddefconfig >/dev/null

# merge_config.sh only warns when an option doesn't stick; fail loudly instead.
missing=0
while IFS= read -r line; do
    grep -qxF "$line" "$KERNEL_OUT/.config" || { warn "kernel option not applied: $line"; missing=1; }
done < <(grep -hE '^CONFIG_[A-Za-z0-9_]+=' "${frags[@]}")
(( missing == 0 )) || warn "some requested kernel options were dropped (unmet dependencies?)"

log "building linux ($ARCH) with $JOBS jobs — this takes a while"
k_make -j"$JOBS" "$(basename "$KIMAGE")"

log "kernel: $KERNEL_OUT/$KIMAGE ($(du -h "$KERNEL_OUT/$KIMAGE" | cut -f1))"
