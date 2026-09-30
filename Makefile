# jk_os — a minimal 64-bit Linux system built from source.
#
#   make                         build the ISO for the host arch
#   make ARCH=aarch64            ... or for another arch (x86_64, aarch64)
#   make run [UEFI=1]            boot the ISO in QEMU (Ctrl-A X quits)
#   make flash DEVICE=/dev/sdX   write the ISO to a USB stick / SD card
#   make deps                    install host build dependencies (sudo)
#
# Steps can be run on their own: busybox, tools, network, toolchain, desktop,
# binaries, rootfs (the OS image), initramfs, kernel, iso.
#   make binaries [UPDATE=1]     pull configs/binaries/*.list from GitHub
# Everything builds from what is in this repo: no network, no git (except
# `make binaries` for a binary that is listed but not fetched yet).

ARCH ?= $(shell uname -m)
export ARCH

# Warnings the vendored C code (BusyBox) triggers with current GCC: unchecked
# write/chown results, const dropped from strchr() results, sprintf/snprintf size
# estimates, variables GCC can't prove initialized, a pointer GCC thinks may
# point at a local. Known and harmless; they would drown out new ones.
# Override with: make NOWARN_CFLAGS=
NOWARN_CFLAGS ?= -Wno-unused-result -Wno-discarded-qualifiers -Wno-format-overflow -Wno-uninitialized \
                 -Wno-format-truncation -Wno-return-local-addr
export NOWARN_CFLAGS

SHELL := /bin/bash
S := scripts

.PHONY: all iso kernel nvidia initramfs rootfs busybox tools network toolchain desktop binaries run flash deps menuconfig clean distclean help

all: iso

busybox:
	$(S)/build-busybox.sh

tools:
	$(S)/build-tools.sh

network:
	$(S)/build-network.sh

toolchain: network
	$(S)/build-toolchain.sh

# The desktop (KDE Plasma), started on demand with jk-gui on tty2.
desktop: toolchain network
	$(S)/build-desktop.sh

binaries:
	$(S)/fetch-binaries.sh

# NVIDIA's driver, built against the kernel (userspace/nvidia/<arch>).
nvidia: kernel
	$(S)/build-nvidia.sh

rootfs: busybox tools network toolchain desktop binaries nvidia
	$(S)/build-rootfs.sh

initramfs: busybox tools
	$(S)/build-initramfs.sh

kernel: initramfs
	$(S)/build-kernel.sh

iso: kernel rootfs
	$(S)/build-iso.sh

run:
	$(S)/run-qemu.sh

flash:
	@test -n "$(DEVICE)" || { echo "usage: make flash DEVICE=/dev/sdX [ARCH=...]"; exit 1; }
	$(S)/flash.sh $(DEVICE)

deps:
	$(S)/install-deps.sh

# Interactive kernel config for the current arch (after one `make kernel`).
# Copy options you want to keep into configs/kernel/*.config.
menuconfig:
	. $(S)/common.sh && make -C "$$KERNEL_SRC" O="$$KERNEL_OUT" ARCH="$$KARCH" \
	    CROSS_COMPILE="$$CROSS_COMPILE" menuconfig

clean:
	. $(S)/common.sh && rm -rf "$$OUT_DIR" "$$ISO"

distclean:
	rm -rf build out

help:
	@sed -n '1,13p' Makefile | sed 's/^# \{0,1\}//'
