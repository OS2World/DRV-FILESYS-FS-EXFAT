/* growtest.c - grow a file with known data, verify it, truncate it back.
   usage: growtest <file> <bytes to append>   - the file keeps its original length when the program ends  */
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char pat(unsigned long i) { return (unsigned char) ((i * 2654435761UL) >> 24); }

int main(int argc, char **argv)
{
    HFILE h; ULONG act, n, rc, orig, pos, total, i, chunk;
    FILESTATUS3 fs;
    static unsigned char buf[8192];
    unsigned long want = argc > 2 ? strtoul(argv[2], NULL, 10) : 200000UL;
    unsigned long bad = 0, firstbad = 0;

    rc = DosOpen((PCSZ) argv[1], &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    if (rc) { printf("open rc=%lu\n", rc); return 1; }
    rc = DosQueryFileInfo(h, FIL_STANDARD, &fs, sizeof fs);
    orig = fs.cbFile;
    printf("original size %lu\n", orig);
    rc = DosSetFilePtr(h, 0, FILE_END, &pos);
    for (total = 0; total < want; total += chunk) {
        chunk = want - total < sizeof buf ? want - total : sizeof buf;
        for (i = 0; i < chunk; i++) buf[i] = pat(total + i);
        rc = DosWrite(h, buf, chunk, &n);
        if (rc || n != chunk) { printf("write at %lu rc=%lu n=%lu\n", total, rc, n); break; }
    }
    DosClose(h);

    rc = DosOpen((PCSZ) argv[1], &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    if (rc) { printf("reopen rc=%lu\n", rc); return 1; }
    DosQueryFileInfo(h, FIL_STANDARD, &fs, sizeof fs);
    printf("after append: size %lu (expected %lu)\n", (unsigned long) fs.cbFile, orig + want);
    DosSetFilePtr(h, (LONG) orig, FILE_BEGIN, &pos);
    for (total = 0; total < want; total += n) {
        rc = DosRead(h, buf, sizeof buf, &n);
        if (rc || !n) break;
        if (n > want - total) n = want - total;
        for (i = 0; i < n; i++) if (buf[i] != pat(total + i)) { if (!bad) firstbad = total + i; bad++; }
    }
    printf("verify: read %lu of %lu bytes, %lu wrong%s\n", total, want, bad, bad ? " (first at offset " : "");
    if (bad) printf("%lu)\n", firstbad);

    rc = DosSetFileSize(h, orig);
    printf("truncate back to %lu: rc=%lu\n", orig, rc);
    DosClose(h);
    rc = DosOpen((PCSZ) argv[1], &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL);
    DosQueryFileInfo(h, FIL_STANDARD, &fs, sizeof fs);
    printf("final size %lu\n", (unsigned long) fs.cbFile);
    DosClose(h);
    return bad != 0;
}
