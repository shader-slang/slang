// unit-test-crash-handler.cpp

#include "core/slang-crash-handler.h"
#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-process.h"
#include "unit-test/slang-unit-test.h"

#include <signal.h>

using namespace Slang;

// A crash ends the process, so each test runs the `test-process` tool, which installs the
// handler as slangc does and then crashes on purpose. The tests read what the handler wrote to
// stderr and to the report file, and how the process ended.
//
// The Windows handler is not written yet, so these tests run where the handler exists.

/// What a crashed `test-process` left behind.
struct CrashOutcome
{
    String standardError;
    int terminationSignal = 0;
    String savedReport;
    String savedReportPath;
};

/// Run `test-process crash <kind> <directory>` in a new, empty directory, wait for it to end,
/// and collect its stderr, the signal that ended it and the report it saved.
static SlangResult _runCrash(UnitTestContext* context, const char* kind, CrashOutcome& outOutcome)
{
    String directory;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slang-crash-handler-"), directory));
    File::remove(directory);
    if (!Path::createDirectory(directory))
        return SLANG_FAIL;

    CommandLine commandLine;
    commandLine.setExecutableLocation(
        ExecutableLocation(context->executableDirectory, "test-process"));
    commandLine.addArg("crash");
    commandLine.addArg(kind);
    commandLine.addArg(directory);

    RefPtr<Process> process;
    SLANG_RETURN_ON_FAIL(Process::create(commandLine, 0, process));
    List<Byte> standardError;
    SLANG_RETURN_ON_FAIL(ProcessUtil::readUntilTermination(process, nullptr, &standardError));

    outOutcome.standardError = String(
        UnownedStringSlice((const char*)standardError.getBuffer(), standardError.getCount()));
    outOutcome.terminationSignal = process->getTerminationSignal();

    // The report names the file that it saved, on its `Saved as:` line.
    static const char kSavedAs[] = "\nSaved as:    ";
    const Index savedAs = outOutcome.standardError.indexOf(toSlice(kSavedAs));
    if (savedAs >= 0)
    {
        const UnownedStringSlice rest =
            outOutcome.standardError.getUnownedSlice().tail(savedAs + Index(sizeof(kSavedAs) - 1));
        const Index end = rest.indexOf('\n');
        outOutcome.savedReportPath = end >= 0 ? String(rest.head(end)) : String(rest);
        File::readAllText(outOutcome.savedReportPath, outOutcome.savedReport);
        File::remove(outOutcome.savedReportPath);
    }
    Path::remove(directory);
    return SLANG_OK;
}

/// Check the parts that every crash report has, for a crash by `signalName`. If
/// `faultsInTool`, the faulting instruction is in the tool's own code; an abort faults inside
/// the C library instead, and the tool's code is further down the stack.
static void _checkCrashReport(
    const CrashOutcome& outcome,
    const char* signalName,
    bool faultsInTool)
{
    const String& text = outcome.standardError;
    const Index reportStart = text.indexOf(toSlice("==== Slang crash report (format 1) ===="));
    SLANG_CHECK_ABORT(reportStart >= 0);

    const UnownedStringSlice report = text.getUnownedSlice().tail(reportStart);
    SLANG_CHECK(report.indexOf(toSlice("\nkind:        crash\n")) >= 0);
    StringBuilder signalLine;
    signalLine << "\nsignal:      " << signalName << "\n";
    SLANG_CHECK(report.indexOf(signalLine.getUnownedSlice()) >= 0);
    SLANG_CHECK(report.indexOf(toSlice("\nslang:       test-version\n")) >= 0);
    SLANG_CHECK(report.indexOf(toSlice("\nfront-end:   test-process\n")) >= 0);
    SLANG_CHECK(report.indexOf(toSlice("\nsignature:   crash-")) >= 0);

    // The stack starts at the faulting code, not in the handler or in the system code that
    // called the handler, and it reaches the tool's code.
    const Index stackStart = report.indexOf(toSlice("\n[stack]\n"));
    const Index detailsStart = report.indexOf(toSlice("\n[details]\n"));
    SLANG_CHECK_ABORT(stackStart >= 0 && detailsStart > stackStart);
    const UnownedStringSlice stack = report.subString(stackStart, detailsStart - stackStart);
    if (faultsInTool)
        SLANG_CHECK(stack.startsWith(toSlice("\n[stack]\n#0  test-process+")));
    SLANG_CHECK(stack.indexOf(toSlice("  test-process+")) >= 0);

    // The report was saved, under its signature, and the file holds the same report.
    SLANG_CHECK(outcome.savedReportPath.indexOf(toSlice("slang-report-crash-")) >= 0);
    SLANG_CHECK(outcome.savedReport.getLength() > 0);
    SLANG_CHECK(report.startsWith(outcome.savedReport.getUnownedSlice()));
}

SLANG_UNIT_TEST(crashHandlerReportsSegmentationFault, NoRenderApi)
{
    if (!CrashHandler::isSupported())
    {
        SLANG_IGNORE_TEST;
    }

    CrashOutcome outcome;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runCrash(unitTestContext, "segv", outcome)));
    _checkCrashReport(outcome, "SIGSEGV", true);

    // A write through a null pointer faults at address 0.
    SLANG_CHECK(outcome.standardError.indexOf(toSlice("\nfault-address: 0x0\n")) >= 0);

    // After the report, the action that was there before runs, so the process ends the way
    // it would have without the handler.
    SLANG_CHECK(outcome.terminationSignal == SIGSEGV);
}

SLANG_UNIT_TEST(crashHandlerReportsAbort, NoRenderApi)
{
    if (!CrashHandler::isSupported())
    {
        SLANG_IGNORE_TEST;
    }

    CrashOutcome outcome;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runCrash(unitTestContext, "abort", outcome)));
    _checkCrashReport(outcome, "SIGABRT", false);
    SLANG_CHECK(outcome.terminationSignal == SIGABRT);
}

SLANG_UNIT_TEST(crashHandlerReportsStackOverflow, NoRenderApi)
{
    if (!CrashHandler::isSupported())
    {
        SLANG_IGNORE_TEST;
    }

    // A stack overflow leaves no stack for the handler. The handler runs on the alternate
    // stack that `install` gave the main thread.
    CrashOutcome outcome;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runCrash(unitTestContext, "stack-overflow", outcome)));
    _checkCrashReport(outcome, "SIGSEGV", true);
    SLANG_CHECK(outcome.terminationSignal == SIGSEGV);
}
