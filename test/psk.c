/* psk.c - list processes (pid, parent, module) and optionally kill those whose module name contains a string.
   usage: psk            list
          psk <name>     list the matching ones and kill them with DosKillProcess  */
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSMODULEMGR
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv)
{
    static char buf[0x20000];
    QSPTRREC *p;
    QSPREC *pr;
    ULONG rc;
    const char *want = argc > 1 ? argv[1] : NULL;
    int found = 0;

    rc = DosQuerySysState(QS_PROCESS, QS_PROCESS, 0, 0, (PCHAR) buf, sizeof buf);
    if (rc) { printf("DosQuerySysState rc=%lu\n", rc); return 1; }
    p = (QSPTRREC *) buf;
    pr = p->pProcRec;
    while (pr && pr->RecType == 1) {
        char name[260] = "";
        char *q;
        QSTREC *t;
        DosQueryModuleName(pr->hMte, sizeof name, (PCHAR) name);
        if (!want || strstr(name, want) || (want[0] >= '0' && want[0] <= '9' && (unsigned) atoi(want) == pr->pid)) {
            printf("pid %5u ppid %5u  %s\n", pr->pid, pr->ppid, name);
            if (want) {
                found++;
                if (pr->pid != (USHORT) _getpid()) {
                    rc = DosKillProcess(DKP_PROCESS, pr->pid);
                    printf("   DosKillProcess rc=%lu\n", rc);
                }
            }
        }
        /* the next process record follows the thread records */
        t = pr->pThrdRec;
        { int n; for (n = 0; n < pr->cTCB; n++) t++; }
        pr = (QSPREC *) t;
    }
    if (want && !found) printf("no process with \"%s\" in its name\n", want);
    return 0;
}
