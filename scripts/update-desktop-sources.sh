#!/usr/bin/env bash
# Maintainer tool: fetch the desktop stack's sources, listed in
# configs/desktop/sources.list, into userland/desktop/<name>. Builds never run
# this; they only use the trees in the repo.
#
#   scripts/update-desktop-sources.sh            fetch what is missing or changed
#   scripts/update-desktop-sources.sh <name>...  only these
#
# List lines: <name> <version> <url> <pin>
#   <url>  with {v} for the version; "git+<repo url>#<tag>" fetches that tag
#   <pin>  the tarball's SHA-256, or the tag's commit for git sources; "-"
#          means not pinned yet: the first fetch records it in the list
#          (check it against the project's published checksum or signature)
source "$(dirname "$0")/common.sh"
need curl tar sha256sum git

LIST="$ROOT_DIR/configs/desktop/sources.list"
DEST_ROOT="$ROOT_DIR/userland/desktop"
mkdir -p "$DEST_ROOT"
only=("$@")

tmp="$(mktemp -d "$ROOT_DIR/.update-desktop.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

# pin_entry <name> <pin>: write the pin into the list.
pin_entry() {
    awk -v n="$1" -v p="$2" '$1 == n && $4 == "-" { $4 = p } { print }' OFS='  ' "$LIST" > "$tmp/list"
    cat "$tmp/list" > "$LIST"
}

fetch_one() {
    local name="$1" ver="$2" url="${3//\{v\}/$2}" pin="$4" dest="$DEST_ROOT/$1" work="$tmp/$1"
    rm -rf "$work"; mkdir -p "$work"
    if [[ "$url" == git+* ]]; then
        local repo tag
        repo="${url#git+}"; tag="${repo##*#}"; repo="${repo%#*}"
        git -c advice.detachedHead=false clone -q --depth 1 --branch "$tag" "$repo" "$work/git" \
            || die "$name: no tag $tag in $repo"
        local commit; commit="$(git -C "$work/git" rev-parse HEAD)"
        if [[ "$pin" == - ]]; then pin_entry "$name" "$commit"; warn "$name: pinned to commit $commit"
        elif [[ "$pin" != "$commit" ]]; then die "$name: $tag is commit $commit, the list pins $pin"; fi
        mkdir "$work/x"
        git -C "$work/git" archive --prefix=src/ HEAD | tar -xf - -C "$work/x"
    else
        local file="$work/${url##*/}"
        curl -fL --silent --show-error --retry 5 --retry-all-errors -o "$file" "$url" || die "$name: cannot download $url"
        local sum; sum="$(sha256sum "$file" | cut -d' ' -f1)"
        if [[ "$pin" == - ]]; then pin_entry "$name" "$sum"; warn "$name: pinned to sha256 $sum"
        elif [[ "$pin" != "$sum" ]]; then die "$name: sha256 $sum does not match the list's $pin"; fi
        mkdir "$work/x"
        tar -xf "$file" -C "$work/x"
    fi
    local top=("$work/x"/*)
    [[ ${#top[@]} -eq 1 && -d "${top[0]}" ]] || die "$name: unexpected archive layout"
    echo "$ver" > "${top[0]}/.jk_os-version"
    rm -rf "$dest"
    mv "${top[0]}" "$dest"
    log "userland/desktop/$name is now $ver"
}

while read -r name ver url pin _; do
    [[ -z "$name" || "$name" == \#* ]] && continue
    if (( ${#only[@]} )) && [[ " ${only[*]} " != *" $name "* ]]; then continue; fi
    if [[ -f "$DEST_ROOT/$name/.jk_os-version" && "$(cat "$DEST_ROOT/$name/.jk_os-version")" == "$ver" \
          && "$pin" != - ]]; then
        continue
    fi
    fetch_one "$name" "$ver" "$url" "$pin"
done < <(cat "$LIST")
echo "Commit with: git add -f userland/desktop configs/desktop/sources.list"
