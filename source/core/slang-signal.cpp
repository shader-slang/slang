#include "slang-signal.h"

#include "slang-exception.h"
#include "slang-platform.h"
#include "stdio.h"

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

// Special handler for assertions that can optionally return based on environment variable
void handleAssert(char const* message)
{
#if _WIN32 && defined(_MSC_VER)
    StringBuilder envValue;
    if (SLANG_SUCCEEDED(
            PlatformUtil::getEnvironmentVariable(UnownedStringSlice("SLANG_ASSERT"), envValue)))
    {
        UnownedStringSlice envSlice = envValue.getUnownedSlice();
        if (envSlice.caseInsensitiveEquals(
                UnownedStringSlice::fromLiteral("release-asserts-only")) ||
            envSlice.caseInsensitiveEquals(UnownedStringSlice::fromLiteral("release-assert-only")))
        {
            // Ignore the assert and continue execution.
            // This is to mimic the behavior of Release build with Debug build.
            return;
        }
        else if (envSlice.caseInsensitiveEquals(UnownedStringSlice::fromLiteral("system")))
        {
            assert(!"SLANG_ASSERT triggered");
        }
        else if (envSlice.caseInsensitiveEquals(UnownedStringSlice::fromLiteral("debugbreak")))
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
    }
#endif

    // Default behavior: delegate to handleSignal
    handleSignal(SignalType::AssertFailure, message);
}

// One point of having as a single function is a choke point both for handling (allowing different
// handling scenarios) as well as a choke point to set a breakpoint to catch 'signal' types
[[noreturn]] void handleSignal(SignalType type, char const* message)
{
    StringBuilder buf;
    const char* const typeText = _getSignalTypeAsText(type);
    buf << typeText << ": " << message;

    g_lastSignalMessage = _getMessage(type, message);

    // Can be useful to enable during debug when problem is on CI
    if (false)
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
