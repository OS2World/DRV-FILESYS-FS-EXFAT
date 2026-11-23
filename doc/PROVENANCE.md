# Provenance of the clean-room headers (2026-11-23)

Replaces the IBM DDK-derived headers `fsd.h`, `fsh.h`, `devhelp.h`, `strat2.h`, `devhdr.h`, `devcmd.h`
and `sas.h` of the upstream netlabs tree (see `HEADER-AUDIT.md`). Licence of the new files: LGPL 2.1 or later,
copyright OS2World.

## New and changed files

| File | Origin |
|---|---|
| `src\include\fsd.h` | Structures and value groups first written for the HFS/2 port (`DRV-FILESYS-HFS\src\Ring0\fsd.h`, OS2World, clean-room 2026, verified on ArcaOS by mounting, reading and writing a volume), completed from the IFS reference (FS_FILEIO command list, `ERROR_VOLUME_NOT_MOUNTED` = 0xEE00 is already defined by the netlabs `fat32def.h`). No FS_xxx prototypes: the driver sources define them. |
| `src\include\fsh.h` | The HFS/2 `fsh.h` (same origin); all FSH_xxx used by the IFS are in it. |
| `src\include\fsproto.h` | Generated from the definitions in `ifs\*.c` (netlabs code), so the entry points can call each other. |
| `src\include\devhelp.h`, `src\ifs\dhwrap.asm` | Written from the *Physical Device Driver Reference* (`os2ddk2004\INF\pddref`): the C calling sequence, the register interface, the function numbers (table of DevHlp functions) and the flag descriptions (VMAlloc, VMLock, GetDOSVar variable numbers) of each service. `AllocGDTSelector`, `LinToGDTSelector`, `VMAlloc`, `VMLock`, `VMUnLock` are the routines verified in the HFS/2 port. |
| `src\include\strat2.h` | Written from the *Storage Device Driver Reference* (`os2ddk2004\INF\storage`): Driver Capabilities Structure, Volume Characteristics Structure, Request Lists, request header, read/write request, scatter/gather descriptor and the command codes, priorities and control bits. Field names are those the driver sources use. |
| `src\ifs\ifsmount.c` | Source change: `devhdr.h`, `devcmd.h` and `sas.h` are no longer needed. The walk over the system's device driver chain (through the system anchor segment) that fetched the disk driver's capabilities is replaced by the pointers the kernel passes in the volume parameters (`vpi_pDCS`, `vpi_pVCS`, documented for FS_MOUNT). The debug messages for the capability bits that are not documented (Read2, DMA, >16 MB) were removed. |

## Sources read

* IBM *OS/2 Installable File Systems* reference (IPF, copy in `References\netlabs-fat32-ifsinf`, outside the repository).
* *Physical Device Driver Reference* and *Storage Device Driver Reference* (OS/2 DDK documentation, text form).
* The HFS/2 clean-room headers and `devhelp_ifs.asm` (OS2World).
* The FAT32 driver sources in this repository, for names, argument lists and usage.

## What was seen of the old headers

The old DDK-derived headers were not used as a source for the new text. For full disclosure, during the audit and
scoping the following was seen: the first 10 to 15 lines of each old header (this included the first
declarations of `devhelp.h`, `fsd.h` (the FSA_ attribute bits with their values), `fsh.h`, `strat2.h`, `devhdr.h`,
`devcmd.h` and the start of the structure `SAS`); the list of macro *names* in them, which the IFS sources use
(printed once to count how much of each header is used); and nine `VMDHA_xxx` flag lines of the old `devhelp.h`
(their values equal the bit positions given in the PDD reference). Layouts, values and prototypes in the new files come
from the documents above, not from that text. The old files were moved out of the tree
(`References\DO-NOT-READ-netlabs-ibm-headers`) before the new ones were written.

## UNVERIFIED items (to be checked on the running system)

1. `FSA_LARGEFILE` = 0x20: not in the 1993 IFS reference; the value is the one other open-source OS/2 file systems use.
2. `struct vpfsi`: a trailing `vpi_flags` word (size only) and the use of `void far *` for the two capability pointers.
3. `struct filelock` / `struct filelockl` (64-bit lock records): layout assumed (offset, length).
4. `WAIT_NORMAL/TIMED_OUT/INTERRUPTED` (0/1/2): the PDD reference gives the names, not the numbers; `dhwrap.asm`
   and `devhelp.h` define them consistently (carry clear = 0, carry set with Z = 1, carry set with NZ = 2).
5. The 64-bit `sfi_sizel` / `sfi_positionl` fields are typed `LONGLONG` (the compiler's OS/2 header type).
6. `dhwrap.asm`: `ProcRun`, `ProcBlock`, `Yield`, `GetDOSVar`, `VerifyAccess`, `InternalError`, `RAS`,
   `PhysToGDTSelector`, `VirtToLin`, `FreeGDTSelector`, `VMProcessToGlobal` are new and have not run yet
   (the other five were exercised by HFS/2).

The first mount of an exFAT volume on the Dev VM is the verification for all of them.
