#!/bin/bash

# Ensure the script exits on errors
set -e

# Variables
KERNEL_IMAGE="bzImage"
INITRAMFS_IMAGE="initramfs.gz"
ISO_OUTPUT="bootable.iso"
WORK_DIR="iso_build"
GRUB_DIR="/usr/lib/grub/i386-pc"  # Update this if GRUB is installed elsewhere

# Check for required tools
if ! command -v grub-mkrescue >/dev/null; then
    echo "Error: grub-mkrescue is not installed. Install it with 'sudo apt install grub-pc-bin grub-common xorriso'."
    exit 1
fi

# Ensure required files exist
if [[ ! -f "$KERNEL_IMAGE" ]]; then
    echo "Error: Kernel image '$KERNEL_IMAGE' not found."
    exit 1
fi

if [[ ! -f "$INITRAMFS_IMAGE" ]]; then
    echo "Error: Initramfs image '$INITRAMFS_IMAGE' not found."
    exit 1
fi

# Clean up any previous build
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR/boot/grub"

# Copy kernel and initramfs
cp "$KERNEL_IMAGE" "$WORK_DIR/boot/"
cp "$INITRAMFS_IMAGE" "$WORK_DIR/boot/"

# Create GRUB configuration file
cat > "$WORK_DIR/boot/grub/grub.cfg" <<EOF
set timeout=5
set default=0

menuentry "Minimal Linux" {
    linux /boot/$(basename "$KERNEL_IMAGE")
    initrd /boot/$(basename "$INITRAMFS_IMAGE")
}
EOF

# Build the ISO
grub-mkrescue -o "$ISO_OUTPUT" "$WORK_DIR" --compress=xz

# Clean up the temporary directory
rm -rf "$WORK_DIR"

echo "Bootable ISO created: $ISO_OUTPUT"

