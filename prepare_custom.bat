@echo off

if not exist source_all\nul exit
if not exist source_custom\nul exit

cd source_all
call :createlink
cd ..
cd source_custom
call :createlink
cd ..

goto :eof


:createlink
if exist common   rmdir common
if exist dll      rmdir dll
if exist exe      rmdir exe
if exist player   rmdir player
if exist property rmdir property
if exist sntp     rmdir sntp
if exist timer    rmdir timer

mklink /d common   ..\source\common
mklink /d dll      ..\source\dll
mklink /d exe      ..\source\exe
mklink /d player   ..\source\player
mklink /d property ..\source\property
mklink /d sntp     ..\source\sntp
mklink /d timer    ..\source\timer
exit /b
