#!/usr/bin/env bash
# Publish the built jk_os to the download server (the AWS machine that serves
# jkrobotics.tech), as the release this kind of system installs or updates to:
#
#   scripts/publish.sh [--no-installer] [--force] [--dry-run]
#
# Every kind of system has its own update channel (JK_CHANNEL, common.sh):
# the generic PC's is its arch (x86_64, aarch64), every other device's
# <device>-<arch> (e.g. samsung-gts7fe-aarch64). From what `make` (and, for
# a device with an Android bootloader, `make bootimg`) built for the current
# target (make showconfig; OS_VERSION from versions.env):
#   jk_os-<ver>-<channel>.squashfs  the OS image (/data/system/jk_os.squashfs)
#   the kernel, by how the device boots:
#     jk_os-<ver>-<channel>.efi              EFI: /boot/EFI/BOOT/BOOT*.EFI
#     jk_os-<ver>-<channel>.vendor_boot.img  through U-Boot: the vendor_boot
#                                            partition (kernel + device tree)
#     jk_os-<ver>-<channel>.boot.img         Android boot image: the boot
#                                            partition
#   the installer (left out with --no-installer):
#     jk_os-<ver>-<channel>.iso      a PC's USB stick (and INSTALL.txt for
#                                    the x86_64 PC, release/INSTALL.txt)
#     jk_os-<ver>-<channel>.tar.md5  a tablet's boot images, for Odin
#   SHA256SUMS                      of the files above
#   release.json                    version, channel, arch, device, boot
#                                   format, git commit, date, and each file's
#                                   name, size and SHA-256
# go to <downloads>/jk_os/<ver>/<channel>/ on the server: a version's
# folder holds every channel published at that version. They are uploaded
# into a hidden folder beside it first, checked there against SHA256SUMS, and
# only then renamed into place; last, jk_os/latest-<channel>.json (a copy of
# release.json, which jk-update reads to learn the newest version) is
# replaced, so whoever reads it always finds a complete release.
#
# Only one release per channel is kept: once the new one is in place, the
# channel's folders in the other versions are deleted (other channels are
# left alone), and so are version folders left empty. The x86_64 channel
# also takes away its files from before the channels, which lay directly in
# jk_os/<ver>/. If the server lacks the space to hold both while uploading,
# the channel's old release is deleted first (its latest json goes with it,
# so nothing points at a half-uploaded release); its downloads are then
# unavailable until the upload finishes.
#
# The kernel and the image are always published together: an image's kernel
# modules are built for that kernel. A version already on the channel is
# refused unless --force (bump OS_VERSION in versions.env instead).
#
#   PUBLISH_HOST  the server        (default ubuntu@13.62.226.224; "local":
#                                    this machine, PUBLISH_ROOT a directory
#                                    here, for tests)
#   PUBLISH_KEY   its SSH key       (default ~/.ssh/ees/jk-ees.pem)
#   PUBLISH_ROOT  the download root (default /var/www/jkrobotics.tech/downloads)
#   PUBLISH_URL   its public URL    (default http://13.62.226.224/downloads)
source "$(dirname "$0")/common.sh"
need rsync sha256sum

HOST="${PUBLISH_HOST:-ubuntu@13.62.226.224}"
KEY="${PUBLISH_KEY:-$HOME/.ssh/ees/jk-ees.pem}"
REMOTE_ROOT="${PUBLISH_ROOT:-/var/www/jkrobotics.tech/downloads}"
PUBLIC_URL="${PUBLISH_URL:-http://13.62.226.224/downloads}"

with_installer=1 force=0 dry=0
for arg do
    case "$arg" in
        --no-installer|--no-iso) with_installer=0 ;;
        --force)   force=1 ;;
        --dry-run) dry=1 ;;
        -h|--help) sed -n '2,/^source /p' "$0" | sed '$d; s/^# \{0,1\}//'; exit 0 ;;
        *) die "unknown option $arg (see --help)" ;;
    esac
done

ver="$OS_VERSION"
channel="$JK_CHANNEL"
base="$OS_NAME-$ver-$channel"
os_dir="$REMOTE_ROOT/$OS_NAME"
ver_dir="$os_dir/$ver"
remote_dir="$ver_dir/$channel"
tmp_dir="$ver_dir/.upload-$channel"
latest="$os_dir/latest-$channel.json"
if [[ "$HOST" == local ]]; then
    remote() { sh -c "$*"; }
    RSYNC_DEST="$tmp_dir/"
else
    need ssh
    SSH=(ssh -i "$KEY" -o ConnectTimeout=20 -o ServerAliveInterval=30 -o BatchMode=yes)
    remote() { "${SSH[@]}" "$HOST" "$@"; }
    RSYNC_DEST="$HOST:$tmp_dir/"
fi

