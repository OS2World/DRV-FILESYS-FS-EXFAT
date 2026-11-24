/* savetest.c - tries the ways an editor can save a text file and reports what the file system did.
   usage: savetest <dir>   (the directory must exist and be on the volume to test, e.g. E:\)  */
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *OLD = "old text\r\n";
static char path[260], tmp[260];

static ULONG put(const char *name, const char *text)       /* create/replace a file with text, close */
{
    HFILE h; ULONG act, n, rc;
    rc = DosOpen((PCSZ) name, &h, &act, 0, FILE_NORMAL, OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                 OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE, NULL);
    if (rc) return rc;
    rc = DosWrite(h, (PVOID) text, strlen(text), &n);
    DosClose(h);
    return rc;
}

static void show(const char *what, ULONG rc)
{
    HFILE h; ULONG act, n = 0; char buf[200] = ""; ULONG rc2;
    FILESTATUS3 fs; memset(&fs, 0, sizeof fs);
    rc2 = DosOpen((PCSZ) path, &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL);
    if (!rc2) { DosRead(h, buf, sizeof buf - 1, &n); DosQueryFileInfo(h, FIL_STANDARD, &fs, sizeof fs); DosClose(h); }
    buf[n] = 0;
    { char *p; while ((p = strpbrk(buf, "\r\n"))) *p = '|'; }
    printf("%-34s rc=%-4lu reopen rc=%-3lu size=%lu content=\"%s\"\n", what, rc, rc2, (unsigned long) fs.cbFile, buf);
}

int main(int argc, char **argv)
{
    HFILE h; ULONG act, n, rc, pos;
    const char *dir = argc > 1 ? argv[1] : "E:\\";
    const char *NEW = "new longer text that replaces the old one\r\nsecond line\r\n";

    sprintf(path, "%sSAVETEST.TXT", dir);
    sprintf(tmp, "%sSAVETEST.TMP", dir);

    /* 1: replace an existing file (create/replace) */
    put(path, OLD);
    rc = put(path, NEW);
    show("1 replace (REPLACE_IF_EXISTS)", rc);

    /* 2: open existing read/write, write over it, no truncate (shorter text) */
    put(path, NEW);
    rc = DosOpen((PCSZ) path, &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    if (!rc) { rc = DosWrite(h, (PVOID) OLD, strlen(OLD), &n); DosClose(h); }
    show("2 overwrite shorter, no truncate", rc);

    /* 3: open existing, truncate to the new length, write */
    put(path, NEW);
    rc = DosOpen((PCSZ) path, &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    if (!rc) { rc = DosWrite(h, (PVOID) OLD, strlen(OLD), &n); if (!rc) rc = DosSetFileSize(h, strlen(OLD)); DosClose(h); }
    show("3 write then SetFileSize(shorter)", rc);

    /* 4: set the size first (grow), then write */
    put(path, OLD);
    rc = DosOpen((PCSZ) path, &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    if (!rc) { rc = DosSetFileSize(h, strlen(NEW)); if (!rc) rc = DosWrite(h, (PVOID) NEW, strlen(NEW), &n); DosClose(h); }
    show("4 SetFileSize(longer) then write", rc);

    /* 5: write a temp file, delete the original, rename the temp over it */
    put(path, OLD);
    rc = put(tmp, NEW);
    if (!rc) rc = DosDelete((PCSZ) path);
    if (!rc) rc = DosMove((PCSZ) tmp, (PCSZ) path);
    show("5 temp file, delete, rename", rc);

    /* 6: write a temp file and DosMove it over the existing original (replace by rename) */
    put(path, OLD);
    rc = put(tmp, NEW);
    if (!rc) rc = DosMove((PCSZ) tmp, (PCSZ) path);
    show("6 temp file, DosMove over original", rc);
    DosDelete((PCSZ) tmp);

    /* 7: write, then set the file time stamps with DosSetFileInfo */
    put(path, OLD);
    rc = DosOpen((PCSZ) path, &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, NULL);
    if (!rc) {
        FILESTATUS3 fs;
        rc = DosWrite(h, (PVOID) NEW, strlen(NEW), &n);
        if (!rc) rc = DosSetFileSize(h, strlen(NEW));
        if (!rc) rc = DosQueryFileInfo(h, FIL_STANDARD, &fs, sizeof fs);
        if (!rc) rc = DosSetFileInfo(h, FIL_STANDARD, &fs, sizeof fs);
        DosClose(h);
    }
    show("7 write + DosSetFileInfo", rc);

    /* 8: write with an extended attribute (.TYPE) through DosSetPathInfo */
    put(path, OLD);
    {
        static char ea[64];
        FEA2LIST *list = (FEA2LIST *) ea;
        EAOP2 eaop; FEA2 *f = list->list;
        const char *name = ".TYPE";
        static const char val[] = { 0xFD, 0xFF, 0x04, 0x00, 'T', 'e', 'x', 't' };   /* EAT_ASCII, 4 bytes */
        memset(ea, 0, sizeof ea);
        f->oNextEntryOffset = 0; f->fEA = 0; f->cbName = (BYTE) strlen(name); f->cbValue = sizeof val;
        strcpy(f->szName, name); memcpy(f->szName + f->cbName + 1, val, sizeof val);
        list->cbList = sizeof(ULONG) + sizeof(FEA2) - 1 + f->cbName + 1 + f->cbValue;
        eaop.fpGEA2List = NULL; eaop.fpFEA2List = list; eaop.oError = 0;
        rc = DosSetPathInfo((PCSZ) path, FIL_QUERYEASIZE, &eaop, sizeof eaop, 0);
    }
    show("8 set EA .TYPE (DosSetPathInfo)", rc);

    /* 9: open with write-through and fail-on-error flags, replace, write, commit */
    rc = DosOpen((PCSZ) path, &h, &act, 0, 0, OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE | OPEN_FLAGS_WRITE_THROUGH | OPEN_FLAGS_FAIL_ON_ERROR, NULL);
    if (!rc) { rc = DosWrite(h, (PVOID) NEW, strlen(NEW), &n); if (!rc) rc = DosResetBuffer(h); DosClose(h); }
    show("9 write-through replace + ResetBuffer", rc);

    DosDelete((PCSZ) path); DosDelete((PCSZ) tmp);
    return 0;
}
