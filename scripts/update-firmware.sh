#!/usr/bin/env bash
# Maintainer tool: refresh the firmware kept in userspace/firmware from
# linux-firmware and wireless-regdb (cdn.kernel.org, checked against the
# published SHA-256 sums). Builds never run this; they only use what is in
# the repo, like the other source trees.
#
#   scripts/update-firmware.sh <linux-firmware version> <wireless-regdb version>
#   scripts/update-firmware.sh 20260916 2026.09.03
#   LINUX_FIRMWARE_TARBALL=~/linux-firmware-20260916.tar.xz scripts/update-firmware.sh ...
#
# What is kept, per arch, comes from configs/firmware/{common,<arch>}.list:
#   driver <name>   every file that built-in driver may load, as the kernel
#                   build lists it (build/<arch>/linux/modules.builtin.modinfo,
#                   so run `make kernel` for each arch first)
#   file <glob>     files no driver declares (board files, regulatory.db, ...)
#   skip <glob>     leave out files a driver line brought in (chips jk_os
#                   won't meet, such as access-point radios)
#   late <glob>     keep those files in the OS image only, not in the kernel's
#                   initramfs: firmware too big to carry in every kernel, for
#                   drivers jk_os binds again once the root is up
#                   (/etc/init.d/S13late-firmware). linux-firmware's links
#                   among them stay links, so shared blobs are stored once.
# Files are stored zstd-compressed, as the kernel loads them (<name>.zst);
# userspace/firmware/<arch>.files lists each arch's share, <arch>.late the
# late files. An arch whose kernel isn't built keeps its current lists.
source "$(dirname "$0")/common.sh"
need curl tar zstd sha256sum

fw_ver="${1:-}" regdb_ver="${2:-}"
[[ -n "$fw_ver" && -n "$regdb_ver" ]] || die "usage: $0 <linux-firmware version> <wireless-regdb version>"

LIST_DIR="$ROOT_DIR/configs/firmware"
DEST="$ROOT_DIR/userspace/firmware"

tmp="$(mktemp -d "$ROOT_DIR/.update-firmware.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

# get <base url> <tarball> [local copy]: fetch (or take) and verify against
# <base>/sha256sums.asc.
get() {
    curl -fsSL "$1/sha256sums.asc" -o "$tmp/sums"
    if [[ -n "${3:-}" ]]; then
        cp "$3" "$tmp/$2"
    else
        log "downloading $1/$2"
        curl -fL --progress-bar --retry 5 --retry-all-errors -o "$tmp/$2" "$1/$2"
    fi
    grep -E "[[:space:]]$2\$" "$tmp/sums" | head -n1 > "$tmp/check"
    [[ -s "$tmp/check" ]] || die "no published checksum for $2"
    (cd "$tmp" && sha256sum --quiet -c check) || die "checksum mismatch for $2"
}
get https://cdn.kernel.org/pub/linux/kernel/firmware "linux-firmware-$fw_ver.tar.xz" "${LINUX_FIRMWARE_TARBALL:-}"
get https://cdn.kernel.org/pub/software/network/wireless-regdb "wireless-regdb-$regdb_ver.tar.xz" "${REGDB_TARBALL:-}"

log "unpacking"
tar -xf "$tmp/linux-firmware-$fw_ver.tar.xz" -C "$tmp"
tar -xf "$tmp/wireless-regdb-$regdb_ver.tar.xz" -C "$tmp"
# copy-firmware.sh lays the tree out as installed, with the links WHENCE
# declares (drivers often ask for a file by an older name).
mkdir "$tmp/fw"
(cd "$tmp/linux-firmware-$fw_ver" && ./copy-firmware.sh "$tmp/fw" >/dev/null)
cp "$tmp/wireless-regdb-$regdb_ver"/regulatory.db{,.p7s} "$tmp/fw/"
cd "$tmp/fw"

# resolve <name>: the file the kernel would load for a declared name. Drivers
# declare the newest API they support (iwlwifi-...-c106.ucode) and fall back
# to older ones; pick the newest one linux-firmware has, down to 20 back.
resolve() {
    [[ -f "$1" ]] && { echo "$1"; return; }
    if [[ "$1" =~ ^(.*-c?)([0-9]+)(\.[a-z]+)$ ]]; then
        local pre="${BASH_REMATCH[1]}" n="${BASH_REMATCH[2]}" ext="${BASH_REMATCH[3]}" i
        for ((i = n - 1; i >= n - 20 && i >= 0; i--)); do
            [[ -f "$pre$i$ext" ]] && { echo "$pre$i$ext"; return; }
        done
    fi
    return 1
}

