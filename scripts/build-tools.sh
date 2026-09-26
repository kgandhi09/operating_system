#!/usr/bin/env bash
# Build the static disk tools BusyBox lacks, for the installer (jk-install):
#   util-linux: sfdisk fdisk lsblk wipefs partx   (GPT partitioning)
#   e2fsprogs:  mke2fs e2fsck resize2fs tune2fs   (journaled ext4)
# Both build out of tree under build/<arch>/ and land in build/<arch>/tools/sbin.
source "$(dirname "$0")/common.sh"
need make "${CROSS_COMPILE}gcc"

for src in "$UTIL_LINUX_SRC" "$E2FSPROGS_SRC"; do
    [[ -x "$src/configure" ]] || die "no source at ${src#"$ROOT_DIR"/} (see versions.env)"
done

HOST_TRIPLE="$ARCH-linux-gnu"
UL_OUT="$OUT_DIR/util-linux"
E2_OUT="$OUT_DIR/e2fsprogs"
# fdisks=check: fdisk and sfdisk, but no cfdisk (it needs ncurses).
UL_PROGS=(sfdisk fdisk lsblk wipefs partx)
# Keep pkg-config from handing the host's shared libraries to a static cross build.
export PKG_CONFIG_LIBDIR=/nonexistent PKG_CONFIG_PATH=
export CC="${CROSS_COMPILE}gcc"

# configure_once <src> <out> <args...>: (re)configure when the tree or the
# arguments changed. The stamp is only written after a successful configure.
configure_once() {
    local src="$1" out="$2"; shift 2
    local stamp="$out/.jk_os-configured" want="$*"
    if [[ -f "$stamp" && "$stamp" -nt "$src/configure" && "$(cat "$stamp")" == "$want" ]]; then
        return
    fi
    log "configuring $(basename "$src") $(tree_version "$src") ($ARCH)"
    rm -rf "$out"; mkdir -p "$out"
    (cd "$out" && "$src/configure" --host="$HOST_TRIPLE" --prefix=/usr \
        --disable-nls --disable-shared --enable-static LDFLAGS=-static "$@" >configure.log 2>&1) \
        || { tail -n 20 "$out/configure.log" >&2; die "configure failed, see $out/configure.log"; }
    echo "$want" > "$stamp"
}

configure_once "$UTIL_LINUX_SRC" "$UL_OUT" \
    --disable-all-programs \
    --enable-libblkid --enable-libuuid --enable-libfdisk --enable-libsmartcols --enable-libmount \
    --enable-fdisks=check --enable-lsblk --enable-wipefs --enable-partx \
    --disable-liblastlog2 --disable-asciidoc --disable-poman --disable-bash-completion \
    --without-python --without-systemd --without-udev --without-ncursesw --without-ncurses \
    --without-tinfo --without-readline --without-cap-ng --without-libmagic --without-user \
    --without-btrfs --without-econf --without-selinux --without-audit
# qmake <dir> <targets...>: make with the (noisy) output kept in a log.
qmake() {
    local dir="$1"; shift
    make -C "$dir" -j"$JOBS" "$@" >>"$dir/build.log" 2>&1 \
        || { tail -n 30 "$dir/build.log" >&2; die "build failed, see $dir/build.log"; }
}

log "building util-linux ($ARCH)"
# libtool reads plain -static as "static libtool libraries" only.
qmake "$UL_OUT" LDFLAGS=-all-static "${UL_PROGS[@]}"

# --enable-libuuid/libblkid: use the copies bundled with e2fsprogs.
configure_once "$E2FSPROGS_SRC" "$E2_OUT" \
    --enable-libuuid --enable-libblkid --disable-fuse2fs --disable-uuidd \
    --disable-e2initrd-helper --disable-debugfs --disable-imager --disable-defrag \
    --without-libarchive
log "building e2fsprogs ($ARCH)"
qmake "$E2_OUT" libs
qmake "$E2_OUT/misc" mke2fs tune2fs
qmake "$E2_OUT/e2fsck" e2fsck
qmake "$E2_OUT/resize" resize2fs

rm -rf "$TOOLS_OUT"
mkdir -p "$TOOLS_OUT/sbin"
for p in "${UL_PROGS[@]}"; do install -m 0755 "$UL_OUT/$p" "$TOOLS_OUT/sbin/"; done
install -m 0755 "$E2_OUT/misc/mke2fs" "$E2_OUT/misc/tune2fs" \
    "$E2_OUT/e2fsck/e2fsck" "$E2_OUT/resize/resize2fs" "$TOOLS_OUT/sbin/"
"${CROSS_COMPILE}strip" "$TOOLS_OUT/sbin/"*
# Names the e2fsprogs tools answer to (they check argv[0]).
ln -s mke2fs  "$TOOLS_OUT/sbin/mkfs.ext4"
ln -s e2fsck  "$TOOLS_OUT/sbin/fsck.ext4"
ln -s tune2fs "$TOOLS_OUT/sbin/e2label"

if command -v file >/dev/null; then
    for f in "$TOOLS_OUT/sbin/"*; do
        [[ -L "$f" ]] && continue
        file -b "$f" | grep -qE 'statically linked|static-pie linked' || warn "$(basename "$f") is not static"
    done
fi
log "tools: $(cd "$TOOLS_OUT/sbin" && echo *)"