# ---------------------------------------------------------------- what to publish
# The kernel file, and the partition it goes to (none: the EFI file).
case "$DEVICE_BOOT" in
    efi-iso)         kfile="$KERNEL_OUT/$KIMAGE" kname="$base.efi" kpart="" ;;
    android-uboot)   kfile="$BOOTIMG_DIR/vendor_boot.img" kname="$base.vendor_boot.img" kpart=vendor_boot ;;
    android-bootimg) kfile="$BOOTIMG_DIR/boot.img" kname="$base.boot.img" kpart=boot ;;
    *) die "no update format for boot format $DEVICE_BOOT" ;;
esac
case "$DEVICE_BOOT" in
    efi-iso) installer="$ISO" iname="$base.iso" ;;
    *)       installer="$BOOT_TAR" iname="$base.tar.md5" ;;
esac

[[ -f "$SQUASHFS_IMG" ]] || die "no image at $SQUASHFS_IMG (run: make)"
[[ -f "$KERNEL_OUT/$KIMAGE" ]] || die "no kernel at $KERNEL_OUT/$KIMAGE (run: make)"
[[ -f "$kfile" ]] || die "no kernel file at $kfile (run: make$([[ -n "$kpart" ]] && echo ' bootimg'))"
(( with_installer == 0 )) || [[ -f "$installer" ]] \
    || die "no installer at $installer (run: make$([[ -n "$kpart" ]] && echo ' bootimg'), or publish with --no-installer)"
krel="$(cat "$KERNEL_OUT/include/config/kernel.release")"
# The image and the kernel must come from the same build when the image
# carries modules for exactly this kernel (the kernel also embeds the
# initramfs).
if grep -qx 'CONFIG_MODULES=y' "$KERNEL_OUT/.config"; then
    [[ -d "$ROOTFS_DIR/usr/lib/modules/$krel" ]] || die "the image has no modules for kernel $krel: rebuild with make"
    [[ "$KERNEL_OUT/$KIMAGE" -nt "$SQUASHFS_IMG" ]] \
        && die "the kernel is newer than the image: run make so both come from the same build"
fi
# A boot image holds the kernel: it must be the newest kernel's.
if [[ -n "$kpart" && "$KERNEL_OUT/$KIMAGE" -nt "$kfile" ]]; then
    die "the kernel is newer than $(basename "$kfile"): run make bootimg"
fi
commit="$(git -C "$ROOT_DIR" rev-parse --short=12 HEAD 2>/dev/null || echo unknown)"
dirty=""
[[ -n "$(git -C "$ROOT_DIR" status --porcelain 2>/dev/null)" ]] && dirty="+changes"
[[ -n "$dirty" ]] && warn "the repo has uncommitted changes: release.json records commit $commit$dirty"

stage="$OUT_DIR/publish/$channel/$ver"
rm -rf "$stage"
mkdir -p "$stage"
# Hard links where possible: the files are large, the staging is only a view.
put() { ln -f "$1" "$stage/$2" 2>/dev/null || cp "$1" "$stage/$2"; }
put "$SQUASHFS_IMG" "$base.squashfs"
put "$kfile" "$kname"
(( with_installer )) && put "$installer" "$iname"
if [[ "$channel" == x86_64 && -f "$ROOT_DIR/release/INSTALL.txt" ]] && (( with_installer )); then
    # The guide names the version and the ISO's hash: fill in this release's.
    sed -e "s/jk_os [0-9][0-9.]* - J.K./$OS_NAME $ver - J.K./" \
        -e "s/$OS_NAME-[0-9][0-9.]*-x86_64\\.iso/$base.iso/g" "$ROOT_DIR/release/INSTALL.txt" \
        > "$stage/INSTALL.txt"
fi

log "checksums ($stage)"
files=("$base.squashfs" "$kname")
(( with_installer )) && files+=("$iname")
(cd "$stage" && sha256sum "${files[@]}" > SHA256SUMS)
if [[ -f "$stage/INSTALL.txt" ]]; then
    iso_sum="$(grep " $base.iso\$" "$stage/SHA256SUMS" | cut -d' ' -f1)"
    sed -i -E "s/^  [0-9a-f]{64}\$/  $iso_sum/" "$stage/INSTALL.txt"
fi
{
    printf '{\n  "name": "%s",\n  "version": "%s",\n  "channel": "%s",\n  "arch": "%s",\n' \
        "$OS_NAME" "$ver" "$channel" "$ARCH"
    printf '  "device": "%s",\n  "kernel_profile": "%s",\n  "boot": "%s",\n' "$JK_DEVICE" "$JK_KERNEL" "$DEVICE_BOOT"
    printf '  "kernel_release": "%s",\n  "commit": "%s%s",\n' "$krel" "$commit" "$dirty"
    printf '  "date": "%s",\n  "url": "%s/%s/%s/%s/",\n  "files": {\n' \
        "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$PUBLIC_URL" "$OS_NAME" "$ver" "$channel"
    n=0
    for f in "${files[@]}"; do
        n=$((n + 1))
        extra=""
        case "$f" in
            "$base.squashfs") role=image ;;
            "$kname") role=kernel; [[ -n "$kpart" ]] && extra=", \"partition\": \"$kpart\"" ;;
            *) role=installer ;;
        esac
        printf '    "%s": {"file": "%s", "size": %s, "sha256": "%s"%s}%s\n' "$role" "$f" \
            "$(stat -c %s "$stage/$f")" "$(grep " $f\$" "$stage/SHA256SUMS" | cut -d' ' -f1)" "$extra" \
            "$( (( n < ${#files[@]} )) && echo ,)"
    done
    printf '  }\n}\n'
} > "$stage/release.json"
cat "$stage/release.json"

