// Regression coverage for explicit unit-test skips transported through test-server.

#include "core/slang-process-util.h"
#include "scoped-env-var.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using namespace SlangUnitTest;

static const char* kChildScenario = "SLANG_TEST_SERVER_IGNORE_SELFTEST";

// Run the same test in a child whose environment selects the otherwise unreachable scenario.
// Unit tests run sequentially within each process, so the scoped environment change cannot be
// inherited by another test in this process. The exact test key prevents recursive parent runs.
static void _checkIgnoredResult(
    UnitTestContext* context,
    const char* testName,
    bool failBeforeIgnoring,
    bool ignoreWithLocation)
{
    if (const char* scenario = getenv(kChildScenario))
    {
        SLANG_CHECK_ABORT(String(scenario) == testName);
        SLANG_CHECK_MSG(!failBeforeIgnoring, "failure before explicit skip");
        getTestReporter()->message(TestMessageType::Info, "explicit skip scenario reached");
        if (ignoreWithLocation)
        {
            getTestReporter()->addResultWithLocation(
                TestResult::Ignored,
                "dependency unavailable after successful setup",
                __FILE__,
                __LINE__);
            // Later successful assertions must also preserve the explicit skip.
            SLANG_CHECK(true);
            return;
        }
        SLANG_IGNORE_TEST
    }

    CommandLine command;
    command.setExecutableLocation(ExecutableLocation(context->executableDirectory, "slang-test"));
    command.addArg("-use-test-server");
    command.addArg("-server-count");
    command.addArg("1");
    command.addArg("-disable-retries");
    StringBuilder testKey;
    testKey << "slang-unit-test-tool/" << testName << ".internal";
    command.addArg(testKey.produceString());

    ExecuteResult result;
    SlangResult executionResult;
    {
        ScopedEnvVar scenario(kChildScenario, testName);
        executionResult = ProcessUtil::execute(command, result);
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executionResult));

    StringBuilder output;
    output << result.standardOutput << result.standardError;
    const auto outputSlice = output.getUnownedSlice();
    const char* expectedSummary = failBeforeIgnoring ? "0% of tests passed (0/1)"
                                                     : "0% of tests passed (0/0), 1 tests ignored";
    const bool expectedExit = failBeforeIgnoring ? result.resultCode != 0 : result.resultCode == 0;
    const bool expectedCounts = outputSlice.indexOf(UnownedStringSlice(expectedSummary)) >= 0;
    if (!expectedExit || !expectedCounts)
    {
        getTestReporter()->message(TestMessageType::Info, output.getBuffer());
    }
    SLANG_CHECK(expectedExit);
    SLANG_CHECK(expectedCounts);
    if (failBeforeIgnoring)
    {
        SLANG_CHECK(outputSlice.indexOf(UnownedStringSlice("failure before explicit skip")) >= 0);
        SLANG_CHECK(outputSlice.indexOf(UnownedStringSlice("1 tests ignored")) < 0);
    }
}

SLANG_UNIT_TEST(testServerIgnoreAfterSuccessfulAssertions)
{
    _checkIgnoredResult(unitTestContext, "testServerIgnoreAfterSuccessfulAssertions", false, false);
}

SLANG_UNIT_TEST(testServerIgnorePreservesEarlierFailure)
{
    _checkIgnoredResult(unitTestContext, "testServerIgnorePreservesEarlierFailure", true, false);
}

SLANG_UNIT_TEST(testServerIgnoreWithLocationPreservesSkip)
{
    _checkIgnoredResult(unitTestContext, "testServerIgnoreWithLocationPreservesSkip", false, true);
}
