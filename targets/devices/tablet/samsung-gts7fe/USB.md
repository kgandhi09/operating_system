# SM-T733 USB-C bring-up

The Samsung profile now implements dual-role USB and the DisplayPort path.
These changes are build-validated; USB enumeration, charging transitions,
and external video must still be validated on an SM-T733. A successful build
does not establish that the hardware bring-up is complete.

USB host with a keyboard and mouse on a hub works on the device
(2026-10-06). DisplayPort (`&mdss_dp`) is enabled since then, with two lanes
(pin assignment D, beside USB 3, as on the Fairphone 5). msm binds the
internal panel only after every enabled display interface has probed, and DP
waits for its whole bridge chain (QMP PHY, PS5169, SM5714 connector): if the
tablet's own screen stays dark after this change, that chain is the suspect
(`sudo jk-update --rollback`, or set `&mdss_dp` back to `disabled`).

On 2026-10-07, an external 1920x1080 display was detected with DP alt mode
active but went black when the desktop enabled it. The kernel logged
`LM_2/LM_3, invalid DSPP_-1` and `failed to reserve hw resources: -119`.
The SC7280 DPU has a DSPP only on LM_0, while DPU advertised gamma/CTM
on every CRTC. Patch 0012 stops advertising those color controls when the
mixers do not all have DSPPs. This preserves the external DP resource path;
the resulting boot image still needs testing on the tablet.

## Hardware and software

| Component | Stock wiring | JK OS implementation |
| --- | --- | --- |
| SM5714 Type-C/PD | I2C8, address 0x33, interrupt GPIO142 | TCPM transport, connector discovery, role negotiation and PD messages |
| SM5714 charger | I2C3, address 0x49 | Owns boost and charging; serializes Type-C, polling and user changes |
| SM5440 direct charger | I2C9, address 0x63 | Requests PPS through TCPM and hands the battery path back to SM5714 after returning to fixed 5 V |
| SM5714 MUIC | I2C3, address 0x25 | Connects D+/D- for host operation, including an unpowered hub |
| DWC3 | USB1 at 0x0a600000 | `dr_mode = "otg"`, role switch linked to connector HS endpoint |
| QMP USB3/DP PHY | 0x088e8000, PM7325 L1 at 0.912 V, L6 at 1.2 V | Orientation and lane-mode switching |
| PS5169 | I2C6, address 0x28 | USB/DP mux and DRM AUX bridge, SM7325 equalizer settings |
| DP AUX switch | GPIO99 select, GPIO100 active-low enable | `gpio-sbu-mux`; L2B supplies AUX pull-up |
| DisplayPort | MDSS DP at 0x0ae90000 | DP source, Type-C HPD bridge, C/D/E pin assignments |

The controller can source VBUS for a passive hub or sink power while hosting
a PD dock. The SM5714 switching path uses fixed 5 V. The SM5440 direct path
starts disabled; `sudo jk-charge direct auto` or `full` enables PPS when the
source advertises a compatible APDO and the battery is within the driver's
temperature and charge window. On any PPS or pump fault, the pump stops and
TCPM returns to fixed 5 V before SM5714 resumes. Hardware validation of this
handoff on the SM-T733 is pending. The switching input limit is the smaller of the user's charger setting and
TCPM's advertised/negotiated limit (default Rp, which a USB-A charger and a PC
port both show, allows the full 3 A and relies on AICL, as before). The
charger follows TCPM only while the SM5714 Type-C driver is bound: should it
fail to probe, charging falls back to plain VBUS detection. Source PDO
advertises 5 V / 500 mA; the boost hardware uses Samsung's 5.1 V / 900 mA operating point. Charging cannot
reprogram the boost while USB host power is active, and boost is refused
while incoming VBUS is present. `sm5714_charger.enable_charging=0` also
prevents enabling boost because it requests that the charger be left alone.

