// unit-test-test-server-loss.cpp
// Regression guards for slang-test's handling of a test server that dies mid-run.
//
// The behaviours under test are all about VERDICTS rather than compilation, and each one
// fails silently if it regresses:
//
//  - a server that dies for its own reasons must not be reported as a failing test;
//  - a server that dies is still counted and named, so the rate cannot climb unnoticed;
//  - an input that kills a freshly spawned server must fail the run rather than be retried
//    until it looks green;
//  - a run that stops scheduling after too many consecutive failures must not exit 0.
//
// That last one is the reason these exist. It was broken before, and a run that verified
// nothing reported success -- the class of bug that is invisible to review once merged,
// because nothing goes red to point at it.
//
// Each case drives a real slang-test child with the server rigged to die on a chosen
// request, via test-server's SLANG_TEST_SERVER_DIE_ON_REQUEST hook. Rigging it is what makes
// these deterministic; the alternative is waiting for a server to die on its own, which is
// exactly the thing that cannot be scheduled.

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "scoped-env-var.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using namespace SlangUnitTest;

/// The inner run's test selection.
///
/// FILE tests, not unit tests, and that choice is load-bearing. Driving these with unit
/// tests was tried first and every assertion here came out vacuous: #11911 means a failed
/// unit test is deferred for a retry that runs nothing ("Retrying unit tests... / no tests
/// run"), so the run reports 100% and exits 0 whatever happened. Both the pass assertions
/// and the fail assertion would hold with the retry logic deleted entirely.
///
/// tests/preprocessor is ~80 small compile-only tests with no device or downstream compiler
/// requirement, which is enough of them to trip the consecutive-failure threshold in the
/// killer case, and it cannot match the tests in THIS file -- slang-test spawning slang-test
/// spawning slang-test.
static const char* kInnerTestPrefix = "tests/preprocessor/";

/// Resolved relative to the working directory, exactly as slang-test resolves its own
/// default test directory. Any invocation that can run file tests at all has a working
/// directory where this exists; the tests ignore themselves rather than fail when it does
/// not, since that is an environment they cannot conclude anything from.
static const char* kInnerTestDir = "tests/preprocessor";

/// Set while a child spawned by these tests is running. A nested invocation would be a
/// recursion, so the tests skip rather than fork-bomb if the name filter above ever stops
/// excluding them.
static const char* kNestedGuardEnvVar = "SLANG_TEST_SERVER_LOSS_SELFTEST";

static bool _contains(const String& text, const char* expected)
{
    return text.getUnownedSlice().indexOf(UnownedStringSlice(expected)) >= 0;
}

/// True when this invocation cannot conclude anything: either it is a child spawned by one
/// of these tests, or the working directory has no file tests to drive the child with.
static bool _cannotRunHere()
{
    StringBuilder value;
    if (SLANG_SUCCEEDED(
            PlatformUtil::getEnvironmentVariable(UnownedStringSlice(kNestedGuardEnvVar), value)))
    {
        return true;
    }
    return !File::exists(kInnerTestDir);
}

/// Run slang-test in a child process with its test server rigged to die.
///
/// `dieOnRequest` of 0 leaves the server healthy, which is how the no-op case is checked
/// against the same code path rather than against a different one.
static SlangResult _runSlangTestWithDyingServer(
    UnitTestContext* context,
    int dieOnRequest,
    ExecuteResult& outResult)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, "slang-test"));
    cmdLine.addArg("-use-test-server");
    cmdLine.addArg("-server-count");
    cmdLine.addArg("1");
    cmdLine.addArg(kInnerTestPrefix);

    // The child inherits this environment, and test-server reads the variable when it
    // starts -- so every server the child spawns, including the ones spawned to retry, is
    // rigged the same way. That is deliberate: a retry landing on a healthy server would
    // make the killer case untestable.
    ScopedEnvVar nestedGuard(kNestedGuardEnvVar, "1");
    if (dieOnRequest <= 0)
    {
        return ProcessUtil::execute(cmdLine, outResult);
    }

    ScopedEnvVar dieAfter("SLANG_TEST_SERVER_DIE_ON_REQUEST", String(dieOnRequest).getBuffer());
    return ProcessUtil::execute(cmdLine, outResult);
}

static String _allOutput(const ExecuteResult& res)
{
    StringBuilder builder;
    builder << res.standardOutput << res.standardError;
    return builder.produceString();
}

/// A healthy server: the control. Whatever the other cases prove, they prove nothing unless
/// the untouched path is unchanged -- no losses reported, every test passing, exit 0.
SLANG_UNIT_TEST(testServerLossHealthyRunIsUnaffected)
{
    if (_cannotRunHere())
    {
        SLANG_IGNORE_TEST
    }

    ExecuteResult res;
    SLANG_CHECK(SLANG_SUCCEEDED(_runSlangTestWithDyingServer(unitTestContext, 0, res)));

    const String output = _allOutput(res);
    SLANG_CHECK(res.resultCode == 0);
    SLANG_CHECK(_contains(output, "100% of tests passed"));
    // The loss block must be absent, not merely zero: a healthy run should look exactly as
    // it did before this feature existed.
    SLANG_CHECK(!_contains(output, "test server loss"));
}

/// A server that dies periodically, where every retry lands on a fresh one that survives.
///
/// The tests are demonstrably fine -- they pass in the control above -- so charging them
/// with the server's death is a false verdict. Before this handling existed, this exact
/// scenario reported 26 failures out of 80 on a suite where all 80 pass.
SLANG_UNIT_TEST(testServerLossInnocentTestIsNotBlamed)
{
    if (_cannotRunHere())
    {
        SLANG_IGNORE_TEST
    }

    // Dies on its 3rd request, so the first two are served and the retry -- which gets a
    // server that has served nothing -- succeeds.
    ExecuteResult res;
    SLANG_CHECK(SLANG_SUCCEEDED(_runSlangTestWithDyingServer(unitTestContext, 3, res)));

    const String output = _allOutput(res);
    SLANG_CHECK(res.resultCode == 0);
    SLANG_CHECK(_contains(output, "100% of tests passed"));

    // Recovered, but NOT hidden. Absorbing losses into the pass is how a rate climbs from
    // 14 a night to 140 without anyone noticing, so the count has to survive the recovery.
    SLANG_CHECK(_contains(output, "test server loss"));
}

/// A server that dies on every request, so no retry can ever succeed.
///
/// This stands in for an input that genuinely kills the compiler. It must fail the run: the
/// whole risk of retrying is that a real crash gets retried until it looks green, and this
/// is the assertion that says it does not.
SLANG_UNIT_TEST(testServerLossPersistentKillerFailsTheRun)
{
    if (_cannotRunHere())
    {
        SLANG_IGNORE_TEST
    }

    ExecuteResult res;
    SLANG_CHECK(SLANG_SUCCEEDED(_runSlangTestWithDyingServer(unitTestContext, 1, res)));

    // Non-zero is the load-bearing assertion, and it covers the abort path too: with every
    // server dying, the consecutive-failure threshold trips, retries are skipped, and the
    // deferred tests still have to reach the totals. When that was broken, this run printed
    // "0% of tests passed (0/0)" and exited 0 -- a green run for a suite that verified
    // nothing.
    SLANG_CHECK(res.resultCode != 0);

    const String output = _allOutput(res);
    SLANG_CHECK(!_contains(output, "100% of tests passed"));
}
