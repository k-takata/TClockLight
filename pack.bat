@echo off
if "%1"=="" goto usage
setlocal

:: Normal version
set execfiles=tcdll.tclock tclock.exe tcplayer.exe tcprop.exe tcsntp.exe tctimer.exe
set pkgfiles=readme-kt.txt config-kt.txt format-kt.txt readme.html

rem 7-zip32 a -mx=9 -m0=PPMd source.7z source pack.bat -xr!out -xr!out64 -xr!work -xr!work64 -xr!*.bak -xr!*.old
rem 7-zip32 a -m0=PPMd:o=31:mem=25 source.7z source pack.bat -xr!out -xr!out64 -xr!work -xr!work64 -xr!*.bak -xr!*.old

robocopy source\out   pkg\x86 %execfiles% > nul
robocopy source\out64 pkg\x64 %execfiles% > nul
robocopy lang pkg\lang > nul
robocopy .    pkg %pkgfiles% > nul

cd pkg
7-zip32 a -mx=9 tclocklight-%1.zip %pkgfiles% lang x86 x64
cd ..


:: All/Custom versions
set execfiles=tcdll.tclock tclock.exe tcprop.exe

robocopy source_all\out      pkg\all\x86    %execfiles% > nul
robocopy source_all\out64    pkg\all\x64    %execfiles% > nul
robocopy source_custom\out   pkg\custom\x86 %execfiles% > nul
robocopy source_custom\out64 pkg\custom\x64 %execfiles% > nul

cd pkg
7-zip32 a -mx=9 tclocklight-%1-custom.7z all custom
cd ..

goto end

:usage
echo.
echo usage: pack ^<version^>
echo.
echo ^<version^>: ex. kt090627

:end
