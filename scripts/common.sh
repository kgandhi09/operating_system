#!/usr/bin/env bash
# Shared settings for every jk_os build step. Source it, don't run it.
#
# Inputs (environment):
#   ARCH           x86_64 | aarch64 (aliases: amd64, arm64). Default: host arch.
#   CROSS_COMPILE  toolchain prefix. Default: none when building for the host
#                  arch, otherwise x86_64-linux-gnu- / aarch64-linux-gnu-.
#   JOBS           parallel build jobs. Default: half the CPUs, so the machine
#                  stays usable while jk_os builds.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck source=../versions.env
source "$ROOT_DIR/versions.env"

log()  { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33mwarning:\033[0m %s\n' "$*" >&2; }
die()  { printf '\033[1;31merror:\033[0m %s\n' "$*" >&2; exit 1; }
need() {
    local c
    for c in "$@"; do
        command -v "$c" >/dev/null 2>&1 || die "missing '$c' (run: make deps)"
    done
}

normalize_arch() {
    case "$1" in
        x86_64|amd64)  echo x86_64 ;;
        aarch64|arm64) echo aarch64 ;;
        *) die "unsupported arch '$1' (use x86_64 or aarch64)" ;;
    esac
}

HOST_ARCH="$(normalize_arch "$(uname -m)")"
ARCH="$(normalize_arch "${ARCH:-$HOST_ARCH}")"

case "$ARCH" in
    x86_64)
        KARCH=x86
        KIMAGE=arch/x86/boot/bzImage
        CROSS_DEFAULT=x86_64-linux-gnu-
        ;;
    aarch64)
        KARCH=arm64
        # The uncompressed arm64 Image is itself a valid EFI application.
        KIMAGE=arch/arm64/boot/Image
        CROSS_DEFAULT=aarch64-linux-gnu-
        ;;
esac

if [[ -z "${CROSS_COMPILE+set}" ]]; then
    if [[ "$ARCH" == "$HOST_ARCH" ]]; then CROSS_COMPILE=""; else CROSS_COMPILE="$CROSS_DEFAULT"; fi
fi
JOBS="${JOBS:-$(( $(nproc) > 1 ? $(nproc) / 2 : 1 ))}"

OUT_DIR="$ROOT_DIR/build/$ARCH"

tree_var="KERNEL_TREE_$ARCH"
[[ -n "${!tree_var:-}" ]] || die "$tree_var is not set in versions.env"
KERNEL_TREE="${!tree_var}"
KERNEL_SRC="$ROOT_DIR/$KERNEL_TREE"
KERNEL_OUT="$OUT_DIR/linux"
BUSYBOX_SRC="$ROOT_DIR/$BUSYBOX_TREE"
BUSYBOX_OUT="$OUT_DIR/busybox"
UTIL_LINUX_SRC="$ROOT_DIR/$UTIL_LINUX_TREE"
E2FSPROGS_SRC="$ROOT_DIR/$E2FSPROGS_TREE"
SHADOW_SRC="$ROOT_DIR/$SHADOW_TREE"
LIBXCRYPT_SRC="$ROOT_DIR/$LIBXCRYPT_TREE"
SUDO_SRC="$ROOT_DIR/$SUDO_TREE"
# Network stack (scripts/build-network.sh): NetworkManager and what it needs.
NET_TREES=(ZLIB LIBFFI PCRE2 GLIB EXPAT DBUS EUDEV LIBNDP LIBNL OPENSSL WPA_SUPPLICANT NCURSES READLINE NETWORKMANAGER)
for t in "${NET_TREES[@]}"; do
    v="${t}_TREE"; declare "${t}_SRC=$ROOT_DIR/${!v}"
done
# Toolchain (scripts/build-toolchain.sh).
TC_TREES=(GCC BINUTILS GDB GMP MPFR MPC LLVM CMAKE NINJA)
for t in "${TC_TREES[@]}"; do
    v="${t}_TREE"; declare "${t}_SRC=$ROOT_DIR/${!v}"
