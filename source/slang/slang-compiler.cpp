// slang-compiler.cpp
#include "slang-compiler.h"

namespace Slang
{

bool isValidSlangLanguageVersion(int version)
{
    switch (version)
    {
    case SLANG_LANGUAGE_VERSION_LEGACY:
    case SLANG_LANGUAGE_VERSION_2025:
    case SLANG_LANGUAGE_VERSION_2026:
        return true;
    default:
        return false;
    }
}

bool isValidGLSLVersion(int version)
{
    switch (version)
    {
    case 100:
    case 110:
    case 120:
    case 130:
    case 140:
    case 150:
    case 300:
    case 310:
    case 320:
    case 330:
    case 400:
    case 410:
    case 420:
    case 430:
    case 440:
    case 450:
    case 460:
        return true;
    default:
        return false;
    }
}

} // namespace Slang
