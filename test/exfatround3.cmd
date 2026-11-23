@echo off
set OUT=C:\EXFAT\round3.log
echo ==== what Windows wrote > %OUT%
dir E:\ /a >> %OUT% 2>&1
dir E:\winfolder /a >> %OUT% 2>&1
echo ==== write 40 files into the Windows-made winfolder >> %OUT%
for %%i in (1 2 3 4) do for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\winfolder\written by OS2 number %%i%%j.txt" > nul
dir E:\winfolder >> %OUT% 2>&1
C:\usr\bin\md5sum.exe C:/CONFIG.GP2 "E:/winfolder/written by OS2 number 10.txt" "E:/winfolder/written by OS2 number 49.txt" >> %OUT% 2>&1
echo ==== end >> %OUT%
