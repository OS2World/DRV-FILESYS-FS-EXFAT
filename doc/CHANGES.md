# Investigated after 0.1a3 (2026-11-24, not released)

Can the 16-bit driver seek past 4 GB by adding 32-bit entry points? Tried in a scratch copy, ArcaOS kernel 14.204 SMP, VM only.

* Exporting `FS32_ATTRIBUTE` (same value as `FS_ATTRIBUTE`) makes the kernel use the 32-bit entry points. `DosSetFilePtrL` then reaches `FS32_CHGFILEPTRL` with a 64-bit offset and flat pointers; a native 32-bit routine that updates `sfi_position`, `sfi_positionl` (open-file structure offsets 22 and 50) works.
* But the kernel then also calls `FS32_READ` / `FS32_WRITE` for all file I/O. With those not exported it jumps to address 0 (trap 000e in module LSWITCH, EIP 0168:00000000).
* So the 32-bit interface is all-or-nothing. It needs `FS32_READ` / `FS32_WRITE` (probably also `FS32_READFILEATCACHE` / `FS32_RETURNFILECACHE`) as 32-to-16 thunks (`KernThunkStackTo16`, flat pointers mapped to selectors with `DevHlp LinToGDTSelector`), or a native 32-bit core. Arca Noae's FAT32 (the same code base) stops at stubs that return "not supported" and leaves `FS32_ATTRIBUTE` commented out.
* Decision: not done. Files stay limited to 2 GB - 1 (4 GB - 1 with `/largefiles`).

# Release 0.1a3 (2026-11-24)

File size limits made safe and correct (found by the first large-file tests), plus test programs. Tested in an ArcaOS 5.1.2 virtual
machine (kernel 14.204 SMP) with exFAT partitions of 20 GB and 57 GB on a USB stick. Not tested on real hardware.

