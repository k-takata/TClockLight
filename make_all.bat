@echo off

echo ===============================
echo Building normal version
echo ===============================
cd source
nmake /nologo %*
if errorlevel 1 goto error_exit
cd ..

echo ===============================
echo Building all version
echo ===============================
cd source_all
call :check_source
if errorlevel 1 goto error_exit
nmake /nologo %*
if errorlevel 1 goto error_exit
cd ..

echo ===============================
echo Building custom version
echo ===============================
cd source_custom
call :check_source
if errorlevel 1 goto error_exit
nmake /nologo %*
if errorlevel 1 goto error_exit
cd ..

exit /b


:check_source
if not exist Makefile (
	echo Souce files not found.
	echo Run prepare_custom.bat in admin mode.
	exit /b 1
)
exit /b 0

:error_exit
cd ..
exit /b 1
