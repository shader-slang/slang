#include "slang-signal.h"

#include "slang-exception.h"
#include "slang-platform.h"
#include "stdio.h"
#include <chrono>
#include <fstream>
#include <iomanip> // for std::put_time
#include <sstream>

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
    // SLANG_ASSERT_OUTPUT environment variable allows redirecting assertion messages to:
    // - A specific file (e.g., SLANG_ASSERT_OUTPUT=D:/logs/assert.txt)
    // - stderr (SLANG_ASSERT_OUTPUT=stderr)
    // - stdout (SLANG_ASSERT_OUTPUT=stdout)
    //
    // This is primarily useful when slang.dll is used as an embedded library within another
    // application that blocks or redirects stdout/stderr. In such cases, assertion messages
    // from slang would be invisible to developers. This environment variable provides a way
    // to capture those assertions to a file or specific stream for debugging.
    //
    StringBuilder outputTarget;
    if (SLANG_SUCCEEDED(
            PlatformUtil::getEnvironmentVariable(UnownedStringSlice("SLANG_ASSERT_OUTPUT"), outputTarget)))
    {
        UnownedStringSlice outputSlice = outputTarget.getUnownedSlice();
        const char* output = outputSlice.begin();

        // Construct the assertion message with timestamp using chrono
        auto now = std::chrono::system_clock::now();
        auto nowTimeT = std::chrono::system_clock::to_time_t(now);

        std::ostringstream oss;

        oss << "[";
#if _WIN32 && defined(_MSC_VER)
        struct tm timeInfo;
        localtime_s(&timeInfo, &nowTimeT);
        oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
#else
        struct tm* timeInfo = localtime(&nowTimeT);
        oss << std::put_time(timeInfo, "%Y-%m-%d %H:%M:%S");
#endif
        oss << "] SLANG_ASSERT ";
        // TODO: We should append filename and line number of where the assertion is.
        oss << "\n";
        oss << message << "\n";

        const std::string& fullMessage = oss.str();

        if (strcmp(output, "stderr"))
        {
            fprintf(stderr, "%s", fullMessage.c_str());
            fflush(stderr);
        }
        else if (strcmp(output, "stdout"))
        {
            fprintf(stdout, "%s", fullMessage.c_str());
            fflush(stdout);
        }
        else
        {
            // Treat as a file path
            std::ofstream outFile(output, std::ios::app);
            if (outFile.is_open())
            {
                outFile << fullMessage;
            }
            else
            {
                // If file can't be opened, fall back to stderr
                fprintf(stderr, "Failed to open SLANG_ASSERT_OUTPUT file: %s\n", outputSlice.begin());
                fprintf(stderr, "%s", fullMessage.c_str());
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
