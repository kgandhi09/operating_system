# Building jk_os on jk_os

jk_os is developed, built and released from jk_os itself as much as from
Ubuntu: the build takes nothing from a package manager (neither the build
machine's apt nor jk_os's), so it works the same on any of them.

## Rules

1. **Build machines.** Any Linux with the floor below (Ubuntu, Debian, ...),
   and an installed jk_os whose device category allows it:
   `CATEGORY_BUILD_HOST=yes` in `config/targets/devices/<category>/category.env`
   (pc: x86_64 and aarch64 PCs; robotics-hpc: Jetson).
2. **Refused.** jk_os on tablets and phones (any category without
   `CATEGORY_BUILD_HOST=yes`) and the live system (the installer USB stick):
   every script stops at once and says why (`scripts/lib-host.sh`).
3. **Every build machine builds every preset**, natively for its own arch
   and cross-compiled for the other (x86_64 and aarch64 both ways).
4. **Nothing from a package manager.** What the build needs beyond the floor
   is built from the sources in this repo, once per build machine, into
   `build/host/<arch>/` (the host tools stage).
5. **Same output wherever it's built.** The cross compiler and glibc are
   built from this repo's sources on every build machine, and every target
   package is built with them.

## The floor

What a build machine must have, and all it must have: what every minimal
Linux system has.

- a C and C++ compiler (GCC), GNU make, bash
- coreutils, sed, grep, awk, find, xargs, tar, gzip, xz, bzip2, patch, diff,
  cmp, flock (BusyBox's are enough: the build makes GNU's first)
- glibc's `localedef` (only to cross-build: it compiles the other arch's
  C.UTF-8 locale)
- git and git-lfs, to get this repo

On Ubuntu: `sudo apt install build-essential git git-lfs`. A jk_os PC or
Jetson has all of it (git-lfs too: /usr/local/bin, set up for every user in
/etc/gitconfig). Every script checks the floor first and names what's
missing.

## How a build goes

```
make PRESET=<preset>
  hosttools   build-hosttools.sh  the build machine's tools, into build/host/<arch>/
  cross       build-cross.sh      jk_os's cross compiler and glibc, for the preset's arch
  busybox, tools, network, toolchain, desktop, ...   all compiled with that compiler
  kernel, uboot, rootfs, iso / bootimg
```

The first two are shared: `hosttools` by every preset built on the machine,
`cross` by every preset of an arch. Both build once and are skipped after
(each piece has its stamp, as everywhere in the build).

## The host tools stage

`scripts/build-hosttools.sh` (make step `hosttools`) builds, with the
floor's compiler, everything the build runs on the build machine, into
`build/host/<arch>/install` (its `bin` comes first on `PATH` in every
script, `common.sh`), and the build machine's own LLVM into
`build/host/<arch>/llvm`:

