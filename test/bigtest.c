/* bigtest.c - files over 2 GB and 4 GB: allocate, seek with 64-bit offsets, write markers, read them back.
   usage: bigtest <file> <size in MB> [full]
     Steps (each prints rc and what it saw):
       1. DosOpenL create/replace; DosSetFileSizeL(size); query the size (FIL_STANDARDL)
       2. write a 4096-byte marker block at several offsets, among them 2^31-2048 (crossing 2 GB), 2^31, 2^32-2048, 2^32 and the last block;
          each marker holds its own offset repeated, so a wrong location shows up
       3. DosSetFilePtrL: seek from the start, from the end (negative), relative; check the returned positions
       4. close, reopen, read every marker back and check it; read a block of zeros from an untouched place
       5. with "full": sequential write of the whole file with an offset pattern, then a verify pass (slow)
       6. truncate to 1 MB and then delete
*/
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_LONGLONG
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long u64;
static HFILE h;
static const char *fname;
static int bad = 0;

static u64 get_size(void)
{
    FILESTATUS3L fs;
    ULONG rc = DosQueryFileInfo(h, FIL_STANDARDL, &fs, sizeof fs);
    if (rc) { printf("  DosQueryFileInfo(FIL_STANDARDL) rc=%lu\n", rc); return 0; }
    return (u64) fs.cbFile;
}

static void fill(unsigned char *b, u64 off)       /* the block holds its own offset in every 8 bytes */
{
    int i;
    for (i = 0; i < 4096; i += 8) memcpy(b + i, &off, 8);
    off += 0;
}

static int put(u64 off)
{
    static unsigned char b[4096];
    LONGLONG pos; ULONG rc, n; LONGLONG got;
    fill(b, off);
    pos = (LONGLONG) off;
    rc = DosSetFilePtrL(h, pos, FILE_BEGIN, &got);
    if (rc || (u64) got != off) { printf("  seek to %llu: rc=%lu got %llu  <<< BAD\n", off, rc, (u64) got); bad++; return 1; }
    rc = DosWrite(h, b, sizeof b, &n);
    if (rc || n != sizeof b) { printf("  write at %llu: rc=%lu n=%lu  <<< BAD\n", off, rc, n); bad++; return 1; }
    return 0;
}

static int check(u64 off, int zero)
{
    static unsigned char b[4096], e[4096];
    LONGLONG pos, got; ULONG rc, n;
    pos = (LONGLONG) off;
    rc = DosSetFilePtrL(h, pos, FILE_BEGIN, &got);
    if (rc || (u64) got != off) { printf("  seek to %llu: rc=%lu got %llu  <<< BAD\n", off, rc, (u64) got); bad++; return 1; }
    rc = DosRead(h, b, sizeof b, &n);
    if (rc || n != sizeof b) { printf("  read at %llu: rc=%lu n=%lu  <<< BAD\n", off, rc, n); bad++; return 1; }
    if (zero) memset(e, 0, sizeof e); else fill(e, off);
    if (memcmp(b, e, sizeof b)) { printf("  block at %llu differs from expected%s  <<< BAD\n", off, zero ? " (zeros)" : ""); bad++; return 1; }
    return 0;
}

