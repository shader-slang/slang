// slang-win-crash-handler.cpp
#include "core/slang-crash-handler.h"

namespace Slang
{

// The Windows handler, an unhandled-exception filter, is not written yet. Until it is, a crash
// on Windows ends the process as it did before, and an internal error is still reported.

/* static */ bool CrashHandler::isSupported()
{
    return false;
}

/* static */ bool CrashHandler::install(const char* frontEnd, const char* slangVersion)
{
    SLANG_UNUSED(frontEnd);
    SLANG_UNUSED(slangVersion);
    return false;
}

/* static */ void CrashHandler::prepareThread() {}

} // namespace Slang
