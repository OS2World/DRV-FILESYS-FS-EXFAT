@echo off
set OUT=C:\EXFAT\test5.log
echo ==== md5 of the files Windows wrote > %OUT%
C:\usr\bin\md5sum.exe "E:/System Volume Information/WPSettings.dat" "E:/System Volume Information/IndexerVolumeGuid" >> %OUT% 2>&1
echo ==== md5 of the large files >> %OUT%
C:\usr\bin\md5sum.exe E:/large/sixty.bin E:/large/ten.bin E:/big1.bin >> %OUT% 2>&1
echo ==== many count >> %OUT%
dir E:\many >> %OUT% 2>&1
echo ==== end >> %OUT%
