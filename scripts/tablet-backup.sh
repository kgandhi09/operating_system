#!/usr/bin/env bash
# Copy a tablet's partitions to the PC, over the USB network its jk_os
# initramfs provides (172.16.42.1, jk-serve on port 5000). Read-only on the
# tablet. Run it before anything writes to the tablet's storage: some
# partitions (efs, sec_efs, persist, modem calibration) are unique to the
# unit and exist nowhere else.
#
#   scripts/tablet-backup.sh [--list] [--all] [<dir>]
#
#   --list   only show the tablet's partitions
#   --all    every partition; by default the big ones Android itself lives
#            in (super, userdata: gigabytes, and in the stock firmware or
#            disposable) are left out, and everything else is copied
#   <dir>    where to (default ~/jk_os-backups/<device>-<date>)
#
# Each copy is checked against the SHA-256 the tablet computes itself;
# SHA256SUMS and partitions.txt (device, name, size) go with them.
set -euo pipefail

HOST=172.16.42.1 PORT=5000
SKIP_RE='^(super|userdata)$'

die() { printf 'error: %s\n' "$*" >&2; exit 1; }
log() { printf '==> %s\n' "$*"; }

# ask <request>: send one request to jk-serve, its answer on stdout.
ask() {
    exec 3<>"/dev/tcp/$HOST/$PORT"
    printf '%s\n' "$1" >&3
    cat <&3
    exec 3<&-
}

list=0 all=0 dir=""
for a in "$@"; do
    case "$a" in
        --list) list=1 ;;
        --all) all=1 ;;
        -h|--help) sed -n '2,/^set -e/p' "$0" | sed '$d; s/^# \{0,1\}//'; exit 0 ;;
        -*) die "unknown option $a" ;;
        *) dir=$a ;;
    esac
done

timeout 5 bash -c "exec 3<>/dev/tcp/$HOST/$PORT" 2>/dev/null \
    || die "no jk_os console at $HOST:$PORT (USB cable in, tablet in the jk_os initramfs?)"
parts=$(ask list)
[[ -n "$parts" ]] || die "the tablet reports no partitions (is its storage up?)"
if (( list )); then
    echo "$parts" | awk '{ printf "%-10s %-24s %10.1f MiB\n", $1, $2, $3 / 1048576 }'
    exit 0
fi

dir=${dir:-$HOME/jk_os-backups/samsung-gts7fe-$(date +%Y%m%d-%H%M%S)}
mkdir -p "$dir"
echo "$parts" > "$dir/partitions.txt"
: > "$dir/SHA256SUMS"

while read -r dev name size; do
    if (( ! all )) && [[ "$name" =~ $SKIP_RE ]]; then
        log "skipping $name ($dev, $(( size / 1048576 )) MiB; --all to include)"
        continue
    fi
    file="$dir/$name.img"
    [[ -e "$file" ]] && file="$dir/$name-$dev.img"   # a name on two LUNs
    log "$name ($dev, $(( size / 1048576 )) MiB)"
    ask "read $dev" > "$file"
    got=$(stat -c %s "$file")
    (( got == size )) || die "$name: got $got bytes, expected $size"
    want=$(ask "sha256 $dev")
    have=$(sha256sum "$file" | cut -d' ' -f1)
    [[ "$have" == "$want" ]] || die "$name: SHA-256 mismatch (copy $have, tablet $want)"
    echo "$have  $(basename "$file")" >> "$dir/SHA256SUMS"
done <<< "$parts"

log "backup complete: $dir"
