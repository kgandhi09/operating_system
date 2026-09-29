# jk_os

A minimal 64-bit Linux system built from source:

- the **official Linux kernel** from Linus Torvalds' mainline releases, kept in this repo as plain source,
- **BusyBox** as the base userspace (shell, init, coreutils, networking),
- **util-linux** and **e2fsprogs** disk tools (GPT partitioning, ext4) for the installer,
- **shadow-utils** (with **libxcrypt**) for users and passwords: `useradd`, `passwd`, `su`, `login`, ..., and **sudo**,
- **NetworkManager** (`nmcli`) for wired, Wi-Fi and IPv6, with the firmware common network chips need,
- a **C/C++ toolchain**: GCC 16 and Clang 23 (C++20 by default), binutils, CMake, Ninja and GDB,
- a short list of **prebuilt static binaries pulled from GitHub releases** (`jq`, `rg`, `fd`, ...),
- no display manager, X or Wayland: text console on screen plus serial console.

It targets **x86_64** and **aarch64**. You get a bootable, hybrid ISO that
works from a CD/DVD, a VM, or written raw to a USB stick.

Every source tree lives in the repo, so **a build needs no network and no
git**: a copied folder or an unpacked archive of this repo builds offline.
The one exception is a GitHub binary newly added to `configs/binaries/`, which
is downloaded once into `userspace/binaries/` (see [Binaries from GitHub](#binaries-from-github)).

## Quick start

```sh
make deps                 # once: install host packages (Debian/Ubuntu or Arch, uses sudo)
make                      # build out/jk_os-<ver>-<host arch>.iso
make run                  # boot it in QEMU on this terminal (Ctrl-A X quits)
```

The ISO boots into the installer menu, before any login. From there you can
install to a disk, or try the live system (a root shell; changes are kept in RAM
and lost on reboot). See [Installing to a disk](#installing-to-a-disk).


### Other architecture

```sh
make ARCH=aarch64         # cross-compiles with aarch64-linux-gnu-gcc
make ARCH=aarch64 run
make ARCH=x86_64          # from an aarch64 host, with x86_64-linux-gnu-gcc
```

Override the toolchain with `CROSS_COMPILE=<prefix>` and the job count with `JOBS=N` (default: half the CPUs).

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
| Boots from | CD/DVD, USB stick, disk | USB stick or disk only: the kernel is larger than the 32 MiB an El Torito (CD) boot image can be |

The OS is a compressed, read-only image, **`jk_os.squashfs`**, as on
Ubuntu's live media. The kernel (built with **no loadable modules**) carries
only a small **initramfs** (`initramfs/init`, `make initramfs`): BusyBox,
`e2fsck`, and the firmware that network drivers load while the kernel starts.
Its `/init`:

1. finds the installer medium (label `JK_OS`) or an installed system's data
   partition (label `JK_DATA`), waiting a few seconds for USB storage;
2. mounts the image from it and puts a writable layer on top with overlayfs:
   in **RAM** on the live medium, on **disk** (`/data/system/root`) when
   installed;
3. switches into that root (`switch_root`), which frees the initramfs.

Programs are read from the image as they are needed, so RAM use doesn't grow
with the size of the OS. Then BusyBox `init` runs `/etc/init.d/rcS`, which
runs `/etc/init.d/S??*` (storage, syslog, udev, D-Bus, NetworkManager).
`getty` starts on each console that actually exists.

## Layout

```
Makefile                  entry point (see `make help`)
versions.env              OS name/version, which source tree each arch uses
kernel/mainline/          Linux source, Torvalds' mainline release (plain files)
userspace/busybox/         BusyBox source (plain files)
userspace/util-linux/      util-linux source: sfdisk, fdisk, lsblk, wipefs, partx
userspace/e2fsprogs/       e2fsprogs source: mkfs.ext4, e2fsck, resize2fs, tune2fs
userspace/shadow/          shadow-utils source: useradd, usermod, passwd, su, login, ...
userspace/libxcrypt/       libxcrypt source: password hashing (yescrypt) for shadow
configs/kernel/           kernel config fragments merged over the arch defconfig
configs/busybox/          BusyBox config fragments merged over defconfig
configs/binaries/         GitHub binaries: common.list (every arch) + <arch>.list
initramfs/init            early boot: find and mount jk_os.squashfs, switch into it
configs/initramfs.list    device nodes added to the initramfs
userspace/binaries/<arch>/ fetched binaries (bin/) and what they came from (sources.lock)
rootfs/                   files copied verbatim into the root filesystem
boot/grub.cfg             x86_64 GRUB menu
scripts/                  one script per build step
build/                    (generated) per-arch build trees
out/                      (generated) ISO images
```

Build steps can run individually: `make busybox | tools | binaries | rootfs | kernel | iso`.
The kernel and BusyBox build out of tree under `build/<arch>/`. The source
trees stay pristine, and both architectures can build side by side from the
same source.

## Sources

`versions.env` chooses the kernel tree for each architecture:

```sh
KERNEL_TREE_x86_64=kernel/mainline
KERNEL_TREE_aarch64=kernel/mainline
BUSYBOX_TREE=userspace/busybox
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
scripts/update-source.sh util-linux 2.42.5
scripts/update-source.sh e2fsprogs 1.47.5
scripts/update-source.sh shadow 4.20.4
scripts/update-source.sh libxcrypt 4.5.3
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

## Installing to a disk

Write the ISO to a USB stick (`make flash`) and boot from it. Before any login
prompt, every console (screen and serial) shows the installer menu:

```
  1) Install jk_os
  2) Try jk_os without installing (root shell, nothing is kept)
  3) Start the jk_os already installed on /dev/sda    (only if one is found)
  r) Reboot
  p) Power off
