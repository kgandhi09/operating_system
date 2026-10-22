# config

What jk_os builds, and how. Three folders, each answering one question:

| Folder        | Question                        | Contents |
|---------------|---------------------------------|----------|
| `presets/`    | Which image do I build?         | One `<preset>.conf` per image: category, device, kernel, arch, name. Built with `make PRESET=<preset>`. |
| `targets/`    | What is this hardware?          | `devices/<category>/<device>/` (boot format, archs, kernels, rootfs/initramfs overlays, U-Boot env) and `kernels/<kernel>/` (source tree, archs, DTS, patches). |
| `components/` | How is each part built?         | Settings shared by every target: kernel and BusyBox config fragments, toolchain and desktop patches, source/firmware/binary/Debian lists, `initramfs.list`. |

A preset picks one device and kernel from `targets/`; the build then applies
`components/` plus that device's and kernel's own files.

```sh
make presets                 # list the presets
make config                  # make a new preset (or change one: PRESET=<preset>)
make PRESET=tablet-gts7fe    # build one
```
