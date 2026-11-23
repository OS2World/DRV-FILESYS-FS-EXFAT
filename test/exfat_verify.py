import struct, sys

path = sys.argv[1]
f = open(path, "rb")

def rd(off, n):
    f.seek(off)
    return f.read(n)

mbr = rd(0, 512)
print("MBR signature", mbr[510:512].hex())
ptype, = struct.unpack_from("<B", mbr, 446 + 4)
lba, cnt = struct.unpack_from("<II", mbr, 446 + 8)
print("partition type 0x%02X start %d sectors %d" % (ptype, lba, cnt))
base = lba * 512

vbr = rd(base, 512)
print("OEM", vbr[3:11], "boot sig", vbr[510:512].hex())
(partoff, vollen, fatoff, fatlen, heapoff, clcount, rootcl, serial, rev, flags,
 bps_sh, spc_sh, nfats, drvsel, pct) = struct.unpack_from("<QQIIIIIIHHBBBBB", vbr, 64)
print("PartitionOffset", partoff, "VolumeLength", vollen, "FatOffset", fatoff, "FatLength", fatlen)
print("ClusterHeapOffset", heapoff, "ClusterCount", clcount, "RootCluster", rootcl)
print("Serial %08X rev %04X flags %04X bytes/sector 2^%d sectors/cluster 2^%d fats %d" % (serial, rev, flags, bps_sh, spc_sh, nfats))
bps = 1 << bps_sh
cs = bps << spc_sh

# main boot region checksum (sectors 0..10), stored in sector 11
region = b"".join(rd(base + i * bps, bps) for i in range(11))
chk = 0
for i, b in enumerate(region):
    if i in (106, 107, 112):
        continue
    chk = (((chk << 31) | (chk >> 1)) + b) & 0xFFFFFFFF
sec11 = rd(base + 11 * bps, bps)
stored = set(struct.unpack_from("<I", sec11, k)[0] for k in range(0, bps, 4))
print("boot checksum computed %08X, sector 11 holds %s" % (chk, sorted("%08X" % x for x in stored)))
print("BOOT CHECKSUM", "OK" if stored == {chk} else "MISMATCH")
# backup boot region
bregion = b"".join(rd(base + (12 + i) * bps, bps) for i in range(11))
print("backup boot region identical:", bregion[:512] == vbr or bregion == region)

fat = rd(base + fatoff * bps, fatlen * bps)
def fatent(c):
    return struct.unpack_from("<I", fat, c * 4)[0]

def cluster_off(c):
    return base + heapoff * bps + (c - 2) * cs

def chain(c, nofat=False, length=None):
    out = []
    if nofat:
        n = (length + cs - 1) // cs
        return [c + i for i in range(n)]
    while 2 <= c < 0xFFFFFFF7:
        out.append(c)
        c = fatent(c)
        if len(out) > clcount:
            raise Exception("loop")
    return out

def read_dir(c, nofat=False, length=None):
    data = b""
    for cl in chain(c, nofat, length):
        data += rd(cluster_off(cl), cs)
    return data

def entryset_checksum(es):
    chk = 0
    for i, b in enumerate(es):
        if i in (2, 3):
            continue
        chk = (((chk << 15) | (chk >> 1)) + b) & 0xFFFF
    return chk

def namehash(name_utf16, upcase):
    h = 0
    for i in range(0, len(name_utf16), 2):
        ch = struct.unpack_from("<H", name_utf16, i)[0]
        ch = upcase.get(ch, ch)
        for b in (ch & 0xFF, ch >> 8):
            h = (((h << 15) | (h >> 1)) + b) & 0xFFFF
    return h

problems = []
files = 0
dirs = 0
bitmap = None
upcase = {}
used_clusters = set()
label = None