```

**Install** runs `jk-install`, and only one console can run it at a time. After a
successful install it asks you to remove the USB stick and reboots into the
installed system. As in Ubuntu's installer, you pick a disk and then either:

1. **Erase disk and install**: creates a new GPT table with the default layout below
   and asks only whether you want swap.
2. **Something else**: a partition editor. It lists the partitions and free space,
   and you can create (`n`), change what a partition is used for (`e`), delete (`d`),
   start a fresh GPT table (`t`), or undo everything (`u`). An existing partition can be
   formatted or kept with its filesystem and files. Nothing is written until you
   confirm the summary by typing `yes`.

An installed disk holds the kernel, the OS image, and everything changed on top
of it:

| Mount | Filesystem | Label | Required | Holds |
|---|---|---|---|---|
| `/boot` | FAT32, EFI system partition | `JK_BOOT` | yes (≥ 256 MiB; default 512 MiB) | the kernel, as `EFI/BOOT/BOOTX64.EFI` / `BOOTAA64.EFI` |
| `/data` | ext4 | `JK_DATA` | yes (default: rest of the disk) | the OS image (`system/jk_os.squashfs`), every change made to the system (`system/root`: `/usr`, `/etc`, `/var`, ...), and `/home` (`/data/home`, unless it has its own partition) |
| `/home`, `/var/log`, `/srv`, `/opt`, `/mnt/<name>` | ext4 | | no | whatever you split out |
| swap | swap | `JK_SWAP` | no | |

The kernel's initramfs picks the mode and records it in `/run/jk-mode`:

- **live:** the installer medium (label `JK_OS`) is attached. The root is its
  `jk_os.squashfs` with changes in RAM. The consoles run the installer menu
  (`/sbin/jk-live`, started by `jk-getty` in place of `login`). No other disk is
  mounted, so every disk except the medium is free to repartition.
- **installed:** no installer medium, but a `JK_DATA` partition. It is checked with
  `fsck.ext4 -p` and mounted on `/data`. The root is `/data/system/jk_os.squashfs`
  with `/data/system/root` on top, so whatever you change or add anywhere
  (`/usr/local`, `/etc`, `/opt`, ...) survives reboots. `/etc/init.d/S05storage`
  then mounts what the installer listed in `/data/etc/fstab` (by UUID), and
  bind-mounts `/data/home` on `/home` unless `/home` has its own partition.
- **setup:** installed, but no user exists yet. The consoles run the first-boot setup.

USB sticks appear a few seconds after the kernel starts, so the initramfs waits
up to 10 s for the installer medium (3 s once an installed system is found).
If it finds neither, it opens a rescue shell on the console.

- **Booting.** An installed disk boots through **UEFI** on both architectures. The
  kernel's EFI stub is the firmware's default boot file, so no boot loader or NVRAM
  entry is involved. Legacy BIOS is only supported for the live ISO. On x86_64,
  consoles come from the built-in command line (`CONFIG_CMDLINE`).
- **Updating.** A new jk_os version is a new kernel file and a new image. Put them
  at `/boot/EFI/BOOT/BOOT*.EFI` and `/data/system/jk_os.squashfs` (or reinstall
  and keep `/data`). Your changes in `/data/system/root` stay on top of the new image.
- **Reinstalling.** Boot the USB stick again. As root on an installed system,
  `jk-install` can install to another disk, but not to the one it runs from, or to
  the installer medium. It installs the kernel and image of the installer medium, or
  the running system's own (`/boot/EFI/BOOT/…`, `/data/system/jk_os.squashfs`).
  `JK_KERNEL=<file>` and `JK_IMAGE=<file>` override them.
- **Scripted installs.** `jk-install --auto /dev/sdX [--swap 2G] --yes` erases
  the disk and uses the default layout without asking. Add
  `--user NAME [--fullname "Full Name"] [--no-admin] [--hostname NAME]`, with the
  passwords in `$JK_USER_PASSWORD` and `$JK_ROOT_PASSWORD`, to create the accounts
  too. Without `--user`, the first boot asks for them (see below).

## Users

The image ships with no user, and root is locked (`*`). Accounts are created
during the install, in a **"Who are you?"** step after the disk layout: your name,
a username and password, the computer name, whether you may become root, and
the root password. If a disk was installed without that step, the first boot of
the installed system shows the same questions on every console before any login
prompt (`/sbin/jk-setup`). Only then do login prompts appear.

The tools are **shadow-utils**, as on Ubuntu/Debian/Arch, built without PAM and
configured in `/etc/login.defs`. Passwords are hashed with yescrypt. BusyBox's own
account applets are disabled.

| Who | May |
|---|---|
| root | everything: `useradd`, `usermod`, `userdel`, `groupadd`, `groupmod`, `groupdel`, `chpasswd`, `chage`, `newusers`, and setting any user's password |
| a user | change only their own password (`passwd`), name and details (`chfn`), and shell (`chsh`, from `/etc/shells`); switch to one of their own groups (`newgrp`) |
| a member of `wheel` (administrator) | run commands as root with `sudo` (their own password), or become root with `su` (the root password). The installer puts the first user in `wheel` if you say so; root adds others with `usermod -aG wheel <user>` |

```sh
sudo useradd -m -c "Robot Operator" op   # as a wheel member (or as root, without sudo)
sudo passwd op                           # set their password
sudo usermod -aG wheel op                # make them an administrator
sudo userdel -r op                       # remove them and their home
sudo visudo                              # edit /etc/sudoers (checked before saving)
```

On an installed system the whole root is the OS image with
`/data/system/root` on top. Anything changed (users, passwords, groups, hostname,
network settings, files added to `/usr/local`) is kept across reboots, and `/home`
is on `/data`. Files never changed keep coming from the image, so a new jk_os
version updates them. A file you changed keeps your version. System accounts a
new image adds are merged into your `/etc/passwd` at boot.

## Packages (apt)

The installer asks for a **package management** choice after the accounts:

1. **apt**: Debian packages (Debian 13 "trixie"), with the usual commands:
   `sudo apt update`, `sudo apt install <name>`, `sudo apt remove <name>`,
   `apt search`, `dpkg -l`, ...
2. **None**: only the programs jk_os comes with. You can add apt later with
   `sudo apt-setup`.

`jk-install --auto` takes `--packages apt|none` (default `none`).

Debian's packages are built against Debian's libraries, not jk_os's, so they
don't go into the OS itself. They go into a Debian system of their own on the
data partition, **`/data/apt/root`**, unpacked from the Debian base system that
the OS image carries (the official Docker image's root filesystem,
`userspace/debian/<arch>`, see `configs/debian/rootfs.list` and
`scripts/update-debian-rootfs.sh`). Because it lives on `/data` and not in
`/data/system/root`, what you install stays when jk_os is updated.

`apt` and `dpkg` (`/usr/lib/jk_os/apt`) run in that system inside a
[bubblewrap](https://github.com/containers/bubblewrap) container, as root. Afterwards:

- each command a package added gets a launcher in `/data/apt/bin`, which is on
  `PATH` (and sudo's `secure_path`) after jk_os's own directories. jk_os's
  commands win when a name is in both, and `apt-run <command>` runs the
  Debian one;
- menu entries go to `/data/apt/share/applications`, and the desktop lists them
  after the next login.

Programs run as you, sharing `/home`, `/tmp`, `/mnt`, `/media`, `/run` (the
desktop's Wayland and D-Bus sockets) and the devices. The Debian system itself
is read-only to them. The container relies on user namespaces
(`CONFIG_USER_NS`). Services that packages would start (`policy-rc.d`) don't
start, since no init runs in there. The live system has no apt, because nothing
is kept there.

## C and C++ development

Every jk_os image carries a toolchain (`scripts/build-toolchain.sh`, `make toolchain`):

| | |
|---|---|
| Compilers | **GCC 16.2** (`gcc`, `g++`, `cc`, `c++`) and **Clang 23.1** (`clang`, `clang++`, with `lld`) |
| C++ | **C++20** (`gnu++20`) unless a project asks for another standard (`-std=`, `CMAKE_CXX_STANDARD`). GCC's own default; Clang reads it from `/etc/clang/clang++.cfg`. Both use GCC's `libstdc++` |
| Language tools | `clangd` (language server for editors), `clang-format`, `clang-tidy` |
| Binutils | `as`, `ld`, `ar`, `objdump`, `nm`, `strip`, ... (2.47) |
| Build systems | **CMake 4.4** (HTTPS works, e.g. `FetchContent`) and **Ninja 1.13**; `CMAKE_GENERATOR=Ninja` is set in `/etc/profile` |
| Debugger | **GDB 18.1** and `gdbserver` |
| Headers | glibc (from the toolchain that builds jk_os) and Linux (`make headers_install` from jk_os's kernel) |

Clang also cross-compiles for the other architectures (`--target=aarch64-linux-gnu`,
`arm-none-eabi`, `riscv64-...`). On an installed system anything you build and
install goes into `/data/system/root` and survives reboots.

```sh
cmake -S . -B build && cmake --build build          # Ninja by default
CC=clang CXX=clang++ cmake -S . -B build-clang
gdb ./build/myrobot
```

For aarch64, built on an x86_64 host, `build-toolchain.sh` first builds a GCC 16
cross compiler (`build/aarch64/cross`), then cross-builds everything with it.
The glibc headers and libraries come from the host's Debian/Ubuntu packages
(`libc6-dev`, `libc6-dev-arm64-cross`), so the toolchain build needs a
Debian-based host.

## Binaries from GitHub

Tools that BusyBox doesn't provide come as prebuilt release assets from GitHub.
They are listed in `configs/binaries/`:

- `common.list`: installed on **every** arch. `@ARCH@` in an asset name becomes
  `x86_64` / `aarch64`, and `@DEBARCH@` becomes `amd64` / `arm64`, so a single line
  covers both architectures.
- `x86_64.list`, `aarch64.list`: installed only on that arch. A line here with the
  same name as one in `common.list` replaces it on that arch, for a project
  whose asset names don't follow a pattern.

```
# name  owner/repo          tag      asset (glob)                                  [member]
jq      jqlang/jq           jq-1.8.2 jq-linux-@DEBARCH@
rg      BurntSushi/ripgrep  15.2.0   ripgrep-*-@ARCH@-unknown-linux-musl.tar.gz
```

`make` runs `make binaries` before assembling the rootfs. For each entry, it
downloads the one matching asset of that release, checks it against GitHub's
published SHA-256 digest, unpacks it (tar.*, zip, gz/xz/bz2/zst, or a bare
binary), and installs the binary as `userspace/binaries/<arch>/bin/<name>`.
The binary ends up at `/usr/local/bin/<name>` in the OS, which comes first in
`PATH`, so it takes precedence over a BusyBox applet with the same name. What was
fetched (tag, asset, sha256) goes into `userspace/binaries/<arch>/sources.lock`.

- **Offline builds keep working.** An entry that still matches its lock line
  is not fetched again, so commit `userspace/binaries/` and the repo builds with no
  network. Only a new or changed entry downloads anything. `OFFLINE=1` turns that
  download into an error.
- **Updating.** `tag` can be `latest`. It is resolved once and then stays pinned
  in the lock until `make binaries UPDATE=1`, which re-resolves and re-downloads
  everything. Remove a line and the next build deletes the binary.
- **Static only.** The OS has no libc, so the fetch rejects dynamically linked or
  wrong-arch ELF files. Pick the `*-linux-musl` or Go build of a project.
- `member` is only needed when an archive holds more than one file with that name,
  or the binary is named differently: it's a glob for its path inside the archive.
- GitHub allows 60 unauthenticated API calls per hour. Set `GITHUB_TOKEN` if you
  hit that limit.
- `make binaries ARCH=aarch64` fetches for the other arch without building anything.

## Customizing

- **Add files to the OS**: drop them under `rootfs/` (e.g. `rootfs/etc/init.d/S50myapp`, executable, taking `start`/`stop`), then `make`.
- **Branding**: the desktop's logo, launcher icon (`jk-os`), wallpaper (`jk_os`), splash screen and Global Theme (`org.jk_os.desktop`, set in `rootfs/etc/xdg/kdeglobals`) come from `J.K. Logo.png`. After changing the logo, run `scripts/make-branding.py` (needs Python with Pillow and NumPy) to regenerate the images under `rootfs/usr/share/`, then `make`.
- **Kernel options**: `make menuconfig` to explore, then put the options you want to keep in `configs/kernel/common.config` or `configs/kernel/<arch>.config`. Fragments are re-applied whenever they change. The build warns if an option you asked for was dropped.
- **Clean**: `make clean` (current arch) or `make distclean` (all of `build/` and `out/`; source trees are left alone).

## Host requirements

`make deps` installs these on Debian/Ubuntu (apt) or Arch (pacman). On other
distros, install the equivalents:

- build: `gcc make bc flex bison libelf-dev libssl-dev cpio file`
- fetching GitHub binaries: `curl jq file` (plus `unzip` / `zstd` for those asset types)
- updating sources (optional): `curl tar xz bzip2`
- cross: `gcc-aarch64-linux-gnu libc6-dev-arm64-cross` (a glibc cross toolchain; BusyBox links statically against it)
- ISO: `xorriso mtools dosfstools squashfs-tools`, and on x86_64 hosts `grub-pc-bin grub-efi-amd64-bin`
- run: `qemu-system-x86 qemu-system-arm ovmf qemu-efi-aarch64`

Building the x86_64 ISO needs GRUB's i386-pc / x86_64-efi platform files.
Distros usually ship these only for x86 hosts, so build x86_64 images on an
x86_64 machine to get both BIOS and UEFI boot. The aarch64 ISO needs no GRUB,
so it can be built on any host.
