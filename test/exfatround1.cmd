@echo off
set OUT=C:\EXFAT\round1.log
echo ==== format > %OUT%
echo y | C:\EXFAT\exfatfmt.exe E: /V:EXFATTEST /FS:EXFAT > nul 2>&1
dir E:\ >> %OUT% 2>&1
echo ==== copy a few files >> %OUT%
md E:\large >> %OUT% 2>&1
md E:\dir1 >> %OUT% 2>&1
copy C:\EXFAT\work\ten.bin E:\large\ten.bin >> %OUT% 2>&1
copy C:\OS2\DLL\PMMERGE.DLL E:\dir1\pmmerge.dll >> %OUT% 2>&1
copy C:\CONFIG.GP2 E:\readme-os2.txt >> %OUT% 2>&1
C:\usr\bin\md5sum.exe E:/large/ten.bin E:/dir1/pmmerge.dll E:/readme-os2.txt >> %OUT% 2>&1
dir E:\ /s >> %OUT% 2>&1
echo ==== end >> %OUT%
