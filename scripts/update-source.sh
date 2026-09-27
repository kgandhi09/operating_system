#!/usr/bin/env bash
# Maintainer tool: replace a vendored source tree with another release.
# Builds never run this; they only use the trees already in the repo.
#
#   scripts/update-source.sh <source> <version> [tree]
#   scripts/update-source.sh kernel  7.3                     # into kernel/mainline
#   scripts/update-source.sh kernel  7.3 kernel/<name>       # into another tree
#   scripts/update-source.sh busybox 1.38.0
#   scripts/update-source.sh networkmanager 1.58.1
#
# Instead of a version you can pass a local tarball (works offline):
#   scripts/update-source.sh kernel ~/Downloads/linux-7.3.tar.xz
#
# Every download is verified: against the project's published SHA-256 sums,
# the GitHub release asset digest, or, for projects that publish neither,
# by fetching the release tag with git (content-addressed) and exporting it.
source "$(dirname "$0")/common.sh"
need tar sha256sum

# source_spec <source> <version>: set $tree (default destination) and how to
# fetch it: $kind (sums | github | git), plus
#   sums:   $base (directory URL), $name (tarball), $sums (checksum file URL)
#   github: $repo, $tag, $name (release asset)
#   git:    $url, $tag
SOURCES="kernel busybox util-linux e2fsprogs shadow libxcrypt sudo zlib libffi pcre2 glib
expat dbus eudev libndp libnl openssl wpa_supplicant ncurses readline networkmanager"
source_spec() {
    local v="$2"
    case "$1" in
        kernel)
            [[ "$v" != *-rc* ]] || die "release candidates have no tarball on cdn.kernel.org; use a release"
            tree=kernel/mainline kind=sums name="linux-$v.tar.xz"
            base="https://cdn.kernel.org/pub/linux/kernel/v${v%%.*}.x" sums="$base/sha256sums.asc" ;;
        busybox)
            tree=$BUSYBOX_TREE kind=sums name="busybox-$v.tar.bz2"
            base="https://busybox.net/downloads" sums="$base/$name.sha256" ;;
        util-linux)
            tree=$UTIL_LINUX_TREE kind=sums name="util-linux-$v.tar.xz"
            base="https://cdn.kernel.org/pub/linux/utils/util-linux/v$(cut -d. -f1-2 <<< "$v")"
            sums="$base/sha256sums.asc" ;;
        e2fsprogs)
            tree=$E2FSPROGS_TREE kind=sums name="e2fsprogs-$v.tar.xz"
            base="https://cdn.kernel.org/pub/linux/kernel/people/tytso/e2fsprogs/v$v"
            sums="$base/sha256sums.asc" ;;
        glib)
            tree=$GLIB_TREE kind=sums name="glib-$v.tar.xz"
            base="https://download.gnome.org/sources/glib/$(cut -d. -f1-2 <<< "$v")"
            sums="$base/glib-$v.sha256sum" ;;
        networkmanager)
            tree=$NETWORKMANAGER_TREE kind=sums name="NetworkManager-$v.tar.xz"
            base="https://gitlab.freedesktop.org/api/v4/projects/411/packages/generic/NetworkManager/$v"
            sums="$base/$name.sha256sum" ;;
        shadow)    tree=$SHADOW_TREE    kind=github repo=shadow-maint/shadow tag="$v"      name="shadow-$v.tar.xz" ;;
        sudo)      tree=$SUDO_TREE      kind=github repo=sudo-project/sudo   tag="v$v"     name="sudo-$v.tar.gz" ;;
        libxcrypt) tree=$LIBXCRYPT_TREE kind=github repo=besser82/libxcrypt  tag="v$v"     name="libxcrypt-$v.tar.xz" ;;
        zlib)      tree=$ZLIB_TREE      kind=github repo=madler/zlib         tag="v$v"     name="zlib-$v.tar.xz" ;;
        libffi)    tree=$LIBFFI_TREE    kind=github repo=libffi/libffi       tag="v$v"     name="libffi-$v.tar.gz" ;;
        pcre2)     tree=$PCRE2_TREE     kind=github repo=PCRE2Project/pcre2  tag="pcre2-$v" name="pcre2-$v.tar.bz2" ;;
        expat)     tree=$EXPAT_TREE     kind=github repo=libexpat/libexpat   tag="R_${v//./_}" name="expat-$v.tar.xz" ;;
        libnl)     tree=$LIBNL_TREE     kind=github repo=thom311/libnl       tag="libnl${v//./_}" name="libnl-$v.tar.gz" ;;
        openssl)   tree=$OPENSSL_TREE   kind=github repo=openssl/openssl     tag="openssl-$v" name="openssl-$v.tar.gz" ;;
        dbus)      tree=$DBUS_TREE      kind=git url=https://gitlab.freedesktop.org/dbus/dbus.git tag="dbus-$v" ;;
        eudev)     tree=$EUDEV_TREE     kind=git url=https://github.com/eudev-project/eudev.git   tag="v$v" ;;
        libndp)    tree=$LIBNDP_TREE    kind=git url=https://github.com/jpirko/libndp.git         tag="v$v" ;;
        ncurses)   tree=$NCURSES_TREE   kind=git url=https://github.com/ThomasDickey/ncurses-snapshots.git tag="v${v//./_}" ;;
        readline)  tree=$READLINE_TREE  kind=git url=https://git.savannah.gnu.org/git/readline.git tag="readline-$v" ;;
        wpa_supplicant)
                   tree=$WPA_SUPPLICANT_TREE kind=git url=https://w1.fi/hostap.git tag="hostap_${v//./_}" ;;
        *) die "unknown source '$1' (one of: $(echo $SOURCES))" ;;
    esac
}

