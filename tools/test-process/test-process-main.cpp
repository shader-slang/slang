// test-process-main.cpp

#include "core/slang-crash-handler.h"
#include "core/slang-crash-report.h"
#include "core/slang-http.h"
#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-string-util.h"
#include "core/slang-string.h"
#include "core/slang-test-tool-util.h"
#include "slang-com-helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace TestProcess
{
using namespace Slang;

static void _makeStdStreamsUnbuffered()
{
    // Make these streams unbuffered
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
}

// The crash mode below faults on purpose, and the sanitizers must not take those faults for
// bugs. AddressSanitizer reads its default options from this function at start-up, before the
// ASAN_OPTIONS of the environment; with them, it leaves the fatal signals to the crash handler
// under test instead of handling them and reporting a finding.
#if defined(__SANITIZE_ADDRESS__)
#define SLANG_TEST_PROCESS_HAS_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define SLANG_TEST_PROCESS_HAS_ASAN 1
#endif
#endif
#if SLANG_TEST_PROCESS_HAS_ASAN
// The runtime looks this function up by name in the executable, so it must stay visible and
// survive the linker even though nothing in the program calls it.
extern "C" __attribute__((visibility("default"), used)) const char* __asan_default_options()
{
    return "handle_segv=0:handle_sigbus=0:handle_sigill=0:handle_sigfpe=0:handle_abort=0";
}
#endif

// UndefinedBehaviorSanitizer would report the deliberate store through a null pointer below.
#if defined(__clang__) || defined(__GNUC__)
#define SLANG_TEST_PROCESS_NO_UBSAN __attribute__((no_sanitize("undefined")))
#else
#define SLANG_TEST_PROCESS_NO_UBSAN
#endif

// Writes through this pointer reach the faulting store at run time, because the compiler
// cannot prove that the pointer stays null.
static int* volatile g_nullPointer = nullptr;

// The recursion below never stops while this is true, which is always. The compiler cannot
// see that, so it neither warns about endless recursion nor removes the calls.
static volatile bool g_keepRecursing = true;

/// Call this function again from inside each call, until the stack overflows.
static SLANG_NO_INLINE int _recurseForever(int depth)
{
    volatile char frame[256];
    frame[0] = char(depth);
    if (!g_keepRecursing)
        return frame[0];
    return _recurseForever(depth + 1) + frame[0];
}

/// Install the crash handler as slangc does, then crash in the way that `argv[2]` names:
/// `segv`, `abort` or `stack-overflow`. Reports are saved in the directory `argv[3]`, if given.
///
/// The crash-handler unit tests run this tool and read its stderr, its exit status and the
/// saved report. The handler must be tested in a process of its own, because the crash ends
/// the process.
static SLANG_TEST_PROCESS_NO_UBSAN SlangResult _crash(int argc, const char* const* argv)
{
    if (argc < 3)
        return SLANG_FAIL;
#if defined(_MSC_VER)
    // A debug build of the Microsoft C runtime shows a dialog box when `abort` is called and
    // waits for someone to close it, which would hang the test that aborts on purpose. We turn
    // the dialog off whatever `SLANG_IGNORE_ABORT_MSG` says, which is what
    // `TestToolUtil::suppressDebugDialogs` keys on.
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
    if (!CrashHandler::install("test-process", "test-version"))
        return SLANG_E_NOT_AVAILABLE;
    if (argc > 3)
        CrashReportUtil::setReportDirectory(argv[3]);

    const UnownedStringSlice kind(argv[2]);
    if (kind == "segv")
        *g_nullPointer = 1;
    else if (kind == "abort")
        abort();
    else if (kind == "stack-overflow")
        return SlangResult(_recurseForever(0));
    return SLANG_FAIL;
}

static SlangResult _outputCount(int argc, const char* const* argv)
{
    if (argc < 3)
    {
        return SLANG_FAIL;
    }
    // Get the count
    const Index count = stringToInt(argv[2]);

    // If we want to crash
    Index crashIndex = -1;
    if (argc > 3)
    {
        // When we crash, we want to make sure everything is flushed...
        _makeStdStreamsUnbuffered();
        crashIndex = stringToInt(argv[3]);
    }

    FILE* fileOut = stdout;

    StringBuilder buf;
    for (Index i = 0; i < count; ++i)
    {
        buf.clear();
        buf << i << "\n";

        fwrite(buf.getBuffer(), 1, buf.getLength(), fileOut);

        if (i == crashIndex)
        {
            // Use to simulate a crash.
            SLANG_BREAKPOINT(0);
            throw;
        }
    }

    // NOTE! There is no flush here, we want to see if everything works if we just stream out.
    return SLANG_OK;
}

static SlangResult _outputReflect()
{
    // Read lines from std input.
    // When hit line with just 'end', terminate

    // Get in as Stream

    RefPtr<Stream> stdinStream;
    Process::getStdStream(StdStreamType::In, stdinStream);

    FILE* fileOut = stdout;

    List<Byte> buffer;

    Index startIndex = 0;

    while (true)
    {
        SLANG_RETURN_ON_FAIL(StreamUtil::read(stdinStream, 0, buffer));

        while (true)
        {
            UnownedStringSlice slice(
                (const char*)buffer.begin() + startIndex,
                (const char*)buffer.end());
            UnownedStringSlice line;

            if (!StringUtil::extractLine(slice, line) || slice.begin() == nullptr)
            {
                break;
            }

            // Process the line
            if (line == UnownedStringSlice::fromLiteral("end"))
            {
                return SLANG_OK;
            }

            // Write the text to the output stream
            fwrite(line.begin(), 1, line.getLength(), fileOut);
            fputc('\n', fileOut);

            // Move the start index forward
            const Index newStartIndex = slice.begin()
                                            ? Index(slice.begin() - (const char*)buffer.getBuffer())
                                            : buffer.getCount();
            SLANG_ASSERT(newStartIndex > startIndex);
            startIndex = newStartIndex;
        }
    }
}

static SlangResult _httpReflect(int argc, const char* const* argv)
{
    SLANG_UNUSED(argc);
    SLANG_UNUSED(argv);

    RefPtr<Stream> stdinStream, stdoutStream;

    Process::getStdStream(StdStreamType::In, stdinStream);
    Process::getStdStream(StdStreamType::Out, stdoutStream);

    RefPtr<BufferedReadStream> readStream(new BufferedReadStream(stdinStream));

    RefPtr<HTTPPacketConnection> connection = new HTTPPacketConnection(readStream, stdoutStream);

    while (connection->isActive())
    {
        // Block waiting for content (or error/closed)
        SLANG_RETURN_ON_FAIL(connection->waitForResult());

        // If we have content do something with it
        if (connection->hasContent())
        {
            auto content = connection->getContent();

            // If it just holds 'end' then we are done
            const UnownedStringSlice slice((const char*)content.begin(), content.getCount());

            if (slice == UnownedStringSlice::fromLiteral("end"))
            {
                break;
            }

            // Else reflect it back
            SLANG_RETURN_ON_FAIL(connection->write(content.begin(), content.getCount()));

            // Consume that content/packet
            connection->consumeContent();
        }
    }

    return SLANG_OK;
}

static SlangResult _httpCrash()
{
    RefPtr<Stream> stdinStream, stdoutStream;

    Process::getStdStream(StdStreamType::In, stdinStream);
    Process::getStdStream(StdStreamType::Out, stdoutStream);

    RefPtr<BufferedReadStream> readStream(new BufferedReadStream(stdinStream));
    RefPtr<HTTPPacketConnection> connection = new HTTPPacketConnection(readStream, stdoutStream);

    // Read a single packet, then exit without replying to simulate a crash.
    SLANG_RETURN_ON_FAIL(connection->waitForResult());
    if (connection->hasContent())
    {
        connection->consumeContent();
    }

    return SLANG_FAIL;
}

static SlangResult _sleep(int argc, const char* const* argv)
{
    Int timeInMs = 30 * 1000;
    if (argc > 2)
    {
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(UnownedStringSlice(argv[2]), timeInMs));
        if (timeInMs < 0)
        {
            return SLANG_FAIL;
        }
    }

    Process::sleepCurrentThread(timeInMs);
    return SLANG_OK;
}

