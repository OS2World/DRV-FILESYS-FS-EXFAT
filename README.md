# FS-EXFAT

User guide: [doc/EXFAT.md](doc/EXFAT.md). Release notes: [doc/CHANGES.md](doc/CHANGES.md).

EXFAT.IFS is an OS/2 / ArcaOS installable file system driver for **exFAT** volumes only (SD/SDXC cards,
USB sticks, external disks formatted as exFAT). It is a downstream build of the netlabs **FAT32.IFS**
project (Henk Kelder, Netlabs, KO Myung-Hun, Valery V. Sedletski and others): the exFAT code is theirs.
This repository builds it as a separate driver, so it can be installed next to a FAT32.IFS that was built
without exFAT support (as Arca Noae distributes it), or on its own.

| Field | Details |
|---|---|
| Version | 0.1a3 (third alpha), based on the netlabs FAT32 trunk at SVN revision 384 |
| Status | Loads on ArcaOS 5.1.2 (VM), formats and mounts exFAT volumes; directories, long names, copy, rename, delete, a 76 MB file and 900 files in one directory verified by MD5, CHKDSK and a host-side structure check; Windows 11 reads our volume and our driver reads what Windows wrote. Directories created by Windows (contiguous) can grow. Not tested: real hardware, Linux, files over 2 GB. |
| Tested on | ArcaOS 5.1.2 virtual machine (VirtualBox): 512 MB virtual disk (0.1a1, 2026-11-23) and exFAT USB sticks written by Windows (0.1a2 and 0.1a3, 2026-11-24; 0.1a3: 20 GB and 57 GB partitions, files up to 3.5 GB). No real hardware. |
| License | GNU Lesser General Public License 2.1, see [LICENSE](LICENSE) (the LGPL covers the copyright, not the exFAT patents); parts under other GPL-compatible licences are listed in `src\doc\license.txt` |
| Authors | Henk Kelder (original FAT32.IFS, 2002 copyright given to netlabs.org), Brian Smith, KO Myung-Hun, Alfredo Fernandez Diaz, David Graser, Lars Erdmann, Valery V. Sedletski (exFAT support, FAT32.IFS 0.10); OS2World (EXFATONLY build) |
| Original source | https://svn.netlabs.org/repos/fat32/trunk (SVN revision 384) |
| Build tools | Open Watcom 2.0 and the OS/2 Toolkit 4.5; build with `compile_wat.cmd` |

## Legal note (please read)

exFAT is a Microsoft file system and is covered by Microsoft patents. That is why Arca Noae distributes
FAT32.IFS without exFAT. **This driver contains exactly the part they leave out.** The source code is LGPL
(free to use and distribute as source), but the patent position of exFAT is a separate question. Whoever
distributes or uses this driver is responsible for checking it for their own situation. This repository does
not give legal advice.

## What is different from FAT32.IFS

Everything is under `#ifdef EXFATONLY` (set by the makefiles), so the changes can be compared with, and
merged from, the upstream trunk.

* The driver is `EXFAT.IFS` and its file system name (`FS_NAME`) is `EXFAT`. It mounts **only exFAT**
  volumes and refuses FAT12/FAT16/FAT32, which are left to FAT32.IFS.
* exFAT is enabled by default; the `/exfat:<drives>` switch can still restrict the drive letters.
* The utility DLL is `UEXFAT.DLL` (the real DLL: `FORMAT`, `CHKDSK`, `SYS`, `RECOVER` and the translation
  table loader), which OS/2 loads for the file system name EXFAT. Its default format type is exFAT. The
  stand-alone tools are `exfatfmt.exe`, `exfatchk.exe`, `exfatsys.exe`. All use the name `EXFAT` towards the IFS.
  The FAT12/16/32 forwarder DLLs are not built.
* The shared memory name for the translation table is `\SHAREMEM\EXFAT\TRANSTBL`, so it cannot clash with
  FAT32.IFS.
* Boot messages follow the EDM2 "Suggestion for OS/2 and eComStation Drivers Arguments": quiet by default,
  every line starts with `EXFAT: `.

## Build

From the project root on ArcaOS:

```text
compile_wat.cmd
```

