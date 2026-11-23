@echo off
rem exFAT test 1 on drive E: (output in C:\EXFAT\test1.log)
set OUT=C:\EXFAT\test1.log
echo ==== start > %OUT%
md E:\dir1 >> %OUT% 2>&1
md E:\dir1\sub >> %OUT% 2>&1
md "E:\A long directory name with spaces" >> %OUT% 2>&1
copy C:\OS2\DLL\PMMERGE.DLL E:\dir1\pmmerge.dll >> %OUT% 2>&1
copy C:\OS2\DLL\PMMERGE.DLL "E:\A long directory name with spaces\Long File Name Example.dll" >> %OUT% 2>&1
copy C:\CONFIG.SYS E:\dir1\sub\config.txt >> %OUT% 2>&1
copy C:\OS2\DLL\PMMERGE.DLL E:\big1.bin >> %OUT% 2>&1
copy E:\big1.bin E:\big2.bin >> %OUT% 2>&1
copy E:\big1.bin E:\big3.bin >> %OUT% 2>&1
echo ==== dir >> %OUT%
dir E:\ >> %OUT% 2>&1
dir E:\dir1 >> %OUT% 2>&1
dir "E:\A long directory name with spaces" >> %OUT% 2>&1
echo ==== checksums >> %OUT%
C:\usr\bin\md5sum.exe C:/OS2/DLL/PMMERGE.DLL E:/dir1/pmmerge.dll "E:/A long directory name with spaces/Long File Name Example.dll" E:/big1.bin E:/big2.bin E:/big3.bin C:/CONFIG.SYS E:/dir1/sub/config.txt >> %OUT% 2>&1
echo ==== rename / delete >> %OUT%
ren E:\big3.bin renamed.bin >> %OUT% 2>&1
del E:\big2.bin >> %OUT% 2>&1
move E:\renamed.bin E:\dir1\sub\ >> %OUT% 2>&1
dir E:\ /s >> %OUT% 2>&1
echo ==== end >> %OUT%
