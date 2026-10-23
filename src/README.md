# src: jk_os's own tools

One directory per tool, holding everything that tool puts into the image.
`scripts/build-desktop.sh` builds every `src/*/` with a `CMakeLists.txt`
(rebuilt when its files or `common/` change); each tool's `install()` rules
say where its parts go. A new tool needs only its directory.

```
src/<tool>/
  CMakeLists.txt   builds and installs the parts below
  cli/             the command(s): shell scripts or C++
  gui/             the window (Qt Quick)
  daemon/          a background service
  init/            its boot script (/etc/init.d/S<NN>name)
  data/            desktop entries, autostart, D-Bus/PAM/polkit/udev files,
                   profile.d/<tool>.sh (what it adds to login shells), ...
  lib/, session/   internal helpers (/usr/lib/jk_os)
```

Only the directories a tool needs exist.

| Tool | What it is |
|---|---|
| `jk-apt` | Debian packages in their own container on `/data` (`apt`, `dpkg`, `add-apt-repository`, `apt-setup`) |
| `jk-charge` | battery charging settings, `jk-charge gui` |
| `jk-cuda` | the CUDA toolkit on demand, `prime-run` |
| `jk-dev` | the dev session on tty1 |
| `jk-gui` | the KDE Plasma desktop on the logged-in console |
| `jk-install` | installing jk_os, first boot, the live system, console login |
| `jk-power` | CPU, GPU and temperature settings, `jk-power gui` |
| `jk-timedate` | the clock for KDE's Date & Time settings: its daemon `jk-timedated` (`org.freedesktop.timedate1`) |
| `jk-update` | OS updates, the release watcher, Software Update and its notification |
| `jk-viz` | the system map (`jk-viz`) and its collector (`jk-vizd`) |

`common/` is shared: `gui/` is compiled into the tools' windows (the CLI
settings bridge, the administrator password dialog), `lib/` goes to
`/usr/lib/jk_os` (`jk-settings-gui`, `jk-audio`, `gpu.sh`).

Device-specific tools (the tablet's `jk-crashlog`, `jk-stress`,
`jk-usb-status`) stay with their device under `config/targets/devices/`:
the tools here are built once per arch, for every device of it.
