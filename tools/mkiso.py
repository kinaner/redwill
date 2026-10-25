#!/usr/bin/env python3
"""Minimal ISO9660 + El Torito (BIOS, no-emulation) writer. Usage: mkiso.py boot.bin out.iso [--pad-mb N]"""
import struct, sys, time, math
SEC = 2048
def b32(v): return struct.pack('<I', v) + struct.pack('>I', v)
def b16(v): return struct.pack('<H', v) + struct.pack('>H', v)
def astr(s, n): return s.encode().ljust(n, b' ')[:n]
now = time.gmtime()
def d7(): return bytes([now.tm_year - 1900, now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, 0])
def d17(): return time.strftime('%Y%m%d%H%M%S', now).encode() + b'00' + b'\0'
def dirrec(name, lba, size, flags):
    n = len(name); ln = 33 + n + (1 if n % 2 == 0 else 0)
    r = bytes([ln, 0]) + b32(lba) + b32(size) + d7() + bytes([flags, 0, 0]) + b16(1) + bytes([n]) + name
    if n % 2 == 0: r += b'\0'
    assert len(r) == ln
    return r
def pad(data, mult=SEC): return data + b'\0' * ((-len(data)) % mult)

def main():
    args = sys.argv[1:]
    padmb = 0
    if '--pad-mb' in args:
        i = args.index('--pad-mb'); padmb = int(args[i + 1]); del args[i:i + 2]
    boot = pad(open(args[0], 'rb').read())
    readme = pad(b"RedWill OS - bootable i386 kernel.\r\nThis CD is bootable via BIOS (El Torito).\r\n")
    boot_secs = len(boot) // SEC
    L_ROOT, L_CAT, L_BOOT = 21, 22, 23
    L_README = L_BOOT + boot_secs
    L_PAD = L_README + len(readme) // SEC
    pad_bytes = padmb * 1024 * 1024
    pad_secs = math.ceil(pad_bytes / SEC)
    total = L_PAD + pad_secs

    pvd = bytearray(SEC)
    pvd[0] = 1; pvd[1:6] = b'CD001'; pvd[6] = 1
    pvd[8:40] = astr('REDWILL', 32); pvd[40:72] = astr('REDWILL', 32)
    pvd[80:88] = b32(total); pvd[120:124] = b16(1); pvd[124:128] = b16(1); pvd[128:132] = b16(SEC)
    pvd[132:140] = b32(10)
    pvd[140:144] = struct.pack('<I', 19); pvd[148:152] = struct.pack('>I', 20)
    pvd[156:190] = dirrec(b'\x00', L_ROOT, SEC, 2)
    for off in (190, 318, 446, 574): pvd[off:off + 128] = astr('', 128)
    for off in (702, 739, 776): pvd[off:off + 37] = astr('', 37)
    for off in (813, 830): pvd[off:off + 17] = d17()
    pvd[847:864] = b'0' * 16 + b'\0'; pvd[864:881] = b'0' * 16 + b'\0'
    pvd[881] = 1

    brvd = bytearray(SEC)
    brvd[0] = 0; brvd[1:6] = b'CD001'; brvd[6] = 1
    brvd[7:39] = b'EL TORITO SPECIFICATION'.ljust(32, b'\0')
    brvd[71:75] = struct.pack('<I', L_CAT)
    term = bytearray(SEC); term[0] = 255; term[1:6] = b'CD001'; term[6] = 1

    lpt = pad(struct.pack('<BBIH', 1, 0, L_ROOT, 1) + b'\0' + b'\0')
    mpt = pad(struct.pack('>BBIH', 1, 0, L_ROOT, 1) + b'\0' + b'\0')

    recs = [(b'\x00', L_ROOT, SEC, 2), (b'\x01', L_ROOT, SEC, 2),
            (b'BOOT.IMG;1', L_BOOT, len(boot), 0)]
    if pad_bytes: recs.append((b'PADDING.BIN;1', L_PAD, pad_bytes, 0))
    recs.append((b'README.TXT;1', L_README, len(readme), 0))
    root = pad(b''.join(dirrec(*r) for r in recs))

    val = bytearray(32); val[0] = 1; val[1] = 0; val[4:28] = b'REDWILL'.ljust(24, b'\0'); val[30] = 0x55; val[31] = 0xAA
    s = sum(struct.unpack('<16H', bytes(val))) & 0xFFFF
    struct.pack_into('<H', val, 28, (-s) & 0xFFFF)
    assert sum(struct.unpack('<16H', bytes(val))) & 0xFFFF == 0
    ent = bytearray(32); ent[0] = 0x88; ent[1] = 0
    struct.pack_into('<H', ent, 2, 0); struct.pack_into('<H', ent, 6, len(boot) // 512); struct.pack_into('<I', ent, 8, L_BOOT)
    cat = pad(bytes(val) + bytes(ent))

    mbr = bytearray(open(args[2], 'rb').read()) if len(args) > 2 else None
    sysarea = bytearray(16 * SEC)
    if mbr is not None:
        assert len(mbr) <= 446, len(mbr)
        i = bytes(mbr).find(struct.pack('<I', 0xCAFEF00D)); assert i >= 0 and bytes(mbr).count(struct.pack('<I', 0xCAFEF00D)) == 1
        mbr[i:i+4] = struct.pack('<I', L_BOOT * 4)
        j = bytes(mbr).find(struct.pack('<H', 0xC0DE)); assert j >= 0
        mbr[j:j+2] = struct.pack('<H', len(boot) // 512)
        sysarea[0:len(mbr)] = mbr
        pt = bytearray(16); pt[0] = 0x80; pt[1:4] = bytes([0, 2, 0]); pt[4] = 0x83; pt[5:8] = bytes([0xFE, 0xFF, 0xFF])
        struct.pack_into('<II', pt, 8, 1, total * 4 - 1); sysarea[446:462] = pt
        sysarea[510] = 0x55; sysarea[511] = 0xAA
    with open(args[1], 'wb') as f:
        f.write(bytes(sysarea))
        for blk in (pvd, brvd, term, lpt, mpt, root, cat, boot, readme): f.write(blk)
        left = pad_secs * SEC
        z = b'\0' * (1 << 20)
        while left > 0:
            n = min(left, len(z)); f.write(z[:n]); left -= n
    print(f"wrote {args[1]}: {total * SEC / 1048576:.1f} MiB, boot image {len(boot)} bytes ({len(boot)//512} x 512B sectors)")
main()