if (( dry )); then
    log "dry run: staged in $stage, nothing uploaded"
    exit 0
fi

# ---------------------------------------------------------------- server checks
log "checking $HOST (channel $channel)"
remote true || die "cannot reach $HOST over SSH (key $KEY)"
if remote "test -e '$remote_dir'"; then
    (( force )) || die "$OS_NAME $ver is already on channel $channel ($remote_dir): bump OS_VERSION, or --force to replace it"
fi
# old_releases [<keep>]: the channel's releases on the server (and leftovers
# of its interrupted uploads), as paths under jk_os/: <ver>/<channel> for
# every version but <keep>; for x86_64 also its files from before the
# channels (<ver>/jk_os-<ver>-x86_64.*, SHA256SUMS, release.json,
# INSTALL.txt directly in a version folder).
old_releases() {
    remote "cd '$os_dir' 2>/dev/null || exit 0
        for d in [0-9]*/; do
            d=\${d%/}; [ -d \"\$d\" ] || continue
            [ \"\$d\" != '${1:-}' ] && [ -d \"\$d/$channel\" ] && echo \"\$d/$channel\"
            [ \"\$d\" != '$ver' ] && [ -d \"\$d/.upload-$channel\" ] && echo \"\$d/.upload-$channel\"
            if [ '$channel' = x86_64 ]; then
                for f in \"\$d\"/$OS_NAME-\"\$d\"-x86_64.* \"\$d\"/SHA256SUMS \"\$d\"/release.json \"\$d\"/INSTALL.txt; do
                    [ -f \"\$f\" ] && echo \"\$f\"
                done
            fi
        done; true"
}
# drop <paths...>: delete them under jk_os/, then the version folders left empty.
drop() {
    remote "cd '$os_dir' && rm -rf $(printf "'%s' " "$@") && for d in [0-9]*/; do rmdir \"\$d\" 2>/dev/null; done; true"
}
remote "mkdir -p '$ver_dir'"
# The user must own the release folders: it renames and deletes releases
# there (no sudo). Check before anything is deleted.
remote "test -w '$os_dir' && test -w '$ver_dir'" \
    || die "$HOST cannot write to $os_dir: run there: sudo chown -R \$(id -un) $REMOTE_ROOT"
need_kb=$(( $(du -sLk "$stage" | cut -f1) + 102400 ))     # and 100 MB to spare
free_kb() { remote "df -Pk '$REMOTE_ROOT' | awk 'NR==2 {print \$4}'"; }
if (( $(free_kb) < need_kb )); then
    mapfile -t old < <(old_releases)
    (( ${#old[@]} )) || die "not enough space on the server: $((need_kb / 1024)) MB needed, $(($(free_kb) / 1024)) MB free (grow the disk)"
    warn "not enough space to keep channel $channel's old release while uploading: deleting it first (${old[*]}); its downloads are unavailable until the upload finishes"
    remote "rm -f '$latest'"
    drop "${old[@]}"
    (( $(free_kb) >= need_kb )) \
        || die "not enough space on the server even without the old release: $((need_kb / 1024)) MB needed, $(($(free_kb) / 1024)) MB free (grow the disk)"
fi

# ---------------------------------------------------------------- upload, verify, switch
remote "mkdir -p '$tmp_dir'"
log "uploading to $HOST:$tmp_dir (resumable: rerun if it stops)"
if [[ "$HOST" == local ]]; then
    rsync -aL --partial "$stage/" "$RSYNC_DEST"
else
    rsync -aL --partial --info=progress2 --timeout=180 -e "${SSH[*]}" "$stage/" "$RSYNC_DEST"
fi
log "verifying on the server"
remote "cd '$tmp_dir' && sha256sum -c --quiet SHA256SUMS" || die "checksum mismatch on the server: rerun to resume the upload"
remote "set -e
    old='$remote_dir.old-\$\$'
    [ -e '$remote_dir' ] && mv '$remote_dir' \"\$old\"
    mv '$tmp_dir' '$remote_dir'
    chmod 755 '$ver_dir' '$remote_dir'; chmod 644 '$remote_dir'/*
    rm -rf \"\$old\"
    cp '$remote_dir/release.json' '$latest.new'
    chmod 644 '$latest.new'
    mv '$latest.new' '$latest'"

# Only this release stays on the channel.
mapfile -t old < <(old_releases "$ver")
if (( ${#old[@]} )); then
    log "deleting channel $channel's previous releases: ${old[*]}"
    drop "${old[@]}"
fi

log "published $OS_NAME $ver on channel $channel"
echo "  $PUBLIC_URL/$OS_NAME/$ver/$channel/"
echo "  $PUBLIC_URL/$OS_NAME/latest-$channel.json"
rm -rf "$stage"
