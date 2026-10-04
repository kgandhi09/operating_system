#!/usr/bin/env bash
# Install jk_os on a tablet from the PC, over the USB network of the tablet's
# jk_os initramfs (172.16.42.1): with the tablet at the initramfs's rescue
# shell (no system installed yet, or to reinstall), this
#   1. checks there is a backup of the tablet's partitions (tablet-backup.sh),
#   2. formats its userdata partition as ext4 JK_DATA (Android's data: the
#      only partition touched, after you type its name),
#   3. copies the OS image (make) there as system/jk_os.squashfs, checked by
#      SHA-256 on the tablet, and
#   4. reboots it into jk_os, which starts with the first-boot setup.
#
#   scripts/tablet-install.sh [--image <jk_os.squashfs>] [--no-format] [--yes]
#
#   --no-format  keep JK_DATA as it is (home, settings) and only replace the
#                image, like jk-update does
#   --yes        don't ask (the backup check still applies)
source "$(dirname "$0")/common.sh"
need telnet sha256sum dd

HOST=172.16.42.1
image=$SQUASHFS_IMG format=1 yes=0
while (( $# )); do
    case "$1" in
        --image) image=$2; shift ;;
        --no-format) format=0 ;;
        --yes) yes=1 ;;
        -h|--help) sed -n '2,/^source /p' "$0" | sed '$d; s/^# \{0,1\}//'; exit 0 ;;
        *) die "unknown option $1 (see --help)" ;;
    esac
    shift
done
[[ "$DEVICE_BOOT" == android-* ]] || die "the build target ($JK_DEVICE) isn't a tablet: see make showconfig"
[[ -f "$image" ]] || die "no OS image at $image (run: make)"

# The partitions that exist only on this unit must be safe first.
backup=$(ls -d "$HOME"/jk_os-backups/"$JK_DEVICE"-*/ 2>/dev/null | tail -n1)
[[ -n "$backup" && -f "$backup/SHA256SUMS" ]] && grep -q ' efs.img$' "$backup/SHA256SUMS" \
    || die "no backup of the tablet in ~/jk_os-backups/$JK_DEVICE-*: run scripts/tablet-backup.sh first"
log "backup: $backup"

# ask <port> <request> [file]: one request to the tablet, the file (if any)
# sent after it; prints the answer.
ask() {
    exec 3<>"/dev/tcp/$HOST/$1"
    printf '%s\n' "$2" >&3
    [[ -n "${3:-}" ]] && dd if="$3" bs=4M status=progress >&3
    head -n1 <&3
    exec 3<&-
}
timeout 5 bash -c "exec 3<>/dev/tcp/$HOST/5000" 2>/dev/null \
    || die "no jk_os console at $HOST (USB cable in, tablet at the jk_os initramfs's shell?)"

dev=$(ask 5000 list | awk '$2 == "userdata" { print $1 }')
[[ $(wc -w <<< "$dev") == 1 ]] || die "expected one userdata partition on the tablet, found: '${dev:-none}'"
size=$(stat -c %s "$image")
sum=$(sha256sum "$image" | cut -d' ' -f1)
log "image: $image ($((size / 1048576)) MB, jk_os $OS_VERSION)"

# The receiver isn't running by default: start it from a shell on the tablet.
log "starting the installer on the tablet"
{ sleep 1; echo 'pgrep -f jk-receive >/dev/null || setsid nc -lk -s 172.16.42.1 -p 5001 -e /bin/jk-receive &'; sleep 1; echo exit; } \
    | telnet "$HOST" >/dev/null 2>&1 || true
timeout 10 bash -c "until (exec 3<>/dev/tcp/$HOST/5001) 2>/dev/null; do sleep 1; done" \
    || die "the installer on the tablet didn't start (is the initramfs up to date? make bootimg && make flash)"

if (( format )); then
    if (( ! yes )); then
        echo
        echo "This ERASES the tablet's userdata partition (/dev/$dev: Android's apps and data)"
        echo "and makes it jk_os's data partition. No other partition is touched."
        read -r -p "Type 'userdata' to continue: " reply
        [[ "$reply" == userdata ]] || die "aborted"
    fi
    log "formatting /dev/$dev as JK_DATA"
    out=$(ask 5001 "format $dev userdata")
    [[ "$out" == ok* ]] || die "${out:-no answer from the tablet}"
    log "$out"
fi

log "copying the image"
out=$(ask 5001 "put $size $sum $OS_VERSION" "$image")
[[ "$out" == ok* ]] || die "${out:-no answer from the tablet}"
log "$out"

out=$(ask 5001 reboot)
log "${out:-rebooting}: the tablet starts jk_os (first-boot setup on its screen)"
