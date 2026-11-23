@echo off
set OUT=C:\EXFAT\test3.log
echo ==== recycle > %OUT%
dir E:\$RECYCLE.BIN >> %OUT% 2>&1
echo ==== svi >> %OUT%
dir "E:\System Volume Information" >> %OUT% 2>&1
echo ==== all with /a >> %OUT%
dir E:\* /a >> %OUT% 2>&1
echo ==== l >> %OUT%
dir E:\l* >> %OUT% 2>&1
echo ==== A >> %OUT%
dir E:\A* >> %OUT% 2>&1
echo ==== dollar >> %OUT%
dir E:\$* >> %OUT% 2>&1
echo ==== S >> %OUT%
dir E:\S* >> %OUT% 2>&1
echo ==== d >> %OUT%
dir E:\d* >> %OUT% 2>&1
echo ==== m >> %OUT%
dir E:\m* >> %OUT% 2>&1
echo ==== attrib >> %OUT%
attrib E:\$RECYCLE.BIN >> %OUT% 2>&1
attrib "E:\System Volume Information" >> %OUT% 2>&1
echo ==== end >> %OUT%
