@echo off
rem exFAT test 2: many files, a large file, chkdsk (output in C:\EXFAT\test2.log)
set OUT=C:\EXFAT\test2.log
echo ==== start > %OUT%
md C:\EXFAT\work >> %OUT% 2>&1
rem build an 80 MB source file on C:
copy /b C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL+C:\OS2\DLL\PMMERGE.DLL C:\EXFAT\work\ten.bin >> %OUT% 2>&1
copy /b C:\EXFAT\work\ten.bin+C:\EXFAT\work\ten.bin+C:\EXFAT\work\ten.bin+C:\EXFAT\work\ten.bin+C:\EXFAT\work\ten.bin+C:\EXFAT\work\ten.bin C:\EXFAT\work\sixty.bin >> %OUT% 2>&1
dir C:\EXFAT\work >> %OUT% 2>&1
echo ==== copy large file to E: >> %OUT%
md E:\large >> %OUT% 2>&1
copy C:\EXFAT\work\sixty.bin E:\large\sixty.bin >> %OUT% 2>&1
copy C:\EXFAT\work\ten.bin E:\large\ten.bin >> %OUT% 2>&1
C:\usr\bin\md5sum.exe C:/EXFAT/work/sixty.bin E:/large/sixty.bin C:/EXFAT/work/ten.bin E:/large/ten.bin >> %OUT% 2>&1
echo ==== many files >> %OUT%
md E:\many >> %OUT% 2>&1
for %%i in (1 2 3 4 5 6 7 8 9) do for %%j in (0 1 2 3 4 5 6 7 8 9) do for %%k in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\many\file number %%i%%j%%k with a longer name.txt" > nul
dir E:\many >> %OUT% 2>&1
echo ==== free space >> %OUT%
dir E:\ >> %OUT% 2>&1
echo ==== end >> %OUT%
