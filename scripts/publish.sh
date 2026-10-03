#!/usr/bin/env bash
# Publish the built jk_os to the download server (the AWS machine that serves
# jkrobotics.tech), as the release users install or update to:
#
#   scripts/publish.sh [--no-iso] [--force] [--dry-run]
#
# From what `make` built for $ARCH (OS_VERSION from versions.env):
#   jk_os-<ver>-<arch>.squashfs   the OS image   (/data/system/jk_os.squashfs)
#   jk_os-<ver>-<arch>.efi        the kernel     (/boot/EFI/BOOT/BOOT*.EFI)
#   jk_os-<ver>-<arch>.iso        the installer  (left out with --no-iso)
#   INSTALL.txt                   release/INSTALL.txt, for this version
#   SHA256SUMS                    of the files above
#   release.json                  version, arch, git commit, date, and each
#                                 file's name, size and SHA-256
# go to <downloads>/jk_os/<ver>/ on the server. They are uploaded into a
# hidden directory first, checked there against SHA256SUMS, and only then
# renamed into place; last, latest-<arch>.json (a copy of release.json) is
# replaced, so whoever reads it always finds a complete release.
#
# Only one release is kept: once the new one is in place, every other version
# on the server is deleted. If the server lacks the space to hold both while
# uploading, the old release is deleted first (latest-<arch>.json goes with
# it, so nothing points at a half-uploaded release); downloads are then
# unavailable until the upload finishes.
#
# The kernel and the image are always published together: the image's NVIDIA
# modules are built for that kernel. A version already on the server is
# refused unless --force (bump OS_VERSION in versions.env instead).
#
#   PUBLISH_HOST  the server        (default ubuntu@13.62.226.224)
#   PUBLISH_KEY   its SSH key       (default ~/.ssh/ees/jk-ees.pem)
#   PUBLISH_ROOT  the download root (default /var/www/jkrobotics.tech/downloads)
#   PUBLISH_URL   its public URL    (default http://13.62.226.224/downloads)
source "$(dirname "$0")/common.sh"
need ssh rsync sha256sum

HOST="${PUBLISH_HOST:-ubuntu@13.62.226.224}"
KEY="${PUBLISH_KEY:-$HOME/.ssh/ees/jk-ees.pem}"
REMOTE_ROOT="${PUBLISH_ROOT:-/var/www/jkrobotics.tech/downloads}"
PUBLIC_URL="${PUBLISH_URL:-http://13.62.226.224/downloads}"

with_iso=1 force=0 dry=0
for arg do
    case "$arg" in
        --no-iso)  with_iso=0 ;;
        --force)   force=1 ;;
        --dry-run) dry=1 ;;
        -h|--help) sed -n '2,/^source /p' "$0" | sed '$d; s/^# \{0,1\}//'; exit 0 ;;
        *) die "unknown option $arg (see --help)" ;;
    esac
done

# Releases are per arch (latest-<arch>.json), which every installed system
# reads, so only the generic PC build is published for now.
[[ "$JK_DEVICE" == generic ]] || die "only the generic PC build can be published yet (this is $JK_DEVICE, see make showconfig)"

ver="$OS_VERSION"
base="$OS_NAME-$ver-$ARCH"
kernel="$KERNEL_OUT/$KIMAGE"
remote_dir="$REMOTE_ROOT/$OS_NAME/$ver"
SSH=(ssh -i "$KEY" -o ConnectTimeout=20 -o ServerAliveInterval=30 -o BatchMode=yes)
remote() { "${SSH[@]}" "$HOST" "$@"; }

# ---------------------------------------------------------------- what to publish
[[ -f "$SQUASHFS_IMG" ]] || die "no image at $SQUASHFS_IMG (run: make)"
[[ -f "$kernel" ]] || die "no kernel at $kernel (run: make)"
(( with_iso == 0 )) || [[ -f "$ISO" ]] || die "no ISO at $ISO (run: make, or publish with --no-iso)"
# The image and the kernel must come from the same build: the kernel embeds
# the initramfs and the image carries modules for exactly this kernel.
krel="$(cat "$KERNEL_OUT/include/config/kernel.release")"
[[ -d "$ROOTFS_DIR/usr/lib/modules/$krel" ]] || die "the image has no modules for kernel $krel: rebuild with make"
if [[ "$kernel" -nt "$SQUASHFS_IMG" ]]; then
    die "the kernel is newer than the image: run make so both come from the same build"
fi
commit="$(git -C "$ROOT_DIR" rev-parse --short=12 HEAD 2>/dev/null || echo unknown)"
dirty=""
[[ -n "$(git -C "$ROOT_DIR" status --porcelain 2>/dev/null)" ]] && dirty="+changes"
[[ -n "$dirty" ]] && warn "the repo has uncommitted changes: release.json records commit $commit$dirty"

stage="$OUT_DIR/publish/$ver"
rm -rf "$stage"
mkdir -p "$stage"
# Hard links where possible: the files are large, the staging is only a view.
ln -f "$SQUASHFS_IMG" "$stage/$base.squashfs" 2>/dev/null || cp "$SQUASHFS_IMG" "$stage/$base.squashfs"
ln -f "$kernel" "$stage/$base.efi" 2>/dev/null || cp "$kernel" "$stage/$base.efi"
(( with_iso )) && { ln -f "$ISO" "$stage/$base.iso" 2>/dev/null || cp "$ISO" "$stage/$base.iso"; }
if [[ -f "$ROOT_DIR/release/INSTALL.txt" ]]; then
    # The guide names the version and the ISO's hash: fill in this release's.
    sed -e "s/jk_os [0-9][0-9.]* - J.K./$OS_NAME $ver - J.K./" \
        -e "s/$OS_NAME-[0-9][0-9.]*-x86_64\\.iso/$base.iso/g" "$ROOT_DIR/release/INSTALL.txt" \
        > "$stage/INSTALL.txt"
