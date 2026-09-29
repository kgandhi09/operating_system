#!/usr/bin/env bash
# Pull the prebuilt binaries listed in configs/binaries/{common,$ARCH}.list
# from GitHub releases into userspace/binaries/$ARCH/bin.
#
# What was fetched is recorded in userspace/binaries/$ARCH/sources.lock. An
# entry that still matches its lock line is left alone, so once the binaries
# are in the repo this step needs no network. Binaries no longer listed are
# removed.
#
#   UPDATE=1        re-resolve and re-download every entry (moves `latest` on)
#   OFFLINE=1       fail instead of downloading
#   GITHUB_TOKEN    optional; raises the GitHub API rate limit (60/hour without)
source "$(dirname "$0")/common.sh"

LIST_DIR="$ROOT_DIR/configs/binaries"
DEST="$BINARIES_DIR/bin"
LOCK="$BINARIES_DIR/sources.lock"
UPDATE="${UPDATE:-0}"
OFFLINE="${OFFLINE:-0}"

case "$ARCH" in
    x86_64)  DEBARCH=amd64; ELF_MACHINE=x86-64 ;;
    aarch64) DEBARCH=arm64; ELF_MACHINE=aarch64 ;;
esac

subst() { local s="${1//@ARCH@/$ARCH}"; echo "${s//@DEBARCH@/$DEBARCH}"; }

# Read the lists. A name in $ARCH.list replaces the same name from common.list.
declare -A entry=() origin=()
order=()
for list in "$LIST_DIR/common.list" "$LIST_DIR/$ARCH.list"; do
    [[ -f "$list" ]] || continue
    declare -A seen=()
    n=0
    while read -r name repo tag asset member extra || [[ -n "${name:-}" ]]; do
        n=$((n + 1))
        [[ -z "$name" || "$name" == \#* ]] && continue
        where="${list#"$ROOT_DIR"/}:$n"
        [[ -n "$asset" && -z "$extra" ]] || die "$where: expected: name owner/repo tag asset [member]"
        [[ "$name" =~ ^[A-Za-z0-9._+-]+$ ]] || die "$where: bad name '$name'"
        [[ "$repo" =~ ^[A-Za-z0-9._-]+/[A-Za-z0-9._-]+$ ]] || die "$where: bad repo '$repo' (want owner/repo)"
        [[ -z "${seen[$name]:-}" ]] || die "$where: '$name' is listed twice"
        seen[$name]=1
        [[ -n "${entry[$name]:-}" ]] || order+=("$name")
        entry[$name]="$repo $tag $(subst "$asset") $(subst "${member:--}")"
        origin[$name]="$where"
    done < "$list"
    unset seen
done

mkdir -p "$DEST"
touch "$LOCK"

# lock_get <name>: "repo tag asset sha256" for name, if recorded.
lock_get() { awk -v n="$1" '$1 == n { $1 = ""; sub(/^ /, ""); print; exit }' "$LOCK"; }
lock_set() {
    local name="$1" tmp_lock
    tmp_lock="$(mktemp)"
    {
        echo "# Written by scripts/fetch-binaries.sh: name repo tag asset sha256"
        grep -vE "^(#|${name//./\\.} )" "$LOCK" || true
        [[ -z "${2:-}" ]] || echo "$name $2"
    } | { read -r header; echo "$header"; sort; } > "$tmp_lock"
    mv "$tmp_lock" "$LOCK"
}

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

gh_api() {
    local auth=()
    [[ -z "${GITHUB_TOKEN:-}" ]] || auth=(-H "Authorization: Bearer $GITHUB_TOKEN")
    curl -fsSL --retry 3 -H "Accept: application/vnd.github+json" "${auth[@]}" \
        "https://api.github.com/$1"
}

# check_binary <file> <label>: must run on a bare $ARCH rootfs with no libc.
check_binary() {
    local info
    info="$(file -b "$1")"
    case "$info" in
        ELF*)
            [[ "$info" == *"$ELF_MACHINE"* ]] || die "$2: not an $ARCH binary ($info)"
            [[ "$info" == *"statically linked"* || "$info" == *"static-pie linked"* ]] \
                || die "$2: dynamically linked, and jk_os has no libc (pick a musl/static asset)"
            ;;
        *script*) ;;
        *) die "$2: not an executable ($info)" ;;
    esac
}

