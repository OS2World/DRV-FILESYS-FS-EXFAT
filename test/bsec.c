/* bsec.c - read the boot sector of a drive (read only DASD open) and print the exFAT / FAT fields.
   usage: bsec E:   */
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    static unsigned char s[4096];
    HFILE h; ULONG act, n, rc, i;
    const char *drv = argc > 1 ? argv[1] : "E:";

    setvbuf(stdout, NULL, _IONBF, 0);
    rc = DosOpen((PCSZ) drv, &h, &act, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL);
    printf("DosOpen(%s, DASD) rc=%lu\n", drv, rc);
    if (rc) return 1;
    rc = DosRead(h, s, 512, &n);
    printf("DosRead 512 rc=%lu n=%lu\n", rc, n);
    if (rc) return 1;
    printf("jump: %02x %02x %02x   oem: \"%.8s\"\n", s[0], s[1], s[2], s + 3);
    printf("signature @510: %02x %02x\n", s[510], s[511]);
    if (!memcmp(s + 3, "EXFAT   ", 8)) {
        unsigned long long vlen = *(unsigned long long *) (s + 72);
        unsigned long fat_off = *(unsigned long *) (s + 80), fat_len = *(unsigned long *) (s + 84);
        unsigned long heap_off = *(unsigned long *) (s + 88), ccount = *(unsigned long *) (s + 92);
        unsigned long root = *(unsigned long *) (s + 96);
        printf("exFAT: volume length %llu sectors (%llu MB), partition offset %llu\n", vlen, vlen * (1ULL << s[108]) / 1048576ULL,
               *(unsigned long long *) (s + 64));
        printf("       FAT offset %lu length %lu, heap offset %lu, clusters %lu, root cluster %lu\n", fat_off, fat_len, heap_off, ccount, root);
        printf("       bytes per sector shift %u (=%lu), sectors per cluster shift %u (cluster %lu bytes), FATs %u, version %u.%u, flags %04x\n",
               s[108], 1UL << s[108], s[109], (1UL << s[108]) << s[109], s[110], s[105], s[104], *(unsigned short *) (s + 106));
    } else {
        printf("FAT-style fields: bytes/sector %u, sectors/cluster %u, reserved %u, FATs %u, total16 %u, total32 %lu, FAT32 label \"%.8s\"\n",
               *(unsigned short *) (s + 11), s[13], *(unsigned short *) (s + 14), s[16], *(unsigned short *) (s + 19),
               *(unsigned long *) (s + 32), s + 82);
    }
    if (s[0] == 0x33 || s[0] == 0xFA || s[0] == 0xEB && s[2] != 0x90) {   /* looks like an MBR: print the partition table */
        int pi;
        printf("MBR partition table:\n");
        for (pi = 0; pi < 4; pi++) {
            unsigned char *e = s + 446 + 16 * pi;
            printf("  [%d] boot %02x type %02x  start LBA %lu  sectors %lu (%lu MB)\n", pi, e[0], e[4],
                   *(unsigned long *) (e + 8), *(unsigned long *) (e + 12), *(unsigned long *) (e + 12) / 2048UL);
        }
    }
    printf("first 64 bytes:");
    for (i = 0; i < 64; i++) printf("%s%02x", i % 16 ? " " : "\n  ", s[i]);
    printf("\n");

    if (argc > 2 && !memcmp(s + 3, "EXFAT   ", 8)) {
        unsigned long heap_off = *(unsigned long *) (s + 88), root = *(unsigned long *) (s + 96);
        unsigned long csz = (1UL << s[108]) << s[109];
        static unsigned char d[131072 * 2];
        unsigned long pos, off, e;
        unsigned long bmp_cluster = 0, bmp_len = 0;
        /* root directory cluster */
        off = (heap_off + (unsigned long long) (root - 2) * (1UL << s[109])) * (1UL << s[108]);
        DosSetFilePtr(h, (LONG) off, FILE_BEGIN, &pos);
        rc = DosRead(h, d, csz > sizeof d ? sizeof d : csz, &n);
        printf("root directory cluster %lu at byte %lu: read rc=%lu n=%lu\n", root, off, rc, n);
        for (e = 0; e + 32 <= n && e < 32 * 40; e += 32) {
            unsigned char t = d[e];
            if (t == 0) { printf("  [%lu] end of directory\n", e / 32); break; }
            printf("  [%lu] type %02x", e / 32, t);
            if (t == 0x81) { bmp_cluster = *(unsigned long *) (d + e + 20); bmp_len = (unsigned long) *(unsigned long long *) (d + e + 24);
                             printf("  ALLOCATION BITMAP: flags %02x first cluster %lu length %lu bytes", d[e + 1], bmp_cluster, bmp_len); }
            else if (t == 0x82) printf("  UP-CASE TABLE: first cluster %lu length %lu", *(unsigned long *) (d + e + 20), (unsigned long) *(unsigned long long *) (d + e + 24));
            else if (t == 0x83) { printf("  VOLUME LABEL: chars %u", d[e + 1]); }
            else if (t == 0x85) printf("  FILE: secondary count %u attr %04x", d[e + 1], *(unsigned short *) (d + e + 4));
            else if (t == 0xC0) printf("  STREAM: flags %02x name len %u first cluster %lu data len %lu", d[e + 1], d[e + 3], *(unsigned long *) (d + e + 20), (unsigned long) *(unsigned long long *) (d + e + 24));
            else if (t == 0xC1) printf("  NAME");
            printf("\n");
        }
        if (bmp_cluster) {
            unsigned long i2, used = 0, total = bmp_len, shown = 0;
            static unsigned char b[131072];
            off = (heap_off + (unsigned long long) (bmp_cluster - 2) * (1UL << s[109])) * (1UL << s[108]);
            DosSetFilePtr(h, (LONG) off, FILE_BEGIN, &pos);
            rc = DosRead(h, b, bmp_len > sizeof b ? sizeof b : bmp_len, &n);
            printf("bitmap at byte %lu: read rc=%lu n=%lu\n", off, rc, n);
            for (i2 = 0; i2 < n; i2++) { int k; for (k = 0; k < 8; k++) if (b[i2] & (1 << k)) used++; }
            printf("bitmap: %lu bits set in the first %lu bytes; first bytes:", used, n);
            for (i2 = 0; i2 < 32 && i2 < n; i2++) printf(" %02x", b[i2]);
            printf("\n");
        }
    }
    DosClose(h);
    return 0;
}
