// slang-static-unit-test-main.cpp
//
// Driver for the statically linked unit tests.
//
// The existing `slang-unit-test` tests are hosted in a shared library that
// `slang-test` loads at runtime, which restricts them to symbols exported from
// `libslang-compiler`. These tests instead live in this executable, which links
// the compiler statically so that non-exported symbols in `source/slang`
// resolve at link time without any export annotation.
//
// The test-authoring surface is unchanged: tests use `SLANG_UNIT_TEST` and
// `SLANG_CHECK` exactly as they do in `slang-unit-test`. Registration goes
// through the same global list in `tools/unit-test`, so all this driver has to
// do is supply an `ITestReporter` and run what registered itself.

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstdio>
#include <cstring>

using namespace Slang;

namespace
{

/// Minimal reporter that prints one line per test and a summary. `slang-test`
/// supplies a richer implementation for the plugin tests; this executable only
/// needs enough to report pass/fail to a terminal and to CI.
class ConsoleTestReporter : public ITestReporter
{
public:
    int failedTestCount = 0;
    int passedTestCount = 0;
    int ignoredTestCount = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL startTest(const char* testName) override
    {
        m_currentTestName = testName;
        m_currentTestFailed = false;
        m_currentTestIgnored = false;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addResult(TestResult result) override
    {
        recordResult(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addResultWithLocation(
        TestResult result,
        const char* testText,
        const char* file,
        int line) override
    {
        if (result == TestResult::Fail)
            printf("    FAILED: %s\n      at %s:%d\n", testText, file, line);
        recordResult(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addResultWithLocation(
        bool testSucceeded,
        const char* testText,
        const char* file,
        int line) override
    {
        if (testSucceeded)
            return;
        recordResult(TestResult::Fail);
        printf("    FAILED: %s\n      at %s:%d\n", testText, file, line);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addExecutionTime(double) override {}

    virtual SLANG_NO_THROW void SLANG_MCALL message(TestMessageType, const char* message) override
    {
        printf("    %s\n", message);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL endTest() override
    {
        const char* label = m_currentTestFailed ? "FAIL" : (m_currentTestIgnored ? "skip" : "ok");
        printf("  %-6s %s\n", label, m_currentTestName);
        if (m_currentTestFailed)
            failedTestCount++;
        else if (m_currentTestIgnored)
            ignoredTestCount++;
        else
            passedTestCount++;
    }

private:
    /// Classify a result the same way regardless of which entry point reported
    /// it. Only `Fail` is a failure; `Ignored` is tracked separately so a test
    /// that skips itself is not silently tallied as a pass.
    void recordResult(TestResult result)
    {
        switch (result)
        {
        case TestResult::Fail:
            m_currentTestFailed = true;
            break;
        case TestResult::Ignored:
            m_currentTestIgnored = true;
            break;
        default:
            break;
        }
    }

    const char* m_currentTestName = "<unknown>";
    bool m_currentTestFailed = false;
    bool m_currentTestIgnored = false;
};

} // namespace

extern "C" IUnitTestModule* slangUnitTestGetModule();

int main(int argc, char** argv)
{
    // Optional substring filter, so a developer can run a single test while
    // iterating: `slang-static-unit-test irDeadCode`.
    const char* filter = (argc > 1) ? argv[1] : nullptr;

    // Creating the global session loads and parses the core module, which
    // dominates the runtime of the whole suite. Do it once and share it with
    // every test through `UnitTestContext`.
    ComPtr<slang::IGlobalSession> globalSession;
    if (SLANG_FAILED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
    {
        fprintf(stderr, "error: failed to create global session\n");
        return 1;
    }

    IUnitTestModule* testModule = slangUnitTestGetModule();
    ConsoleTestReporter reporter;
    testModule->setTestReporter(&reporter);

    UnitTestContext context = {};
    context.slangGlobalSession = globalSession.get();
    context.workDirectory = ".";
    context.executableDirectory = ".";

    const SlangInt testCount = testModule->getTestCount();

    // Tests register themselves through static initializers. If that ever
    // stopped happening — a refactor moving them into an intermediate static
    // library, or aggressive dead-section stripping — the suite would run
    // nothing and still report success.
    if (testCount == 0)
    {
        fprintf(stderr, "error: no tests were registered\n");
        testModule->destroy();
        return 1;
    }

    SlangInt selectedCount = 0;
    for (SlangInt i = 0; i < testCount; i++)
    {
        const char* testName = testModule->getTestName(i);
        if (filter && !strstr(testName, filter))
            continue;

        selectedCount++;
        reporter.startTest(testName);

        // These tests call compiler entry points directly, and a failed `SLANG_ASSERT` or
        // `SLANG_RELEASE_ASSERT` throws by default (see the assertion-behaviour table in
        // CLAUDE.md). The `SLANG_UNIT_TEST` wrapper catches only `AbortTestException`, so
        // anything else would escape this loop and tear the process down mid-suite:
        // `endTest()` would never run for this test, the remaining selected tests would
        // never run, and the pass/fail summary would be lost. Report it as one failed test
        // and carry on, so the run still says which test broke and how the rest fared.
        try
        {
            testModule->getTestFunc(i)(&context);
        }
        catch (...)
        {
            printf("    FAILED: uncaught exception escaped the test body\n");
            reporter.addResult(TestResult::Fail);
        }

        reporter.endTest();
    }

    // A filter that matches nothing would otherwise run zero tests and report
    // success, so a mistyped name would look like a passing run.
    if (filter && selectedCount == 0)
    {
        fprintf(
            stderr,
            "error: filter \"%s\" matched none of the %d test(s)\n",
            filter,
            (int)testCount);
        testModule->destroy();
        return 1;
    }

    // Ignored tests are reported explicitly. Counting them separately from passes is
    // only half the job: leaving them out of the summary makes a skipped test invisible
    // rather than merely not-a-pass, which is the same silent-success failure the
    // zero-tests and empty-filter checks exist to prevent.
    printf(
        "\n%d passed, %d failed, %d ignored\n",
        reporter.passedTestCount,
        reporter.failedTestCount,
        reporter.ignoredTestCount);

    testModule->destroy();
    return reporter.failedTestCount == 0 ? 0 : 1;
}