what="${1:-}" ver="${2:-}"
[[ -n "$what" && -n "$ver" ]] || die "usage: $0 <source> <version|tarball> [tree]   (source: $(echo $SOURCES))"
source_spec "$what" "$ver"
tree="${3:-$tree}"

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
elif [[ "$kind" == git ]]; then
    need git
    log "fetching $url at $tag"
    git -c advice.detachedHead=false clone -q --depth 1 --branch "$tag" "$url" "$tmp/git" \
        || die "no tag $tag in $url"
    log "$tag is commit $(git -C "$tmp/git" rev-parse HEAD)"
    tarball="$tmp/$what-$ver.tar"
    git -C "$tmp/git" archive --prefix="$what-$ver/" -o "$tarball" HEAD
else
    if [[ "$kind" == github ]]; then
        need jq
        base="https://github.com/$repo/releases/download/$tag"
        log "reading $repo $tag from the GitHub API"
        curl -fsSL "https://api.github.com/repos/$repo/releases/tags/$tag" > "$tmp/release.json" \
            || die "no release $tag of $repo"
        jq -r --arg n "$name" '.assets[] | select(.name == $n) | .digest // empty' "$tmp/release.json" \
            | sed -n "s/^sha256:\(.*\)/\1  $name/p" > "$tmp/sums"
    else
        fetch "$sums" "$tmp/sums"
    fi
    fetch "$base/$name" "$tmp/$name"
    # "<sum>  <name>" or "<sum> *<name>" (binary mode, as NetworkManager writes it)
    grep -E "[[:space:]]\*?$name\$" "$tmp/sums" | head -n1 | sed 's/ \*/  /' > "$tmp/check"
    [[ -s "$tmp/check" ]] || die "no published checksum for $name"
    (cd "$tmp" && sha256sum --quiet -c check) || die "checksum mismatch for $name"
    tarball="$tmp/$name"
fi

log "unpacking $(basename "$tarball")"
mkdir "$tmp/x"
tar -xf "$tarball" -C "$tmp/x"
top=("$tmp/x"/*)
[[ ${#top[@]} -eq 1 && -d "${top[0]}" ]] || die "unexpected tarball layout: $tarball"
for f in Makefile configure meson.build CMakeLists.txt autogen.sh Configure wpa_supplicant/Makefile; do
    [[ -e "${top[0]}/$f" ]] && break
done || die "unexpected tarball layout: $tarball (no build files)"

# Swap the new tree in; the old one is only deleted once the new one is there.
dest="$ROOT_DIR/$tree"
mkdir -p "$(dirname "$dest")"
[[ -e "$dest" ]] && mv "$dest" "$tmp/old"
mv "${top[0]}" "$dest"
[[ "$kind" == git ]] && echo "$ver" > "$dest/.jk_os-version"

log "$tree is now $what $(tree_version "$dest")"
echo "Rebuild with: make clean && make"
echo "Commit with:  git add -f $tree   (-f: upstream ships a few files its own .gitignore matches)"
