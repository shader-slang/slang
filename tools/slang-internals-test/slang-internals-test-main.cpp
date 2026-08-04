// slang-internals-test-main.cpp
//
// Driver for the internals unit tests.
//
// The existing `slang-unit-test` tests are hosted in a shared library that
// `slang-test` loads at runtime, which restricts them to symbols exported from
// `libslang-compiler`. Internals tests instead live in this executable, which
// links the compiler statically so that ordinary internal symbols in
// `source/slang` resolve at link time without any export annotation.
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

    virtual SLANG_NO_THROW void SLANG_MCALL startTest(const char* testName) override
    {
        m_currentTestName = testName;
        m_currentTestFailed = false;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addResult(TestResult result) override
    {
        if (result == TestResult::Fail)
            recordFailure();
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    addResultWithLocation(TestResult result, const char* testText, const char* file, int line)
        override
    {
        addResultWithLocation(result == TestResult::Pass, testText, file, line);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line)
        override
    {
        if (testSucceeded)
            return;
        recordFailure();
        printf("    FAILED: %s\n      at %s:%d\n", testText, file, line);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL addExecutionTime(double) override {}

    virtual SLANG_NO_THROW void SLANG_MCALL message(TestMessageType, const char* message) override
    {
        printf("    %s\n", message);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL endTest() override
    {
        printf("  %-6s %s\n", m_currentTestFailed ? "FAIL" : "ok", m_currentTestName);
        if (m_currentTestFailed)
            failedTestCount++;
        else
            passedTestCount++;
    }

private:
    void recordFailure() { m_currentTestFailed = true; }

    const char* m_currentTestName = "<unknown>";
    bool m_currentTestFailed = false;
};

} // namespace

extern "C" IUnitTestModule* slangUnitTestGetModule();

int main(int argc, char** argv)
{
    // Optional substring filter, so a developer can run a single test while
    // iterating: `slang-internals-test irDeadCode`.
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
    for (SlangInt i = 0; i < testCount; i++)
    {
        const char* testName = testModule->getTestName(i);
        if (filter && !strstr(testName, filter))
            continue;

        reporter.startTest(testName);
        testModule->getTestFunc(i)(&context);
        reporter.endTest();
    }

    printf(
        "\n%d passed, %d failed\n",
        reporter.passedTestCount,
        reporter.failedTestCount);

    testModule->destroy();
    return reporter.failedTestCount == 0 ? 0 : 1;
}
