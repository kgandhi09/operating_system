#!/usr/bin/env python3
"""Check that jk_os's device tree reserves every memory region the stock
firmware does.

    check-reserved-memory.py <jk_os.dtb> <stock base .dtb>... -- <stock dtbo entry .dtb>...

The stock tree the bootloader really uses is a base tree with Samsung's board
overlay (dtbo) applied, and the overlay can move or grow regions: on the
Galaxy Tab S7 FE it grows removed@c0000000 by 14 MB that the secure firmware
owns. Linux using such memory is fatal (a reset as soon as a busy system
touches it). So each base tree is merged with each dtbo entry that applies to
it (fdtoverlay), and every fixed region in the result (a reg, not reusable,
not disabled) must lie inside jk_os's reserved regions, no-map ones where the
stock region is no-map. Regions only allocated at run time (size and
alloc-ranges, no reg) are Android drivers' and are left out.

Exits 1 and names the regions when one is missing. Needs dtc and fdtoverlay.
"""
import os
import re
import subprocess
import sys
import tempfile


def decompile(dtb):
    return subprocess.run(["dtc", "-q", "-I", "dtb", "-O", "dts", dtb],
                          check=True, capture_output=True, text=True).stdout


def cells(text):
    return [int(c, 0) for c in text.split()]


def reserved_regions(dts):
    """[(name, start, end, nomap)] of /reserved-memory's fixed regions."""
    out = []
    path = []
    props = {}
    ac = sc = 2
    for raw in dts.splitlines():
        line = raw.strip()
        # "name {", or with labels (a newer dtc prints them): "lbl: name {"
        m = re.match(r'^(?:[\w]+:\s*)*([^\s=;:]+)\s*\{$', line)
        if m:
            path.append(m.group(1))
            props[len(path)] = {}
            continue
        if line == "};":
            depth = len(path)
            node = props.pop(depth, {})
            if depth == 3 and path[1] == "reserved-memory":
                reg = node.get("reg")
                if reg and "reusable" not in node and node.get("status") not in ('"disabled"',):
                    v = cells(reg)
                    step = ac + sc
                    for i in range(0, len(v) - step + 1, step):
                        start = 0
                        for c in v[i:i + ac]:
                            start = (start << 32) | c
                        size = 0
                        for c in v[i + ac:i + step]:
                            size = (size << 32) | c
                        if size:
                            out.append((path[-1], start, start + size - 1, "no-map" in node))
            path.pop()
            continue
        m = re.match(r'^([#\w,.-]+)\s*(?:=\s*(.*))?;$', line)
        if m and path:
            key, val = m.group(1), (m.group(2) or "")
            if val.startswith("<") and val.endswith(">"):
                val = val[1:-1]
            props[len(path)][key] = val
            if len(path) == 2 and path[1] == "reserved-memory":
                if key == "#address-cells":
                    ac = int(val, 0)
                elif key == "#size-cells":
                    sc = int(val, 0)
    return out


def covered(start, end, regions):
    """Whether [start, end] lies inside the union of regions."""
    pos = start
    for _, s, e, _ in sorted(regions, key=lambda r: r[1]):
        if s <= pos <= e:
            pos = e + 1
            if pos > end:
                return True
    return pos > end


def main(argv):
    if "--" not in argv or len(argv) < 4:
        sys.exit(__doc__)
    ours_dtb = argv[1]
    sep = argv.index("--")
    bases, overlays = argv[2:sep], argv[sep + 1:]
    ours = reserved_regions(decompile(ours_dtb))
    ours_nomap = [r for r in ours if r[3]]

    missing = {}
    merged_any = False
    with tempfile.TemporaryDirectory() as tmp:
        for b in bases:
            for o in overlays:
                merged = os.path.join(tmp, "m.dtb")
                r = subprocess.run(["fdtoverlay", "-i", b, "-o", merged, o],
                                   capture_output=True, text=True)
                if r.returncode:
                    continue  # this dtbo entry is for another base tree
                merged_any = True
                for name, s, e, nomap in reserved_regions(decompile(merged)):
                    pool = ours_nomap if nomap else ours
                    if not covered(s, e, pool):
                        key = (name, s, e, nomap)
                        missing.setdefault(key, []).append(
                            f"{os.path.basename(b)} + {os.path.basename(o)}")
    if not merged_any:
        sys.exit("check-reserved-memory: no dtbo entry applies to any stock base tree")
    if missing:
        print("check-reserved-memory: the stock firmware reserves memory jk_os's "
              "device tree doesn't:", file=sys.stderr)
        for (name, s, e, nomap), where in sorted(missing.items(), key=lambda i: i[0][1]):
            print(f"  {name}: 0x{s:x}-0x{e:x}{' (no-map)' if nomap else ''}"
                  f"  [{', '.join(sorted(set(where)))}]", file=sys.stderr)
        sys.exit(1)
    print(f"check-reserved-memory: every stock reserved region is reserved "
          f"({len(ours)} regions in {os.path.basename(ours_dtb)})")


if __name__ == "__main__":
    main(sys.argv)