It sets the Open Watcom and Toolkit environment (`C:\WATCOM`, `C:\os2tk45`), builds the include files, the
private C library, the IFS and the utilities, and prints `OK: src\bin\os2\boot\exfat.ifs` (or `FAILED:`).
The log is `compile.log`. `compile_wat.cmd clean` runs the upstream `clean` target. Outputs are in `src\bin\os2\`:
`boot\exfat.ifs`, `dll\uexfat.dll`, `exfatfmt.exe`, `exfatchk.exe`, `exfatsys.exe`.

The upstream makefiles call `lxlite` (compression) and `awk` (symbol files) when the environment variable `OS`
is `os2`; `compile_wat.cmd` sets `OS=owbuild` to skip those steps.

## Install

1. Back up `CONFIG.SYS` and know how to restore it: a bad file system driver can stop the boot.
2. Copy `exfat.ifs` to `C:\OS2\BOOT` and `uexfat.dll` to `C:\OS2\DLL`.
3. Add `IFS=C:\OS2\BOOT\EXFAT.IFS [parameters]` to `CONFIG.SYS`, after the FAT32.IFS line if there is one.
4. A partition must be recognised by LVM / the partition manager before it gets a drive letter (see
   `src\doc\fat32.txt`, "Making OS/2 recognize FAT32 partitions": the same applies to exFAT).

## Parameters and boot messages

| Parameter | Effect |
|---|---|
| `/v` | print the version and the settings in use |
| `/q` | no messages at all, not even errors |
| `/w` | wait for a key after the messages |
| `/aa` | beep when the driver loaded |
| `/exfat:<drives>` | restrict the mounted drive letters (`/exfat:efg`, `/exfat:*`, `/exfat:-ab` excludes) |
| `/largefiles` | raise the largest file from 2 GB - 1 to 4 GB - 1 (see Limits below) |

The other upstream switches (`/cache:`, `/rasectors:`, `/eas`, `/monitor`, `/autocheck:` ...)
are described in `src\doc\fat32.txt`. Failure message form: `EXFAT: EXFAT.IFS could not be loaded. <reason>`.

## Limits (please read)

* **File size: 2 GB - 1 by default, 4 GB - 1 with `/largefiles`.** exFAT itself allows far bigger files, but the OS/2 kernel
  (tested on ArcaOS 14.204) hands a 16-bit file system driver only **32-bit seek offsets**: `DosSetFilePtrL` reaches the driver
  through the old 32-bit entry point, and offsets above 4 GB lose their top bits. JFS, a 32-bit driver, seeks correctly past 2 GB
  and 4 GB on the same system. So a file bigger than 4 GB could not be addressed correctly here; the driver refuses to create
  one (error 87, invalid parameter). Going further would need a 32-bit file system driver; see `doc\CHANGES.md` (0.1a3).
  **0.1a2 with `/largefiles` could create files over 4 GB; do not rely on files over 4 GB written by 0.1a2.**
  A 32-bit entry point was tried (see `doc\CHANGES.md`, "Investigated after 0.1a3"): the kernel then needs the whole 32-bit I/O set (`FS32_READ`, `FS32_WRITE`), so a 4 GB+ driver means a 32-bit core, not a patch. Not done.
* **The disk needs a partition table.** A disk with no partition table (the exFAT boot sector in the very first sector, as
  some cameras and consoles make) is not offered to the driver by the ArcaOS USB stack; it stays "unknown". Format the stick
  with a partition (Windows Explorer does not; Disk Management, or `diskpart` with `create partition primary`, does).

## Known issues / open points

* Tested on virtual disks and USB sticks in a VM, and with Windows 11 mounting the volume (both directions). Not tested with Linux or real hardware. Files up to 3.5 GB were tested.
* `CHKDSK` (`exfatchk`) can report 0 free clusters on a volume whose FAT holds stale chains for free clusters (seen on a stick formatted by another device); the driver counts free space from the bitmap and is not affected. Not fixed.
* `FORMAT` says "The new type of file system is FAT32" although it makes an exFAT volume (wrong message text only).
* `DosSetFileSizeL` does not clear the added area; it holds old disk data until written (OS/2 does not promise zeros).
* An insert that fails halfway (disk full) can leave a dangling directory entry or unreleased clusters (upstream behaviour).
* **Headers:** the upstream tree contained IBM DDK-derived headers. Files with explicit IBM/Microsoft notices
  and the IBM IFS reference were removed, and the seven headers the IFS needs were replaced by clean-room
  headers (`doc\HEADER-AUDIT.md`, `doc\PROVENANCE.md`). They are **unverified on the running system** until the
  first exFAT volume is mounted (list of unverified items in `PROVENANCE.md`).
* The messages of the utility DLL (CHKDSK/FORMAT text, `src\doc\message.txt`) and the documentation still say FAT32.
* The loop driver, the partition filter, the cache daemon, the other upstream utilities, the Win32 and Korean parts and the IBM-compiler/Microsoft-C build files of the upstream project were removed from this tree (they are in the upstream repository, SVN revision 384).

## Links

* netlabs FAT32 project: https://trac.netlabs.org/fat32/ (source: https://svn.netlabs.org/repos/fat32/trunk)

## Changelog

See [doc/CHANGES.md](doc/CHANGES.md).