def walk(c, nofat, length, path):
    global files, dirs
    data = read_dir(c, nofat, length)
    i = 0
    while i < len(data):
        t = data[i]
        if t == 0:
            break
        if t == 0x85:
            sec = data[i + 1]
            es = data[i:i + 32 * (sec + 1)]
            stored = struct.unpack_from("<H", es, 2)[0]
            attr = struct.unpack_from("<H", es, 4)[0]
            if entryset_checksum(es) != stored:
                problems.append("entry set checksum mismatch at %s" % path)
            s = es[32:64]
            if s[0] != 0xC0:
                problems.append("missing stream extension at %s" % path)
                i += 32
                continue
            else:
                flagsb = s[1]
                namelen = s[3]
                nh = struct.unpack_from("<H", s, 4)[0]
                vdl, = struct.unpack_from("<Q", s, 8)
                first, = struct.unpack_from("<I", s, 20)
                dlen, = struct.unpack_from("<Q", s, 24)
                nm = b"".join(es[64 + 32 * k + 2:64 + 32 * k + 32] for k in range(sec - 1))[:namelen * 2]
                name = nm.decode("utf-16-le", "replace")
                if upcase and namehash(nm, upcase) != nh:
                    problems.append("name hash mismatch for %s/%s" % (path, name))
                nofat2 = bool(flagsb & 2)
                if first:
                    for cl in chain(first, nofat2, dlen):
                        if cl in used_clusters:
                            problems.append("cluster %d used twice (%s/%s)" % (cl, path, name))
                        used_clusters.add(cl)
                        if bitmap and not (bitmap[(cl - 2) // 8] >> ((cl - 2) % 8)) & 1:
                            problems.append("cluster %d of %s/%s not marked in the bitmap" % (cl, path, name))
                if attr & 0x10:
                    dirs += 1
                    nclus = len(chain(first, nofat2, dlen)) if first else 0
                    if dlen != nclus * cs:
                        problems.append("directory %s/%s: DataLength %d but %d clusters (%d bytes)" % (path, name, dlen, nclus, nclus * cs))
                    walk(first, nofat2, dlen, path + "/" + name)
                else:
                    files += 1
            i += 32 * (sec + 1)
        else:
            if t >= 0xC0:
                problems.append("orphan secondary entry (type %02X) at %s, offset %d" % (t, path, i))
            i += 32

# root directory: find bitmap, upcase table, label
root = read_dir(rootcl)
j = 0
while j < len(root) and root[j] != 0:
    t = root[j]
    if t == 0x83:
        n = root[j + 1]
        label = root[j + 2:j + 2 + 2 * n].decode("utf-16-le")
    elif t == 0x81:
        bflags = root[j + 1]
        first, = struct.unpack_from("<I", root, j + 20)
        ln, = struct.unpack_from("<Q", root, j + 24)
        bitmap = b"".join(rd(cluster_off(cl), cs) for cl in chain(first))[:ln]
        for cl in chain(first):
            used_clusters.add(cl)
        print("allocation bitmap at cluster", first, "length", ln)
    elif t == 0x82:
        cks, = struct.unpack_from("<I", root, j + 4)
        first, = struct.unpack_from("<I", root, j + 20)
        ln, = struct.unpack_from("<Q", root, j + 24)
        tbl = b"".join(rd(cluster_off(cl), cs) for cl in chain(first))[:ln]
        c2 = 0
        for b in tbl:
            c2 = (((c2 << 31) | (c2 >> 1)) + b) & 0xFFFFFFFF
        print("up-case table at cluster", first, "length", ln, "checksum %08X stored %08X" % (c2, cks), "OK" if c2 == cks else "MISMATCH")
        upcase = {}
        k = 0
        n = 0
        while k + 1 < len(tbl) and n < 0x10000:
            v = struct.unpack_from("<H", tbl, k)[0]
            if v == 0xFFFF and k + 3 < len(tbl):
                n += struct.unpack_from("<H", tbl, k + 2)[0]
                k += 4
                continue
            upcase[n] = v
            n += 1
            k += 2
        for cl in chain(first):
            used_clusters.add(cl)
    j += 32
print("volume label:", label)
walk(rootcl, False, None, "")
for cl in chain(rootcl):
    used_clusters.add(cl)
print("files", files, "directories", dirs)
# bitmap vs used
if bitmap:
    marked = sum(bin(b).count("1") for b in bitmap)
    print("bitmap marks %d clusters, the files and directories account for %d" % (marked, len(used_clusters)))
    if marked != len(used_clusters):
        problems.append("bitmap count %d != used %d" % (marked, len(used_clusters)))
print("PROBLEMS:", problems[:20] if problems else "none")
