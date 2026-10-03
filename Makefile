# jk_os — a minimal 64-bit Linux system built from source.
#
#   make config                  choose what to build for: device category,
#                                device, kernel, arch and a name (build.conf)
#   make showconfig              show the chosen target
#   make                         build it (without build.conf: the generic PC
#                                for the host arch)
#   make ARCH=aarch64            ... the generic PC for another arch
#   make run [UEFI=1]            boot the ISO in QEMU (Ctrl-A X quits)
#   make flash DEVICE=/dev/sdX   write the ISO to a USB stick / SD card
#   make deps                    install host build dependencies (sudo)
#
# Steps can be run on their own: busybox, tools, network, toolchain, desktop,
# binaries, rootfs (the OS image), initramfs, kernel, iso.
#   make binaries [UPDATE=1]     pull configs/binaries/*.list from GitHub
#   make menuconfig              the kernel's own config menu (after make kernel)
#   make BUILD_CONF=<file> ...   use another saved target than build.conf
# Everything builds from what is in this repo: no network, no git (except
# `make binaries` for a binary that is listed but not fetched yet).

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

.PHONY: all check iso kernel nvidia initramfs rootfs busybox tools network toolchain desktop binaries run flash deps config showconfig menuconfig clean distclean help

all: iso

# The chosen target (build.conf) must be one jk_os can build. Every build
# chain starts at one of the steps that depend on this.
check:
	@$(S)/configure.sh --check

busybox: check
	$(S)/build-busybox.sh

tools: check
	$(S)/build-tools.sh

network: check
	$(S)/build-network.sh

toolchain: network
	$(S)/build-toolchain.sh

# The desktop (KDE Plasma), started on demand with jk-gui.
desktop: toolchain network
	$(S)/build-desktop.sh

binaries: check
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

config:
	$(S)/configure.sh

showconfig:
	$(S)/configure.sh --show

# Interactive kernel config for the current target (after one `make kernel`).
# Copy options you want to keep into configs/kernel/*.config.
menuconfig:
	. $(S)/common.sh && make -C "$$KERNEL_SRC" O="$$KERNEL_OUT" ARCH="$$KARCH" \
	    CROSS_COMPILE="$$CROSS_COMPILE" menuconfig

clean:
	. $(S)/common.sh && rm -rf "$$OUT_DIR" "$$ISO"

distclean:
	rm -rf build out

help:
	@sed -n '1,19p' Makefile | sed 's/^# \{0,1\}//'
