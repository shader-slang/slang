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

#include "../slang-test/test-reporter.h"
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

/// Set while a child spawned by these tests is running.
///
/// ScopedEnvVar mutates THIS process's environment, not the child's, so it is only safe
/// because unit tests never run concurrently within one process: slang-test parallelises by
/// dispatching to one test-server process per thread (m_jsonRpcConnections is indexed by
/// thread), and each server serves its requests from a single loop. Verified under
/// -server-count 4, where all cases here still pass.
///
/// If unit tests are ever run concurrently inside one process, this breaks in two ways at
/// once: a sibling would inherit the death variables and lose servers for no reason, and a
/// sibling reading the sentinel below would ignore itself silently -- a vacuous pass. Prefer
/// a per-child environment block if that day comes. A nested invocation would be a
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
    ExecuteResult& outResult,
    bool killBySignal = false)
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

    ScopedEnvVar dieAfter(
        killBySignal ? "SLANG_TEST_SERVER_KILL_ON_REQUEST" : "SLANG_TEST_SERVER_DIE_ON_REQUEST",
        String(dieOnRequest).getBuffer());
    return ProcessUtil::execute(cmdLine, outResult);
}

/// Run slang-test in a child process with its test server rigged to write an unreadable
/// reply on the Nth request.
///
/// Separate from the dying-server helper because the scenario differs where it matters: the
/// server stays alive, so the client must conclude "unusable reply" from the bytes alone.
static SlangResult _runSlangTestWithGarblingServer(
    UnitTestContext* context,
    int garbleOnRequest,
    ExecuteResult& outResult)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, "slang-test"));
    cmdLine.addArg("-use-test-server");
    cmdLine.addArg("-server-count");
    cmdLine.addArg("1");
    cmdLine.addArg(kInnerTestPrefix);

    ScopedEnvVar nestedGuard(kNestedGuardEnvVar, "1");
    ScopedEnvVar garbleAfter(
        "SLANG_TEST_SERVER_GARBLE_ON_REQUEST",
        String(garbleOnRequest).getBuffer());
    return ProcessUtil::execute(cmdLine, outResult);
}

static String _allOutput(const ExecuteResult& res)
{
    StringBuilder builder;
    builder << res.standardOutput << res.standardError;
    return builder.produceString();
}

/// Print the child's exit code and output.
///
/// Called whenever a case is about to fail. Without it a failure says only which assertion
/// tripped, and the run that actually misbehaved -- a whole slang-test invocation, with its
/// per-loss diagnostics and its summary -- is discarded. That happened: the first CI failure
/// of these tests reported "res.resultCode == 0" and nothing else, on a platform that cannot
/// be reproduced locally, which is the worst possible combination.
static void _dumpChildRun(const char* caseName, int dieOnRequest, const ExecuteResult& res)
{
    const String output = _allOutput(res);
    printf(
        "\n--- %s: inner slang-test (SLANG_TEST_SERVER_DIE_ON_REQUEST=%d) exited %d ---\n"
        "%s\n--- end inner run ---\n",
        caseName,
        dieOnRequest,
        res.resultCode,
        output.getBuffer());
    fflush(stdout);
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
    if (res.resultCode != 0 || !_contains(output, "100% of tests passed"))
    {
        _dumpChildRun("healthy", 0, res);
    }
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
    if (res.resultCode != 0 || !_contains(output, "100% of tests passed"))
    {
        _dumpChildRun("innocent", 3, res);
    }
    SLANG_CHECK(res.resultCode == 0);
    SLANG_CHECK(_contains(output, "100% of tests passed"));

    // Recovered, but NOT hidden. Absorbing losses into the pass is how a rate climbs from
    // 14 a night to 140 without anyone noticing, so the count has to survive the recovery.
    //
    // Pinned past the block header, which on its own matches while every load-bearing part
    // of the diagnostic is broken. The full sentence catches a reworded summary; the ordinal
    // catches the accounting -- the server was rigged to die on its 3rd request having
    // answered 2, so those exact numbers are the whole off-by-one contract, and a reset that
    // stopped happening per connection would drift them run to run.
    SLANG_CHECK(_contains(output, "test server loss(es); the server died under each test below"));
    SLANG_CHECK(_contains(output, "on request #3 of this connection (it had answered 2)"));
}

/// A server that returns one unreadable reply, where the retry lands on a fresh one.
///
/// Counterpart of the innocent-loss case. A malformed reply used not to be retried at all, so
/// whichever test was in flight failed -- and one unreadable reply reds a whole suite.
SLANG_UNIT_TEST(testServerProtocolErrorInnocentTestIsNotBlamed)
{
    if (_cannotRunHere())
    {
        SLANG_IGNORE_TEST
    }

    // Garbles its 3rd reply, so the first two are served and the retry -- on a server that
    // has garbled nothing -- succeeds.
    ExecuteResult res;
    SLANG_CHECK(SLANG_SUCCEEDED(_runSlangTestWithGarblingServer(unitTestContext, 3, res)));

    const String output = _allOutput(res);
    if (res.resultCode != 0 || !_contains(output, "100% of tests passed"))
    {
        _dumpChildRun("garbled", 3, res);
    }
    SLANG_CHECK(res.resultCode == 0);
    SLANG_CHECK(_contains(output, "100% of tests passed"));

    // Recovered, and counted separately from a loss -- a combined total could not show
    // whether a change to the crash rate moved the malformed-reply rate.
    SLANG_CHECK(_contains(
        output,
        "test server protocol error(s); the server returned a reply the client could not "
        "parse"));
    // The loss block must NOT appear: nothing died here.
    SLANG_CHECK(!_contains(output, "the server died under each test below"));
}