fi

log "checksums ($stage)"
files=("$base.squashfs" "$base.efi")
(( with_iso )) && files+=("$base.iso")
(cd "$stage" && sha256sum "${files[@]}" > SHA256SUMS)
if [[ -f "$stage/INSTALL.txt" ]] && (( with_iso )); then
    iso_sum="$(grep " $base.iso\$" "$stage/SHA256SUMS" | cut -d' ' -f1)"
    sed -i -E "s/^  [0-9a-f]{64}\$/  $iso_sum/" "$stage/INSTALL.txt"
fi
{
    printf '{\n  "name": "%s",\n  "version": "%s",\n  "arch": "%s",\n' "$OS_NAME" "$ver" "$ARCH"
    printf '  "kernel_release": "%s",\n  "commit": "%s%s",\n' "$krel" "$commit" "$dirty"
    printf '  "date": "%s",\n  "url": "%s/%s/%s/",\n  "files": {\n' \
        "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$PUBLIC_URL" "$OS_NAME" "$ver"
    n=0
    for f in "${files[@]}"; do
        n=$((n + 1))
        case "$f" in *.squashfs) role=image ;; *.efi) role=kernel ;; *.iso) role=installer ;; esac
        printf '    "%s": {"file": "%s", "size": %s, "sha256": "%s"}%s\n' "$role" "$f" \
            "$(stat -c %s "$stage/$f")" "$(grep " $f\$" "$stage/SHA256SUMS" | cut -d' ' -f1)" \
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
log "checking $HOST"
remote true || die "cannot reach $HOST over SSH (key $KEY)"
if remote "test -e '$remote_dir'"; then
    (( force )) || die "$OS_NAME $ver is already on the server ($remote_dir): bump OS_VERSION, or --force to replace it"
fi
# old_versions [<keep>]: the releases on the server (and leftovers of other
# interrupted uploads), but not this version's upload in progress, nor <keep>.
old_versions() {
    remote "cd '$REMOTE_ROOT/$OS_NAME' 2>/dev/null && for d in */ .upload-*/; do
        d=\${d%/}; [ -d \"\$d\" ] && [ \"\$d\" != '.upload-$ver' ] && [ \"\$d\" != '${1:-}' ] && echo \"\$d\"; done; true"
}
remote "mkdir -p '$REMOTE_ROOT/$OS_NAME'"
# The SSH user must own the release folder: it renames and deletes releases
# there (no sudo). Check before anything is deleted.
remote "test -w '$REMOTE_ROOT/$OS_NAME' && test -w '$REMOTE_ROOT'" \
    || die "$HOST cannot write to $REMOTE_ROOT/$OS_NAME: run there: sudo chown -R \$(id -un) $REMOTE_ROOT"
need_kb=$(( $(du -sLk "$stage" | cut -f1) + 102400 ))     # and 100 MB to spare
free_kb() { remote "df -Pk '$REMOTE_ROOT' | awk 'NR==2 {print \$4}'"; }
if (( $(free_kb) < need_kb )); then
    old=$(old_versions | grep -v '^\.upload-' || true)
    [[ -n "$old" ]] || die "not enough space on the server: $((need_kb / 1024)) MB needed, $(($(free_kb) / 1024)) MB free (grow the disk)"
    warn "not enough space to keep the old release while uploading: deleting it first ($(echo $old)); downloads are unavailable until the upload finishes"
    remote "cd '$REMOTE_ROOT/$OS_NAME' && rm -f latest-$ARCH.json && rm -rf $(printf "'%s' " $old)"
    (( $(free_kb) >= need_kb )) \
        || die "not enough space on the server even without the old release: $((need_kb / 1024)) MB needed, $(($(free_kb) / 1024)) MB free (grow the disk)"
fi

# ---------------------------------------------------------------- upload, verify, switch
tmp_dir="$REMOTE_ROOT/$OS_NAME/.upload-$ver"
remote "mkdir -p '$tmp_dir'"
log "uploading to $HOST:$tmp_dir (resumable: rerun if it stops)"
rsync -aL --partial --info=progress2 --timeout=180 -e "${SSH[*]}" "$stage/" "$HOST:$tmp_dir/"
log "verifying on the server"
remote "cd '$tmp_dir' && sha256sum -c --quiet SHA256SUMS" || die "checksum mismatch on the server: rerun to resume the upload"
remote "set -e
    old='$remote_dir.old-\$\$'
    [ -e '$remote_dir' ] && mv '$remote_dir' \"\$old\"
    mv '$tmp_dir' '$remote_dir'
    chmod 755 '$remote_dir'; chmod 644 '$remote_dir'/*
    rm -rf \"\$old\"
    cp '$remote_dir/release.json' '$REMOTE_ROOT/$OS_NAME/.latest-$ARCH.json.new'
    chmod 644 '$REMOTE_ROOT/$OS_NAME/.latest-$ARCH.json.new'
    mv '$REMOTE_ROOT/$OS_NAME/.latest-$ARCH.json.new' '$REMOTE_ROOT/$OS_NAME/latest-$ARCH.json'"

# Only this release stays.
old=$(old_versions "$ver")
if [[ -n "$old" ]]; then
    log "deleting the previous releases: $(echo $old)"
    remote "cd '$REMOTE_ROOT/$OS_NAME' && rm -rf $(printf "'%s' " $old)"
fi

log "published $OS_NAME $ver ($ARCH)"
echo "  $PUBLIC_URL/$OS_NAME/$ver/"
echo "  $PUBLIC_URL/$OS_NAME/latest-$ARCH.json"
rm -rf "$stage"
