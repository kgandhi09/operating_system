# jk_os

A minimal 64-bit Linux system built from source:

- the **official Linux kernel** from Linus Torvalds' mainline releases, kept in this repo as plain source,
- **BusyBox** as the entire userland (shell, init, coreutils, networking),
- no display manager, X or Wayland: text console on screen plus serial console.

It targets **x86_64** and **aarch64**. You get a bootable, hybrid ISO that
works from a CD/DVD, a VM, or written raw to a USB stick.

Every source tree lives in the repo, so **a build needs no network and no
git**: a copied folder or an unpacked archive of this repo builds offline.

## Quick start

```sh
make deps                 # once: install host packages (Debian/Ubuntu or Arch, uses sudo)
make                      # build out/jk_os-<ver>-<host arch>.iso
make run                  # boot it in QEMU on this terminal (Ctrl-A X quits)
```

Log in as `root` (no password). The system runs entirely from RAM, so changes
are lost on reboot.

### Other architecture

```sh
make ARCH=aarch64         # cross-compiles with aarch64-linux-gnu-gcc
make ARCH=aarch64 run
make ARCH=x86_64          # from an aarch64 host, with x86_64-linux-gnu-gcc
```

Override the toolchain with `CROSS_COMPILE=<prefix>` and the job count with `JOBS=N`.

### Flash to USB / SD card

```sh
lsblk -d -o NAME,SIZE,MODEL,TRAN      # find the stick, e.g. /dev/sdb
make flash DEVICE=/dev/sdb            # add ARCH=aarch64 for the arm image
```

`scripts/flash.sh` refuses partitions, mounted devices, and the disk holding
the running system. It asks you to type the device path before running `dd`
through `sudo`. **Everything on the target device is erased.**

## How it boots

| | x86_64 | aarch64 |
|---|---|---|
| Firmware | legacy BIOS or UEFI | UEFI (servers, QEMU `virt`, UEFI-capable boards) |
| Bootloader | GRUB (`grub-mkrescue`) | none: the kernel's EFI stub is `EFI/BOOT/BOOTAA64.EFI` |
| Kernel | `bzImage` | `Image` |
| Console | `tty0` + `ttyS0` (GRUB menu picks which is primary) | from firmware (DT `stdout-path` / ACPI SPCR) + `tty1` |

The kernel is built with **no loadable modules**, and the root filesystem is
its **built-in initramfs**. The whole OS is therefore one kernel file.
BusyBox `init` runs `/etc/init.d/rcS`, which mounts `/proc`, `/sys`, `/dev`,
and so on, then runs `/etc/init.d/S??*` (syslog, DHCP on wired NICs). `getty`
then starts on each console that actually exists.

## Layout

```
Makefile                  entry point (see `make help`)
versions.env              OS name/version, which source tree each arch uses
kernel/mainline/          Linux source, Torvalds' mainline release (plain files)
userland/busybox/         BusyBox source (plain files)
configs/kernel/           kernel config fragments merged over the arch defconfig
configs/busybox/          BusyBox config fragments merged over defconfig
configs/initramfs.list    device nodes added to the initramfs
rootfs/                   files copied verbatim into the root filesystem
boot/grub.cfg             x86_64 GRUB menu
scripts/                  one script per build step
build/                    (generated) per-arch build trees
out/                      (generated) ISO images
```

Build steps can run individually: `make busybox | rootfs | kernel | iso`.
The kernel and BusyBox build out of tree under `build/<arch>/`. The source
trees stay pristine, and both architectures can build side by side from the
same source.

## Sources

`versions.env` chooses the kernel tree for each architecture:

```sh
KERNEL_TREE_x86_64=kernel/mainline
KERNEL_TREE_aarch64=kernel/mainline
BUSYBOX_TREE=userland/busybox
```

Both x86_64 and aarch64 are fully supported by Torvalds' mainline kernel, so
they share `kernel/mainline` (currently Linux 7.2). A tree's version is just
what it contains; the build reads it from the tree's `Makefile`.

**Move to a new release.** `scripts/update-source.sh` is a maintainer tool;
builds never call it. It downloads the release from cdn.kernel.org or
busybox.net, checks the published SHA-256 sum, and swaps the tree in place:

```sh
scripts/update-source.sh kernel 7.3                 # kernel/mainline -> 7.3
scripts/update-source.sh busybox 1.38.0
scripts/update-source.sh kernel ./linux-7.3.tar.xz  # offline, from a tarball you have
make clean && make
```

**Add a different kernel for an arch** that mainline can't boot, such as a
vendor tree for a specific board. Unpack it as `kernel/<name>`, or use
`scripts/update-source.sh kernel <tarball> kernel/<name>`, then set
`KERNEL_TREE_<arch>=kernel/<name>`.

**Committing a tree.** Use `git add -f kernel/<tree>`. The kernel's own
`.gitignore` matches a few files that upstream ships anyway (for example
`Documentation/.renames.txt`), and a plain `git add` would silently skip them.
The trees never collect build output, because everything builds out of tree
in `build/`.

## Customizing

- **Add files to the OS**: drop them under `rootfs/` (e.g. `rootfs/etc/init.d/S50myapp`, executable, taking `start`/`stop`), then `make`.
- **Kernel options**: `make menuconfig` to explore, then put the options you want to keep in `configs/kernel/common.config` or `configs/kernel/<arch>.config`. Fragments are re-applied whenever they change. The build warns if an option you asked for was dropped.
- **Clean**: `make clean` (current arch) or `make distclean` (all of `build/` and `out/`; source trees are left alone).

## Host requirements

`make deps` installs these on Debian/Ubuntu (apt) or Arch (pacman). On other
distros, install the equivalents:

- build: `gcc make bc flex bison libelf-dev libssl-dev cpio file`
- updating sources (optional): `curl tar xz bzip2`
- cross: `gcc-aarch64-linux-gnu libc6-dev-arm64-cross` (a glibc cross toolchain; BusyBox links statically against it)
- ISO: `xorriso mtools dosfstools`, and on x86_64 hosts `grub-pc-bin grub-efi-amd64-bin`
- run: `qemu-system-x86 qemu-system-arm ovmf qemu-efi-aarch64`

Building the x86_64 ISO needs GRUB's i386-pc / x86_64-efi platform files.
Distros usually ship these only for x86 hosts, so build x86_64 images on an
x86_64 machine to get both BIOS and UEFI boot. The aarch64 ISO needs no GRUB,
so it can be built on any host.
