rem This batch file helps coding agents (e.g. Codex, Claude Code, Copilot) to build
rem the Slang project on Windows. The script will attempt to load the MSVC environment
rem by locating and calling `vcvarsall.bat`, then it will run CMake to configure and
rem build the project using specified presets.
rem Agents can simply run this script to build all targets. By default it builds with
rem the debug configuration.

@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
set "DEFAULT_CONFIGURE_PRESET=default"
set "DEFAULT_BUILD_PRESET=debug"
set "DEFAULT_ARCH=x64"

set "BUILD_PRESET=%~1"
if not defined BUILD_PRESET set "BUILD_PRESET=%DEFAULT_BUILD_PRESET%"

set "CONFIGURE_PRESET=%~2"
if not defined CONFIGURE_PRESET set "CONFIGURE_PRESET=%DEFAULT_CONFIGURE_PRESET%"

set "TARGET_ARCH=%~3"
if not defined TARGET_ARCH set "TARGET_ARCH=%DEFAULT_ARCH%"

set "VCVARSALL="

if defined VSINSTALLDIR if exist "%VSINSTALLDIR%VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARSALL=%VSINSTALLDIR%VC\Auxiliary\Build\vcvarsall.bat"
)

if not defined VCVARSALL (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "%VSWHERE%" (
        for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            if exist "%%~I\VC\Auxiliary\Build\vcvarsall.bat" (
                set "VCVARSALL=%%~I\VC\Auxiliary\Build\vcvarsall.bat"
            )
        )
    )
)

if not defined VCVARSALL (
    for %%I in (
        "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
        "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
        "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
        "%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
        "%ProgramFiles%\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvarsall.bat"
    ) do (
        if not defined VCVARSALL if exist "%%~I" set "VCVARSALL=%%~I"
    )
)

if not defined VCVARSALL (
    echo Failed to locate vcvarsall.bat.
    echo Install Visual Studio C++ tools or update this script with the correct path.
    exit /b 1
)

where cmake.exe >nul 2>&1
if errorlevel 1 (
    echo Failed to find cmake.exe in PATH.
    exit /b 1
)

rem If using the default configure preset, ensure Ninja is available since it uses
rem the Ninja Multi-Config generator.
if /I "%CONFIGURE_PRESET%"=="%DEFAULT_CONFIGURE_PRESET%" (
    where ninja.exe >nul 2>&1
    if errorlevel 1 (
        echo The "%CONFIGURE_PRESET%" CMake preset requires ninja.exe, but it was not found in PATH.
        echo Install Ninja or select a Visual Studio-based CMake preset instead.
        exit /b 1
    )
)
echo Loading MSVC environment from:
echo   %VCVARSALL%
call "%VCVARSALL%" %TARGET_ARCH%
if errorlevel 1 (
    echo vcvarsall.bat failed.
    exit /b 1
)

pushd "%REPO_ROOT%"

echo Configuring with preset "%CONFIGURE_PRESET%"...
cmake.exe --preset "%CONFIGURE_PRESET%"
if errorlevel 1 (
    popd
    exit /b 1
)

echo Building with preset "%BUILD_PRESET%"...
cmake.exe --build --preset "%BUILD_PRESET%"
set "BUILD_EXIT=%ERRORLEVEL%"

popd
exit /b %BUILD_EXIT%
