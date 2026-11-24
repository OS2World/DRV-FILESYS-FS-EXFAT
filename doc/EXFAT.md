# EXFAT.IFS 0.1a2 - user guide

EXFAT.IFS lets OS/2 and ArcaOS use exFAT volumes (SDXC cards, USB sticks and external disks formatted as exFAT,
files over 4 GB). It mounts **only exFAT**. FAT12/FAT16/FAT32 volumes are left to the normal FAT32.IFS, and the two
drivers can be installed together.

> **Alpha.** Tested on ArcaOS 5.1.2 in a VirtualBox virtual machine, with a 512 MB virtual disk, and by exchanging the
> disk with Windows 11 in both directions. **Not tested on real hardware, with Linux, with files over 2 GB, or as a
> swap or boot volume.** Keep a copy of `CONFIG.SYS` and a way to edit it at boot. Do not use it on data you cannot lose.

> **Legal note.** exFAT is a Microsoft file system and is covered by Microsoft patents. This is why Arca Noae ships
> FAT32.IFS without exFAT. The source code is LGPL, which does not give a patent licence. You are responsible for
> checking whether you may use or distribute this driver.

## Files

| File | What it is |
|---|---|
| `EXFAT.IFS` | the installable file system driver |
| `UEXFAT.DLL` | used by the OS/2 `FORMAT` and `CHKDSK` commands (file system name EXFAT) |
| `EXFATFMT.EXE`, `EXFATCHK.EXE`, `EXFATSYS.EXE` | stand-alone format, check and system tools |

## Install

1. Back up `CONFIG.SYS`. A bad file system driver can stop the boot, so know how to edit it at boot (for example Alt+F1).
2. Copy `EXFAT.IFS` to a directory of your choice, for example `C:\OS2\BOOT`.
3. Copy `UEXFAT.DLL` to a directory in your `LIBPATH`, for example `C:\OS2\DLL`.
4. Add a line to `CONFIG.SYS`, after the FAT32.IFS line if there is one:
   `IFS=C:\OS2\BOOT\EXFAT.IFS /cache:2048 /h`
   (the cache size is in KB; without `/cache:` the default is 1024 KB. Use `/v` to see the version at boot.)
5. Reboot.

A partition or disk must get a drive letter before the driver can use it: LVM (the Logical Volume Manager) assigns
letters to partitions of type 07 ("HPFS/NTFS/exFAT") as compatibility volumes. Removable media (USB sticks, cards in a
USB reader) get a letter from the USB drivers.

## Use

* **Format** a volume as exFAT: `EXFATFMT.EXE X: /FS:EXFAT /V:label` (tested). `FORMAT X: /FS:EXFAT` should work too, because the OS/2 `FORMAT` loads `UEXFAT.DLL` for the file system name EXFAT as `CHKDSK` does, but it has not been tried. Formatting erases the volume.
* **Check** a volume: `CHKDSK X:` (it reports the file system type EXFAT and loads `UEXFAT.DLL`); `/F` is meant to fix errors (not tried).
* Everything else is the normal file system interface: copy, rename, delete, long names (Unicode), directories.

The standard `CHKDSK`/`FORMAT` messages come from the OS/2 message file `OSO001.MSG`. In a session whose `DPATH` is empty (for example a remote ssh session) the tools print "SYSxxxx (Message file not found!)"; with `DPATH` containing `C:\OS2\SYSTEM` they print the normal text (verified).

## Parameters

Boot messages follow the EDM2 "Suggestion for OS/2 and eComStation Drivers Arguments": quiet by default (only errors and
warnings), every line starts with `EXFAT: `, failures read `EXFAT: EXFAT.IFS could not be loaded. <reason>`.

| Parameter | Effect |
|---|---|
| `/v` | print the version and the settings in use |
| `/q` | no messages at all, not even errors |
| `/w` | wait for a key after the messages |
| `/aa` | beep when the driver loaded |
| `/exfat:<drives>` | mount only these drive letters, for example `/exfat:efg`; `/exfat:*` all (the default); `/exfat:-ab` all except A and B |
| `/cache:<KB>` | cache size, default 1024 |
| `/h` | put the cache in high memory |
| `/rasectors:<n>` | read-ahead sectors |
| `/eas`, `/eas2` | extended attribute handling (see `src\doc\fat32.txt`) |
| `/largefiles` | support for files over 2 GB |
| `/monitor` | trace to the OS/2 trace buffer (diagnostics) |
| `/autocheck:<drives>` | check these drives at boot |

The full description of the shared options is in the upstream manual `src\doc\fat32.txt` (written for FAT32.IFS).

## What is different from FAT32.IFS

* It mounts only exFAT volumes, its file system name is `EXFAT`, and the utility DLL is `UEXFAT.DLL`.
* The FAT12/16/32 forwarder DLLs, the loop driver, the partition filter and the other upstream utilities are not included.
* The device-chain walk that fetched the disk driver's capabilities was replaced by the pointers the kernel passes to the
  file system at mount time.
* Directories that Windows made contiguous (flag "NoFatChain") are converted to a normal cluster chain when they must grow,
  and the length in the parent directory entry is kept up to date.

## Known issues

* Alpha: tested only in a virtual machine and with Windows 11. Not tested with real cards or readers, Linux, files over
  2 GB, or as swap/boot volume.
* An insert that fails halfway (for example disk full) can leave a dangling directory entry or unreleased clusters
  (upstream behaviour).
* Messages of `FORMAT` and `CHKDSK` come from the OS/2 message file and speak of the "FAT" file system in general.
* Extended attributes (`/eas`) were not tried.
* Creating, deleting and re-creating many files (entry sets of all lengths, across block boundaries) was tested and checked by Windows `chkdsk` (`test\exfattest7.cmd`). Renaming across block boundaries on its own was not; the directory code is the upstream code with the fixes listed in `CHANGES.md`, so expect more bugs of this kind in an alpha.

## Reporting problems

Say which OS/2 version, which device, the exact `CONFIG.SYS` line, and what `EXFAT: ...` messages appeared at boot.
Source and history: https://github.com/OS2World/DRV-FILESYS-FS-EXFAT
