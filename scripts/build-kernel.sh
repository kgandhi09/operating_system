#!/usr/bin/env bash
# Configure and build the Linux kernel for the build target (device, kernel
# tree, arch: see make config) with the small early-boot initramfs
# (build-initramfs.sh) built in.
source "$(dirname "$0")/common.sh"
need make flex bison bc "${CROSS_COMPILE}gcc"

[[ -f "$KERNEL_SRC/Makefile" ]] || die "no kernel source at $KERNEL_TREE (see targets/kernels/$JK_KERNEL/kernel.env)"

[[ -x "$INITRAMFS_DIR/init" ]] || die "initramfs not assembled yet (run: make initramfs)"

frags=("${KERNEL_FRAGMENTS[@]}")
mkdir -p "$KERNEL_OUT"
k_make() { make -C "$KERNEL_SRC" O="$KERNEL_OUT" ARCH="$KARCH" CROSS_COMPILE="$CROSS_COMPILE" "$@"; }

# The stamp is written only after a complete configure, so an interrupted
# one is redone rather than leaving a half-merged .config behind.
stamp="$KERNEL_OUT/.jk_os-configured"
reconfigure=0
[[ -f "$stamp" && -f "$KERNEL_OUT/.config" ]] || reconfigure=1
# Also when the list of fragments changes (a profile gained or lost one).
[[ "$(cat "$stamp" 2>/dev/null)" == "${frags[*]}" ]] || reconfigure=1
for f in "${frags[@]}"; do
    [[ "$f" -nt "$stamp" ]] && reconfigure=1
done

if (( reconfigure )); then
    log "configuring linux $(tree_version "$KERNEL_SRC") from $KERNEL_TREE ($JK_DEVICE, $ARCH)"
    k_make defconfig >/dev/null
    (cd "$KERNEL_OUT" && "$KERNEL_SRC/scripts/kconfig/merge_config.sh" -m -O "$KERNEL_OUT" .config "${frags[@]}" >/dev/null)
    # Everything is built in, except what the fragments ask for as a module
    # (=m: nouveau) and what can only follow those: each other "m" defconfig
    # chose becomes "y", until nothing changes.
    asked_m=$(grep -hE '^CONFIG_[A-Za-z0-9_]+=m$' "${frags[@]}" | cut -d= -f1 || true)
    for _ in 1 2 3 4 5; do
        grep -E '^CONFIG_[A-Za-z0-9_]+=m$' "$KERNEL_OUT/.config" | cut -d= -f1 | sort > "$KERNEL_OUT/.m.before"
        while read -r opt; do
            grep -qxF "$opt" <<< "$asked_m" || "$KERNEL_SRC/scripts/config" --file "$KERNEL_OUT/.config" --enable "${opt#CONFIG_}"
        done < "$KERNEL_OUT/.m.before"
        k_make olddefconfig >/dev/null
        grep -E '^CONFIG_[A-Za-z0-9_]+=m$' "$KERNEL_OUT/.config" | cut -d= -f1 | sort > "$KERNEL_OUT/.m.after"
        cmp -s "$KERNEL_OUT/.m.before" "$KERNEL_OUT/.m.after" && break
    done
    rm -f "$KERNEL_OUT/.m.before" "$KERNEL_OUT/.m.after"
    echo "${frags[*]}" > "$stamp"
fi

# Paths depend on where the repo is checked out, so set them on every build.
# ROOT_UID/GID map the builder's files to root:root inside the initramfs.
"$KERNEL_SRC/scripts/config" --file "$KERNEL_OUT/.config" \
    --set-str INITRAMFS_SOURCE "$INITRAMFS_DIR $ROOT_DIR/configs/initramfs.list" \
    --set-val INITRAMFS_ROOT_UID "$(id -u)" \
    --set-val INITRAMFS_ROOT_GID "$(id -g)"
k_make olddefconfig >/dev/null

# merge_config.sh only warns when an option doesn't stick; fail loudly instead.
# A later fragment overrides an earlier one (a device turning an option off).
declare -A wanted=()
while IFS= read -r line; do
    if [[ "$line" =~ ^#\ (CONFIG_[A-Za-z0-9_]+)\ is\ not\ set$ ]]; then unset "wanted[${BASH_REMATCH[1]}]"
    else wanted[${line%%=*}]=$line; fi
done < <(cat "${frags[@]}" | grep -E '^CONFIG_[A-Za-z0-9_]+=|^# CONFIG_[A-Za-z0-9_]+ is not set$')
missing=0
for line in "${wanted[@]}"; do
    grep -qxF "$line" "$KERNEL_OUT/.config" || { warn "kernel option not applied: $line"; missing=1; }
done
(( missing == 0 )) || warn "some requested kernel options were dropped (unmet dependencies?)"

log "building linux ($ARCH) with $JOBS jobs — this takes a while"
k_make -j"$JOBS" "$(basename "$KIMAGE")"

# Loadable modules (nouveau; NVIDIA's are built by build-nvidia.sh), installed
# into build/<arch>/modules for the OS image.
rm -rf "$MODULES_OUT"
if grep -q '^CONFIG_MODULES=y' "$KERNEL_OUT/.config"; then
    k_make -j"$JOBS" modules
    k_make INSTALL_MOD_PATH="$MODULES_OUT" INSTALL_MOD_STRIP=1 modules_install >/dev/null
    rm -f "$MODULES_OUT"/lib/modules/*/build "$MODULES_OUT"/lib/modules/*/source
fi

log "kernel: $KERNEL_OUT/$KIMAGE ($(du -h "$KERNEL_OUT/$KIMAGE" | cut -f1))"

# The device's tree, for a bootloader that doesn't bring its own (DEVICE_DTB).
[[ -z "$DEVICE_DTB" ]] || "$ROOT_DIR/scripts/build-dtbs.sh"
