#include "slang-signal.h"

#include "slang-exception.h"
#include "slang-platform.h"
#include "stdio.h"
#include <ctime>

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
    // Check for SLANG_ASSERT_OUTPUT environment variable
    StringBuilder outputTarget;
    if (SLANG_SUCCEEDED(
            PlatformUtil::getEnvironmentVariable(UnownedStringSlice("SLANG_ASSERT_OUTPUT"), outputTarget)))
    {
        // Construct the assertion message with timestamp
        time_t now = time(nullptr);
        char timeBuffer[64];
#if _WIN32 && defined(_MSC_VER)
        struct tm timeInfo;
        localtime_s(&timeInfo, &now);
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
#else
        struct tm* timeInfo = localtime(&now);
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);
#endif

        // TODO: We should append filename and line number of where the assertion is.
        StringBuilder fullMessage;
        fullMessage << "SLANG_ASSERT[" << timeBuffer << "]\n";
        fullMessage << message << "\n";

        String assertMessage = fullMessage.produceString();
        char const* msg = assertMessage.getBuffer();

        UnownedStringSlice outputSlice = outputTarget.getUnownedSlice();

        if (outputSlice.caseInsensitiveEquals(UnownedStringSlice::fromLiteral("stderr")))
        {
            // Write to stderr
            fprintf(stderr, "%s", msg);
            fflush(stderr);
        }
        else if (outputSlice.caseInsensitiveEquals(UnownedStringSlice::fromLiteral("stdout")))
        {
            // Write to stdout
            fprintf(stdout, "%s", msg);
            fflush(stdout);
        }
        else
        {
            // Treat as a file path
            FILE* fp = nullptr;
#if _WIN32 && defined(_MSC_VER)
            fopen_s(&fp, outputSlice.begin(), "a");
#else
            fp = fopen(outputSlice.begin(), "a");
#endif
            if (fp)
            {
                fprintf(fp, "%s", msg);
                fflush(fp);
                fclose(fp);
            }
            else
            {
                // If file can't be opened, fall back to stderr
                fprintf(stderr, "Failed to open SLANG_ASSERT_OUTPUT file: %s\n", outputSlice.begin());
                fprintf(stderr, "%s", msg);
                fflush(stderr);
            }
        }
    }

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

    String msg = _getMessage(type, message);
    g_lastSignalMessage = msg;

    // Can be useful to enable during debug when problem is on CI
    if (false)
    {
        printf("%s\n", msg.getBuffer());
    }

#if SLANG_HAS_EXCEPTIONS
    switch (type)
    {
    case SignalType::InvalidOperation:
        throw InvalidOperationException(msg);
    case SignalType::AbortCompilation:
        throw AbortCompilationException(msg);
    default:
        throw InternalError(msg);
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
