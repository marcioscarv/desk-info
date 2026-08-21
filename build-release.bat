@echo off
setlocal

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo Visual Studio Build Tools nao encontrado.
    echo Instale o workload "Desenvolvimento para Desktop com C++".
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_PATH=%%i"
if not defined VS_PATH (
    echo Compilador C++ do Visual Studio nao encontrado.
    exit /b 1
)

call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" || exit /b 1
cmake -S "%~dp0." -B "%~dp0build-native" -G Ninja -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build "%~dp0build-native" || exit /b 1
cmake --install "%~dp0build-native" --prefix "%~dp0dist-cpp" || exit /b 1

echo.
echo Compilacao concluida: %~dp0dist-cpp\DeskInfo.exe
