# jk_os

A minimal 64-bit Linux system built from source:

- the **official Linux kernel** from Linus Torvalds' mainline releases, kept in this repo as plain source,
- **BusyBox** as the base userspace (shell, init, coreutils, networking),
- **util-linux** and **e2fsprogs** disk tools (GPT partitioning, ext4) for the installer,
- **shadow-utils** (with **libxcrypt**) for users and passwords: `useradd`, `passwd`, `su`, `login`, ..., and **sudo**,
- **NetworkManager** (`nmcli`) for wired, Wi-Fi and IPv6, with the firmware common network chips need,
- **sound** (PipeWire), **Bluetooth** (BlueZ) and USB webcams,
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


### Choosing a target device

`make config` asks what to build for and saves the answers in `build.conf`;
`make` then builds that, with no questions. The device comes first, and each
later list shows only what fits it:

1. **device category**: `pc`, `tablet`, `robotics-hpc`, ...
2. **device**: `generic` (any PC or VM), `jetson-orin-nx`, `samsung-gts7fe`, ...
3. **kernel**: the kernels that device uses, e.g. `mainline`, or
   `samsung-mainline` (mainline plus Samsung device trees)
4. **arch**: what both the device and the kernel support
5. **name**: your own, for the output file, e.g. `asus-laptop`
   gives `out/jk_os-<ver>-asus-laptop-x86_64.iso`

```sh
make config               # choose from menus (arrow keys, Enter)
make showconfig           # what build.conf says
make                      # build it
make BUILD_CONF=jetson.conf   # build from another saved target
```

Without `build.conf`, `make` builds the generic PC for the host arch, as
before. The choices are defined in [`targets/`](#build-targets). A device
whose boot format jk_os can't make yet (the Samsung tablet's Android
`boot.img`) can be chosen, but `make` refuses it.

### Other architecture

Without a `build.conf`, `ARCH` picks the arch of the generic PC build (with
one, `make config` does):

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
targets/kernels/<name>/   kernel profiles: which tree, which archs (make config)
targets/devices/<category>/<device>/  device profiles: archs, kernels, boot format
build.conf                (generated) the target make config chose
userspace/binaries/<arch>/ fetched binaries (bin/) and what they came from (sources.lock)
rootfs/                   files copied verbatim into the root filesystem
boot/grub.cfg             x86_64 GRUB menu
scripts/                  one script per build step
build/                    (generated) build trees: build/<arch>/ (userspace),
                          build/<arch>/<device>-<kernel>/ (kernel, image)