rm -rf "$DEST.new"; mkdir -p "$DEST.new"
for arch in x86_64 aarch64; do
    modinfo="$ROOT_DIR/build/$arch/linux/modules.builtin.modinfo"
    : > "$tmp/$arch.files"; : > "$tmp/$arch.skip"; : > "$tmp/$arch.latepat"; : > "$tmp/$arch.late"
    if [[ ! -f "$modinfo" && -f "$DEST/$arch.files" ]]; then
        warn "$arch: no kernel built (make kernel ARCH=$arch); keeping its current firmware lists"
        sed 's/\.zst$//' "$DEST/$arch.files" > "$tmp/$arch.files"
        [[ -f "$DEST/$arch.late" ]] && sed 's/\.zst$//' "$DEST/$arch.late" > "$tmp/$arch.late"
        sed 's/$/.zst/' "$tmp/$arch.files" > "$DEST.new/$arch.files"
        sed 's/$/.zst/' "$tmp/$arch.late" > "$DEST.new/$arch.late"
        continue
    fi
    for list in "$LIST_DIR/common.list" "$LIST_DIR/$arch.list"; do
        [[ -f "$list" ]] || continue
        while read -r kind what _; do
            [[ -z "$kind" || "$kind" == \#* ]] && continue
            case "$kind" in
                driver)
                    if [[ ! -f "$modinfo" ]]; then
                        warn "$arch: no kernel built yet (make kernel ARCH=$arch), skipping driver $what"
                        continue
                    fi
                    names=$(tr '\0' '\n' < "$modinfo" | sed -n "s/^$what\.firmware=//p")
                    [[ -n "$names" ]] || warn "$arch: driver $what is not built in, or declares no firmware"
                    for n in $names; do
                        resolve "$n" >> "$tmp/$arch.files" || true   # many are for chips never released
                    done ;;
                file)
                    # shellcheck disable=SC2086 # a glob on purpose
                    compgen -G "$what" >> "$tmp/$arch.files" || warn "${list##*/}: nothing matches $what" ;;
                skip) echo "$what" >> "$tmp/$arch.skip" ;;
                late) echo "$what" >> "$tmp/$arch.latepat" ;;
                *) die "${list##*/}: unknown line '$kind $what'" ;;
            esac
        done < "$list"
    done
    sort -u "$tmp/$arch.files" -o "$tmp/$arch.files"
    if [[ -s "$tmp/$arch.skip" ]]; then
        while IFS= read -r f; do
            keep=1
            while IFS= read -r pat; do
                # shellcheck disable=SC2053 # a glob on purpose
                [[ "$f" == $pat ]] && { keep=0; break; }
            done < "$tmp/$arch.skip"
            (( keep )) && echo "$f"
        done < "$tmp/$arch.files" > "$tmp/$arch.kept"
        mv "$tmp/$arch.kept" "$tmp/$arch.files"
    fi
    # The late ones, and whatever their links point at.
    if [[ -s "$tmp/$arch.latepat" ]]; then
        : > "$tmp/$arch.early"
        while IFS= read -r f; do
            late=0
            while IFS= read -r pat; do
                # shellcheck disable=SC2053 # a glob on purpose
                [[ "$f" == $pat ]] && { late=1; break; }
            done < "$tmp/$arch.latepat"
            if (( late )); then echo "$f" >> "$tmp/$arch.late"; else echo "$f" >> "$tmp/$arch.early"; fi
        done < "$tmp/$arch.files"
        mv "$tmp/$arch.early" "$tmp/$arch.files"
        while IFS= read -r f; do
            if [[ -L "$f" ]]; then realpath --relative-to=. "$f"; fi
        done < "$tmp/$arch.late" >> "$tmp/$arch.late"
        sort -u "$tmp/$arch.late" -o "$tmp/$arch.late"
    fi
    sed 's/$/.zst/' "$tmp/$arch.files" > "$DEST.new/$arch.files"
    sed 's/$/.zst/' "$tmp/$arch.late" > "$DEST.new/$arch.late"
    log "$arch: $(wc -l < "$tmp/$arch.files") files, $(wc -l < "$tmp/$arch.late") late"
done

log "compressing"
# Late files keep linux-firmware's links (to the .zst of their target).
sort -u "$tmp"/*.late | while IFS= read -r f; do
    [[ -n "$f" ]] || continue
    mkdir -p "$DEST.new/$(dirname "$f")"
    if [[ -L "$f" ]]; then
        ln -sfn "$(readlink "$f").zst" "$DEST.new/$f.zst"
    else
        zstd -q -19 -f -o "$DEST.new/$f.zst" "$f"
    fi
done
# linux-firmware ships some blobs as identical copies (NVIDIA's GSP firmware
# per chip): each late copy after the first becomes a link to it.
if compgen -G "$tmp/*.late" >/dev/null; then
    sort -u "$tmp"/*.late | while IFS= read -r f; do
        if [[ -f "$DEST.new/$f.zst" && ! -L "$DEST.new/$f.zst" ]]; then
            echo "$(sha256sum < "$DEST.new/$f.zst" | cut -d' ' -f1) $f"
        fi
    done | sort -k1,1 -k2,2 | awk '$1 == prev { print first " " $2; next } { prev = $1; first = $2 }' |
    while read -r first dup; do
        ln -sfn "$(realpath -m --relative-to="$(dirname "$DEST.new/$dup")" "$DEST.new/$first.zst")" "$DEST.new/$dup.zst"
    done
fi
sort -u "$tmp"/*.files | while IFS= read -r f; do
    mkdir -p "$DEST.new/$(dirname "$f")"
    # Copy first: zstd skips links (every name the kernel may ask for must be
    # a file), and the kernel needs the size in the frame header, which zstd
    # only writes when it reads a regular file.
    cp -L "$f" "$tmp/one"
    zstd -q -19 -o "$DEST.new/$f.zst" "$tmp/one"
    rm -f "$tmp/one"
done
printf 'linux-firmware %s\nwireless-regdb %s\n' "$fw_ver" "$regdb_ver" > "$DEST.new/VERSIONS"
cp "$tmp/linux-firmware-$fw_ver/WHENCE" "$DEST.new/WHENCE"   # licence of every file
[[ -d "$DEST" ]] && mv "$DEST" "$tmp/old"
mv "$DEST.new" "$DEST"
log "userspace/firmware: $(du -sh "$DEST" | cut -f1) (linux-firmware $fw_ver, wireless-regdb $regdb_ver)"
