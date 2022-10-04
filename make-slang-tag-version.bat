@echo off
:: Generate slang version files 
git describe --tags > slang-tag-version.txt
set /p SLANG_TAG_VERSION=<slang-tag-version.txt
echo #define SLANG_TAG_VERSION "%SLANG_TAG_VERSION%" > slang-tag-version.h
