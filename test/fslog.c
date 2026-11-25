/* fslog.c - switch the trace log of EXFAT.IFS on or off and read it.
   usage: fslog on <mask>     (mask: 1 = FS calls, 2 = functions, 4 = cache, 0xFF = all)
          fslog off
          fslog read          (prints and clears what the IFS has logged; waits 1 s for more)
   Only talks to the file system named EXFAT, with real parameter buffers (see FS_FSCTL notes in CHANGES.md). */
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAT32_GETLOGDATA 0x8000
#define FAT32_SETMESSAGE 0x8001

int main(int argc, char **argv)
{
    static char data[4096];
    ULONG rc, dl, pl;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc >= 2 && !strcmp(argv[1], "on")) {
        USHORT mask = (USHORT) (argc > 2 ? strtoul(argv[2], NULL, 0) : 0xFF);
        pl = sizeof mask; dl = 0;
        rc = DosFSCtl(NULL, 0, &dl, &mask, sizeof mask, &pl, FAT32_SETMESSAGE, (PCSZ) "EXFAT", (HFILE) -1, FSCTL_FSDNAME);
        printf("logging on, mask 0x%x: rc=%lu\n", mask, rc);
        return (int) rc;
    }
    if (argc >= 2 && !strcmp(argv[1], "off")) {
        USHORT mask = 0;
        pl = sizeof mask; dl = 0;
        rc = DosFSCtl(NULL, 0, &dl, &mask, sizeof mask, &pl, FAT32_SETMESSAGE, (PCSZ) "EXFAT", (HFILE) -1, FSCTL_FSDNAME);
        printf("logging off: rc=%lu\n", rc);
        return (int) rc;
    }
    if (argc >= 2 && !strcmp(argv[1], "read")) {
        int rounds;
        for (rounds = 0; rounds < 200; rounds++) {
            ULONG timeout = 1000;          /* milliseconds the IFS waits for log data */
            pl = sizeof timeout; dl = sizeof data - 1;
            memset(data, 0, sizeof data);
            rc = DosFSCtl(data, sizeof data - 1, &dl, &timeout, sizeof timeout, &pl, FAT32_GETLOGDATA, (PCSZ) "EXFAT", (HFILE) -1, FSCTL_FSDNAME);
            if (data[0]) fputs(data, stdout);
            if (rc != ERROR_BUFFER_OVERFLOW) break;   /* one pass: more only when the buffer did not hold it all */
            if (rc && rc != ERROR_BUFFER_OVERFLOW) { printf("\n[GETLOGDATA rc=%lu]\n", rc); break; }
        }
        return 0;
    }
    if (argc >= 3 && !strcmp(argv[1], "remount")) {
        HFILE h; ULONG act;
        UCHAR cmd = 0, dat = 0; ULONG p1 = 1, d1 = 1;
        rc = DosOpen((PCSZ) argv[2], &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                     OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL);
        printf("DosOpen(%s, DASD) rc=%lu\n", argv[2], rc);
        if (rc) return 1;
        /* lock the drive, ask for the media to be determined again (the kernel then mounts it), unlock */
        rc = DosDevIOCtl(h, IOCTL_DISK, DSK_LOCKDRIVE, &cmd, 1, &p1, &dat, 1, &d1);
        printf("DSK_LOCKDRIVE rc=%lu\n", rc);
        if (!rc) {
            p1 = 1; d1 = 1;
            rc = DosDevIOCtl(h, IOCTL_DISK, DSK_REDETERMINEMEDIA, &cmd, 1, &p1, &dat, 1, &d1);
            printf("DSK_REDETERMINEMEDIA rc=%lu\n", rc);
            p1 = 1; d1 = 1;
            DosDevIOCtl(h, IOCTL_DISK, DSK_UNLOCKDRIVE, &cmd, 1, &p1, &dat, 1, &d1);
        }
        DosClose(h);
        return (int) rc;
    }
    if (argc >= 3 && !strcmp(argv[1], "params")) {
        HFILE h; ULONG act;
        UCHAR cmd = 0; static UCHAR dat[256]; ULONG p1 = 1, d1 = sizeof dat;
        rc = DosOpen((PCSZ) argv[2], &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                     OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL);
        if (rc) { printf("DosOpen rc=%lu\n", rc); return 1; }
        rc = DosDevIOCtl(h, IOCTL_DISK, DSK_GETDEVICEPARAMS, &cmd, 1, &p1, dat, sizeof dat, &d1);
        printf("DSK_GETDEVICEPARAMS rc=%lu len=%lu\n", rc, d1);
        if (!rc) {
            int i;
            printf("  BPB: bytes/sector %u, sectors/cluster %u, reserved %u, FATs %u, root entries %u, total sectors (16 bit) %u, media %02x,\n"
                   "       sectors/FAT %u, sectors/track %u, heads %u, hidden %lu, large total sectors %lu\n",
                   *(USHORT *) dat, dat[2], *(USHORT *) (dat + 3), dat[5], *(USHORT *) (dat + 6), *(USHORT *) (dat + 8), dat[10],
                   *(USHORT *) (dat + 11), *(USHORT *) (dat + 13), *(USHORT *) (dat + 15), *(ULONG *) (dat + 17), *(ULONG *) (dat + 21));
            printf("  cylinders %u, device type %u, attributes %04x\n", *(USHORT *) (dat + 31), dat[33], *(USHORT *) (dat + 34));
            printf("  raw:"); for (i = 0; i < 40; i++) printf("%s%02x", i % 16 ? " " : "\n   ", dat[i]); printf("\n");
        }
        DosClose(h);
        return 0;
    }
    printf("usage: fslog on <mask> | off | read | remount <drive> | params <drive>\n");
    return 1;
}
