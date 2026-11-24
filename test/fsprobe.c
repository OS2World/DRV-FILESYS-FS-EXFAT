/* fsprobe.c - the queries a removable-media monitor can make on a drive, and what each returns.
   usage: fsprobe E:   */
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#include <os2.h>
#include <stdio.h>
#include <string.h>

static void r(const char *what, ULONG rc) { printf("%-52s rc=%lu\n", what, rc); }

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    const char *drv = argc > 1 ? argv[1] : "E:";
    char buf[512];
    ULONG rc, len, act, n, i;
    HFILE h;
    char root[8];

    /* the file system name attached to the drive (kernel) */
    len = sizeof buf;
    rc = DosQueryFSAttach((PCSZ) drv, 0, FSAIL_QUERYNAME, (PFSQBUFFER2) buf, &len);
    r("DosQueryFSAttach(drive, QUERYNAME)", rc);
    if (!rc) { FSQBUFFER2 *q = (FSQBUFFER2 *) buf; printf("   type=%u  drive name=\"%s\"  fsd name=\"%s\"  fsd data len=%u\n", q->iType, q->szName, q->szName + q->cbName + 1, q->cbFSAData); }

    /* drive number and label */
    {
        ULONG d = (ULONG) (drv[0] & 0xDF) - 'A' + 1;
        FSINFO fi; FSALLOCATE fa;
        rc = DosQueryFSInfo(d, FSIL_VOLSER, &fi, sizeof fi);
        r("DosQueryFSInfo level 2 (volume label)", rc);
        if (!rc) printf("   label=\"%.*s\"\n", fi.vol.cch, fi.vol.szVolLabel);
        rc = DosQueryFSInfo(d, FSIL_ALLOC, &fa, sizeof fa);
        r("DosQueryFSInfo level 1 (allocation)", rc);
        if (!rc) printf("   unit=%lu avail=%lu sectors/unit=%lu bytes/sector=%u\n", fa.cUnit, fa.cUnitAvail, fa.cSectorUnit, fa.cbSector);
        rc = DosQueryFSInfo(d, 3, &buf, sizeof buf);
        r("DosQueryFSInfo level 3", rc);
    }

    /* open the drive for DASD access and ask the disk driver */
    rc = DosOpen((PCSZ) drv, &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL);
    r("DosOpen(drive, DASD, read only)", rc);
    if (!rc) {
        UCHAR cmd = 0, data[256]; ULONG pl, dl;
        pl = sizeof cmd; dl = sizeof data; memset(data, 0, sizeof data);
        rc = DosDevIOCtl(h, IOCTL_DISK, DSK_GETLOCKSTATUS, &cmd, 1, &pl, data, sizeof data, &dl);
        r("DSK_GETLOCKSTATUS", rc);
        if (!rc) printf("   data: %02x %02x %02x %02x (len %lu)\n", data[0], data[1], data[2], data[3], dl);
        pl = sizeof cmd; dl = sizeof data; memset(data, 0, sizeof data);
        rc = DosDevIOCtl(h, IOCTL_DISK, DSK_GETDEVICEPARAMS, &cmd, 1, &pl, data, sizeof data, &dl);
        r("DSK_GETDEVICEPARAMS", rc);
        if (!rc) printf("   bytes/sector=%u (len %lu)\n", *(USHORT *) data, dl);
        pl = sizeof cmd; dl = sizeof data; memset(data, 0, sizeof data);
        rc = DosDevIOCtl(h, IOCTL_DISK, DSK_QUERYMEDIASENSE, &cmd, 1, &pl, data, sizeof data, &dl);
        r("DSK_QUERYMEDIASENSE", rc);
        if (!rc) printf("   data: %02x %02x (len %lu)\n", data[0], data[1], dl);
        pl = sizeof cmd; dl = sizeof data; memset(data, 0, sizeof data);
        rc = DosDevIOCtl(h, IOCTL_DISK, DSK_GETLOGICALMAP, &cmd, 1, &pl, data, sizeof data, &dl);
        r("DSK_GETLOGICALMAP", rc);
        DosClose(h);
    }

    /* no DosFSCtl calls here: private function codes without a parameter buffer crash FAT32.IFS (and crashed EXFAT.IFS before 0.1a2) */

    /* path info of the root, and a directory read */
    {
        FILESTATUS3 fs;
        sprintf(root, "%s\\", drv);
        rc = DosQueryPathInfo((PCSZ) root, FIL_STANDARD, &fs, sizeof fs);
        r("DosQueryPathInfo(root, FIL_STANDARD)", rc);
        rc = DosQueryPathInfo((PCSZ) drv, FIL_STANDARD, &fs, sizeof fs);
        r("DosQueryPathInfo(\"E:\", FIL_STANDARD)", rc);
    }
    return 0;
}
