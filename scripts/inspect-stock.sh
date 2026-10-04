#!/usr/bin/env bash
# Read a device's stock Android firmware, for porting jk_os to it: what its
# bootloader expects from a boot image, and what its device tree says about
# the hardware. Changes nothing on the device; reads only the files given.
#
#   scripts/inspect-stock.sh <firmware.tar[.md5]>... [-o <dir>]
#
# Give it a Samsung firmware's AP_ and BL_ tars (or any tar holding Android
# images: boot, vendor_boot, dtbo, vbmeta, recovery; plain or .lz4). It
# writes into <dir> (default build/stock/<name of the first tar>):
#
#   images/           the images, decompressed
#   boot/             unpack_bootimg's output: kernel, ramdisk, dtb, ...
#   dtb/NN.dts        every device tree in boot / vendor_boot, decompiled
#   dtbo/NN.dts       every overlay in dtbo.img, decompiled
#   vendor/firmware/  the vendor partition's /firmware, from super.img: the
#                     firmware of the device's chips, for its device.env's
#                     DEVICE_STOCK_FIRMWARE
#   summary.txt       what device.env and the board's .dts need: boot image
#                     header version, page size, base and offsets, command
#                     line, kernel version, and each tree's model,
#                     qcom,msm-id and qcom,board-id; vbmeta's flags
JK_NO_TARGET=1
source "$(dirname "$0")/common.sh"
need tar dtc fdtget python3

AND="$ROOT_DIR/scripts/android"
tars=() out=
while (( $# )); do
    case "$1" in
        -o) out=$2; shift ;;
        -h|--help) sed -n '2,/^source /p' "$0" | sed '$d; s/^# \{0,1\}//'; exit 0 ;;
        *) [[ -f "$1" ]] || die "no such file: $1"; tars+=("$(realpath "$1")") ;;
    esac
    shift
