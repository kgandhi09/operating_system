#!/usr/bin/env bash
# Install host packages needed to build, package, run and flash jk_os.
# Supports Debian/Ubuntu (apt) and Arch (pacman); other distros: install the
# equivalents by hand (see README.md).
set -euo pipefail

sudo=()
[[ $EUID -eq 0 ]] || sudo=(sudo)
host="$(uname -m)"

if command -v apt-get >/dev/null; then
    pkgs=(build-essential bc bison flex libelf-dev libssl-dev
          cpio bzip2 xz-utils zstd curl jq unzip file rsync
          meson ninja-build pkg-config gperf autoconf automake libtool python3-packaging squashfs-tools
          python3-mako python3-yaml python3-ply glslang-tools gettext xsltproc docbook-xsl
          grub-common xorriso mtools dosfstools
          qemu-system-x86 qemu-system-arm qemu-utils qemu-efi-aarch64 ovmf)
    case "$host" in
        x86_64)  pkgs+=(grub-pc-bin grub-efi-amd64-bin gcc-aarch64-linux-gnu libc6-dev-arm64-cross) ;;
        aarch64) pkgs+=(gcc-x86-64-linux-gnu libc6-dev-amd64-cross) ;;
    esac
    "${sudo[@]}" apt-get update
    "${sudo[@]}" apt-get install -y "${pkgs[@]}"
elif command -v pacman >/dev/null; then
    pkgs=(base-devel bc cpio bzip2 xz zstd curl jq unzip file rsync libelf openssl
          meson ninja pkgconf gperf autoconf automake libtool python-packaging squashfs-tools
          python-mako python-yaml python-ply glslang gettext libxslt docbook-xsl
          grub libisoburn mtools dosfstools
          qemu-system-x86 qemu-system-aarch64 edk2-ovmf edk2-aarch64)
    [[ "$host" == x86_64 ]] && pkgs+=(aarch64-linux-gnu-gcc aarch64-linux-gnu-glibc)
    "${sudo[@]}" pacman -S --needed --noconfirm "${pkgs[@]}"
else
    echo "unsupported package manager; install the packages listed in README.md" >&2
    exit 1
fi
