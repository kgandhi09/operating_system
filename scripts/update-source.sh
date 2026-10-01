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
# the GitHub release asset digest, the GNU project's signature (GNU keyring
# from ftp.gnu.org), or, for projects that publish none of these, by
# fetching the release tag with git (content-addressed) and exporting it.
source "$(dirname "$0")/common.sh"
need tar sha256sum

# source_spec <source> <version>: set $tree (default destination) and how to
# fetch it: $kind (sums | github | gnu | git), plus
#   sums:   $base (directory URL), $name (tarball), $sums (checksum file URL)
#   gnu:    $base, $name (checked against $name.sig)
#   github: $repo, $tag, $name (release asset)
#   git:    $url, $tag
SOURCES="kernel busybox util-linux e2fsprogs shadow libxcrypt sudo zlib libffi pcre2 glib
expat dbus eudev libndp libnl openssl wpa_supplicant ncurses readline networkmanager
gcc binutils gdb gmp mpfr mpc llvm cmake ninja make m4 flex bison perl autoconf automake bash python sqlite bzip2
curl cacert openssh git"
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
        git)
            tree=$GIT_TREE kind=sums name="git-$v.tar.xz"
            base="https://mirrors.edge.kernel.org/pub/software/scm/git" sums="$base/sha256sums.asc" ;;
        cacert)    # the Mozilla CA certificates as curl publishes them (version: the date)
            tree=$CACERT_TREE kind=sums name="cacert-$v.pem"
            base="https://curl.se/ca" sums="$base/$name.sha256" ;;
        curl)      tree=$CURL_TREE      kind=github repo=curl/curl tag="curl-${v//./_}" name="curl-$v.tar.xz" ;;
        shadow)    tree=$SHADOW_TREE    kind=github repo=shadow-maint/shadow tag="$v"      name="shadow-$v.tar.xz" ;;
        sudo)      tree=$SUDO_TREE      kind=github repo=sudo-project/sudo   tag="v$v"     name="sudo-$v.tar.gz" ;;
        libxcrypt) tree=$LIBXCRYPT_TREE kind=github repo=besser82/libxcrypt  tag="v$v"     name="libxcrypt-$v.tar.xz" ;;
        zlib)      tree=$ZLIB_TREE      kind=github repo=madler/zlib         tag="v$v"     name="zlib-$v.tar.xz" ;;
        libffi)    tree=$LIBFFI_TREE    kind=github repo=libffi/libffi       tag="v$v"     name="libffi-$v.tar.gz" ;;
        pcre2)     tree=$PCRE2_TREE     kind=github repo=PCRE2Project/pcre2  tag="pcre2-$v" name="pcre2-$v.tar.bz2" ;;
        expat)     tree=$EXPAT_TREE     kind=github repo=libexpat/libexpat   tag="R_${v//./_}" name="expat-$v.tar.xz" ;;
        libnl)     tree=$LIBNL_TREE     kind=github repo=thom311/libnl       tag="libnl${v//./_}" name="libnl-$v.tar.gz" ;;
        openssl)   tree=$OPENSSL_TREE   kind=github repo=openssl/openssl     tag="openssl-$v" name="openssl-$v.tar.gz" ;;
        gcc)       tree=$GCC_TREE       kind=gnu base="https://ftp.gnu.org/gnu/gcc/gcc-$v" name="gcc-$v.tar.xz" ;;
        binutils)  tree=$BINUTILS_TREE  kind=gnu base="https://ftp.gnu.org/gnu/binutils"  name="binutils-$v.tar.xz" ;;
        gdb)       tree=$GDB_TREE       kind=gnu base="https://ftp.gnu.org/gnu/gdb"       name="gdb-$v.tar.xz" ;;
        gmp)       tree=$GMP_TREE       kind=gnu base="https://ftp.gnu.org/gnu/gmp"       name="gmp-$v.tar.xz" ;;
        mpfr)      tree=$MPFR_TREE      kind=gnu base="https://ftp.gnu.org/gnu/mpfr"      name="mpfr-$v.tar.xz" ;;
        mpc)       tree=$MPC_TREE       kind=gnu base="https://ftp.gnu.org/gnu/mpc"       name="mpc-$v.tar.xz" ;;
        llvm)      tree=$LLVM_TREE      kind=github repo=llvm/llvm-project tag="llvmorg-$v" name="llvm-project-$v.src.tar.xz" ;;
        cmake)     tree=$CMAKE_TREE     kind=github repo=Kitware/CMake     tag="v$v"     name="cmake-$v.tar.gz" ;;
        make)      tree=$MAKE_TREE      kind=gnu base="https://ftp.gnu.org/gnu/make"      name="make-$v.tar.gz" ;;
        m4)        tree=$M4_TREE        kind=gnu base="https://ftp.gnu.org/gnu/m4"        name="m4-$v.tar.xz" ;;
        bison)     tree=$BISON_TREE     kind=gnu base="https://ftp.gnu.org/gnu/bison"     name="bison-$v.tar.xz" ;;
        autoconf)  tree=$AUTOCONF_TREE  kind=gnu base="https://ftp.gnu.org/gnu/autoconf"  name="autoconf-$v.tar.xz" ;;
        automake)  tree=$AUTOMAKE_TREE  kind=gnu base="https://ftp.gnu.org/gnu/automake"  name="automake-$v.tar.xz" ;;
        # flex's release assets have no GitHub digest, only a signature: pass
        # the tarball, checked against flex-<v>.tar.gz.sig, as a local file.
        flex)      tree=$FLEX_TREE      kind=github repo=westes/flex tag="v$v" name="flex-$v.tar.gz" ;;
        # CPAN's <tarball>.sha256.txt holds the bare checksum.
        perl)      tree=$PERL_TREE      kind=sums name="perl-$v.tar.xz"
                   base="https://www.cpan.org/src/5.0" sums="$base/$name.sha256.txt" ;;
        # bash's official patches (bash53-NNN) are in configs/toolchain/patches/bash.
        bash)      tree=$BASH_TREE      kind=gnu base="https://ftp.gnu.org/gnu/bash"      name="bash-$v.tar.gz" ;;
        # python.org publishes each file's SHA-256 through its downloads API
        # (3.14 and newer are signed with Sigstore only, no PGP).
        python)    tree=$PYTHON_TREE    kind=python name="Python-$v.tar.xz"
                   base="https://www.python.org/ftp/python/$v" ;;
        # Checked by hand: SQLite publishes SHA3-256 sums on its download page,
        # bzip2 a signature by its maintainer (not in the GNU keyring). Pass
        # the verified tarball as a local file.
        sqlite)    tree=$SQLITE_TREE    kind=manual url="https://www.sqlite.org/download.html" ;;
        bzip2)     tree=$BZIP2_TREE     kind=manual url="https://sourceware.org/pub/bzip2/bzip2-$v.tar.gz (.sig)" ;;
        ninja)     tree=$NINJA_TREE     kind=git url=https://github.com/ninja-build/ninja.git tag="v$v" ;;
        dbus)      tree=$DBUS_TREE      kind=git url=https://gitlab.freedesktop.org/dbus/dbus.git tag="dbus-$v" ;;
        eudev)     tree=$EUDEV_TREE     kind=git url=https://github.com/eudev-project/eudev.git   tag="v$v" ;;
        libndp)    tree=$LIBNDP_TREE    kind=git url=https://github.com/jpirko/libndp.git         tag="v$v" ;;
        ncurses)   tree=$NCURSES_TREE   kind=git url=https://github.com/ThomasDickey/ncurses-snapshots.git tag="v${v//./_}" ;;
        readline)  tree=$READLINE_TREE  kind=git url=https://git.savannah.gnu.org/git/readline.git tag="readline-$v" ;;
        openssh)   # 10.5p1: tag V_10_5_P1
                   t=${v//./_}; tree=$OPENSSH_TREE kind=git url=https://github.com/openssh/openssh-portable.git tag="V_${t/p/_P}" ;;
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
elif [[ "$kind" == manual ]]; then
    die "$what: download and verify it yourself ($url), then pass the tarball: $0 $what <file>"
elif [[ "$kind" == git ]]; then
    need git
    log "fetching $url at $tag"
    git -c advice.detachedHead=false clone -q --depth 1 --branch "$tag" "$url" "$tmp/git" \
        || die "no tag $tag in $url"
    log "$tag is commit $(git -C "$tmp/git" rev-parse HEAD)"
    tarball="$tmp/$what-$ver.tar"
    git -C "$tmp/git" archive --prefix="$what-$ver/" -o "$tarball" HEAD
else
    if [[ "$kind" == gnu ]]; then
        need gpgv
        fetch "$base/$name" "$tmp/$name"
        fetch "$base/$name.sig" "$tmp/$name.sig"
        fetch https://ftp.gnu.org/gnu/gnu-keyring.gpg "$tmp/gnu-keyring.gpg"
        gpgv --keyring "$tmp/gnu-keyring.gpg" "$tmp/$name.sig" "$tmp/$name" 2>"$tmp/gpgv.log" \
            || { cat "$tmp/gpgv.log" >&2; die "bad GNU signature on $name"; }
        log "$(grep -o 'Good signature from.*' "$tmp/gpgv.log" | head -n1)"
        sha256sum "$tmp/$name" | sed "s|$tmp/||" > "$tmp/sums"
    elif [[ "$kind" == python ]]; then
        need jq
        id=$(curl -fsSL "https://www.python.org/api/v2/downloads/release/?name=Python%20$ver" \
            | jq -r '.[0].resource_uri // empty' | sed 's|/$||; s|.*/||')
        [[ -n "$id" ]] || die "no Python $ver on python.org"
        curl -fsSL "https://www.python.org/api/v2/downloads/release_file/?release=$id" \
            | jq -r --arg n "$name" '.[] | select(.url | endswith("/" + $n)) | .sha256_sum // empty' \
            | sed "s/\$/  $name/" > "$tmp/sums"
    elif [[ "$kind" == github ]]; then
        need jq
        base="https://github.com/$repo/releases/download/$tag"
        log "reading $repo $tag from the GitHub API"
        curl -fsSL "https://api.github.com/repos/$repo/releases/tags/$tag" > "$tmp/release.json" \
            || die "no release $tag of $repo"
        jq -r --arg n "$name" '.assets[] | select(.name == $n) | .digest // empty' "$tmp/release.json" \
            | sed -n "s/^sha256:\(.*\)/\1  $name/p" > "$tmp/sums"
    else
        fetch "$sums" "$tmp/sums"
        # a file with only the checksum (CPAN's): name the tarball
        grep -q "$name" "$tmp/sums" || echo "$(tr -d '[:space:]' < "$tmp/sums")  $name" > "$tmp/sums"
    fi
    [[ -f "$tmp/$name" ]] || fetch "$base/$name" "$tmp/$name"
    # "<sum>  <name>" or "<sum> *<name>" (binary mode, as NetworkManager writes it)
    grep -E "[[:space:]]\*?$name\$" "$tmp/sums" | head -n1 | sed 's/ \*/  /' > "$tmp/check"
    [[ -s "$tmp/check" ]] || die "no published checksum for $name"
    (cd "$tmp" && sha256sum --quiet -c check) || die "checksum mismatch for $name"
    tarball="$tmp/$name"
fi

if [[ "$tarball" == *.pem ]]; then     # a single file, not a source tarball
    dest="$ROOT_DIR/$tree"
    rm -rf "$dest"; mkdir -p "$dest"
    cp "$tarball" "$dest/cacert.pem"
    echo "$ver" > "$dest/.jk_os-version"
    log "$tree is now $what $ver"
    exit 0
fi

log "unpacking $(basename "$tarball")"
mkdir "$tmp/x"
tar -xf "$tarball" -C "$tmp/x"
top=("$tmp/x"/*)
[[ ${#top[@]} -eq 1 && -d "${top[0]}" ]] || die "unexpected tarball layout: $tarball"
for f in Makefile configure meson.build CMakeLists.txt autogen.sh Configure wpa_supplicant/Makefile llvm/CMakeLists.txt bootstrap; do
    [[ -e "${top[0]}/$f" ]] && break
done || die "unexpected tarball layout: $tarball (no build files)"

# Swap the new tree in; the old one is only deleted once the new one is there.
dest="$ROOT_DIR/$tree"
mkdir -p "$(dirname "$dest")"
[[ -e "$dest" ]] && mv "$dest" "$tmp/old"
mv "${top[0]}" "$dest"
# git exports carry no version; some tarballs name it only in a header.
if [[ "$kind" == git ]]; then echo "$ver" > "$dest/.jk_os-version"
elif [[ ! -f "$ver" && "$(tree_version "$dest")" == @(|-|unknown) ]]; then echo "$ver" > "$dest/.jk_os-version"; fi

log "$tree is now $what $(tree_version "$dest")"
echo "Rebuild with: make clean && make"
echo "Commit with:  git add -f $tree   (-f: upstream ships a few files its own .gitignore matches)"