USB data role and power role are independent: a powered dock may correctly
show `host` together with `sink`. DisplayPort needs successful PD altmode
negotiation in addition to USB enumeration. A DisplayLink USB graphics adapter
uses a different driver stack and is not covered by this DP implementation.

The USB console gadget is prepared even when booting with a hub attached.
Console startup waits in the background for device mode; once registered,
the gadget core rebinds the console when DWC3 returns from host to device.
The initramfs console remains unauthenticated only during rescue/install;
the installed-system service continues to require login.

## Build and install

With the existing SM-T733 `build.conf`, `make` builds the kernel, device tree,
boot package and OS squashfs. Both the boot image and OS image must be updated
to include the console startup changes and `jk-usb-status` utility.

Use the normal Download-mode/Odin or `make flash` workflow for the boot
package. Preserve a copy of the previously working package for rollback.
Then use the existing OS-update workflow, or the rescue menu and:

```sh
scripts/tablet-install.sh --no-format
```

`--no-format` preserves the existing JK_DATA/home/settings. No flash or
tablet partition write is performed by the implementation/build itself.

## On-device acceptance checks

1. Start with the hub disconnected. Connect the PC cable and verify the
   existing USB network and serial console. Check charging current/status.
2. Disconnect the PC, attach the hub with only keyboard and mouse, and check
   `lsusb` and actual input. Test both USB-C plug orientations.
3. Repeat with the hub present during boot. Then unplug it and reconnect the
   PC cable; verify that the console appears without rebooting.
4. If the hub accepts power, test attaching/removing its charger. The tablet
   should keep hosting its USB devices while it becomes a power sink.
5. Check that the tablet's own screen still comes up. Then attach the
   external monitor. Check `/sys/class/drm/card*-DP-*/status` and
   available modes, then enable the monitor in Plasma's display settings.
   Repeat after a reboot with the powered dock connected.

Collect a report after each failing scenario, while the hub remains attached:

```sh
sudo jk-usb-status > /tmp/usb-status.txt
```

Use the tablet touchscreen or Wi-Fi SSH to collect it: the USB-C port cannot
simultaneously be the PC gadget console and the hub host. The report reads
state and logs; it does not change USB roles or write hardware registers.
An empty `/sys/class/typec` or deferred SM5714/PS5169 probe is an integration
failure; a host role with root hubs but no attached hub points to data routing,
power or link bring-up. USB devices working without a DP connector/modes
narrows the remaining problem to the display/PD/AUX path.

## Source provenance

- Board addresses, GPIOs and supply mappings: extracted SM-T733 firmware
  `T733XXS9DYF1`, `build/stock/.../dtb/00-00.dts` and `dtbo/entry.0.dts`
  (the corresponding USB wiring is also present in revisions 1 and 2).
- GPL Type-C transport and redriver starting point:
  [Tab S9 Ultra Linux port](https://github.com/agcarbajo/postmarketos-galaxy-tab-s9-ultra/tree/b1dcca03fdc7952de60c3e9ab49e860b6bd45e8c/pmaports/device/testing/linux-samsung-gts9uwifi-mainline).
  Adaptations use JK OS's charger, supplier references instead of singleton
  APIs, standard TCPM without retained-role core patches, a USB/DP mux callback
  for both USB and altmode transitions, and DRM AUX/HPD bridges.
- SM7325 register checks and tuning:
  [Samsung kernel source](https://github.com/LineageOS/android_kernel_samsung_sm7325/tree/12334aaea98147c1739de857b4f36f8949a3f095),
  specifically `drivers/usb/typec/sm/sm5714`,
  `drivers/battery/charger/sm5714_charger/sm5714_charger_oper.c`,
  `drivers/muic/sm/sm5714` and `drivers/redriver/ps5169.c`.

The S9-specific OTG-detect GPIO pulse and TCPM core quirks are not imported.
The SM5440 PPS charge-pump sequence is adapted from that port's GPL driver.
Warm powered-dock state is reset through a CC detach before
standard TCPM negotiation. Runtime suspend remains outside this board's
current bring-up scope.
