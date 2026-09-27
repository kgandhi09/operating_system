#!/usr/bin/env bash
# Build a static BusyBox for $ARCH (out of tree, in build/<arch>/busybox).
source "$(dirname "$0")/common.sh"
need make "${CROSS_COMPILE}gcc"

[[ -f "$BUSYBOX_SRC/Makefile" ]] || die "no BusyBox source at $BUSYBOX_TREE (see BUSYBOX_TREE in versions.env)"

frags=("$ROOT_DIR/configs/busybox/common.config")
[[ -f "$ROOT_DIR/configs/busybox/$ARCH.config" ]] && frags+=("$ROOT_DIR/configs/busybox/$ARCH.config")
mkdir -p "$BUSYBOX_OUT"
# NOWARN_CFLAGS (set in the Makefile) for BusyBox itself and for the helper
# programs it builds for the host (kconfig, usage text).
bb_make() {
    make -C "$BUSYBOX_SRC" O="$BUSYBOX_OUT" ARCH="$KARCH" CROSS_COMPILE="$CROSS_COMPILE" \
        EXTRA_CFLAGS="${NOWARN_CFLAGS:-}" \
        HOSTCFLAGS="-Wall -Wstrict-prototypes -O2 -fomit-frame-pointer ${NOWARN_CFLAGS:-}" "$@"
}

# The stamp is written only after a complete configure, so an interrupted
# one is redone rather than leaving a half-merged .config behind.
stamp="$BUSYBOX_OUT/.jk_os-configured"
reconfigure=0
[[ -f "$stamp" && -f "$BUSYBOX_OUT/.config" ]] || reconfigure=1
for f in "${frags[@]}"; do
    [[ "$f" -nt "$stamp" ]] && reconfigure=1
done

if (( reconfigure )); then
    log "configuring busybox $(tree_version "$BUSYBOX_SRC") from $BUSYBOX_TREE ($ARCH)"
    bb_make defconfig >/dev/null
    # BusyBox's kconfig has no merge_config.sh: drop each option the fragment
    # mentions from .config, append the fragment, then let oldconfig settle it.
    opts="$(grep -hoE '^(# )?CONFIG_[A-Za-z0-9_]+' "${frags[@]}" | sed 's/^# //')"
    for o in $opts; do
        sed -i -E "/^(# )?$o[= ]/d" "$BUSYBOX_OUT/.config"
    done
    grep -hE '^(CONFIG_|# CONFIG_)' "${frags[@]}" >> "$BUSYBOX_OUT/.config"
    { yes "" || true; } | bb_make oldconfig >/dev/null   # yes dies of SIGPIPE
    touch "$stamp"
fi

log "building busybox ($ARCH)"
bb_make -j"$JOBS"
if command -v file >/dev/null; then
    file "$BUSYBOX_OUT/busybox" | grep -q 'statically linked' \
        || warn "busybox does not look statically linked"
fi
