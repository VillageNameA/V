@echo off

rem Clean previous build artifacts
if exist "Makefile" del /F /Q "Makefile"
if exist "Makefile.Debug" del /F /Q "Makefile.Debug"
if exist "Makefile.Release" del /F /Q "Makefile.Release"
if exist "debug" rmdir /s /q "debug"
if exist "release" rmdir /s /q "release"
if exist "object_script.*" del /F /Q "object_script.*"
if exist ".qmake.stash" del /F /Q ".qmake.stash"

rem Set environment and execute build
set PATH=J:\QT\Tools\mingw1310_64\bin;J:\QT\6.10.2\mingw_64\bin;%PATH%
J:\QT\6.10.2\mingw_64\bin\lrelease.exe resources\translations\v_*.ts
J:\QT\6.10.2\mingw_64\bin\qmake.exe MusicPlayer.pro
J:\QT\Tools\mingw1310_64\bin\mingw32-make.exe
pause
