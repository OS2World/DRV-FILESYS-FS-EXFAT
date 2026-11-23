# Header and third-party file audit of FS-EXFAT (2026-11-23)

Question: does the tree contain material from IBM (the IFS DDK) or Microsoft that cannot be redistributed?
Method: search of every file for IBM / Microsoft / SCCS / "CONFIDENTIAL" / copyright markers, reading the
header of every file in `src\include`, and a build test (remove a file, build from scratch). No IBM DDK file
was opened for this audit: the DDK is not needed to decide what in *this* tree is derived from it.

## 1. Files with explicit IBM or Microsoft notices (REMOVED, none is needed by the build)

| File (upstream path) | Evidence |
|---|---|
| `lib\save\dhcalls.h` | "COPYRIGHT IBM CORPORATION, 1991, 1992 ... IBM CONFIDENTIAL" |
| `lib\save\fsd.h` | old copy of the IFS header (see section 2) |
| `partfilt\devhlp.inc` (the whole `partfilt\` directory, an unbuilt partition filter) | "IBM/Microsoft Confidential ... Copyright (c) IBM Corporation 1987, 1990, Microsoft Corp." |
| `ifsinf\fsd.h`, `ifsinf\fsh.h` | SCCS ids "@(#)fsd.h 6.5 91/12/16", "@(#)fsh.h 6.5 91/10/20" (the Microsoft/IBM IFS headers, written for Microsoft C 6.0) |
| `ifsinf\ifs\ifs.ipf` and the rest of `ifsinf\` | "Copyright International Business Machines Corporation 1991": the IBM *Installable File Systems* reference, converted to IPF by a third party |
| `lib\fshelper.lib`, `lib\dhcalls.lib` (removed earlier) | IBM DDK import libraries, not used by the Open Watcom build |

The `ifsinf\` documentation was copied to `References\netlabs-fat32-ifsinf` (outside the repository) for use as a
specification when the headers below are replaced.

Also removed: `src\doc\icc.opt` (output of the IBM VisualAge C++ help, "Licensed Materials - Program Property of IBM") and `src\doc\msg.txt` (a dump of OS/2 system messages, "Copyright IBM Corporation").

## 2. Headers in `src\include` with no notice but IBM DDK origin

These carry no copyright line, but `src\doc\build.txt` (Brian Smith) says the original build needed "the IFS DDK
Build Environment", and the content (names, layout, comments) is that of the DDK headers.

Removed because the exFAT-only build does not use them (verified by a clean rebuild without them):
`dhcalls.h`, `iorb.h`, `reqpkt.h`.

**Still in the tree and used by the IFS** (`ifs\*.c` through `include\fat32ifs.h`, and `ifs\ifsmount.c`):

| Header | Lines | Names defined / used by the IFS (approx.) | What it is |
|---|---|---|---|
| `fsd.h` | 837 | 186 / 117 | FS_* entry prototypes and the records the kernel passes (`sffsi`, `sffsd`, `cdfsi`, `vpfsi`, ...), flags |
| `fsh.h` | 338 | 99 / 44 | FSH_* (fshelper) prototypes |
| `devhelp.h` | 1628 | 199 / 28 | DevHlp calls as inline assembler for Open Watcom |
| `strat2.h` | 200 | 64 / 17 | request list header / strategy-2 structures |
| `devhdr.h` | 54 | 27 / 4 | device header |
| `devcmd.h` | 44 | 35 / 1 | device command codes |
| `sas.h` | 94 | 12 / 2 | system anchor segment |

Everything else in `src\include` (`fat32*.h`, `exfatdef.h`, `portable.h`, `ver.h`) is netlabs code.

## 3. Third-party code that is not IBM's (kept, licences stated upstream in `src\doc\license.txt`)

QEMU files in `util\qemu-img` (GPL v2), zlib (zlib licence), `Fat32Format` based code in `ufat32\format.c` (GPL),
`partfilt` was Deon van der Westhuysen's (removed with the directory). Not affected by this audit.

## 4. Replacement (done 2026-11-23)

The seven headers were replaced by clean-room headers: `fsd.h`, `fsh.h`, `devhelp.h` (+ `ifs\dhwrap.asm`), `strat2.h` and the
generated `fsproto.h`; `devhdr.h`, `devcmd.h` and `sas.h` are no longer needed (the walk over the device driver chain
was replaced by the pointers the kernel gives to FS_MOUNT). The IFS builds from scratch with them. Method, sources
read, what was seen of the old files and the list of unverified items: `PROVENANCE.md`.

Open: the first mount of an exFAT volume verifies the layouts (see the unverified list); the old files must not be
read again (`References\DO-NOT-READ-netlabs-ibm-headers`).
