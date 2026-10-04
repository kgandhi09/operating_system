#!/usr/bin/env python3
# lpextract.py: copy logical partitions (vendor, system, ...) out of an
# Android super image, reading it once as a stream, so a multi-gigabyte
# super.img.lz4 inside a firmware tar never has to be unpacked to disk:
#
#   tar -xOf AP_....tar.md5 super.img.lz4 | lz4 -dc | lpextract.py - vendor:vendor.img
#   lpextract.py super.img --list
#
# Reads sparse images (simg) and raw ones. Only the partitions asked for are
# written. Format: system/core/fs_mgr/liblp (metadata) and libsparse.

import struct
import sys

SPARSE_MAGIC = 0xED26FF3A
CHUNK_RAW, CHUNK_FILL, CHUNK_DONT_CARE, CHUNK_CRC = 0xCAC1, 0xCAC2, 0xCAC3, 0xCAC4
GEOMETRY_MAGIC = 0x616C4467
HEADER_MAGIC = 0x414C5030
RESERVED = 4096          # LP_PARTITION_RESERVED_BYTES
SECTOR = 512


def die(msg):
    sys.stderr.write("lpextract: " + msg + "\n")
    sys.exit(1)


def blocks(stream):
    """Yield (offset, bytes) runs of the raw image, from a sparse or raw stream."""
    head = stream.read(28)
    if len(head) < 28:
        die("input too short")
    magic, = struct.unpack_from("<I", head)
    if magic != SPARSE_MAGIC:
        yield 0, head
        off = len(head)
        while True:
            buf = stream.read(1 << 20)
            if not buf:
                return
            yield off, buf
            off += len(buf)
    (_, _, _, file_hdr, chunk_hdr, blk, _, nchunks, _) = struct.unpack("<IHHHHIIII", head)
    stream.read(file_hdr - 28)
    off = 0
    for _ in range(nchunks):
        ch = stream.read(chunk_hdr)
        ctype, _, nblk, total = struct.unpack_from("<HHII", ch)
        size = nblk * blk
        data_len = total - chunk_hdr
        if ctype == CHUNK_RAW:
            left = size
            while left:
                buf = stream.read(min(left, 1 << 20))
                if not buf:
                    die("truncated sparse image")
                yield off, buf
                off += len(buf)
                left -= len(buf)
        elif ctype == CHUNK_FILL:
            pattern = stream.read(4)
            if pattern != b"\0\0\0\0":
                fill = pattern * (blk // 4)
                for i in range(nblk):
                    yield off + i * blk, fill
            off += size
        else:
            stream.read(data_len)
            if ctype == CHUNK_DONT_CARE:
                off += size


def parse_metadata(meta):
    """meta: the image's first bytes, from offset 0. Returns {name: [(start, length)]}."""
    g = RESERVED
    gmagic, _, = struct.unpack_from("<II", meta, g)
    if gmagic != GEOMETRY_MAGIC:
        die("no LP metadata geometry at 4096: not a super image")
    max_size, slots, _ = struct.unpack_from("<III", meta, g + 40)
    h = RESERVED + 2 * 4096  # after the geometry and its backup
    hmagic, _, _, hsize = struct.unpack_from("<IHHI", meta, h)
    if hmagic != HEADER_MAGIC:
        die("bad LP metadata header")
    tables_size, = struct.unpack_from("<I", meta, h + 44)
    descs = struct.unpack_from("<12I", meta, h + 80)
    tables = h + hsize
    p_off, p_num, p_sz, e_off, e_num, e_sz = descs[0:6]
    extents = []
    for i in range(e_num):
        nsec, ttype, tdata, _ = struct.unpack_from("<QIQI", meta, tables + e_off + i * e_sz)
        extents.append((nsec, ttype, tdata))
    parts = {}
    for i in range(p_num):
        name, _, first, num, _ = struct.unpack_from("<36sIIII", meta, tables + p_off + i * p_sz)
        name = name.rstrip(b"\0").decode()
        runs = []
        for nsec, ttype, tdata in extents[first:first + num]:
            if ttype == 0:  # LP_TARGET_TYPE_LINEAR
                runs.append((tdata * SECTOR, nsec * SECTOR))
        parts[name] = runs
    return parts


def main():
    args = sys.argv[1:]
    if not args:
        die("usage: lpextract.py <super.img|-> --list | <name>:<out.img> ...")
    stream = sys.stdin.buffer if args[0] == "-" else open(args[0], "rb")
    gen = blocks(stream)

    # The metadata sits in the first 1 MiB.
    meta = bytearray(1 << 20)
    pending = None
    for off, buf in gen:
        end = off + len(buf)
        if off < len(meta):
            n = min(end, len(meta)) - off
            meta[off:off + n] = buf[:n]
        if end >= len(meta):
            pending = (off, buf)
            break
    parts = parse_metadata(bytes(meta))

    if args[1:] == ["--list"]:
        for name, runs in parts.items():
            print("%-24s %10d MiB" % (name, sum(l for _, l in runs) >> 20))
        return

    wanted = {}
    for spec in args[1:]:
        name, _, out = spec.partition(":")
        if name not in parts:
            die("no partition %s (have: %s)" % (name, " ".join(parts)))
        f = open(out or name + ".img", "wb")
        f.truncate(sum(l for _, l in parts[name]))
        # (image offset, length, offset in the output) per extent
        pos = 0
        for start, length in parts[name]:
            wanted.setdefault(name, []).append((start, length, pos, f))
            pos += length
    runs = [r for rs in wanted.values() for r in rs]

    def copy(off, buf):
        end = off + len(buf)
        for start, length, pos, f in runs:
            a, b = max(off, start), min(end, start + length)
            if a < b:
                f.seek(pos + a - start)
                f.write(buf[a - off:b - off])

    if pending:
        copy(*pending)
    for off, buf in gen:
        copy(off, buf)
    for _, _, _, f in runs:
        f.close()


if __name__ == "__main__":
    main()
