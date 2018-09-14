@echo off
setlocal
pushd %~dp0

:: Argument parsing loop, for arguments that we need to handle at the .bat level

:ARGLOOP

if "%1"=="-debug" (
	set SLANG_TEST_CONFIG=Debug
	shift
	goto :ARGLOOP
)
if "%1"=="-release" (
	set SLANG_TEST_CONFIG=Release
	shift
	goto :ARGLOOP
)
if "%1"=="-platform" (
	set SLANG_TEST_PLATFORM=%2
	shift
	shift
	goto :ARGLOOP
)
if "%1"=="-configuration" (
	set SLANG_TEST_CONFIG=%2
	shift
	shift
	goto :ARGLOOP
)

:: When done with arguments, we'll just fall through here




:: Set root directory to the directory where `test.bat` resides
:: (which should be the root of the source tree)
SET "SLANG_TEST_ROOT=%~dp0"

:: If  platform and configuration haven't been set, then set
:: them to default values.
IF "%SLANG_TEST_PLATFORM%" == "" ( SET "SLANG_TEST_PLATFORM=x86" )
IF "%SLANG_TEST_CONFIG%" == "" ( SET "SLANG_TEST_CONFIG=Debug" )

:: If the user specified a platform of "Win32" swap that to "x86"
:: to match how we are generating our output directories.
IF "%SLANG_TEST_PLATFORM%"=="Win32" ( Set "SLANG_TEST_PLATFORM=x86" )

:: Establish the directory where the binaries to be tested reside
set "SLANG_TEST_BIN_DIR=%SLANG_TEST_ROOT%bin\windows-%SLANG_TEST_PLATFORM%\%SLANG_TEST_CONFIG%\"

:: ensure that any built tools are visible
SET "PATH=%PATH%;%SLANG_TEST_BIN_DIR%"

:: TODO: Maybe we should actually invoke `msbuild` to make sure all the code is up to date?

"%SLANG_TEST_BIN_DIR%slang-test.exe" -bindir "%SLANG_TEST_BIN_DIR%\" %*