#if defined(_WIN32)
static SlangResult _waitEvent(int argc, const char* const* argv)
{
    if (argc < 3)
    {
        return SLANG_FAIL;
    }

    OSString eventName = String(argv[2]).toWString();
    HANDLE event = OpenEventW(SYNCHRONIZE, FALSE, eventName.begin());
    if (!event)
    {
        return SLANG_FAIL;
    }

    // Keep the helper bounded so a missed test signal cannot leave a child process around
    // indefinitely.
    DWORD waitResult = WaitForSingleObject(event, 60 * 1000);
    CloseHandle(event);
    return waitResult == WAIT_OBJECT_0 ? SLANG_OK : SLANG_FAIL;
}
#endif

static SlangResult execute(int argc, const char* const* argv)
{
    if (argc < 2)
    {
        return SLANG_FAIL;
    }

    // Get the tool name
    const String toolName = argv[1];
    if (toolName == "reflect")
    {
        return _outputReflect();
    }
    else if (toolName == "count")
    {
        return _outputCount(argc, argv);
    }
    else if (toolName == "http-reflect")
    {
        return _httpReflect(argc, argv);
    }
    else if (toolName == "http-crash")
    {
        return _httpCrash();
    }
    else if (toolName == "crash")
    {
        return _crash(argc, argv);
    }
    else if (toolName == "sleep")
    {
        return _sleep(argc, argv);
    }
#if defined(_WIN32)
    else if (toolName == "wait-event")
    {
        return _waitEvent(argc, argv);
    }
#endif
    return SLANG_E_NOT_AVAILABLE;
}

} // namespace TestProcess

int main(int argc, const char* const* argv)
{
    SlangResult res = TestProcess::execute(argc, argv);
    return (int)Slang::TestToolUtil::getReturnCode(res);
}
