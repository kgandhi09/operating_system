# Android boot image tools (AOSP)

Host tools for devices whose bootloader loads an Android boot image
(`DEVICE_BOOT=android-bootimg`), vendored so builds stay offline. Unmodified
copies from AOSP:

| File | From | Commit |
|---|---|---|
| `mkbootimg.py`, `unpack_bootimg.py`, `gki/generate_gki_certificate.py` | platform/system/tools/mkbootimg | d2bb0af5ba6d3198a3e99529c97eda1be0b5a093 |
| `avbtool.py`, `LICENSE.avb` | platform/external/avb | 761178607206f4cb2af79ed9eec52d8cbd814adb |
| `mkdtboimg.py` | platform/system/libufdt (utils/src) | 9b70f594cf149c7c73989c78f03d210f8aa1c1f6 |

mkbootimg and libufdt are Apache-2.0 (see each file's header); avb's licenses
are listed in `LICENSE.avb`.

Used by `scripts/build-bootimg.sh` (boot.img, vbmeta.img, dtbo.img) and
`scripts/inspect-stock.sh` (reading a stock firmware).