# fetch <name>: resolve, download, verify, extract and install one entry.
fetch() {
    local name="$1" repo tag asset member where rel url digest sum cand work bin
    read -r repo tag asset member <<< "${entry[$name]}"
    where="${origin[$name]}"
    (( OFFLINE == 0 )) || die "$where: $name needs downloading but OFFLINE=1"
    need curl jq sha256sum file

    if [[ "$tag" == latest ]]; then rel="releases/latest"; else rel="releases/tags/$tag"; fi
    gh_api "repos/$repo/$rel" > "$tmp/release.json" \
        || die "$where: cannot read $repo $rel from GitHub (network, typo, or rate limit: set GITHUB_TOKEN)"
    tag="$(jq -r .tag_name "$tmp/release.json")"

    local matches=()
    while IFS=$'\t' read -r cand url digest; do
        # shellcheck disable=SC2053 # $asset is a glob on purpose
        [[ "$cand" == $asset ]] && matches+=("$cand"$'\t'"$url"$'\t'"$digest")
    done < <(jq -r '.assets[] | [.name, .browser_download_url, (.digest // "")] | @tsv' "$tmp/release.json")
    if (( ${#matches[@]} != 1 )); then
        echo "assets in $repo $tag:" >&2
        jq -r '.assets[].name' "$tmp/release.json" | sed 's/^/  /' >&2
        die "$where: '$asset' matches ${#matches[@]} assets of $repo $tag, need exactly 1"
    fi
    IFS=$'\t' read -r asset url digest <<< "${matches[0]}"

    log "downloading $name: $repo $tag $asset"
    curl -fL --progress-bar --retry 5 --retry-all-errors -o "$tmp/$asset" "$url"
    sum="$(sha256sum "$tmp/$asset" | cut -d' ' -f1)"
    if [[ "$digest" == sha256:* ]]; then
        [[ "$sum" == "${digest#sha256:}" ]] || die "$asset: sha256 mismatch with GitHub's published digest"
    else
        warn "$asset: GitHub publishes no digest for it; recording its sha256 unverified"
    fi

    work="$tmp/x-$name"
    rm -rf "$work"; mkdir "$work"
    case "$asset" in
        *.tar|*.tar.*|*.tgz|*.tbz|*.tbz2|*.txz|*.tzst) tar -xf "$tmp/$asset" -C "$work" ;;
        *.zip) need unzip; unzip -q "$tmp/$asset" -d "$work" ;;
        *.gz)  gzip  -dc "$tmp/$asset" > "$work/$name" ;;
        *.xz)  xz    -dc "$tmp/$asset" > "$work/$name" ;;
        *.bz2) bzip2 -dc "$tmp/$asset" > "$work/$name" ;;
        *.zst) zstd  -qdc "$tmp/$asset" > "$work/$name" ;;
        *)     cp "$tmp/$asset" "$work/$name" ;;   # a bare binary
    esac

    local found=()
    if [[ "$member" == - ]]; then
        mapfile -t found < <(find "$work" -type f -name "$name")
    else
        mapfile -t found < <(find "$work" -type f -path "$work/$member")
    fi
    if (( ${#found[@]} != 1 )); then
        (cd "$work" && find . -type f | sed 's|^\./|  |') >&2
        die "$where: found ${#found[@]} files for $name in $asset, need exactly 1 (set the member column)"
    fi
    bin="${found[0]}"

    check_binary "$bin" "$where: $name"
    install -m 0755 "$bin" "$DEST/$name"
    lock_set "$name" "$repo $tag $asset $sum"
}

for name in "${order[@]}"; do
    read -r repo tag asset _ <<< "${entry[$name]}"
    read -r l_repo l_tag l_asset _ <<< "$(lock_get "$name")"
    # shellcheck disable=SC2053 # $asset is a glob on purpose
    if (( UPDATE == 0 )) && [[ -f "$DEST/$name" && "${l_repo:-}" == "$repo" \
          && ( "$tag" == latest || "${l_tag:-}" == "$tag" ) && "${l_asset:-}" == $asset ]]; then
        continue
    fi
    fetch "$name"
done

# Drop binaries that are no longer listed.
for f in "$DEST"/* ; do
    [[ -e "$f" ]] || continue
    name="$(basename "$f")"
    if [[ -z "${entry[$name]:-}" ]]; then
        log "removing $name (no longer listed)"
        rm -f "$f"
    fi
done
while read -r name _; do
    [[ -z "$name" || "$name" == \#* || -n "${entry[$name]:-}" ]] || lock_set "$name"
done < "$LOCK"

log "binaries ($ARCH): ${order[*]:-none}"
