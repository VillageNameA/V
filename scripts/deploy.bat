@echo off
setlocal

:: 配置路径
set QT_BIN=J:\QT\6.10.2\mingw_64\bin
set PATH=%QT_BIN%;J:\QT\Tools\mingw1310_64\bin;%PATH%
set APP_DIR=%~dp0..\bin
set DEPLOY_EXE=%APP_DIR%\v.exe

echo [1/4] 清理旧编译文件...
if exist "%~dp0..\Makefile" del /F /Q "%~dp0..\Makefile*"
if exist "%~dp0..\debug" rmdir /s /q "%~dp0..\debug"
if exist "%~dp0..\release" rmdir /s /q "%~dp0..\release"

echo [2/4] 开始编译 Release 版本...
pushd "%~dp0.."
qmake MusicPlayer.pro CONFIG+=release
mingw32-make -j8
popd

if not exist "%DEPLOY_EXE%" (
    echo [错误] 编译失败，未找到 %DEPLOY_EXE%
    pause
    exit /b 1
)

echo [3/4] 运行 windeployqt 收集依赖...
windeployqt --release --force --no-opengl-sw --no-translations "%DEPLOY_EXE%"

echo [4/4] 拷贝 FFmpeg 依赖...
copy /Y "%~dp0..\third_party\ffmpeg\bin\*.dll" "%APP_DIR%\"

echo.
echo ==================================================
echo 部署完成！
echo 程序目录: %APP_DIR%
echo 您可以将 bin 目录打包发布，或使用 Inno Setup 制作安装包。
echo ==================================================
pause