| For | Tools |
|---|---|
| Everything after | the GNU basics: coreutils, sed, grep, gawk, findutils, diffutils, patch, tar, gzip |
| The build itself | Python (with the modules the build runs: Mesa's mako, PyYAML, ply, pycparser, packaging; Jinja2 for elogind; lxml for breeze-icons), Perl, pkg-config (pkgconf), ninja, CMake, meson, m4, bison, flex, gperf, autoconf, automake, libtool, gettext, libxml2, libxslt, bc and cpio (BusyBox) |
| Kernel, U-Boot host programs | OpenSSL, libelf, zlib, bzip2, xz, zstd, lz4 |
| The OS image and the ISO | squashfs-tools, xorriso, mtools, dosfstools (GRUB: built by `build-iso.sh`, below) |
| Device trees, Android images | dtc (`fdtget`, `fdtoverlay`), debugfs |
| The desktop build | GLib's and gdk-pixbuf's generators, glslang, wayland-scanner; LLVM and Clang |
| Kernel modules | kmod (`depmod`, `modinfo`) |
| Release, flashing, upkeep | rsync, curl, jq, file, unzip, telnet, ssh, `gpgv`, heimdall (with libusb), lsblk and findmnt |
| `make run` | QEMU (x86_64 and aarch64 systems, user networking, VNC screen), nasm and iasl for the firmware |

The ones this repo didn't already carry for jk_os are listed in
`config/components/hosttools/sources.list` and fetched into
`userspace/hosttools/` (`scripts/update-desktop-sources.sh --component
hosttools`), each pinned to its SHA-256 or git commit.

## The cross compiler and glibc

`scripts/build-cross.sh` (make step `cross`) builds, for the preset's arch:

- binutils and GCC (C, C++) for `<arch>-linux-gnu`, running on the build
  machine, in `build/<arch>/cross`
- glibc from `userspace/toolchain/glibc`, the Linux headers and the C.UTF-8
  locale, in `build/<arch>/toolchain` (their sysroot, and later the
  toolchain the image carries)

through the usual bootstrap: binutils, a first GCC (C only, no C library),
the Linux headers, glibc, the full GCC. This happens for the build machine's
own arch too: every target package, on any build machine, is built by jk_os's
own compiler against jk_os's own glibc, as a cross build (the build machine
is named `<arch>-buildhost-linux-gnu`), so configure scripts never run
target programs here. Perl is cross-built with perl-cross, Python with the
host tools' Python of the same version.

## GRUB and the UEFI firmware

- `build-iso.sh` (x86_64) builds GRUB from `userspace/hosttools/grub` for
  BIOS (i386-pc) and UEFI (x86_64-efi) into `build/x86_64/grub`: its
  programs (`grub-mkrescue`) for the build machine, its platforms' modules
  with jk_os's x86_64 compiler.
- `run-qemu.sh` builds EDK2's firmware from `userspace/hosttools/edk2`
  (OVMF for x86_64, ArmVirtQemu for aarch64) with jk_os's compiler for the
  preset's arch, once, into `build/<arch>/edk2`. `make run GUI=1` shows the
  screen over VNC (`localhost:5901`).

## Device access

Every jk_os image carries `/usr/lib/udev/rules.d/69-jk-devices.rules`: the
user at the console may flash a Samsung device in Download mode with
heimdall, and ModemManager leaves a jk_os tablet's USB console alone. On
Ubuntu the same two rules are the one setup outside the repo:

```sh
sudo tee /etc/udev/rules.d/69-jk-devices.rules <<'EOF'
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb_device", ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="685d", TAG+="uaccess"
ATTRS{idVendor}=="1d6b", ATTRS{idProduct}=="0104", ENV{ID_MM_DEVICE_IGNORE}="1"
EOF
sudo udevadm control --reload
```

## Requirements of a build machine

- disk: about 200 GB free for every preset at peak (`/data` on jk_os), about
  50 GB once built and pruned (`make PRESET=<preset> prune` also prunes the
  host tools' work directories)
- memory: 16 GB, zram recommended (LLVM and Qt link in big pieces); a Jetson
  Orin NX with 8-16 GB builds, slowly
- time: the host tools add about 1.5-2 hours to a build machine's first
  build (LLVM, CMake, Python, QEMU, GCC for the cross compiler), once
- network: only to clone the repo (and its LFS files) and to publish; the
  build itself is offline

## Plan and status

| Phase | What | Status |
|---|---|---|
| 1 | Build machine check (`scripts/lib-host.sh`), the floor, audit of host assumptions | done |
| 2 | glibc from source, both arches; C.UTF-8 from `localedef` | done (`build-cross.sh`) |
| 3 | Cross-building both ways, with jk_os's compiler for the build machine's own arch too; Perl cross-built | done |
| 4 | Host tools stage; `make deps` removed | done |
| 5 | Device access: udev rules for heimdall and ModemManager | done |
| 6 | `make run`: QEMU in the host tools, EDK2 firmware built per arch | done |
| 7 | Proof: every preset from scratch on Ubuntu with only the floor, on a jk_os x86_64 PC, and cross from an aarch64 jk_os; the tablet refuses; a release published from jk_os | to do |
