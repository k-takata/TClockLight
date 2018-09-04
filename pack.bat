@echo off
if "%1"=="" goto usage
setlocal DISABLEDELAYEDEXPANSION

:: 7z (official version) or 7-zip32 (undll + common archiver) can be used
if "%SEVENZIP%"=="" set SEVENZIP=7-zip32

:: Normal version
set execfiles=tcdll.tclock tclock.exe tcplayer.exe tcprop.exe tcsntp.exe tctimer.exe
set pkgfiles=readme-kt.txt config-kt.txt format-kt.txt readme.html
set srcfiles=source source_all\Makefile source_all\config.h source_custom\Makefile source_custom\config.h pack.bat make_all.bat prepare_custom.bat

if exist pkg rd /s /q pkg
mkdir pkg

rem "%SEVENZIP" a -mx=9 -m0=PPMd source.7z %srcfiles% -xr!out -xr!out64 -xr!work -xr!work64 -xr!*.bak -xr!*.old -xr!*.sw? -xr!*~ -xr!*.aps -xr!tags
"%SEVENZIP%" a -m0=PPMd:o=31:mem=25 source.7z %srcfiles% -xr!out -xr!out64 -xr!work -xr!work64 -xr!*.bak -xr!*.old -xr!*.sw? -xr!*~ -xr!*.aps -xr!tags
move /y source.7z pkg > nul

robocopy source\out   pkg\x86 %execfiles% > nul
robocopy source\out64 pkg\x64 %execfiles% > nul
robocopy lang pkg\x86\lang /xf *~ *.sw? > nul
robocopy lang pkg\x64\lang /xf *~ *.sw? > nul
robocopy .    pkg\x86 %pkgfiles% > nul
robocopy .    pkg\x64 %pkgfiles% > nul
copy pkg\source.7z pkg\x86 > nul
copy pkg\source.7z pkg\x64 > nul

pushd pkg\x86
rem "%SEVENZIP%" a -mx=9 ..\tclocklight-%1-x86.zip %pkgfiles% lang source.7z
"%SEVENZIP%" a -mx=9 ..\tclocklight-%1-x86.zip .
popd
pushd pkg\x64
rem "%SEVENZIP%" a -mx=9 ..\tclocklight-%1-x86.zip %pkgfiles% lang source.7z
"%SEVENZIP%" a -mx=9 ..\tclocklight-%1-x64.zip .
popd


:: All/Custom versions
set execfiles=tcdll.tclock tclock.exe tcprop.exe

robocopy source_all\out      pkg\all\x86    %execfiles% > nul
robocopy source_all\out64    pkg\all\x64    %execfiles% > nul
robocopy source_custom\out   pkg\custom\x86 %execfiles% > nul
robocopy source_custom\out64 pkg\custom\x64 %execfiles% > nul

cd pkg
"%SEVENZIP%" a -mx=9 tclocklight-%1-custom.7z all custom
cd ..

goto end

:usage
echo.
echo usage: pack ^<version^>
echo.
echo ^<version^>: ex. kt090627

:end