done
(( ${#tars[@]} )) || die "usage: $0 <firmware.tar[.md5]>... [-o <dir>]"
[[ -n "$out" ]] || out="$ROOT_DIR/build/stock/$(basename "${tars[0]}" | sed 's/\.tar\(\.md5\)\{0,1\}$//')"
rm -rf "$out"
mkdir -p "$out/images" "$out/dtb" "$out/dtbo"
out=$(realpath "$out")
cd "$out"

# ---------------------------------------------------------------- the images
want='^(boot|vendor_boot|init_boot|dtbo|vbmeta|vbmeta_system|recovery)\.img(\.lz4)?$'
for t in "${tars[@]}"; do
    while IFS= read -r f; do
        [[ "$f" =~ $want ]] || continue
        tar -xf "$t" -C images "$f"
        if [[ "$f" == *.lz4 ]]; then
            need lz4
            lz4 -dqf "images/$f" "images/${f%.lz4}" && rm "images/$f"
        fi
    done < <(tar -tf "$t")
done
ls images/*.img >/dev/null 2>&1 || die "no boot, dtbo or vbmeta image in ${tars[*]}"
log "images: $(cd images && echo *.img)"

# ---------------------------------------------------------------- vendor
# The vendor partition's /firmware, out of super.img in one pass over the tar
# (the image is several GB; only the vendor partition is written, then only
# /firmware kept).
for t in "${tars[@]}"; do
    super=$(tar -tf "$t" | grep -m1 -E '^super\.img(\.lz4)?$' || true)
    [[ -n "$super" ]] || continue
    need debugfs
    log "vendor partition: extracting /firmware from $super"
    if [[ "$super" == *.lz4 ]]; then
        need lz4
        tar -xOf "$t" "$super" | lz4 -dc | python3 "$AND/lpextract.py" - vendor:vendor.img
    else
        tar -xOf "$t" "$super" | python3 "$AND/lpextract.py" - vendor:vendor.img
    fi
    mkdir -p vendor
    if debugfs -R "rdump /firmware vendor" vendor.img >/dev/null 2>&1 && [[ -d vendor/firmware ]]; then
        log "vendor/firmware: $(find vendor/firmware -type f | wc -l) files"
    else
        warn "could not read the vendor partition (not ext4?): no vendor/firmware"
    fi
    rm -f vendor.img
    break
done

# split_dtbs <file> <prefix>: write each flattened device tree found in a file
# (Qualcomm boot images concatenate several, one per board revision).
split_dtbs() {
    python3 - "$1" "$2" <<'PY'
import struct, sys
data = open(sys.argv[1], 'rb').read()
n = i = 0
while (i := data.find(b'\xd0\x0d\xfe\xed', i)) >= 0:
    size = struct.unpack('>I', data[i + 4:i + 8])[0]
    if 0x40 < size <= len(data) - i:
        open(f'{sys.argv[2]}{n:02d}.dtb', 'wb').write(data[i:i + size])
        n += 1
        i += size
    else:
        i += 4
PY
}

# describe_dtb <file.dtb>: decompile it next to itself, print its identity.
describe_dtb() {
    dtc -q -I dtb -O dts -o "${1%.dtb}.dts" "$1" 2>/dev/null || true
    printf '  %s: model "%s"\n' "${1%.dtb}.dts" "$(fdtget "$1" / model 2>/dev/null || echo ?)"
    printf '      compatible  %s\n' "$(fdtget "$1" / compatible 2>/dev/null || echo -)"
    printf '      msm-id      %s\n' "$(fdtget -t x "$1" / qcom,msm-id 2>/dev/null || echo -)"
    printf '      board-id    %s\n' "$(fdtget -t x "$1" / qcom,board-id 2>/dev/null || echo -)"
}

{
echo "stock firmware: ${tars[*]##*/}"
echo

# ---------------------------------------------------------------- boot.img
if [[ -f images/boot.img ]]; then
    mkdir -p boot
    python3 "$AND/unpack_bootimg.py" --boot_img images/boot.img --out boot > boot/info.txt 2>&1 \
        || warn "unpack_bootimg could not read boot.img (see boot/info.txt)"
    echo "== boot.img (unpack_bootimg)"
    sed 's/^/  /' boot/info.txt
    echo
    echo "  as mkbootimg arguments:"
    echo "  (for DEVICE_MKBOOTIMG_ARGS; the files and the command line left out)"
    python3 "$AND/unpack_bootimg.py" --boot_img images/boot.img --out boot --format=mkbootimg 2>/dev/null \
        | python3 -c '
import shlex, sys
a, out = shlex.split(sys.stdin.read()), []
skip = {"--kernel", "--ramdisk", "--dtb", "--second", "--recovery_dtbo", "--cmdline", "--board"}
while a:
    if a[0] in skip: a = a[2:]
    else: out.append(a.pop(0))
print("    " + " ".join(out))' || true
    echo
    if [[ -f boot/kernel ]]; then
        k=boot/kernel
        if [[ "$(head -c2 "$k" | od -An -tx1 | tr -d ' ')" == 1f8b ]]; then
            echo "  kernel: gzip-compressed Image"
            gzip -dc "$k" > boot/kernel.raw 2>/dev/null || true
            k=boot/kernel.raw
        fi
        echo "  kernel version: $(strings "$k" | grep -m1 '^Linux version' || echo '?')"
        # A device tree appended to the kernel (boot image header v0/v1).
        split_dtbs boot/kernel boot/appended-
        compgen -G 'boot/appended-*.dtb' >/dev/null && echo "  device trees appended to the kernel: $(ls boot/appended-*.dtb | wc -l)"
    fi
    echo
fi

# ---------------------------------------------------------------- vendor_boot
if [[ -f images/vendor_boot.img ]]; then
    mkdir -p vendor_boot
    python3 "$AND/unpack_bootimg.py" --boot_img images/vendor_boot.img --out vendor_boot > vendor_boot/info.txt 2>&1 || true
    echo "== vendor_boot.img"
    sed 's/^/  /' vendor_boot/info.txt
    echo
fi

# ---------------------------------------------------------------- for device.env
# The boot image format, from boot.img and vendor_boot.img together: only
# the header, version and load address fields; jk_os gives its own files.
echo "== DEVICE_MKBOOTIMG_ARGS"
for img in boot vendor_boot; do
    [[ -f "images/$img.img" ]] || continue
    python3 "$AND/unpack_bootimg.py" --boot_img "images/$img.img" --out "$img" --format=mkbootimg 2>/dev/null || true
    echo
done | python3 -c '
import shlex, sys
keep = ["--header_version", "--os_version", "--os_patch_level", "--pagesize", "--base",
        "--kernel_offset", "--ramdisk_offset", "--second_offset", "--tags_offset", "--dtb_offset",
        "--board"]
seen = {}
for line in sys.stdin:
    a = shlex.split(line)
    for i, x in enumerate(a[:-1]):
        if x in keep and a[i + 1]: seen.setdefault(x, a[i + 1])
print("  DEVICE_MKBOOTIMG_ARGS=\"" + " ".join(f"{k} {seen[k]}" for k in keep if k in seen) + "\"")'
echo

# ---------------------------------------------------------------- device trees
srcs=()
for f in boot/dtb vendor_boot/dtb boot/appended-*.dtb; do [[ -f "$f" ]] && srcs+=("$f"); done
echo "== device trees (from ${srcs[*]:-nowhere})"
i=0
for f in "${srcs[@]}"; do
    split_dtbs "$f" "dtb/$(printf '%02d' $i)-"
    i=$(( i + 1 ))
done
for d in dtb/*.dtb; do [[ -f "$d" ]] && describe_dtb "$d"; done
echo

# ---------------------------------------------------------------- dtbo.img
if [[ -f images/dtbo.img ]]; then
    echo "== dtbo.img (overlays the bootloader applies over the device tree)"
    python3 "$AND/mkdtboimg.py" dump images/dtbo.img -b dtbo/entry > dtbo/info.txt 2>&1 || true
    grep -E 'dt_entry_count|id = |rev = |custom' dtbo/info.txt | sed 's/^/  /' || true
    for d in dtbo/entry.*; do
        [[ -f "$d" ]] || continue
        mv "$d" "$d.dtb"
        dtc -q -I dtb -O dts -o "${d}.dts" "$d.dtb" 2>/dev/null || true
    done
    echo
fi

# ---------------------------------------------------------------- vbmeta
for v in images/vbmeta*.img; do
    [[ -f "$v" ]] || continue
    echo "== ${v#images/} (avbtool info_image)"
    python3 "$AND/avbtool.py" info_image --image "$v" 2>&1 | grep -E 'Algorithm|Flags|Rollback|Partition Name|Image Size' | sed 's/^/  /' || true
    echo
done
} | tee summary.txt

log "written to $out (summary.txt)"
