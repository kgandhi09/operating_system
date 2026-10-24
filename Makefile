# jk_os — a minimal 64-bit Linux system built from source.
#
# Every build names its target, a preset (config/presets/<preset>.conf:
# device category, device, kernel, arch and a name); make without one stops.
#   make presets                          list the presets
#   make config [PRESET=<preset>]         make a new preset, or change one
#   make PRESET=<preset>                  build its image (ISO or boot image)
#   make PRESET=<preset> showconfig       show what it builds
#   make PRESET=<preset> run [UEFI=1]     boot its ISO in QEMU (Ctrl-A X quits)
#   make PRESET=<preset> flash DEVICE=/dev/sdX
#                                         write its ISO to a USB stick / SD card
#                                         (Android devices: over USB, no DEVICE)
#
# A build machine needs only what every Linux has (a C/C++ compiler, make,
# bash, the basic file tools, git); the build makes everything else itself,
# first (docs/self-hosting.md). jk_os builds on Ubuntu and the like, and on
# a jk_os PC or Jetson; not on a jk_os tablet.
#
# Steps can be run on their own, with PRESET too: hosttools (the build
# machine's tools), cross (the cross compiler and glibc), busybox, tools,
# network, toolchain, desktop, binaries, rootfs (the OS image), initramfs,
# kernel, uboot, iso, bootimg, clean.
#   make PRESET=<preset> prune            free disk space after a build: delete
#                                         finished packages' work directories
#                                         (nothing has to be rebuilt)
#   make PRESET=<preset> binaries [UPDATE=1]
#                                         pull config/components/binaries/*.list
#   make PRESET=<preset> menuconfig       the kernel's config menu (after kernel)
#   make BUILD_CONF=<file> ...            a target file kept elsewhere
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

.PHONY: all check hosttools cross image iso bootimg uboot kernel nvidia initramfs rootfs busybox tools network toolchain desktop binaries run flash config presets showconfig menuconfig prune clean distclean help

all: image

# The chosen preset (PRESET=) must be one jk_os can build; without one this
# stops with how to use make. Every build chain starts at one of the steps
# that depend on this.
check:
	@$(S)/configure.sh --check

# What the build runs on this machine, from this repo's sources (shared by
# every preset and arch built here).
hosttools: check
	$(S)/build-hosttools.sh

# jk_os's cross compiler for the preset's arch, and glibc: every later step
# compiles the target's code with them.
cross: hosttools
	$(S)/build-cross.sh

busybox: cross
	$(S)/build-busybox.sh

tools: cross
	$(S)/build-tools.sh

network: cross
	$(S)/build-network.sh

toolchain: network
	$(S)/build-toolchain.sh

# The desktop (KDE Plasma), started on demand with jk-gui.
desktop: toolchain network
	$(S)/build-desktop.sh

binaries: hosttools
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

# The bootable image in the device's boot format (DEVICE_BOOT, make config):
# the installer ISO, or an Android boot image (bootimg) for Odin.
image: kernel uboot rootfs
	@. $(S)/common.sh && case "$$DEVICE_BOOT" in \
	    efi-iso) $(S)/build-iso.sh ;; \
	    android-*) $(S)/build-bootimg.sh ;; \
	esac

iso: kernel rootfs
	$(S)/build-iso.sh

# Only the kernel and its device tree, in an Android boot image: enough to
# bring up a new device before the whole OS image is built.
bootimg: kernel uboot
	$(S)/build-bootimg.sh

# U-Boot, for a device that boots through it (DEVICE_BOOT=android-uboot);
# nothing for the others.
uboot: kernel
	$(S)/build-uboot.sh

run: hosttools
	$(S)/run-qemu.sh

# A disk (DEVICE=/dev/sdX) for the ISO; a Samsung device in Download mode
# over USB (Heimdall) for an Android boot image.
flash: hosttools
	$(S)/flash.sh $(DEVICE)

config:
	$(S)/configure.sh

presets:
	@$(S)/configure.sh --presets

showconfig:
	$(S)/configure.sh --show

# Interactive kernel config for the current target (after one `make kernel`).
# Copy options you want to keep into config/components/kernel/*.config.
menuconfig:
	. $(S)/common.sh && make -C "$$KERNEL_SRC" O="$$KERNEL_OUT" ARCH="$$KARCH" \
	    CROSS_COMPILE="$$CROSS_COMPILE" menuconfig

# The work directories of packages that finished building (scripts/prune.sh).
prune: check
	$(S)/prune.sh

clean:
	. $(S)/common.sh && rm -rf "$$OUT_DIR" "$$ISO"

distclean:
	rm -rf build out

help:
	@sed -n '1,/^$$/p' Makefile | sed '$$d; s/^# \{0,1\}//'