/// A server that garbles every reply, so no retry can ever succeed.
///
/// The guard on the retry: a genuinely broken channel -- the version-mismatch case the old
/// no-retry rule assumed -- must still fail the run rather than be ground until it looks green.
SLANG_UNIT_TEST(testServerProtocolErrorPersistentGarbleFailsTheRun)
{
    if (_cannotRunHere())
    {
        SLANG_IGNORE_TEST
    }

    ExecuteResult res;
    SLANG_CHECK(SLANG_SUCCEEDED(_runSlangTestWithGarblingServer(unitTestContext, 1, res)));

    const String output = _allOutput(res);
    if (res.resultCode == 0)
    {
        _dumpChildRun("persistent-garble", 1, res);
    }
    SLANG_CHECK(res.resultCode != 0);
    SLANG_CHECK(_contains(output, "unreadable reply from a freshly spawned test server twice"));
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
    if (res.resultCode == 0)
    {
        _dumpChildRun("killer", 1, res);
    }
    SLANG_CHECK(res.resultCode != 0);

    // Non-zero alone is satisfied by either half of the abort fix on its own, so it does not
    // guard them independently: the `!aborted` term reds the run even with the deferred
    // tests dropped, and recording them reds it even without the term. Requiring the
    // deferred tests to have REACHED the totals separates the two -- with the recording
    // removed the summary degrades to "(0/0)" and names nothing, which this rejects.
    const String output = _allOutput(res);
    SLANG_CHECK(!_contains(output, "100% of tests passed"));
    SLANG_CHECK(!_contains(output, "(0/0)"));
    SLANG_CHECK(_contains(output, "failing tests:"));
}

/// The signal-death path, which is the one the whole diagnostic exists for.
///
/// Every other case here kills the server with _Exit, so they all take the ordinary
/// exit-status branch and none of them touches getTerminationSignal(), the WIFSIGNALED
/// recording behind it, or the SIGKILL gloss. That left the headline claim of this change --
/// telling an OOM kill apart from a crash apart from a clean stop -- as the one behaviour
/// with no test driving it.
///
/// Unix only. Windows reports every termination as an exit code, so there is no signal for
/// the client to report and nothing here to assert.
SLANG_UNIT_TEST(testServerLossReportsTheKillingSignal)
{
    if (_cannotRunHere())
    {
        SLANG_IGNORE_TEST
    }

#if SLANG_UNIX_FAMILY
    ExecuteResult res;
    SLANG_CHECK(SLANG_SUCCEEDED(_runSlangTestWithDyingServer(unitTestContext, 3, res, true)));

    const String output = _allOutput(res);
    if (!_contains(output, "killed by signal"))
    {
        _dumpChildRun("signal", 3, res);
    }

    // The number, and the name that makes it actionable. Before this path existed the same
    // death reported "server exited with status -1", because a Unix exit status is only
    // recorded for WIFEXITED -- so SIGKILL and SIGSEGV were indistinguishable from each other
    // and from a reader that never ran.
    SLANG_CHECK(_contains(output, "killed by signal"));
    SLANG_CHECK(_contains(output, "SIGKILL"));

    // And it must NOT fall back to the exit-status wording, which is what a regression in
    // getTerminationSignal() returning 0 would produce.
    SLANG_CHECK(!_contains(output, "server exited with status"));

    // The run itself still recovers: a signal death is a lost server like any other.
    SLANG_CHECK(res.resultCode == 0);
#else
    // Ignored, not silently passed. Without this the whole body compiles out on Windows and
    // the case reports success having run zero checks -- the vacuous pass this suite exists
    // to make impossible, in the test guarding its headline diagnostic.
    SLANG_IGNORE_TEST
#endif
}

/// consolidateWith merges the loss accounting across per-thread sub-reporters.
///
/// Nothing else here reaches it: every child run above uses -server-count 1, so all the
/// losses land in one reporter and the merge is never exercised. Production CI runs
/// -server-count 4. Deleting the two merge lines leaves every other case in this file green.
SLANG_UNIT_TEST(testServerLossConsolidatesAcrossReporters)
{
    TestReporter parent;
    TestReporter worker;

    parent.m_testServerLossCount = 1;
    parent.m_testServerLossTests.add("main/a.slang");

    worker.m_testServerLossCount = 2;
    worker.m_testServerLossTests.add("worker/b.slang");
    worker.m_testServerLossTests.add("worker/b.slang"); // same test, two servers lost

    parent.consolidateWith(&worker);

    SLANG_CHECK(parent.m_testServerLossCount == 3);
    // Concatenated, not unioned: a repeat is the frequency signal, so the duplicate survives.
    SLANG_CHECK(parent.m_testServerLossTests.getCount() == 3);
}
