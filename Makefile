# jk_os — a minimal 64-bit Linux system built from source.
#
#   make                         build the ISO for the host arch
#   make ARCH=aarch64            ... or for another arch (x86_64, aarch64)
#   make run [UEFI=1]            boot the ISO in QEMU (Ctrl-A X quits)
#   make flash DEVICE=/dev/sdX   write the ISO to a USB stick / SD card
#   make deps                    install host build dependencies (sudo)
#
# Steps can be run on their own: busybox, rootfs, kernel, iso.
# Everything builds from the source trees in this repo: no network, no git.

ARCH ?= $(shell uname -m)
export ARCH

SHELL := /bin/bash
S := scripts

.PHONY: all iso kernel rootfs busybox run flash deps menuconfig clean distclean help

all: iso

busybox:
	$(S)/build-busybox.sh

rootfs: busybox
	$(S)/build-rootfs.sh

kernel: rootfs
	$(S)/build-kernel.sh

iso: kernel
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
	@sed -n '1,11p' Makefile | sed 's/^# \{0,1\}//'
