/* captest.c - the 4 GB - 1 cap: size a file just below it, then write across it and see what the driver does.
   usage: captest <file>   */
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_LONGLONG
#include <os2.h>
#include <stdio.h>
#include <string.h>

typedef unsigned long long u64;

int main(int argc, char **argv)
{
    static unsigned char b[65536];
    HFILE h; ULONG act, rc, n;
    LONGLONG got, pos;
    u64 start = 4294967295ULL - 200000ULL, total = 0, size;
    FILESTATUS3L fs;
    int rounds = 0;

    setvbuf(stdout, NULL, _IONBF, 0);
    memset(b, 0x5A, sizeof b);
    rc = DosOpenL((PCSZ) argv[1], &h, &act, 0, FILE_NORMAL, OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                  OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    printf("open rc=%lu\n", rc);
    if (rc) return 1;
    rc = DosSetFileSizeL(h, (LONGLONG) start);
    printf("DosSetFileSizeL(%llu) rc=%lu (just below the cap)\n", start, rc);
    pos = (LONGLONG) start;
    rc = DosSetFilePtrL(h, pos, FILE_BEGIN, &got);
    printf("seek to %llu rc=%lu pos=%llu\n", start, rc, (u64) got);
    /* write 64 KB blocks past the end: the cap is 4294967295 */
    for (rounds = 0; rounds < 10; rounds++) {
        rc = DosWrite(h, b, sizeof b, &n);
        total += n;
        printf("write %d: rc=%lu wrote %lu bytes (total %llu)\n", rounds, rc, n, total);
        if (rc || n < sizeof b) break;
    }
    rc = DosQueryFileInfo(h, FIL_STANDARDL, &fs, sizeof fs);
    size = (u64) fs.cbFile;
    printf("final size %llu (cap 4294967295): %s\n", size, size <= 4294967295ULL ? "OK, not beyond the cap" : "BEYOND THE CAP  <<< BAD");
    rc = DosSetFileSizeL(h, 1048576LL);
    DosClose(h);
    rc = DosDelete((PCSZ) argv[1]);
    printf("deleted: rc=%lu\n", rc);
    return size > 4294967295ULL;
}
