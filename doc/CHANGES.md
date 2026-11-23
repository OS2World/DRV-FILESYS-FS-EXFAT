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
* **Delete leaving orphan name entries fixed.** Windows `chkdsk` reported damage in the root directory after files had been deleted: the delete code of `ModifyDirectory` (exFAT) stopped one entry early, so the last name entry of a deleted set stayed in use. Fix: `>=` instead of `>` in the end-of-block test (DELETE and UPDATE branches, `src\ifsat32b.c`). `exfat_verify.py` now also reports orphan secondary entries. After the fix `exfattest7.cmd` gives: checker no problems, Windows `chkdsk` (read-only) "examined without finding problems".
* Leftovers of the failed attempt before the fix (a dangling entry-set primary, leaked clusters) show that an insert that fails halfway leaves garbage (upstream behaviour, not changed).
* Not tested: a volume written by Linux, real hardware, files over 2 GB, swap files.
