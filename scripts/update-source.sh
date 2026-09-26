#!/usr/bin/env bash
# Maintainer tool: replace a vendored source tree with another release.
# Builds never run this; they only use the trees already in the repo.
#
#   scripts/update-source.sh kernel  7.3                     # into kernel/mainline
#   scripts/update-source.sh kernel  7.3 kernel/<name>       # into another tree
#   scripts/update-source.sh busybox 1.38.0
#   scripts/update-source.sh util-linux 2.42.4
#   scripts/update-source.sh e2fsprogs 1.47.4
#
# Instead of a version you can pass a local tarball (works offline):
#   scripts/update-source.sh kernel ~/Downloads/linux-7.3.tar.xz
#
# Downloads come from cdn.kernel.org (Torvalds' mainline releases, util-linux,
# e2fsprogs) and busybox.net, and are checked against the published SHA-256 sums.
source "$(dirname "$0")/common.sh"
need tar sha256sum

what="${1:-}" ver="${2:-}"
[[ -n "$what" && -n "$ver" ]] || die "usage: $0 kernel|busybox|util-linux|e2fsprogs <version|tarball> [tree]"

case "$what" in
    kernel)     tree="${3:-kernel/mainline}" ;;
    busybox)    tree="${3:-$BUSYBOX_TREE}" ;;
    util-linux) tree="${3:-$UTIL_LINUX_TREE}" ;;
    e2fsprogs)  tree="${3:-$E2FSPROGS_TREE}" ;;
    *) die "unknown source '$what' (kernel, busybox, util-linux or e2fsprogs)" ;;
esac

tmp="$(mktemp -d "$ROOT_DIR/.update-source.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

# fetch <url> <dest>
fetch() {
    need curl
    log "downloading $1"
    curl -fL --progress-bar --retry 5 --retry-all-errors -o "$2" "$1"
}

if [[ -f "$ver" ]]; then
    tarball="$ver"
else
    case "$what" in
        kernel)
            [[ "$ver" != *-rc* ]] || die "release candidates have no tarball on cdn.kernel.org; use a release"
            base="https://cdn.kernel.org/pub/linux/kernel/v${ver%%.*}.x"
            name="linux-$ver.tar.xz"
            fetch "$base/sha256sums.asc" "$tmp/sums"
            ;;
        busybox)
            base="https://busybox.net/downloads"
            name="busybox-$ver.tar.bz2"
            fetch "$base/$name.sha256" "$tmp/sums"
            ;;
        util-linux)
            major="$(cut -d. -f1-2 <<< "$ver")"
            base="https://cdn.kernel.org/pub/linux/utils/util-linux/v$major"
            name="util-linux-$ver.tar.xz"
            fetch "$base/sha256sums.asc" "$tmp/sums"
            ;;
        e2fsprogs)
            base="https://cdn.kernel.org/pub/linux/kernel/people/tytso/e2fsprogs/v$ver"
            name="e2fsprogs-$ver.tar.xz"
            fetch "$base/sha256sums.asc" "$tmp/sums"
            ;;
    esac
    fetch "$base/$name" "$tmp/$name"
    grep -E "[[:space:]]$name\$" "$tmp/sums" | head -n1 > "$tmp/check"
    [[ -s "$tmp/check" ]] || die "no published checksum for $name"
    (cd "$tmp" && sha256sum --quiet -c check) || die "checksum mismatch for $name"
    tarball="$tmp/$name"
fi

log "unpacking $(basename "$tarball")"
mkdir "$tmp/x"
tar -xf "$tarball" -C "$tmp/x"
top=("$tmp/x"/*)
[[ ${#top[@]} -eq 1 && ( -f "${top[0]}/Makefile" || -f "${top[0]}/configure" ) ]] || die "unexpected tarball layout: $tarball"

# Swap the new tree in; the old one is only deleted once the new one is there.
dest="$ROOT_DIR/$tree"
mkdir -p "$(dirname "$dest")"
[[ -e "$dest" ]] && mv "$dest" "$tmp/old"
mv "${top[0]}" "$dest"

log "$tree is now $what $(tree_version "$dest")"
echo "Rebuild with: make clean && make"
echo "Commit with:  git add -f $tree   (-f: upstream ships a few files its own .gitignore matches)"
