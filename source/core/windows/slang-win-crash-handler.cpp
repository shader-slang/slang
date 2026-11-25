// slang-win-crash-handler.cpp
#include "core/slang-crash-handler.h"
#include "core/slang-crash-report.h"
#include "core/slang-stack-trace.h"

#include <atomic>
#include <signal.h>
#include <windows.h>

namespace Slang
{

// On Windows a crash is an unhandled structured exception, such as an access violation. The
// system passes it to the process's unhandled-exception filter, which is where we report it.
// An abort is not an exception: the C runtime raises `SIGABRT` and then ends the process with
// exit code 3, so we also handle that signal.

/// The Windows exceptions that a crash raises, with their names for the report.
struct FatalException
{
    DWORD code;
    const char* name;
};
static const FatalException kFatalExceptions[] = {
    {EXCEPTION_ACCESS_VIOLATION, "EXCEPTION_ACCESS_VIOLATION"},
    {EXCEPTION_STACK_OVERFLOW, "EXCEPTION_STACK_OVERFLOW"},
    {EXCEPTION_ILLEGAL_INSTRUCTION, "EXCEPTION_ILLEGAL_INSTRUCTION"},
    {EXCEPTION_PRIV_INSTRUCTION, "EXCEPTION_PRIV_INSTRUCTION"},
    {EXCEPTION_INT_DIVIDE_BY_ZERO, "EXCEPTION_INT_DIVIDE_BY_ZERO"},
    {EXCEPTION_INT_OVERFLOW, "EXCEPTION_INT_OVERFLOW"},
    {EXCEPTION_IN_PAGE_ERROR, "EXCEPTION_IN_PAGE_ERROR"},
    {EXCEPTION_DATATYPE_MISALIGNMENT, "EXCEPTION_DATATYPE_MISALIGNMENT"},
    {EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"},
    {EXCEPTION_BREAKPOINT, "EXCEPTION_BREAKPOINT"},
    {EXCEPTION_FLT_DIVIDE_BY_ZERO, "EXCEPTION_FLT_DIVIDE_BY_ZERO"},
    {EXCEPTION_FLT_INVALID_OPERATION, "EXCEPTION_FLT_INVALID_OPERATION"},
    {EXCEPTION_FLT_OVERFLOW, "EXCEPTION_FLT_OVERFLOW"},
    {EXCEPTION_FLT_UNDERFLOW, "EXCEPTION_FLT_UNDERFLOW"},
    {EXCEPTION_FLT_STACK_CHECK, "EXCEPTION_FLT_STACK_CHECK"},
    {EXCEPTION_NONCONTINUABLE_EXCEPTION, "EXCEPTION_NONCONTINUABLE_EXCEPTION"},
    // The code that the Microsoft C++ runtime raises for a C++ exception that nothing caught.
    {0xE06D7363, "uncaught C++ exception"},
};

// The filter that was installed before ours, which runs after our report.
static LPTOP_LEVEL_EXCEPTION_FILTER g_previousFilter = nullptr;

// The handler builds its report here and not on the stack, because after a stack overflow it
// runs on the small part of the stack that `SetThreadStackGuarantee` keeps back.
static CrashReportText g_report;

// The thread that is writing a report, or 0. A second thread that crashes meanwhile waits for
// the process to end, so two reports cannot interleave. The same thread can arrive twice: an
// uncaught C++ exception reaches the filter, the runtime's own filter after ours aborts, and
// the abort raises `SIGABRT`. The second arrival writes nothing.
static std::atomic<DWORD> g_reportingThread{0};

// The stack that the filter has after a stack overflow. The default guarantee is too small for
// the walk and the formatting.
static const ULONG kStackGuarantee = 64 * 1024;

/// Return the name of the exception `code`, writing a name of the form
/// `exception 0xc0000409` into `buffer` for a code that we do not list.
static const char* _getExceptionName(DWORD code, CrashReportText& buffer)
{
    for (const FatalException& exception : kFatalExceptions)
    {
        if (exception.code == code)
            return exception.name;
    }
    buffer.append("exception 0x");
    buffer.appendHex(code);
    return buffer.getBuffer();
}

/// Write the report text to stderr. `WriteFile` writes everything to a file or a pipe in one
/// call, and a console accepts the text in pieces, so we continue until all of it is written.
static void _writeToStandardError(const char* text, size_t length)
{
    const HANDLE handle = ::GetStdHandle(STD_ERROR_HANDLE);
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
        return;

    size_t writtenLength = 0;
    while (writtenLength < length)
    {
        DWORD count = 0;
        if (!::WriteFile(
                handle,
                text + writtenLength,
                DWORD(length - writtenLength),
                &count,
                nullptr) ||
            count == 0)
        {
            return;
        }
        writtenLength += count;
    }
}

/// Claim the right to write the one report of this process. Return false if this thread is
/// already writing it. A thread that finds another one writing waits for the process to end.
static bool _beginReport()
{
    const DWORD thisThread = ::GetCurrentThreadId();
    DWORD expected = 0;
    if (g_reportingThread.compare_exchange_strong(expected, thisThread))
        return true;
    if (expected == thisThread)
        return false;
    for (;;)
        ::Sleep(INFINITE);
}

/// Build and write the report of the crash `signalName`, whose stack is `stack`.
static void _report(
    const char* signalName,
    bool hasFaultAddress,
    uintptr_t faultAddress,
    const StackTrace& stack)
{
    g_report.append("\n");
    if (CrashReportUtil::appendCrashReportOfThisProcess(
            signalName,
            hasFaultAddress,
            faultAddress,
            stack,
            g_report))
    {
        _writeToStandardError(g_report.getBuffer(), size_t(g_report.getLength()));
    }
}

static LONG WINAPI _handleUnhandledException(EXCEPTION_POINTERS* pointers)
{
    if (_beginReport())
    {
        const EXCEPTION_RECORD* record = pointers->ExceptionRecord;

        StackTrace stack;
        StackTraceUtil::captureFromContext(pointers->ContextRecord, stack);

        // For an access violation or an in-page error, the second parameter is the address
        // that the instruction tried to read or write.
        const bool hasFaultAddress = (record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ||
                                      record->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) &&
                                     record->NumberParameters >= 2;
        const uintptr_t faultAddress =
            hasFaultAddress ? uintptr_t(record->ExceptionInformation[1]) : 0;

        static CrashReportText nameBuffer;
        _report(
            _getExceptionName(record->ExceptionCode, nameBuffer),
            hasFaultAddress,
            faultAddress,
            stack);
    }

    // After the report, the exception goes on as it would have without us: to the filter that
    // was there before, or to the system, which ends the process with the exception code as its
    // exit code and runs Windows Error Reporting.
    if (g_previousFilter)
        return g_previousFilter(pointers);
    return EXCEPTION_CONTINUE_SEARCH;
}

static void __cdecl _handleAbort(int signalNumber)
{
    if (_beginReport())
    {
        // The stack walk starts here, inside the C runtime's abort, so the frames of the code
        // that called `abort` come after a few frames of the runtime.
        StackTrace stack;
        StackTraceUtil::capture(0, stack);
        _report("SIGABRT", false, 0, stack);
    }

    // The runtime reset the handler to the default before calling us, so raising the signal
    // again ends the process as `abort` would have: with exit code 3.
    ::signal(signalNumber, SIG_DFL);
    ::raise(signalNumber);
}

/* static */ bool CrashHandler::isSupported()
{
    return true;
}

/* static */ void CrashHandler::prepareThread()
{
    ULONG guarantee = kStackGuarantee;
    ::SetThreadStackGuarantee(&guarantee);
}

/* static */ bool CrashHandler::install(const char* frontEnd, const char* slangVersion)
{
    CrashReportUtil::enableReports(frontEnd, slangVersion);
    prepareThread();

    g_previousFilter = ::SetUnhandledExceptionFilter(_handleUnhandledException);
    ::signal(SIGABRT, _handleAbort);
    return true;
}

} // namespace Slang