out/                      (generated) ISO images
```

Build steps can run individually: `make busybox | tools | binaries | rootfs | kernel | iso`.
The kernel and BusyBox build out of tree under `build/`. The source trees
stay pristine, and both architectures can build side by side from the same
source. Userspace (BusyBox, toolchain, network stack, desktop) is the same for
every device of an arch and builds once in `build/<arch>/`; the kernel,
initramfs, rootfs and ISO depend on the device and kernel and go in
`build/<arch>/<device>-<kernel>/`.

## Build targets

`make config` offers what `targets/` defines. Each profile is a plain
`KEY=value` file:

```
targets/kernels/mainline/kernel.env              KERNEL_TREE, KERNEL_ARCHS
targets/devices/pc/category.env                  CATEGORY_DESC
targets/devices/pc/generic/device.env            DEVICE_ARCHS, DEVICE_KERNELS, DEVICE_BOOT
```

A kernel, category or device directory can also hold a `kernel.config`
fragment, merged after `configs/kernel/common.config` and `<arch>.config` in
the order kernel, category, device. A category or device directory can hold a
`rootfs/` overlay, copied over the root filesystem. The image records its
target in `/etc/jk_os/target` and its device as `VARIANT_ID` in
`/etc/os-release`.

To add a device, create `targets/devices/<category>/<device>/device.env`
(device names are unique across categories). To add a kernel, create
`targets/kernels/<name>/kernel.env` and list it in the device's
`DEVICE_KERNELS`. `scripts/lib-target.sh` describes every field.

Releases (`scripts/publish.sh`, `jk-update`) are still per arch, so only the
generic PC build can be published for now.

### Android-bootloader devices (Samsung Galaxy Tab S7 FE)

A device with `DEVICE_BOOT=android-bootimg` boots mainline Linux with a device
tree jk_os adds (`targets/kernels/<kernel>/dts/`, built against the kernel
tree by `scripts/build-dtbs.sh`) from an Android `boot.img` its bootloader
loads. Drivers not in the kernel yet are patches in the kernel profile
(`targets/kernels/<kernel>/patches/`), applied to a hard-linked copy of the
tree (`scripts/prepare-kernel.sh`); either way `kernel/<tree>` stays untouched.

```sh
scripts/inspect-stock.sh AP_*.tar.md5 BL_*.tar.md5   # read the stock firmware: build/stock/...
make bootimg              # kernel + device tree -> build/.../bootimg, and an Odin tar in out/
make flash                # write them with Heimdall, the tablet in Download mode
```

`inspect-stock.sh` prints what the device profile needs from the stock
firmware: the boot image format (`DEVICE_MKBOOTIMG_ARGS`), and the stock device
trees' `qcom,msm-id` / `qcom,board-id`, memory map and hardware (decompiled
into `dtb/*.dts`). It also takes the vendor partition's `/firmware` out of
`super.img` (`vendor/firmware/`, via `scripts/android/lpextract.py`): the
firmware of the device's chips, which jk_os doesn't redistribute; the device
profile lists what the build copies from there (`DEVICE_STOCK_FIRMWARE`,
`DEVICE_STOCK_FIRMWARE_EARLY` for the initramfs). `make bootimg` makes
`boot.img`, `vendor_boot.img`,
`vbmeta.img` (verified boot off) and `dtbo.img`, and an Odin tar of them;
`make flash` writes them with Heimdall from Linux, each to the partition the
device's own partition table names for it (`scripts/flash-heimdall.sh --pit`
only lists the partitions). The AOSP tools this uses are vendored in
`scripts/android/`.

The Tab S7 FE's bootloader refuses mainline device trees, so it boots
through U-Boot (`DEVICE_BOOT=android-uboot`, `bootloader/u-boot`,
`scripts/build-uboot.sh`): the bootloader starts U-Boot with Samsung's own
tree, and U-Boot starts jk_os's kernel with jk_os's tree, both in a FIT image
in `vendor_boot`. U-Boot shows its messages on the display; volume keys move
in its menu, power selects (hold volume down while it starts to open it).

On the tablet, jk_os's initramfs gives the PC on the USB cable a console
(`targets/devices/tablet/samsung-gts7fe/initramfs`): the tablet is
172.16.42.1 and hands the PC 172.16.42.2.

```sh
telnet 172.16.42.1                  # a root shell (or: screen /dev/ttyACM0)
scripts/tablet-backup.sh --list     # the tablet's partitions
scripts/tablet-backup.sh            # optional: copy them to ~/jk_os-backups/..., checked by SHA-256
```

The Book Cover Keyboard (pogo pins) works on the console too.

To install jk_os on the tablet, with it at that console (no system yet):

```sh
make                                # the OS image (and the boot images)
scripts/tablet-install.sh           # format userdata as JK_DATA, copy the image, reboot
```

`tablet-install.sh` formats only the `userdata` partition, after you type
its name. The tablet then boots jk_os
from its storage and starts the first-boot setup on its screen; the
installed system serves the PC on the USB cable too (`telnet 172.16.42.1`
or `/dev/ttyACM0`, with a login). `jk-update` writes a new kernel to the
`vendor_boot` partition (keeping the old one for `--rollback`).

To reinstall over an installed system, choose "Rescue shell (reinstall over
USB)" in U-Boot's menu (hold volume down while it starts): the initramfs
then stays at its USB console (kernel option `jk.rescue`), and
`scripts/tablet-install.sh --no-format` copies the new image.

On the PC, keep ModemManager off the tablet's serial port: it takes
`/dev/ttyACM0` for a modem and types AT commands into the tablet's console.
Once:

```sh
echo 'ATTRS{idVendor}=="1d6b", ATTRS{idProduct}=="0104", ENV{ID_MM_DEVICE_IGNORE}="1"' |
    sudo tee /etc/udev/rules.d/70-jk-os-tablet.rules
sudo udevadm control --reload
```

What works on the tablet, and where it comes from (kernel patches in
`targets/kernels/samsung-mainline/patches/`, wiring in its device tree):

| Hardware | Driver | Notes |
| --- | --- | --- |
| Display (TS124QDM, FocalTech FT8203, DSC) | msm + `panel-samsung-ts124qdm-ft8203` (0002-0004) | brightness in `/sys/class/backlight`; the desktop never switches it off (DPMS hangs the link on the SM-T735) |
| Touchscreen (FT8203 touch, SPI) | `focaltech-ft8203` (0005) | firmware from the stock vendor partition, loaded at every power-up |
| Battery, charging (SM5714) | `sm5714_charger` (0006) | charges from plain 5 V, so at most 15 W from any charger (no USB-PD or SM5440 direct-charger driver yet); input 3 A (lowered by AICL on weaker sources), 2.1 A into the battery by default, adjustable with `jk-charge` (below); `sm5714_charger.enable_charging=0` on the kernel command line leaves the charger alone |
| S Pen (Wacom W9021) | `samsung-w90xx` (0007) | |
| Wi-Fi, Bluetooth (WCN6750) | ath11k, hci_qca | WPSS firmware and board data from the stock vendor partition; `/etc/init.d/S07wireless` starts Wi-Fi once the system is mounted |
| GPU (Adreno 642L) | msm, Mesa freedreno | zap shader from the stock apnhlos partition |
| Book Cover keyboard, cover magnet | `samsung-stm32-pogo` (0001), gpio-keys | closing the cover doesn't suspend (suspend isn't brought up) |
| UFS, USB device mode | mainline | USB console and network to a PC |
| Not yet: audio, sensors (rotation), USB host, cameras, suspend | | audio and sensors run on the ADSP; USB host needs the SM5714's Type-C side |

Charging is set with `jk-charge`, kept in `/etc/jk_os/charge` and applied
at boot (`/etc/init.d/S08charge`), within Samsung's own limits for this
battery (the driver refuses anything outside them):

```sh
jk-charge                      # the settings and the battery right now
sudo jk-charge normal          # 3000 mA in, 2100 mA into the battery, 4.38 V (default)
sudo jk-charge fast            # 3200 mA into the battery
sudo jk-charge gentle          # 1500 / 1000 mA, 4.30 V, stop at 80%
sudo jk-charge input 2000      # or one setting: input <mA>, current <mA>, voltage <mV>
sudo jk-charge limit 80        # battery care: stop at 80%, charge again below 75%
```

While held at the limit the tablet runs from the charger. Charging always
pauses below 0 or above 50 degC.

## Sources

`versions.env` chooses the userspace trees (`BUSYBOX_TREE=userspace/busybox`,
...); the kernel profile chosen with `make config` chooses the kernel tree
(`KERNEL_TREE=kernel/mainline` in `targets/kernels/mainline/kernel.env`).
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

**Add a different kernel tree** that a device needs, such as a vendor tree
for a specific board. Unpack it as `kernel/<name>`, or use
`scripts/update-source.sh kernel <tarball> kernel/<name>`, then add a kernel
profile `targets/kernels/<name>/kernel.env` with `KERNEL_TREE=kernel/<name>`
(see [Build targets](#build-targets)).

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
- **Next to another Linux (dual boot).** Free space for jk_os beforehand (e.g. shrink Ubuntu with GParted from its live USB), then pick
  **Something else** and create a new 512 MiB `/boot` and an ext4 `/data` in the free space, leaving the other system's partitions
  as they are. If that system boots with GRUB (Ubuntu, Debian, Fedora, ... on ext4), the installer offers to add jk_os to its boot
  menu: it writes an entry to that GRUB's `custom.cfg` (`/boot/grub/custom.cfg` on Ubuntu), which GRUB reads at every start and
  `update-grub` leaves alone, and makes the menu show if it was hidden. Reinstalling replaces the entry; delete the `jk_os` block
  there to remove it. GRUB starts jk_os only with Secure Boot off (the installer warns if it is on).
- **Updating.** A new jk_os version is a new kernel file and a new image.
  `sudo jk-update` fetches the newest published release
  (`<server>/downloads/jk_os/latest-<arch>.json`), checks its SHA-256s, swaps
  both files in and keeps the previous pair (`/data/system/jk_os.squashfs.old`,
  `/data/system/kernel.old`) for `sudo jk-update --rollback`; the new release
  starts at the next reboot. `jk-update --check` only says whether there is one.
  It lists the files you changed that the update also changes (your copies win).
  The server is set in `/etc/jk_os/update` (`JK_UPDATE_URL=`). Releases are
  published with `scripts/publish.sh` (image, kernel, ISO, `INSTALL.txt`,
  `SHA256SUMS`, `release.json`; only the newest is kept on the server). By hand:
  put the files at `/boot/EFI/BOOT/BOOT*.EFI` and `/data/system/jk_os.squashfs`
  (or reinstall and keep `/data`). Your changes in `/data/system/root` stay on
  top of the new image.
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

## Consoles

| Console | Keys | After logging in |
|---|---|---|
| tty1 - tty6 | Ctrl+Alt+F1 - F6 | a plain shell. `jk-dev` starts **the dev session** there: one full-screen terminal ([foot](https://codeberg.org/dnkl/foot)) in the kiosk compositor [cage](https://github.com/cage-kiosk/cage), for Neovim, herdr and the like, one per console. Leaving the terminal (`exit`) comes back to the shell, where `jk-dev` starts it again |
| any of them | | `jk-gui` starts the desktop, KDE Plasma, there (one desktop per user); logging out of it comes back to the shell |
| ttyS0, ... | serial line | a plain text console |

Ctrl+Alt+F1-F6 switch between them, from the dev session and the desktop too.
With an external screen, the dev session shows the same terminal on the laptop's
screen and on it (mirror: every screen is scaled to the narrowest one's width,
and the terminal fills the part all of them show). KDE extends the
desktop across the screens by default; Meta+P, or System Settings → Display
Configuration, switches it to mirroring.
The dev session runs on the GPU like the desktop does (jk-session opens an elogind
session for it). It runs as a regular user, not root (root gets a plain
shell); if it can't start, the shell stays and the reason is in
`~/.local/state/jk-dev.log`. Nothing starts it (or the desktop) by itself.
Its terminal opens with the J.K. Robotics banner (`/usr/share/jk_os/banner`,
shown by `/etc/profile`, which sets `JK_DEV_SESSION=1` there and
`JK_BANNER_SHOWN=1` once the banner is up).

Settings:

- the terminal: `~/.config/foot/foot.ini` (jk_os's defaults: `/etc/xdg/foot/foot.ini`:
  login shell, FiraCode Nerd Font Mono 9 pt (in the image, with DejaVu Sans
  Mono for what it lacks), pure black background and colour 0, underline
  cursor, `TERM=xterm-256color`, true colour). Other fonts go in
  `~/.local/share/fonts`.
- the session: `~/.config/jk_os/dev-session`, shell variables, e.g.

  ```sh
  JK_DEV_COMMAND=herdr        # run this in the terminal first, then the shell
  JK_DEV_OUTPUTS=extend       # external screen: mirror (default), extend or last
  XKB_DEFAULT_LAYOUT=de       # keyboard layout (XKB_DEFAULT_VARIANT, XKB_DEFAULT_OPTIONS)
  ```

## NVIDIA GPUs

jk_os carries NVIDIA's own driver (`userspace/nvidia/<arch>`, from
`scripts/update-nvidia.sh <version>`): its open kernel modules, built against
jk_os's kernel by `scripts/build-nvidia.sh` (`make nvidia`), and its libraries
for OpenGL/EGL/GLES (through GLVND, next to Mesa), Vulkan, GBM (KWin, cage),
CUDA, OpenCL, NVENC/NVDEC and `nvidia-smi`. `/etc/init.d/S11gpu` loads it when
an NVIDIA GPU is present (Turing, RTX 20, and newer), and falls back to nouveau
for older cards. These are the kernel's only loadable modules; everything else
is built in.

- Which GPU draws the desktop and the dev session: `JK_GPU=nvidia|auto|builtin`
  in `~/.config/jk_os/gpu` (or `/etc/jk_os/gpu`). The default, `nvidia`, draws
  on the NVIDIA GPU whenever its driver runs; `auto` only when a screen is
  connected to it at login (on most laptops the HDMI port), the built-in GPU
  otherwise (longer battery life); `builtin` always on the built-in GPU.
  `JK_DEV_GPU` (same choices) is for the dev session; it defaults to `builtin`,
  because cage can't show a picture drawn on the NVIDIA GPU on the laptop's
  own screen (that screen freezes), while the other way round works.
- `prime-run <program>` runs one program on the NVIDIA GPU.
- Programs from apt get the same NVIDIA libraries (OpenGL, EGL, Vulkan, CUDA):
  `/usr/lib/jk_os/apt` binds them into its container, next to Debian's Mesa.
  A container set up before this gets its mount point at the next `sudo apt ...`.
- `NVIDIA_DRIVER=nouveau` in `/etc/jk_os/gpu` uses nouveau instead.
- Power: `sudo jk-power performance|balanced|quiet` sets the laptop firmware's
  profile (fans, power limits), the CPUs' energy preference, NVIDIA persistence
  mode and Dynamic Boost (`nvidia-powerd`: on AC power the GPU gets power the
  CPU doesn't use); `jk-power` shows the current state. Kept in
  `/etc/jk_os/power`, applied at boot. `nvidia-settings` is there for looking
  at the GPU (clocks, temperatures, PowerMizer levels); on Wayland it can't
  change settings, `jk-power` and `nvidia-smi` do that.
- CUDA programs run as they are. The CUDA toolkit (`nvcc`, cuBLAS, cuFFT, ...,
  about 6 GB) is installed on demand: `sudo jk-cuda install` (the newest
  release the driver runs; `--minimal` for the compiler and runtime only,
  ~0.4 GB) puts NVIDIA's archives, checked against their SHA-256, in
  `/usr/local/cuda-<version>` on the data partition. `jk-cuda status`,
  `jk-cuda list`, `sudo jk-cuda remove`. jk_os's GCC 16 is newer than CUDA
  lists as supported, so `/etc/profile` sets
  `NVCC_APPEND_FLAGS=-allow-unsupported-compiler`.

## Sound, Bluetooth and cameras

- **Sound**: the kernel has every HDA codec family, USB audio, and Intel's
  audio DSP (Sound Open Firmware, which laptops with digital microphones or
  SoundWire need; its firmware and topologies come from sof-bin, in the
  initramfs), plus AMD's digital microphones. ALSA's card profiles (UCM) are
  in `/usr/share/alsa/ucm2`. **PipeWire**, WirePlumber and PipeWire's
  PulseAudio server run with the desktop and the dev session
  (`/usr/lib/jk_os/jk-audio`); programs using ALSA directly go through it too
  (`/etc/alsa/conf.d`). The desktop's volume applet and settings page are
  plasma-pa. In a terminal: `wpctl status`, `wpctl set-volume @DEFAULT_SINK@ 50%`,
  `pactl list sinks`, `speaker-test -c 2`, `aplay -l`.
- **Bluetooth**: BlueZ's `bluetoothd`, started at boot by
  `/etc/init.d/S42bluetooth` after it loads the adapters' drivers (`btusb`,
  `hci_uart`: modules, so their firmware stays in the image). Pair from the
  desktop's Bluetooth applet or System Settings → Bluetooth, or with
  `bluetoothctl`. Headphones and speakers play through PipeWire (SBC codec).
  `BLUETOOTH=no` in `/etc/jk_os/bluetooth` turns it off.
- **Cameras**: USB webcams, which laptops' built-in cameras are, through
  V4L2 (`/dev/video*`); PipeWire offers them to applications too. Cameras
  behind Intel's IPU6 (some recent thin laptops) are not supported.
- Access: a user logged in on a console gets the sound and camera devices
  from elogind; users created by the installer are also in the `audio` and
  `video` groups.

## jk-viz: the system map

`jk-viz` (in the desktop's menu, or `jk-viz` in a terminal) shows the whole
machine live, in layers from the top down: **0 Silicon** (CPU packages and
cores, memory, PCI and USB devices, disks, network interfaces), **1 Firmware**
(UEFI/BIOS, CPU microcode), **2 Kernel** (the kernel, the drivers each device
uses, the kernel's threads in groups, its memory), **3 System services** (init
and what it starts), **4 Sessions** (getty, login, the desktop's compositor,
shell, session bus and audio) and **5 Applications**. Lines join parents to
children, and, each switchable in the bottom bar: Unix sockets, D-Bus
connections (labelled with the bus names), pipes (writer → reader), local TCP
connections, network use, the devices processes have open, and the driver of
each device.

Pick a resource at the top (CPU, memory, GPU, video memory, disk I/O,
network, power): every node shows its **share of 100%** of it, and a bar.
Everything adds up to 100%: the processes, the kernel, *idle / free*, and
*unattributed* (time of processes that ended during the second, kernel
writeback, network traffic no socket matches, ...). "% of what's used" leaves
the idle part out. Devices show their own load instead. Click a node for its
details and connections, double-click (or right-click) to fold what is under
it (folded, it shows the total under it, Σ), drag to move, wheel to zoom;
the side panel lists the top consumers.

Where the numbers come from: `/proc` (CPU time, PSS memory, disk I/O, open
files), sock_diag (which socket is connected to which), `/proc/net` and a
packet socket that counts the bytes of each local port (headers only; nothing
is kept), the kernel's DRM usage counters and NVIDIA's NVML (GPU, video
memory), and the CPU's energy counters (RAPL). Power per process is an
estimate: the package's power shared out by CPU time, each GPU's by its load.

The collector, `jk-vizd`, runs as root (`/etc/init.d/S50jk-vizd`) and
samples only while a jk-viz is open; it serves administrators (root and
group `wheel`) on `/run/jk-viz.sock`. `sudo jk-vizd --dump` prints one
sample as JSON. `JK_VIZD=no` in `/etc/jk_os/jk-viz` turns it off.

## jk-charge-monitor: the battery

`jk-charge-monitor` ("Charge Monitor" in the desktop's menu) shows the
battery live: level, charging or discharging and how fast (W, mA, % per
hour), time to full (or to the battery care limit) or to empty, voltage,
temperature, capacity and wear (where the fuel gauge reports them: laptops'
do, the tablet's doesn't), the charger and its settings, and the last hour
as graphs. It reads `/sys/class/power_supply` (another tree with
`JK_POWER_SUPPLY_DIR`), so it works on laptops as on the tablet.

## Packages (apt)

The installer asks for a **package management** choice after the accounts:

1. **apt**: Debian packages (Debian 13 "trixie"), with the usual commands:
   `sudo apt update`, `sudo apt install <name>`, `sudo apt remove <name>`,
   `apt search`, `dpkg -l`, ... Package sources are added with
   `sudo add-apt-repository` (or `apt-add-repository`), e.g.
   `sudo add-apt-repository contrib non-free` or
   `sudo add-apt-repository -k https://example.com/key.asc "deb https://example.com/apt stable main"`
   (Debian 13 dropped `software-properties-common`, so this one is jk_os's own:
   `add-apt-repository --help`; `-k URL` stores a repository's signing key.) The Debian
   system is `/data/apt/root`, so a repository's key goes in
   `/data/apt/root/etc/apt/keyrings/` (seen there as `/etc/apt/keyrings/`). Ubuntu PPAs
   (`ppa:...`) are built for Ubuntu, not Debian, and usually don't install here.
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
| Classic build tools | **GNU make 4.4** (`make`, `gmake`), **m4 1.4**, **flex 2.6** (`lex`), **bison 3.8** (`yacc`), **Perl 5.44**, **autoconf 2.73**, **automake 1.19**, **libtool 2.6**, `pkg-config` (pkgconf); `autoreconf -fi && ./configure && make` works. Perl is native builds only (not on aarch64 built on x86_64) |
| Interpreters | **bash 5.3** (patch level 20; `/bin/bash` too, and allowed as a login shell: `chsh -s /bin/bash`) and **Python 3.14** (`python3`, `python`) with OpenSSL (`ssl`, `hashlib`, HTTPS), SQLite, bz2, lzma, zlib, ctypes, readline, curses, uuid, and **pip**, **venv** and **ensurepip**. `python3 -m venv ~/venv && ~/venv/bin/pip install ...`; `pip install --user ...` installs into `~/.local`. Python is native builds only, like Perl |
| Debugger | **GDB 18.1** and `gdbserver` |
| Headers | glibc (from the toolchain that builds jk_os) and Linux (`make headers_install` from jk_os's kernel), and every library in the image: headers, pkg-config and CMake files, static libraries (OpenGL/EGL/Vulkan, Wayland/X11, GLib, D-Bus, PipeWire, ALSA, OpenSSL, curl, ...). Qt and KDE Frameworks' headers are there, but building Qt programs needs Qt's code generators (moc, rcc, uic), which jk_os doesn't carry yet |

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
- **Branding**: the desktop's logo, launcher icon (`jk-os`), wallpaper (`jk_os`), splash screen and the Global Themes' previews come from `J.K. Logo.png`. After changing the logo, run `scripts/make-branding.py` (needs Python with Pillow and NumPy) to regenerate the images under `rootfs/usr/share/`, then `make`.
- **The desktop's look**: two Global Themes, **jk_os Light** (`org.jk_os.desktop`, the default) and **jk_os Dark** (`org.jk_os.desktop.dark`), set as the light/dark pair in `rootfs/etc/xdg/kdeglobals` (System Settings → Global Theme switches between them, or follows the time of day). Both have a menu bar along the top (the J.K. Robotics menu, the active application and its menus, tray, clock) and a floating dock, in glass: KWin blurs and saturates what is behind panels, pop-ups, title bars and menus. Title bars have red, yellow and green buttons on the left and rounded corners (Aurorae, with `configs/desktop/patches/aurorae`); the fonts are Inter and FiraCode Nerd Font Mono; Konsole opens a translucent profile, *JK Glass*. The colour schemes, title bars and Plasma style are generated by `scripts/make-theme.py`; KWin's settings are in `rootfs/etc/xdg/kwinrc`. The panel layout is set up on a user's first desktop; for an existing one, apply the Global Theme with *Desktop and window layout* ticked.
- **Kernel options**: `make menuconfig` to explore, then put the options you want to keep in `configs/kernel/common.config` or `configs/kernel/<arch>.config`. Fragments are re-applied whenever they change. The build warns if an option you asked for was dropped.
- **Clean**: `make clean` (the current target's arch) or `make distclean` (all of `build/` and `out/`; source trees are left alone).

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
