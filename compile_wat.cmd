@echo off
rem compile_wat.cmd - build the IFS and the utility DLLs with Open Watcom 2 (run from the project root)
rem   compile_wat.cmd         build, log in compile.log
rem   compile_wat.cmd clean   remove src\bin, src\bld and the logs
rem Needs Open Watcom 2 (C:\WATCOM) and the OS/2 Toolkit 4.5 (C:\os2tk45).
rem OS is set to a value other than "os2" on purpose: it switches off the upstream
rem LXLITE compression and the .SYM generation (those need lxlite and awk).
set d=p
set DEBUG=0
set OS=owbuild
set NUL=\dev\nul
set CP=copy
set RM=del
set SEP=\
set WATCOM=C:\WATCOM
set TOOLKIT=C:\os2tk45
set SVNREV=384
set PROJSTR=FS-EXFAT local build
set PATH=%WATCOM%\BINP;%WATCOM%\BINW;%TOOLKIT%\bin;%PATH%
set BEGINLIBPATH=%TOOLKIT%\dll;%WATCOM%\binp\dll
set EDPATH=%WATCOM%\EDDAT
set INCLUDE=%WATCOM%\H;%WATCOM%\H\OS21X;%WATCOM%\H\OS2

if not exist "%WATCOM%\BINP\WMAKE.EXE" goto missing_wmake
if "%1"=="clean" goto clean

rem the version header is generated only when it is missing: always make a new one
if exist src\include\ver.h del src\include\ver.h
cd src
echo ==== dirs > ..\compile.log
wmake -h -f makefile.wcc .\bld\bld.flg >> ..\compile.log 2>&1 < nul
echo ==== include >> ..\compile.log
cd include
wmake -h -f makefile.wcc targets >> ..\..\compile.log 2>&1 < nul
cd ..
echo ==== libc >> ..\compile.log
cd ifs\libc
wmake -h -f makefile.wcc targets >> ..\..\..\compile.log 2>&1 < nul
cd ..\..
echo ==== ifs >> ..\compile.log
cd ifs
wmake -h -f makefile.wcc targets >> ..\..\compile.log 2>&1 < nul
cd ..
echo ==== ufat32 >> ..\compile.log
cd ufat32
wmake -h -f makefile.wcc targets >> ..\..\compile.log 2>&1 < nul
cd ..\..
if not exist src\bin\os2\boot\exfat.ifs goto build_failed
if not exist src\bin\os2\dll\uexfat.dll goto build_failed
echo OK: src\bin\os2\boot\exfat.ifs
dir src\bin\os2\boot\exfat.ifs src\bin\os2\dll\uexfat.dll
echo Build log: compile.log
exit 0

:clean
cd src
wmake -h -f makefile.wcc clean < nul
cd ..
if exist src\bin\os2\boot\exfat.ifs del src\bin\os2\boot\exfat.ifs
if exist compile.log del compile.log
echo OK: cleaned.
exit 0

:missing_wmake
echo FAILED: Open Watcom wmake was not found at %WATCOM%\BINP\WMAKE.EXE.
exit 1

:build_failed
type compile.log
echo FAILED: the build failed. See compile.log.
exit 1
