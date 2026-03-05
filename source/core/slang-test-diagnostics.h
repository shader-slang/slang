// slang-test-diagnostics.h
// Diagnostic control for slang-test debugging
// Enabled via SLANG_TEST_DIAGNOSTICS environment variable

#ifndef SLANG_TEST_DIAGNOSTICS_H
#define SLANG_TEST_DIAGNOSTICS_H

#include "slang-platform.h"
#include "slang-string.h"

namespace Slang
{

// Check if a diagnostic category is enabled via SLANG_TEST_DIAGNOSTICS env var.
// Categories: timing, timing-phases, rpc, fd, pipe, all
// Thread-safe: reads the env var once and caches the result.
inline bool isDiagnosticEnabled(const char* category)
{
    // Cache the environment variable value. The static String is initialized once
    // in a thread-safe manner (C++11 guarantees thread-safe static initialization).
    static String diagnostics = []() -> String
    {
        StringBuilder envValue;
        if (SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(
                UnownedStringSlice("SLANG_TEST_DIAGNOSTICS"),
                envValue)))
        {
            return envValue.toString();
        }
        return String();
    }();

    if (diagnostics.getLength() == 0)
        return false;
    UnownedStringSlice diagSlice = diagnostics.getUnownedSlice();
    if (diagSlice.caseInsensitiveEquals(UnownedStringSlice("all")) ||
        diagSlice.caseInsensitiveEquals(UnownedStringSlice("1")))
        return true;
    // Case-insensitive substring search
    String lowerDiag = diagnostics.toLower();
    String lowerCat = String(category).toLower();
    return lowerDiag.indexOf(lowerCat) != Index(-1);
}

} // namespace Slang

#endif // SLANG_TEST_DIAGNOSTICS_H