* **A 16-bit file system driver cannot seek past 4 GB on this kernel.** With `/largefiles`, 0.1a2 let an exFAT file grow past 4 GB, but a seek
  into such a file went to the wrong place. The driver trace (`test\fslog.c`) shows why: `DosSetFilePtrL` reaches the driver through the old
  32-bit `FS_CHGFILEPTR`, never through `FS_CHGFILEPTRL`, and the kernel passes only 32 bits of the offset (2 GB - 4 GB arrive as an unsigned number,
  4 GB and above lose their top bits; `DosSetFileSizeL` does arrive as `FS_NEWSIZEL` with 64 bits). The same test (`test\bigtest.c`) passes on JFS,
  a 32-bit driver, on the same system, so the kernel and the test are fine. Setting the large-file attribute earlier (at load time, not in
  `FS_INIT`) changed nothing. Going beyond 4 GB would need a 32-bit file system driver (the kernel's 32-bit interface passes 64-bit offsets); that is not done.
* **New limit: 2 GB - 1 by default, 4 GB - 1 with `/largefiles`, for exFAT too** (`MAXFILESIZE` in `src\ifs\ifsfile.c`). `FS_NEWSIZEL`, `FS_WRITE` and the initial allocation of
  `FS_OPENCREATE` refuse to go past it (error 87); the upstream FAT32 code exempted exFAT from the 4 GB test. Test `test\captest.c`: writes stop at 4,294,963,903 bytes.
* **Seeks between 2 GB and 4 GB work.** `FS_CHGFILEPTR` (the entry point the kernel uses) took the 32-bit offset as signed, so an absolute seek to 2 GB - 4 GB failed with
  "negative seek" (error 131). With `/largefiles` an absolute offset is now read as unsigned. `FS_CHGFILEPTRL` stored the new position cut to 32 bits (`(ULONG)llNewOffset`, a FAT32 leftover);
  it now keeps 64 bits, and the 32-bit copies of the file size and position in the open-file structure are clamped to 4 GB - 1 instead of wrapping around (a 5 GB file looked 947,912,704 bytes long).
  Result (3,500 MB file): seeks from the start, from the end and relative, and markers across 2 GB read back correctly; a 5,000 MB file is refused. Regression: the nine save patterns,
  growth of a driver-made file, free space before and after, and CHKDSK were unchanged and clean.
* Test programs added: `test\bigtest.c` (sizes, seeks and markers for big files), `test\captest.c` (the cap), `test\fslog.c` (switch the IFS trace on and read it; the safe way to call the private `DosFSCtl` functions),
  `test\bsec.c` (read a drive's boot sector or MBR), `test\psk.c` (list and kill processes by name or number on the test VM).
* Found, documented, not fixed: **a disk without a partition table is not offered to the driver** by the ArcaOS USB stack (the volume stays "unknown"; no `FS_MOUNT` call arrives). Windows Explorer formats
  such a stick without creating a partition table; `diskpart` with `create partition primary` does. A FAT32 (type 0C) partition on the same stick was not mapped either (not investigated).
  **`CHKDSK` (`exfatchk`) can report 0 free clusters** on a volume whose FAT holds a pre-filled chain for free clusters (a 57 GB stick formatted by another device); it trusts the FAT over the allocation bitmap. The driver counts from the bitmap and is not affected.
  `FORMAT` prints "The new type of file system is FAT32" for an exFAT volume (message text only).
* Not changed: the large-file default stays off. `DosSetFileSizeL` does not clear the added area (OS/2 does not promise zeros), as before.

# Release 0.1a2 (2026-11-24)

Two fixes (below) and test programs.
Tested in the ArcaOS 5.1.2 virtual machine with the same exFAT USB stick as 0.1a1. Not tested on real hardware.


* **Data loss fixed: growing a file that Windows wrote (found by the release test, 2026-11-24).** Windows stores an exFAT file contiguously and sets the
  NoFatChain flag, so the file has no chain in the FAT. When such a file had to grow, `NewSize` (`src\ifs\ifsfile.c`) took the new clusters from anywhere on the volume and linked
  them through the FAT, which a contiguous file does not use. If the cluster after the file's last one was already in use, the write stopped short at the end of the
  last cluster and returned success: the data after it was lost (in the test, a 200 KB append to a 41-byte file made by Windows stopped at 32 KB, the cluster size).
  Now the FAT chain of the clusters the file already has is written and the flag cleared before the file is extended (the same conversion directories have had since 0.1a1);
  the directory entry is updated with the next size change. Possible cause of the "`AE.EXE` does not save" report on a file made by Windows. Test: `test\growtest.c`.
* **FS_FSCTL hardening (found by a test program, ArcaOS 5.1.2 VM, 2026-11-24).** Several private `DosFSCtl` function codes of the upstream FAT32 code read their
  parameter or data buffer without checking that the caller supplied one. `DosFSCtl(..., FAT32_GETLOGDATA = 0x8000, no parameter buffer)` dereferenced a null
  pointer at ring 0 (`TRAP 000d` in `FS_FSCTL`) and stopped the system; any program could do it. Added the missing length checks in `src\ifs\fat32a.c` for
  `FAT32_GETLOGDATA`, `FAT32_SETPARMS` (the size test was `>` instead of `<`), `FAT32_WIN2OS`, `FAT32_QUERYSHORTNAME` (it also used the volume before checking it
  was found), `FAT32_MOUNTED`, `FAT32_GETVOLLABEL`, `FAT32_GETALLOC` and `FAT32_SETVOLLABEL`. A call without the buffer now returns an error code.
  The Arca Noae `FAT32.IFS` has the same weakness (the test program stopped the VM through it too).
* Tests (`test\savetest.c`, `test\growtest.c`, `test\fsprobe.c`, run on an 8 GB exFAT USB stick written by Windows): nine editor save patterns (replace, truncate, set size, temp file
  and rename, EA, time stamps, write-through); growing a Windows-made (contiguous) file by 200 KB and 5 MB and truncating it back, verified byte for byte; the queries
  a removable-media monitor can make (file system name, label, allocation, DASD ioctls) give the same answers on exFAT and on a FAT32 stick.
  Final 0.1a2 binary: after the fix, an append of 5 MB to the Windows-made file and of 3 MB to a driver-made file, each truncated back, left the free space unchanged and CHKDSK clean
  (no lost clusters, no cross-links). CHKDSK on the stick before the fix had reported lost clusters (164 in 8 chains, left by the failed and truncated appends of earlier runs and by hard stops of the VM).
* Known cosmetic issue, not in the driver: the Arca Noae "Removable Media" XCenter widget (Rich Walsh, RemMedia.dll) shows `Error` for an `EXFAT` drive (`Fat32`
  for FAT32). With the same driver renamed `FATEX` it shows `Unknown`, so the label depends on the file system name; the driver answers all queries of the widget exactly as FAT32.IFS does.

# Release 0.1a1 (2026-11-23), first alpha

First release of the exFAT-only build of the netlabs FAT32.IFS (SVN revision 384). Tested in an ArcaOS 5.1.2 virtual machine and
with Windows 11 (see Tests). Not tested on real hardware. User guide: `EXFAT.md`.

Changes in the last step of this release:
* Version 0.1a1 (`_VER` in `src\makefile.mk`, vendor OS2World in the BLDLEVEL string).
* Messages: the `FAT32:` prefix of the IFS run-time messages and of the utility messages is now `EXFAT:`; the utilities print `EXFAT version`.
  The boot-sector tests that must still recognise FAT32 volumes were not touched.
* Added `LICENSE` (LGPL 2.1 with a note on the third-party parts and on the patents) and `doc\EXFAT.md` (user guide).

# Changes to FS-EXFAT since the upstream state (SVN revision 384 of https://svn.netlabs.org/repos/fat32/trunk)

Upstream: https://svn.netlabs.org/repos/fat32/trunk, SVN revision 384, the starting point of this tree.

## Clean-up for release 0.1a1
* Removed what the release does not use: `src\util`, `loop`, `f32chk`, `problem`, `test`, `lib`; the Win32 and IBM C/Set (`icc`) and Microsoft C (`msc`) build files; the upstream `build*`, `_wcc`, `svnrev`, `envwic` scripts (replaced by `compile_wat.cmd`); `src\ufat32\win32.c`, `sys.cmd`; the documentation of removed parts (`build.txt`, `partfilt.*`, `fat32.kor`, `addenda`, `os2fat32.txt`, `problems.txt`, `deamon.txt`). The root makefile and the build directory list only name what is left. Everything is still in the upstream repository (SVN revision 384).

## Build (no source change)
* Added `compile_wat.cmd` (root), `.gitignore`. It builds `src\include`, `src\ifs\libc`, `src\ifs` and
  `src\ufat32` with `wmake -f makefile.wcc targets`, using the upstream makefiles with the environment of
  `buildw.cmd` (without the SVN query: `SVNREV=384`). `OS=owbuild` skips the upstream `lxlite` and `awk` steps.
* Removed `src\lib\dhcalls.lib` and `src\lib\fshelper.lib` (IBM DDK libraries, not used by the Open Watcom build;
  the IFS imports the FSHELPER entry points by ordinal in `src\ifs\makefile.wcc`).

## EXFATONLY (an exFAT-only driver, `-dEXFATONLY` in the makefiles)
* `src\ifs\makefile.wcc`: output `exfat.ifs` (was `fat32.ifs`), description text, `DOSBEEP` import (for `/aa`).
* `src\ifs\fat32a.c`: `FS_NAME` = `EXFAT`; new boot messages and switches `/v`, `/q`, `/w`, `/aa`;
  exFAT enabled by default; FAT12/16/32 mount masks cleared; `InitMessage` honours `/q`;
  messages prefixed `EXFAT: `.
* `src\ifs\ifsmount.c` (`FS_MOUNT`): any volume that is not exFAT is refused with `ERROR_VOLUME_NOT_MOUNTED`.
* `src\ifs\ifsachk.c`: shared memory name `\SHAREMEM\EXFAT\TRANSTBL`.
* `src\ufat32\makefile.wcc`: the real utility DLL is `uexfat.dll` (was `uunifat.dll`), the forwarder DLLs
  `ufat12/16/32.dll` and the old `uexfat.dll` forwarder are not built, the stand-alone tools are
  `exfatfmt.exe`, `exfatchk.exe`, `exfatsys.exe` (were `fat32fmt/chk/sys.exe`).
* `src\ufat32\os2.c`: `FS_NAME` = `EXFAT`; the volume label calls use `FS_NAME` instead of the literal `"FAT32"`.
* `src\ufat32\format.c`: exFAT is the default file system type.

## Header audit (see HEADER-AUDIT.md)
* Removed `src\ifsinf`, `src\lib\save`, `src\partfilt` (IBM/Microsoft notices; unbuilt) and `src\include\dhcalls.h`,
  `iorb.h`, `reqpkt.h` (IBM DDK origin; not used by the exFAT-only build). Clean rebuild verified.

## Clean-room headers (see PROVENANCE.md)
* New `src\include\fsd.h`, `fsh.h`, `devhelp.h`, `strat2.h`, `fsproto.h` and `src\ifs\dhwrap.asm` replace the IBM DDK-derived headers.
* `src\ifs\ifsmount.c`: no device-chain walk (kernel-provided capability pointers), no `devhdr.h`/`devcmd.h`/`sas.h`.
* `src\ifs\makefile.wcc`: `dhwrap.obj` added. `src\ifs\ifsswap.c`: one cast.

## Tests (ArcaOS 5.1.2 VM, 2026-11-23; scripts in `test\`)
* `IFS=C:\EXFAT\EXFAT.IFS /v /w /aa` loads next to the stripped FAT32.IFS (boot messages, wait for a key, beep).
* Blank 512 MB disk made an LVM compatibility volume (E:); `exfatfmt E: /FS:EXFAT` formats it; OS/2 `CHKDSK E:` reports "The type of file system for the disk is EXFAT" and loads `UEXFAT.DLL`.
* `test\exfattest1.cmd`: directories, long names with spaces, copy, rename, delete: MD5 sums identical.
* `test\exfattest2.cmd`: 76 MB and 12 MB files (MD5 identical), 900 files with long names in one directory, `CHKDSK`: no errors, 0% fragmented.
* **Windows interoperability** (the VDI was converted to a VHD, attached in Windows 11 with Disk Management, then converted back):
  * Windows mounted the volume our driver formatted, listed the files, all MD5 sums matched, `chkdsk` was clean.
  * A host-side script (`test\exfat_verify.py`) found no problems: boot region checksum, up-case table checksum, entry set checksums, name hashes, allocation bitmap equal to the clusters in use.
  * Windows added `$RECYCLE.BIN` and `System Volume Information` (directories with the NoFatChain flag set) and a few files; back on OS/2 the driver lists them and reads the files with MD5 sums equal to the ones read on the host (`exfattest3-5.cmd`).
* **Directory growth fixed.** `exfattest6.cmd` showed that a Windows-created (NoFatChain) directory could not grow beyond its first cluster. Fix in `src\ifs\fat32b.c`: `FindDirCluster` remembers the absolute path of the directories it resolves; when `ModifyDirectory` must add a cluster to an exFAT directory it converts a NoFatChain directory to a FAT chain, and afterwards `GrowDirParentUpdate` rewrites the directory's entry in its parent (length += cluster size, NoFatChain cleared). The length is now also updated for directories that were already chained.
* **Clean verification of the fix** (`test\exfatround1.cmd`, `exfatround3.cmd`): freshly formatted volume; Windows 11 created `winfolder` (contiguous), `café.txt` and a long-named file; OS/2 wrote 40 files into `winfolder` (MD5 identical); host check `exfat_verify.py`: no problems (boot/up-case/entry-set checksums, name hashes, directory lengths equal to their clusters, allocation bitmap exact); Windows `chkdsk` (read-only): "The file system was examined without finding problems", 49 files, 10 indexes.
* **Directory corruption fixed (found by the release check).** When an entry was added to a directory block that had free (deleted) entries, `CompactDir1` slid the entries of the block down. An entry set (a file or directory with its name entries) that started in that block and continued in the next one was split: Windows-made long names do this. In the release test the new directory `E:\final` landed in the middle of the Windows file `Text File Made in Windows 10 with long name.txt` and corrupted it. Fix: new `TailSetOffset1` in `src\ifs\fat32a.c`; `ModifyDirectory` (insert, exFAT) no longer rearranges the entry set at the end of a block that continues into the next block. Test `test\exfattest7.cmd` (fresh format; 50 files with names of different lengths, 25 deleted, 30 files and 10 directories added, 10 deleted, 10 added): 55 entry sets, 14 straddling a block boundary, 7 straddling a cluster, all intact; `exfat_verify.py` finds no problems.
* **Delete leaving orphan name entries fixed.** Windows `chkdsk` reported damage in the root directory after files had been deleted: the delete code of `ModifyDirectory` (exFAT) stopped one entry early, so the last name entry of a deleted set stayed in use. Fix: `>=` instead of `>` in the end-of-block test (DELETE and UPDATE branches, `src\ifs\fat32b.c`). `exfat_verify.py` now also reports orphan secondary entries. After the fix `exfattest7.cmd` gives: checker no problems, Windows `chkdsk` (read-only) "examined without finding problems".
* Leftovers of the failed attempt before the fix (a dangling entry-set primary, leaked clusters) show that an insert that fails halfway leaves garbage (upstream behaviour, not changed).
* Not tested: a volume written by Linux, real hardware, files over 2 GB, swap files.