done
unset t v
# Disk tools (util-linux, e2fsprogs) are installed here, then into the rootfs.
TOOLS_OUT="$OUT_DIR/tools"
ROOTFS_DIR="$OUT_DIR/rootfs"
# The OS image made from it, and the small initramfs built into the kernel.
SQUASHFS_IMG="$OUT_DIR/jk_os.squashfs"
INITRAMFS_DIR="$OUT_DIR/initramfs"
# Prebuilt binaries from GitHub releases (scripts/fetch-binaries.sh).
BINARIES_DIR="$ROOT_DIR/userspace/binaries/$ARCH"
ISO_DIR="$OUT_DIR/iso"

# tree_version <dir>: "7.2", "1.37.0", ... Autotools release tarballs record
# it in .tarball-version (util-linux), version.h (e2fsprogs) or configure's
# PACKAGE_VERSION; Linux and BusyBox in a Kbuild-style Makefile
# (VERSION/PATCHLEVEL/SUBLEVEL).
tree_version() {
    if [[ -f "$1/.jk_os-version" ]]; then cat "$1/.jk_os-version"; return; fi   # fetched by git tag
    if [[ -f "$1/gcc/BASE-VER" ]]; then cat "$1/gcc/BASE-VER"; return; fi       # GCC
    if [[ -f "$1/gdb/version.in" ]]; then cat "$1/gdb/version.in"; return; fi   # GDB
    if [[ -f "$1/binutils/configure" ]]; then                                   # binutils
        sed -n "s/^PACKAGE_VERSION='\\(.*\\)'$/\\1/p" "$1/binutils/configure" | head -n1; return
    fi
    if [[ -f "$1/Source/CMakeVersion.cmake" ]]; then                           # CMake
        sed -n 's/^set(CMake_VERSION_\(MAJOR\|MINOR\|PATCH\) \([0-9]*\))/\2/p' "$1/Source/CMakeVersion.cmake" \
            | paste -sd.; return
    fi
    if [[ -f "$1/cmake/Modules/LLVMVersion.cmake" ]]; then                      # llvm-project
        sed -n 's/^ *set(LLVM_VERSION_\(MAJOR\|MINOR\|PATCH\) \([0-9]*\))/\2/p' \
            "$1/cmake/Modules/LLVMVersion.cmake" | paste -sd.; return
    fi
    if [[ -f "$1/.tarball-version" ]]; then cat "$1/.tarball-version"; return; fi
    if [[ -f "$1/VERSION.dat" ]]; then                                          # OpenSSL
        awk -F= '{ v[$1] = $2 } END { print v["MAJOR"] "." v["MINOR"] "." v["PATCH"] }' "$1/VERSION.dat"; return
    fi
    if [[ -f "$1/zlib.h" ]]; then
        sed -n 's/^#define ZLIB_VERSION "\(.*\)"/\1/p' "$1/zlib.h"; return
    fi
    if [[ ! -f "$1/Makefile" && -f "$1/meson.build" ]]; then                  # project(..., version: 'x')
        sed -n "1,40s/^[[:space:]]*version[[:space:]]*:[[:space:]]*'\\([0-9][^']*\\)'.*/\\1/p" "$1/meson.build" | head -n1; return
    fi
    if [[ -f "$1/version.h" ]]; then
        sed -n 's/^#define E2FSPROGS_VERSION "\(.*\)"/\1/p' "$1/version.h"; return
    fi
    if [[ ! -f "$1/Makefile" && -f "$1/configure" ]]; then   # any other autoconf tarball
        sed -n "s/^PACKAGE_VERSION='\\(.*\\)'$/\\1/p" "$1/configure" | head -n1; return
    fi
    [[ -f "$1/Makefile" ]] || { echo unknown; return; }
    awk -F' *= *' '
        $1 == "VERSION"      { v = $2 }
        $1 == "PATCHLEVEL"   { p = $2 }
        $1 == "SUBLEVEL"     { s = $2 }
        $1 == "EXTRAVERSION" { e = $2 }
        /^$/ && v != ""      { exit }
        END { printf "%s.%s", v, p; if (s != "") printf ".%s", s; print e }
    ' "$1/Makefile"
}

IMAGE_DIR="$ROOT_DIR/out"
ISO="$IMAGE_DIR/$OS_NAME-$OS_VERSION-$ARCH.iso"

export ARCH CROSS_COMPILE

