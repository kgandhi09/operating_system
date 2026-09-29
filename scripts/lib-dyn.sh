# Shared by build-network.sh and build-desktop.sh (source it after
# common.sh): packages built as shared libraries into build/<arch>/dyn, the
# install root the image's /usr is copied from.
#
# Set before sourcing:
#   DYN_WORK       build directory under build/<arch> (net, desktop, ...)
#   DYN_CC/DYN_CXX compilers (default: ${CROSS_COMPILE}gcc / g++)
#   DYN_SYSROOT    extra sysroot to find (and rpath-link) libraries in

HOST_TRIPLE="$ARCH-linux-gnu"
NET_OUT="$OUT_DIR/${DYN_WORK:?}"   # build directories
DYN="$OUT_DIR/dyn"                # install root (DESTDIR); the image's /usr comes from here
mkdir -p "$NET_OUT" "$DYN"

export CC="${DYN_CC:-${CROSS_COMPILE}gcc}" CXX="${DYN_CXX:-${CROSS_COMPILE}g++}"
export AR="${DYN_AR:-${CROSS_COMPILE}ar}" RANLIB="${DYN_RANLIB:-${CROSS_COMPILE}ranlib}"
export STRIP="${DYN_STRIP:-${CROSS_COMPILE}strip}"
# Find (and link) only what was built here, never the host's libraries.
export PKG_CONFIG_SYSROOT_DIR="$DYN" PKG_CONFIG_PATH=
export PKG_CONFIG_LIBDIR="$DYN/usr/lib/pkgconfig:$DYN/usr/share/pkgconfig"
export CPPFLAGS="-I$DYN/usr/include"
export LDFLAGS="-L$DYN/usr/lib -Wl,-rpath-link,$DYN/usr/lib"
AUTOCONF_ARGS=(--host="$HOST_TRIPLE" --prefix=/usr --libdir=/usr/lib --sysconfdir=/etc
               --localstatedir=/var --disable-static)

case "$ARCH" in
    x86_64)  MESON_CPU=x86_64;  OPENSSL_TARGET=linux-x86_64 ;;
    aarch64) MESON_CPU=aarch64; OPENSSL_TARGET=linux-aarch64 ;;
esac

# Meson cross file: target compiler, and pkg-config inside $DYN.
CROSS_FILE="$NET_OUT/meson-cross.ini"
cat > "$CROSS_FILE.new" <<EOF
[binaries]
c = '$CC'
cpp = '$CXX'
ar = '$AR'
strip = '$STRIP'
pkg-config = 'pkg-config'
# GLib's code generators are Python scripts: the ones in \$DYN run on the host.
glib-mkenums = ['python3', '$DYN/usr/bin/glib-mkenums']
glib-genmarshal = ['python3', '$DYN/usr/bin/glib-genmarshal']
gdbus-codegen = ['env', 'PYTHONPATH=$DYN/usr/share/glib-2.0', 'python3', '$DYN/usr/bin/gdbus-codegen']

[properties]
sys_root = '$DYN'
pkg_config_libdir = ['$DYN/usr/lib/pkgconfig', '$DYN/usr/share/pkgconfig']

[built-in options]
c_args = ['-I$DYN/usr/include']
c_link_args = ['-L$DYN/usr/lib', '-Wl,-rpath-link,$DYN/usr/lib']

[host_machine]
system = 'linux'
cpu_family = '$MESON_CPU'
cpu = '$MESON_CPU'
endian = 'little'
EOF
cmp -s "$CROSS_FILE.new" "$CROSS_FILE" && rm "$CROSS_FILE.new" || mv "$CROSS_FILE.new" "$CROSS_FILE"

# Build machine side: GLib's code generators are Python scripts, so the ones
# built into $DYN run on the host too (NetworkManager needs them).
NATIVE_FILE="$NET_OUT/meson-native.ini"
cat > "$NATIVE_FILE" <<EOF
[binaries]
glib-mkenums = ['python3', '$DYN/usr/bin/glib-mkenums']
glib-genmarshal = ['python3', '$DYN/usr/bin/glib-genmarshal']
gdbus-codegen = ['env', 'PYTHONPATH=$DYN/usr/share/glib-2.0', 'python3', '$DYN/usr/bin/gdbus-codegen']
EOF

# step <name> <src> <function> <options...>: run "<function> <src> <build dir>
# <options...>" unless the same options were already built from this tree.
step() {
    local name="$1" src="$2" fn="$3"; shift 3
    local out="$NET_OUT/$name" stamp="$NET_OUT/$name.done" want
    want="$(tree_version "$src") $*"
    if [[ -f "$stamp" && "$stamp" -nt "$src" && "$(cat "$stamp")" == "$want" ]]; then
        return
    fi
    log "building $name $(tree_version "$src") ($ARCH)"
    rm -rf "$out"; mkdir -p "$out"
    # A subshell with errexit, outside any "if": the first failing command stops it.
    local rc=0
    set +e
    ( set -e; "$fn" "$src" "$out" "$@" ) >"$out.log" 2>&1
    rc=$?
    set -e
    if (( rc != 0 )); then
        tail -n 30 "$out.log" >&2
        die "$name failed, see $out.log"
    fi
    echo "$want" > "$stamp"
}

# copy_tree: packages that build only inside their source tree.
copy_tree() { cp -a "$1/." "$2/"; }

autotools() {   # autotools <src> <out> <configure args...>
    local src="$1" out="$2"; shift 2
    (cd "$out" && "$src/configure" "${AUTOCONF_ARGS[@]}" "$@")
    make -C "$out" -j"$JOBS"
    make -C "$out" install DESTDIR="$DYN"
    # libtool archives name /usr/lib paths, which later links would take
    # from the build host; pkg-config carries everything they would.
    find "$DYN/usr/lib" -name '*.la' -delete
}

mesonpkg() {    # mesonpkg <src> <out> <meson options...>
    local src="$1" out="$2"; shift 2
    # The cross file points pkg-config at $DYN for the target; the variables
    # are unset so they don't also redirect lookups of build-host tools.
    env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_LIBDIR \
    ${MESON:-meson} setup "$out" "$src" --cross-file "$CROSS_FILE" --native-file "$NATIVE_FILE" \
        --prefix=/usr --libdir=lib --sysconfdir=/etc --localstatedir=/var \
        --buildtype=release --wrap-mode=nodownload -Ddefault_library=shared "$@"
    ninja -C "$out" -j"$JOBS"
    DESTDIR="$DYN" ${MESON:-meson} install -C "$out" --no-rebuild
}

