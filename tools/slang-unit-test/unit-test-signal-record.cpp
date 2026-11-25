// unit-test-signal-record.cpp

#include "core/slang-exception.h"
#include "core/slang-signal.h"
#include "core/slang-stack-trace.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>
#include <thread>

using namespace Slang;

// The tests below raise internal errors and catch them, which a build without exceptions
// cannot do: there `handleSignal` ends the process.
#if SLANG_HAS_EXCEPTIONS

/// Return true if `path` names this source file, whatever directory the build recorded.
static bool _isThisFile(const char* path)
{
    static const char kName[] = "unit-test-signal-record.cpp";
    const size_t pathLength = strlen(path);
    const size_t nameLength = sizeof(kName) - 1;
    return pathLength >= nameLength && strcmp(path + pathLength - nameLength, kName) == 0;
}

SLANG_UNIT_TEST(signalRecordKeepsLocation, NoRenderApi)
{
    clearLastSignalRecord();
    SignalRecord record;
    SLANG_CHECK(!getLastSignalRecord(record));

    int raiseLine = 0;
    try
    {
        // The line number is read on the line that raises, so the two cannot drift apart.
        // clang-format off
        raiseLine = __LINE__; SLANG_UNEXPECTED("signal record test");
        // clang-format on
    }
    catch (const Exception&)
    {
    }

    SLANG_CHECK_ABORT(getLastSignalRecord(record));
    SLANG_CHECK(record.type == SignalType::Unexpected);
    SLANG_CHECK(_isThisFile(record.file));
    SLANG_CHECK(record.line == raiseLine);

    // The record holds the stack of the raise wherever a stack can be captured.
    SLANG_CHECK_ABORT(record.stackTrace != nullptr);
    if (StackTraceUtil::isSupported())
        SLANG_CHECK(record.stackTrace->frameCount > 0);

    clearLastSignalRecord();
    SLANG_CHECK(!getLastSignalRecord(record));
}

SLANG_UNIT_TEST(signalRecordKeepsKind, NoRenderApi)
{
    SignalRecord record;

    // An assertion reports the location of the assertion, not of the assert machinery.
    int raiseLine = 0;
    try
    {
        // clang-format off
        raiseLine = __LINE__; SLANG_ASSERT_FAILURE("signal record test");
        // clang-format on
    }
    catch (const Exception&)
    {
    }
    SLANG_CHECK_ABORT(getLastSignalRecord(record));
    SLANG_CHECK(record.type == SignalType::AssertFailure);
    SLANG_CHECK(_isThisFile(record.file));
    SLANG_CHECK(record.line == raiseLine);

    // A newer error replaces the record of an older one.
    try
    {
        SLANG_UNREACHABLE("signal record test");
    }
    catch (const Exception&)
    {
    }
    SLANG_CHECK_ABORT(getLastSignalRecord(record));
    SLANG_CHECK(record.type == SignalType::Unreachable);

    clearLastSignalRecord();
}

SLANG_UNIT_TEST(signalRecordIsPerThread, NoRenderApi)
{
    clearLastSignalRecord();

    // An internal error on another thread is visible on that thread and leaves this thread's
    // record empty, so a host that compiles on several threads never reads a foreign error.
    bool otherThreadHasRecord = false;
    SignalType otherThreadType = SignalType::Unexpected;
    std::thread other(
        [&]()
        {
            try
            {
                SLANG_UNIMPLEMENTED_X("signal record test");
            }
            catch (const Exception&)
            {
            }
            SignalRecord record;
            otherThreadHasRecord = getLastSignalRecord(record);
            if (otherThreadHasRecord)
                otherThreadType = record.type;
        });
    other.join();

    SLANG_CHECK(otherThreadHasRecord);
    SLANG_CHECK(otherThreadType == SignalType::Unimplemented);

    SignalRecord record;
    SLANG_CHECK(!getLastSignalRecord(record));
}

#endif // SLANG_HAS_EXCEPTIONS

SLANG_UNIT_TEST(signalTypeNames, NoRenderApi)
{
    // These names are part of the crash-report format.
    SLANG_CHECK(strcmp(getSignalTypeName(SignalType::Unexpected), "unexpected") == 0);
    SLANG_CHECK(strcmp(getSignalTypeName(SignalType::Unimplemented), "unimplemented") == 0);
    SLANG_CHECK(strcmp(getSignalTypeName(SignalType::AssertFailure), "assert") == 0);
    SLANG_CHECK(strcmp(getSignalTypeName(SignalType::Unreachable), "unreachable") == 0);
    SLANG_CHECK(strcmp(getSignalTypeName(SignalType::InvalidOperation), "invalid-operation") == 0);
    SLANG_CHECK(strcmp(getSignalTypeName(SignalType::AbortCompilation), "abort-compilation") == 0);
}
