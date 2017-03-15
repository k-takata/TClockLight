@echo off

if not exist source_all\nul goto :eof
if not exist source_custom\nul goto :eof

cd source_all
call :createlink
cd ..
cd source_custom
call :createlink
cd ..

goto :eof


:createlink
call :removelink common
call :removelink dll
call :removelink exe
call :removelink player
call :removelink property
call :removelink sntp
call :removelink timer

mklink /d common   ..\source\common
mklink /d dll      ..\source\dll
mklink /d exe      ..\source\exe
mklink /d player   ..\source\player
mklink /d property ..\source\property
mklink /d sntp     ..\source\sntp
mklink /d timer    ..\source\timer
exit /b


:removelink
if exist %1\nul (
	rmdir %1
) else if exist %1 (
	rem Remove Cygwin's symlink.
	del /a %1
)
exit /b
