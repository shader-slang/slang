#include "slang-signal.h"

#include "slang-exception.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if _WIN32 && defined(_MSC_VER)
#include <cassert>
#include <windows.h>
#endif

namespace Slang
{

thread_local String g_lastSignalMessage;

static const char* _getSignalTypeAsText(SignalType type)
{
    switch (type)
    {
    case SignalType::AssertFailure:
        return "assert failure";
    case SignalType::Unimplemented:
        return "unimplemented";
    case SignalType::Unreachable:
        return "hit unreachable code";
    case SignalType::Unexpected:
        return "unexpected";
    case SignalType::InvalidOperation:
        return "invalid operation";
    case SignalType::AbortCompilation:
        return "abort compilation";
    default:
        return "unhandled";
    }
}

String _getMessage(SignalType type, char const* message)
{
    StringBuilder buf;
    const char* const typeText = _getSignalTypeAsText(type);
    buf << typeText;
    if (message)
    {
        buf << ": " << message;
    }

    return buf.produceString();
}

// Returns true if the NUL-terminated strings `a` and `b` are equal ignoring ASCII case.
static bool _caseInsensitiveEquals(const char* a, const char* b)
{
    for (; *a && *b; ++a, ++b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
        {
            return false;
        }
    }
    return *a == *b;
}

/// Reads the `SLANG_ASSERT` environment variable into `buffer`, returning true if it was set and
/// fit. An over-long value is reported as unset, which is harmless since no mode name is that long.
///
/// Uses only C library calls and stack storage: Slang's own containers assert internally, so
/// building a `StringBuilder` here would let an assert inside them re-enter `handleAssert`
/// and recurse until the stack overflowed.
static bool _readAssertEnvVar(char* buffer, size_t bufferSize)
{
#if _WIN32 && defined(_MSC_VER)
    size_t requiredSize = 0;
    return getenv_s(&requiredSize, buffer, bufferSize, "SLANG_ASSERT") == 0 && requiredSize > 0;
#else
    const char* value = getenv("SLANG_ASSERT");
    if (!value)
    {
        return false;
    }
    // Copy with the same length that was bounds-checked, so the check and the copy cannot drift
    // apart the way a `strlen` guard followed by an unbounded `strcpy` can.
    const size_t length = strlen(value);
    if (length >= bufferSize)
    {
        return false;
    }
    memcpy(buffer, value, length + 1);
    return true;
#endif
}

void handleAssert(char const* message, char const* file, int line, bool isReleaseAssert)
{
    // Sized generously past the longest recognized mode name, "release-asserts-only".
    char envValue[64];
    if (_readAssertEnvVar(envValue, sizeof(envValue)))
    {
        if (_caseInsensitiveEquals(envValue, "release-asserts-only") ||
            _caseInsensitiveEquals(envValue, "release-assert-only"))
        {
            // Ignore the assert and continue execution.
            // This is to mimic the behavior of Release build with Debug build.
            if (!isReleaseAssert)
            {
                return;
            }
        }
#if _WIN32 && defined(_MSC_VER)
        else if (_caseInsensitiveEquals(envValue, "system"))
        {
            assert(!"SLANG_ASSERT triggered");
        }
        else if (_caseInsensitiveEquals(envValue, "debugbreak"))
        {
            if (IsDebuggerPresent())
            {
                SLANG_BREAKPOINT(0);
            }
            else
            {
                // Fallback when no debugger is attached
                assert(!"SLANG_ASSERT triggered (no debugger attached)");
            }
        }
#endif
    }

    // Strip any remaining directory prefix for readability (the build system already
    // maps the source root away via -fmacro-prefix-map / /d1trimfile).
    const char* basename = file ? file : "unknown";
    if (file)
    {
        for (const char* p = file; *p; ++p)
        {
            if (*p == '/' || *p == '\\')
                basename = p + 1;
        }
    }
    // Use a stack buffer to avoid heap allocation on the assertion path, which could
    // mask the original failure if the heap is corrupted.
    char locMsg[1024];
    const char* safeMessage = message ? message : "unknown assert";
    snprintf(locMsg, sizeof(locMsg), "%s(%d): %s", basename, line, safeMessage);
    handleSignal(SignalType::AssertFailure, locMsg);
}

// One point of having as a single function is a choke point both for handling (allowing different
// handling scenarios) as well as a choke point to set a breakpoint to catch 'signal' types
[[noreturn]] void handleSignal(SignalType type, char const* message)
{
    g_lastSignalMessage = _getMessage(type, message);

    // Can be useful to enable during debug when problem is on CI
    static bool enableSignalPrint = false;
    if (enableSignalPrint)
    {
        printf("%s\n", g_lastSignalMessage.getBuffer());
    }

#if SLANG_HAS_EXCEPTIONS
    switch (type)
    {
    case SignalType::InvalidOperation:
        throw InvalidOperationException(_getMessage(type, message));
    case SignalType::AbortCompilation:
        throw AbortCompilationException(_getMessage(type, message));
    default:
        throw InternalError(_getMessage(type, message));
    }
#else
    // Attempt to drop out into the debugger. If a debugger isn't attached this will likely crash -
    // which is probably the best we can do.

    SLANG_BREAKPOINT(0);

    // 'panic'. Exit with an error code as we can't throw or catch.
    exit(-1);
#endif
}

const char* getLastSignalMessage()
{
    return g_lastSignalMessage.getBuffer();
}

} // namespace Slang
