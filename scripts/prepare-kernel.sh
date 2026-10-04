#!/usr/bin/env bash
# The kernel source for the build target: kernel/<tree> as it is, or, when
# the kernel profile has patches (targets/kernels/<kernel>/patches/*.patch,
# applied in name order), a patched copy in build/<arch>/<device>-<kernel>/
# linux-src. The copy is made of hard links (quick, nearly no space); each
# file a patch touches is first replaced by a real copy, so patching never
# writes through a link into kernel/<tree>. Remade when the tree or a
# patch changes. build-kernel.sh runs this first.
source "$(dirname "$0")/common.sh"

(( ${#KERNEL_PATCHES[@]} )) || exit 0
need patch

stamp="$KERNEL_SRC/.jk_os-patched"
want="$(tree_version "$KERNEL_TREE_SRC") $(cat "${KERNEL_PATCHES[@]}" | sha256sum | cut -c1-16)"
[[ -f "$stamp" && "$(cat "$stamp")" == "$want" ]] && exit 0

log "patching $KERNEL_TREE ($(tree_version "$KERNEL_TREE_SRC")) for $JK_KERNEL: ${#KERNEL_PATCHES[@]} patch(es)"
rm -rf "$KERNEL_SRC"
mkdir -p "$(dirname "$KERNEL_SRC")"
cp -al "$KERNEL_TREE_SRC" "$KERNEL_SRC"
for p in "${KERNEL_PATCHES[@]}"; do
    # Break the links of the files this patch changes (new files have none,
    # and a file an earlier patch changed is a real copy already: copying
    # the tree's over it would undo that patch).
    sed -n 's|^+++ [^/]*/\([^[:space:]]*\).*|\1|p' "$p" | while IFS= read -r f; do
        [[ -f "$KERNEL_SRC/$f" ]] || continue
        (( $(stat -c %h "$KERNEL_SRC/$f") > 1 )) || continue
        cp --remove-destination "$KERNEL_TREE_SRC/$f" "$KERNEL_SRC/$f"
    done
    patch -d "$KERNEL_SRC" -p1 -s --no-backup-if-mismatch < "$p" \
        || die "$(basename "$p") doesn't apply to $KERNEL_TREE $(tree_version "$KERNEL_TREE_SRC")"
    log "  applied $(basename "$p")"
done
echo "$want" > "$stamp"
