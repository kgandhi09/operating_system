#!/usr/bin/env bash
# Build the static tools BusyBox lacks or does too little of:
#   util-linux: sfdisk fdisk lsblk wipefs partx   (GPT partitioning, jk-install)
#   e2fsprogs:  mke2fs e2fsck resize2fs tune2fs   (journaled ext4)
#   shadow:     useradd usermod passwd su login ... (user management), linked
#               against libxcrypt (yescrypt / sha512 password hashes)
#   sudo:       sudo visudo (group wheel runs commands as root)
# All build out of tree under build/<arch>/ and land in build/<arch>/tools.
source "$(dirname "$0")/common.sh"
need make "${CROSS_COMPILE}gcc"

for src in "$UTIL_LINUX_SRC" "$E2FSPROGS_SRC" "$SHADOW_SRC" "$LIBXCRYPT_SRC" "$SUDO_SRC"; do
    [[ -x "$src/configure" ]] || die "no source at ${src#"$ROOT_DIR"/} (see versions.env)"
done

HOST_TRIPLE="$ARCH-linux-gnu"
UL_OUT="$OUT_DIR/util-linux"
E2_OUT="$OUT_DIR/e2fsprogs"
XC_OUT="$OUT_DIR/libxcrypt"
SH_OUT="$OUT_DIR/shadow"
SU_OUT="$OUT_DIR/sudo"
SYSROOT="$OUT_DIR/sysroot"   # static libraries built here for later packages
# fdisks=check: fdisk and sfdisk, but no cfdisk (it needs ncurses).
UL_PROGS=(sfdisk fdisk lsblk wipefs partx)
# Account tools, root only (they land in sbin/) ...
SH_ADMIN=(useradd usermod userdel groupadd groupmod groupdel chpasswd chgpasswd
          newusers pwck grpck pwconv grpconv chage)
# ... and what users run on their own account (bin/; setuid root, but they
# only let a non-root user change themselves).
SH_USER=(passwd chfn chsh su login newgrp)
SH_PROGS=("${SH_ADMIN[@]}" "${SH_USER[@]}")
# Keep pkg-config from handing the host's shared libraries to a static cross build.
export PKG_CONFIG_LIBDIR=/nonexistent PKG_CONFIG_PATH=
export CC="${CROSS_COMPILE}gcc"

# configure_once <src> <out> <args...>: (re)configure when the tree or the
# arguments changed. The stamp is only written after a successful configure.
configure_once() {
    local src="$1" out="$2"; shift 2
    # The source path is part of it: a configured tree names its source by path.
    local stamp="$out/.jk_os-configured" want="$src $*"
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

# libxcrypt: only the static library, installed into the build sysroot.
configure_once "$LIBXCRYPT_SRC" "$XC_OUT" \
    --enable-hashes=strong,glibc --enable-obsolete-api=no --disable-failure-tokens \
    --disable-valgrind --disable-werror
log "building libxcrypt ($ARCH)"
qmake "$XC_OUT"
qmake "$XC_OUT" DESTDIR="$SYSROOT" install-libLTLIBRARIES install-nodist_includeHEADERS

# shadow: no PAM, so the tools read /etc/login.defs and hash with libxcrypt.
# nscd support stays on: shadow 4.20's --without-nscd stub doesn't compile, and
# with no nscd running the cache flush is a no-op.
configure_once "$SHADOW_SRC" "$SH_OUT" \
    --sysconfdir=/etc --with-su --with-yescrypt --without-libpam --without-audit \
    --without-selinux --without-acl --without-attr --without-btrfs --without-tcb \
    --without-sssd --without-libbsd --without-skey --disable-logind \
    --disable-subordinate-ids --disable-man --enable-shadowgrp \
    CPPFLAGS="-I$SYSROOT/usr/include" LDFLAGS="-static -L$SYSROOT/usr/lib"
log "building shadow ($ARCH)"
qmake "$SH_OUT/lib"
qmake "$SH_OUT/src" LDFLAGS="-all-static -L$SYSROOT/usr/lib" "${SH_PROGS[@]}"

# sudo: no PAM, so it checks the user's own password in /etc/shadow with
# libxcrypt. The sudoers policy is built into the binary, and the helpers
# that would be loaded as shared objects (noexec, intercept) are left out.
configure_once "$SUDO_SRC" "$SU_OUT" \
    --sysconfdir=/etc --libexecdir=/usr/lib --without-pam --enable-static-sudoers \
    --disable-shared-libutil --without-noexec --disable-intercept --disable-log-server \
    --disable-log-client --enable-zlib=no --disable-python --without-sendmail \
    --with-editor=/bin/vi --with-env-editor --with-logging=syslog --with-rundir=/run/sudo \
    --with-vardir=/var/lib/sudo --with-iologdir=/var/log/sudo-io \
    CPPFLAGS="-I$SYSROOT/usr/include" LDFLAGS="-static -L$SYSROOT/usr/lib"
log "building sudo ($ARCH)"
qmake "$SU_OUT" LDFLAGS="-all-static -L$SYSROOT/usr/lib"

rm -rf "$TOOLS_OUT"
mkdir -p "$TOOLS_OUT/sbin" "$TOOLS_OUT/bin"
for p in "${UL_PROGS[@]}"; do install -m 0755 "$UL_OUT/$p" "$TOOLS_OUT/sbin/"; done
install -m 0755 "$E2_OUT/misc/mke2fs" "$E2_OUT/misc/tune2fs" \
    "$E2_OUT/e2fsck/e2fsck" "$E2_OUT/resize/resize2fs" "$TOOLS_OUT/sbin/"
for p in "${SH_ADMIN[@]}"; do install -m 0755 "$SH_OUT/src/$p" "$TOOLS_OUT/sbin/"; done
for p in "${SH_USER[@]}"; do install -m 0755 "$SH_OUT/src/$p" "$TOOLS_OUT/bin/"; done
install -m 0755 "$SU_OUT/src/sudo" "$TOOLS_OUT/bin/"
install -m 0755 "$SU_OUT/plugins/sudoers/visudo" "$TOOLS_OUT/sbin/"
"${CROSS_COMPILE}strip" "$TOOLS_OUT/sbin/"* "$TOOLS_OUT/bin/"*
# setuid root: a user changes their own password, name or shell, switches to
# one of their own groups (newgrp), or becomes root with su or sudo (only
# group wheel: /etc/login.defs, /etc/sudoers). login is run by root (getty)
# and needs none.
chmod 4755 "$TOOLS_OUT/bin/"{passwd,chfn,chsh,su,newgrp,sudo}
# Names the e2fsprogs tools answer to (they check argv[0]).
ln -s mke2fs  "$TOOLS_OUT/sbin/mkfs.ext4"
ln -s e2fsck  "$TOOLS_OUT/sbin/fsck.ext4"
ln -s tune2fs "$TOOLS_OUT/sbin/e2label"

if command -v file >/dev/null; then
    for f in "$TOOLS_OUT/sbin/"* "$TOOLS_OUT/bin/"*; do
        [[ -L "$f" ]] && continue
        file -b "$f" | grep -qE 'statically linked|static-pie linked' || warn "$(basename "$f") is not static"
    done
fi
log "tools: $(cd "$TOOLS_OUT/sbin" && echo *) $(cd "$TOOLS_OUT/bin" && echo *)"
