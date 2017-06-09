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

:: When done with arguments, we'll just fall through here

SET "SLANG_TEST_ROOT=%~dp0"

IF "%SLANG_TEST_PLATFORM%" == "" ( SET "SLANG_TEST_PLATFORM=x86" )
IF "%SLANG_TEST_CONFIG%" == "" ( SET "SLANG_TEST_CONFIG=Debug" )

set "SLANG_TEST_BIN_DIR=%SLANG_TEST_ROOT%bin\%SLANG_TEST_PLATFORM%\%SLANG_TEST_CONFIG%\\"

:: ensure that any built tools are visible
SET "PATH=%PATH%;%SLANG_TEST_BIN_DIR%"

:: TODO: ensure that everything is built?

"%SSLANG_TEST_BIN_DIR%slang-test.exe" --bindir "%SLANG_TEST_BIN_DIR%" %*