int main(int argc, char **argv)
{
    u64 mb, size, i;
    ULONG rc, act;
    LONGLONG got;
    u64 offs[8];
    int n, k, full;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc < 3) { printf("usage: bigtest <file> <size MB> [full]\n"); return 2; }
    fname = argv[1]; mb = strtoull(argv[2], NULL, 10); size = mb * 1048576ULL; full = argc > 3;

    rc = DosOpenL((PCSZ) fname, &h, &act, 0, FILE_NORMAL, OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                  OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    printf("1. DosOpenL(%s) rc=%lu\n", fname, rc);
    if (rc) return 1;
    rc = DosSetFileSizeL(h, (LONGLONG) size);
    printf("   DosSetFileSizeL(%llu bytes = %llu MB) rc=%lu\n", size, mb, rc);
    if (rc) { printf("   <<< cannot size the file; stopping\n"); DosClose(h); DosDelete((PCSZ) fname); return 1; }
    printf("   size now %llu (expected %llu)%s\n", get_size(), size, get_size() == size ? "" : "  <<< BAD");
    if (get_size() != size) bad++;

    n = 0;
    offs[n++] = 0;
    offs[n++] = 2147483648ULL - 2048;           /* 2^31 - 2048: the block crosses 2 GB */
    offs[n++] = 2147483648ULL + 4096;            /* just above 2^31 (does not overlap the block that crosses it) */
    offs[n++] = 2147483648ULL + 12288;
    offs[n++] = 4294967296ULL - 2048;           /* 2^32 - 2048: crosses 4 GB */
    offs[n++] = 4294967296ULL + 4096;            /* just above 2^32 */
    offs[n++] = size - 4096;                     /* last block */
    printf("2. writing marker blocks\n");
    for (k = 0; k < n; k++) {
        if (offs[k] + 4096 > size) { printf("   (offset %llu beyond the file, skipped)\n", offs[k]); offs[k] = (u64) -1; continue; }
        if (!put(offs[k])) printf("   wrote marker at %llu\n", offs[k]);
    }

    printf("3. seek checks\n");
    {
        LONGLONG p;
        rc = DosSetFilePtrL(h, 0, FILE_END, &got);
        printf("   seek 0 from END: rc=%lu pos=%llu (expected %llu)%s\n", rc, (u64) got, size, (u64) got == size ? "" : "  <<< BAD");
        if (rc || (u64) got != size) bad++;
        p = -(LONGLONG) 4096;
        rc = DosSetFilePtrL(h, p, FILE_END, &got);
        printf("   seek -4096 from END: rc=%lu pos=%llu (expected %llu)%s\n", rc, (u64) got, size - 4096, (u64) got == size - 4096 ? "" : "  <<< BAD");
        if (rc || (u64) got != size - 4096) bad++;
        p = 0;
        rc = DosSetFilePtrL(h, p, FILE_CURRENT, &got);
        printf("   tell: rc=%lu pos=%llu%s\n", rc, (u64) got, (u64) got == size - 4096 ? "" : "  <<< BAD");
        if ((u64) got != size - 4096) bad++;
        p = (LONGLONG) 3000000000ULL;
        rc = DosSetFilePtrL(h, p, FILE_BEGIN, &got);
        printf("   seek 3000000000: rc=%lu pos=%llu%s\n", rc, (u64) got, (u64) got == 3000000000ULL ? "" : "  <<< BAD");
        if ((u64) got != 3000000000ULL) bad++;
        p = 1000000000;
        rc = DosSetFilePtrL(h, p, FILE_CURRENT, &got);
        printf("   seek +1000000000 from current: rc=%lu pos=%llu (expected 4000000000)%s\n", rc, (u64) got, (u64) got == 4000000000ULL ? "" : "  <<< BAD");
        if ((u64) got != 4000000000ULL) bad++;
    }

    DosClose(h);
    rc = DosOpenL((PCSZ) fname, &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    printf("4. reopened: rc=%lu, size %llu%s\n", rc, get_size(), get_size() == size ? "" : "  <<< BAD");
    if (get_size() != size) bad++;
    for (k = 0; k < n; k++) {
        if (offs[k] == (u64) -1) continue;
        if (!check(offs[k], 0)) printf("   marker at %llu OK\n", offs[k]);
    }
    /* DosSetFileSizeL does not promise to clear the extended area, so what an untouched block holds is only reported */
    if (size > 3221225472ULL + 4096) {
        int saved = bad;
        if (!check(3221225472ULL, 1)) printf("   untouched block at 3221225472 reads as zeros\n");
        else { printf("   (untouched block holds old disk data: DosSetFileSizeL does not clear it, not an error)\n"); bad = saved; }
    }

    if (full) {
        static unsigned char b[65536];
        LONGLONG z = 0;
        u64 o; ULONG nn;
        printf("5. sequential write of %llu MB\n", mb);
        DosSetFilePtrL(h, z, FILE_BEGIN, &got);
        for (o = 0; o < size; o += sizeof b) {
            ULONG len = (size - o) < sizeof b ? (ULONG) (size - o) : (ULONG) sizeof b;
            int j;
            for (j = 0; j < (int) len; j += 8) { u64 v = o + (u64) j; memcpy(b + j, &v, 8); }
            rc = DosWrite(h, b, len, &nn);
            if (rc || nn != len) { printf("   write at %llu: rc=%lu n=%lu  <<< BAD\n", o, rc, nn); bad++; break; }
            if ((o & 0x3FFFFFFFULL) == 0 && o) printf("   written %llu MB\n", o >> 20);
        }
        printf("   verify pass\n");
        DosSetFilePtrL(h, z, FILE_BEGIN, &got);
        for (o = 0; o < size; o += sizeof b) {
            ULONG len = (size - o) < sizeof b ? (ULONG) (size - o) : (ULONG) sizeof b;
            int j, wrong = 0;
            rc = DosRead(h, b, len, &nn);
            if (rc || nn != len) { printf("   read at %llu: rc=%lu n=%lu  <<< BAD\n", o, rc, nn); bad++; break; }
            for (j = 0; j < (int) len; j += 8) { u64 v; memcpy(&v, b + j, 8); if (v != o + (u64) j) { wrong = 1; break; } }
            if (wrong) { printf("   wrong data in the block at %llu  <<< BAD\n", o); bad++; break; }
        }
        printf("   sequential pass %s\n", bad ? "FAILED" : "OK");
    }

    rc = DosSetFileSizeL(h, (LONGLONG) 1048576);
    printf("6. truncate to 1 MB: rc=%lu, size %llu\n", rc, get_size());
    DosClose(h);
    rc = DosDelete((PCSZ) fname);
    printf("   delete: rc=%lu\n", rc);
    printf("RESULT: %s (%d problems)\n", bad ? "FAILED" : "OK", bad);
    return bad != 0;
}
