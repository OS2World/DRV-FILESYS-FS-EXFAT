@echo off
set OUT=C:\EXFAT\test4.log
echo ==== dir E:\ > %OUT%
dir E:\ >> %OUT% 2>&1
echo ==== dir E:\*.* >> %OUT%
dir E:\*.* >> %OUT% 2>&1
echo ==== cd E:\ >> %OUT%
cd E:\ >> %OUT% 2>&1
echo ==== dir E:\dir1\ >> %OUT%
dir E:\dir1\ >> %OUT% 2>&1
echo ==== dir E:\large\ >> %OUT%
dir E:\large\ >> %OUT% 2>&1
echo ==== dir "E:\System Volume Information\" >> %OUT%
dir "E:\System Volume Information\" >> %OUT% 2>&1
echo ==== exist >> %OUT%
if exist E:\nul echo E:\nul exists >> %OUT%
if exist E:\dir1\nul echo E:\dir1\nul exists >> %OUT%
echo ==== end >> %OUT%
