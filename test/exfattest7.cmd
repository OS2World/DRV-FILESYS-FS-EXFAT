@echo off
rem exFAT test 7: directory entries that get deleted, renamed and re-used (entry sets of different sizes,
rem sets that straddle the block boundary). Fresh format first. Output: C:\EXFAT\test7.log
set OUT=C:\EXFAT\test7.log
echo ==== format > %OUT%
echo y | C:\EXFAT\exfatfmt.exe E: /V:EXFATTEST /FS:EXFAT > nul 2>&1
echo ==== phase A: 50 files with short, medium and long names >> %OUT%
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\short%%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\medium length file name number %%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\a rather long file name that needs four name entries %%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\x%%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\yet another quite long name for testing entry sets over block borders %%j.txt" > nul
echo ==== phase B: delete every second file of each group >> %OUT%
for %%j in (1 3 5 7 9) do del "E:\short%%j.txt" > nul
for %%j in (1 3 5 7 9) do del "E:\medium length file name number %%j.txt" > nul
for %%j in (0 2 4 6 8) do del "E:\a rather long file name that needs four name entries %%j.txt" > nul
for %%j in (1 3 5 7 9) do del "E:\x%%j.txt" > nul
for %%j in (0 2 4 6 8) do del "E:\yet another quite long name for testing entry sets over block borders %%j.txt" > nul
echo ==== phase C: new files and directories re-use the deleted entries >> %OUT%
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\new file %%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do md "E:\new directory with a longer name %%j" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\the third wave of files has names of every possible length %%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do del "E:\new file %%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\z%%j.txt" > nul
echo ==== phase D: rename, delete and create again >> %OUT%
for %%j in (0 1 2 3 4 5 6 7 8 9) do ren "E:\z%%j.txt" "renamed to a name that is quite a bit longer %%j.txt" > nul
for %%j in (0 2 4 6 8) do del "E:\renamed to a name that is quite a bit longer %%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\w%%j.txt" > nul
for %%j in (0 1 2 3 4 5 6 7 8 9) do copy C:\CONFIG.GP2 "E:\another name of medium size %%j.txt" > nul
for %%j in (1 3 5 7 9) do del "E:\w%%j.txt" > nul
for %%j in (0 2 4 6 8) do del "E:\another name of medium size %%j.txt" > nul
for %%j in (0 1 2 3 4) do copy C:\CONFIG.GP2 "E:\last round with long names to fill the holes %%j.txt" > nul
echo ==== result >> %OUT%
dir E:\ >> %OUT% 2>&1
C:\usr\bin\md5sum.exe C:/CONFIG.GP2 E:/short0.txt "E:/the third wave of files has names of every possible length 9.txt" "E:/last round with long names to fill the holes 4.txt" >> %OUT%
echo ==== end >> %OUT%
