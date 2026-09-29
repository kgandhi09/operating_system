#!/usr/bin/env bash
# Maintainer tool: fetch the Debian base system that apt runs in, listed in
# configs/debian/rootfs.list, into userspace/debian/<arch>/rootfs.tar.gz.
# Builds never run this; they only use the tarballs in the repo.
#
#   scripts/update-debian-rootfs.sh              fetch what is missing or changed
#   scripts/update-debian-rootfs.sh --latest     move every arch to the newest build
#   scripts/update-debian-rootfs.sh <arch>...    only these
#
# A commit of "-" (or --latest) takes the newest commit of the artifacts
# branch and records it with the layer digest from its OCI manifest; the
# downloaded tarball must match that digest.
source "$(dirname "$0")/common.sh"
need curl sha256sum python3

LIST="$ROOT_DIR/configs/debian/rootfs.list"
REPO=debuerreotype/docker-debian-artifacts
latest=0
[[ "${1:-}" == --latest ]] && { latest=1; shift; }
only=("$@")

tmp="$(mktemp -d "$ROOT_DIR/.update-debian.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT
cp "$LIST" "$tmp/list.in"   # the loop reads a copy: pinning rewrites the list

debian_arch() { case "$1" in x86_64) echo amd64 ;; aarch64) echo arm64v8 ;; *) die "unknown arch $1" ;; esac; }

# layer_digest <commit> <suite>: the rootfs layer's sha256, from index.json.
layer_digest() {
    curl -fsSL "https://raw.githubusercontent.com/$REPO/$1/$2/slim/oci/index.json" | python3 -c '
import base64, json, sys
index = json.load(sys.stdin)
manifest = json.loads(base64.b64decode(index["manifests"][0]["data"]))
layers = manifest["layers"]
assert len(layers) == 1, "expected one layer"
print(layers[0]["digest"].removeprefix("sha256:"))'
}

while read -r arch suite commit sum; do
    [[ -z "$arch" || "$arch" == \#* ]] && continue
    if (( ${#only[@]} )) && [[ ! " ${only[*]} " == *" $arch "* ]]; then continue; fi
    dest="$ROOT_DIR/userspace/debian/$arch"
    if (( latest )) || [[ "$commit" == - ]]; then
        commit="$(curl -fsSL "https://api.github.com/repos/$REPO/commits/dist-$(debian_arch "$arch")" |
            python3 -c 'import json, sys; print(json.load(sys.stdin)["sha"])')" || die "$arch: cannot look up the artifacts branch"
        sum="$(layer_digest "$commit" "$suite")" || die "$arch: cannot read the OCI manifest"
        awk -v a="$arch" -v c="$commit" -v s="$sum" '$1 == a { $3 = c; $4 = s } { print }' OFS='  ' \
            "$LIST" > "$tmp/list"
        cat "$tmp/list" > "$LIST"
        warn "$arch: pinned to $commit (layer sha256 $sum)"
    fi
    if [[ -f "$dest/rootfs.tar.gz" && "$(cat "$dest/pin" 2>/dev/null)" == "$suite $commit $sum" ]]; then
        continue
    fi
    url="https://raw.githubusercontent.com/$REPO/$commit/$suite/slim/oci/blobs/rootfs.tar.gz"
    curl -fL --silent --show-error --retry 5 -o "$tmp/rootfs.tar.gz" "$url" || die "$arch: cannot download $url"
    got="$(sha256sum "$tmp/rootfs.tar.gz" | cut -d' ' -f1)"
    [[ "$got" == "$sum" ]] || die "$arch: sha256 $got does not match the pinned $sum"
    mkdir -p "$dest"
    mv "$tmp/rootfs.tar.gz" "$dest/rootfs.tar.gz"
    echo "$suite $commit $sum" > "$dest/pin"
    log "userspace/debian/$arch: Debian $suite ($(du -h "$dest/rootfs.tar.gz" | cut -f1))"
done < "$tmp/list.in"
