#include "slang-test/options.h"
#include "slang-test/test-reporter.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// A sub-reporter (the per-worker reporter used by parallel runs) must receive the same
// Options-derived display configuration as the main reporter, so that verbosity, dump-on-failure,
// hide-ignored, output mode, and the expected-failure list all take effect on every worker.
SLANG_UNIT_TEST(slangTestReporterInitFromOptions)
{
    Options options;
    options.outputMode = TestOutputMode::TeamCity;
    options.verbosity = VerbosityLevel::Failure;
    options.dumpOutputOnFailure = true;
    options.hideIgnored = true;
    options.expectedFailureList.add(String("tests/some/expected-failure.slang"));

    // isSubReporter=true selects the worker/parallel configuration path.
    TestReporter reporter;
    SLANG_CHECK(SLANG_SUCCEEDED(reporter.init(options, /*isSubReporter*/ true)));

    SLANG_CHECK(reporter.m_outputMode == TestOutputMode::TeamCity);
    SLANG_CHECK(reporter.m_verbosity == VerbosityLevel::Failure);
    SLANG_CHECK(reporter.m_dumpOutputOnFailure == true);
    SLANG_CHECK(reporter.m_hideIgnored == true);
    SLANG_CHECK(reporter.m_isSubReporter == true);
    SLANG_CHECK(
        reporter.m_expectedFailureList.contains(String("tests/some/expected-failure.slang")));
}

// Retry reconciliation (issue #11911).
//
// A unit test that fails while slang-test runs in test-server mode is first reported as
// `TestResult::PendingRetry`, which defers its result out of the statistics on the promise that a
// later retry pass will report a real result for the same test. Nothing guarantees the retry
// happens -- it can be skipped wholesale by an early abort, or fail to re-discover the test -- so
// `reconcilePendingRetries()` must turn any deferral that no final result ever redeemed into a
// counted failure. Without it a run that contained real failures reported "100% passed". These
// tests pin that accounting directly on `TestReporter`, so the silent-false-green bug cannot come
// back. The reporter is used bare (no `init()`): its constructor already zeroes the counters and
// selects the plain-`printf` Default output mode.

// A `PendingRetry` that no retry ever redeems must become a counted failure after reconciliation.
// This is the exact bug: the deferred failure must not vanish.
SLANG_UNIT_TEST(slangTestReporterReconcileFailsUnredeemedRetry)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    const String command("slang-unit-test-tool/probeAlwaysFails.internal");

    // First pass defers the failure, which is what test-server mode does before a retry.
    reporter.addTest(command, TestResult::PendingRetry);

    // While deferred the test is invisible to the statistics -- the window the bug lived in.
    SLANG_CHECK(reporter.m_totalTestCount == 0);
    SLANG_CHECK(reporter.m_failedTestCount == 0);
    SLANG_CHECK(reporter.m_pendingRetryTests.contains(command));

    // No final result ever arrived, so reconciliation must count the deferral as a failure.
    reporter.reconcilePendingRetries();

    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(reporter.m_totalTestCount == 1);
    SLANG_CHECK(!reporter.didAllSucceed());
}

// A `PendingRetry` that a later final result redeems must NOT be counted as a failure: a flaky test
// that passes on retry is a pass. This guards against over-correcting the bug above into counting
// every retried test as failed.
SLANG_UNIT_TEST(slangTestReporterReconcileKeepsRedeemedRetry)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    const String command("slang-unit-test-tool/probeFlaky.internal");

    reporter.addTest(command, TestResult::PendingRetry); // first pass: deferred
    reporter.addTest(command, TestResult::Pass);         // retry pass: redeemed as a pass

    SLANG_CHECK(reporter.m_passedTestCount == 1);
    SLANG_CHECK(reporter.m_finalResultTests.contains(command));

    // The deferral is already redeemed, so reconciliation must leave the pass untouched.
    reporter.reconcilePendingRetries();

    SLANG_CHECK(reporter.m_failedTestCount == 0);
    SLANG_CHECK(reporter.m_passedTestCount == 1);
    SLANG_CHECK(reporter.didAllSucceed());
}

// Reconciliation routes the synthesized failure through the ordinary `addTest(..., Fail)` path, so
// a test on the expected-failure list is still downgraded to `ExpectedFail` and does not break the
// run. This pins the promise made in `reconcilePendingRetries()` that the expected-failure gate
// still applies to a reconciled deferral.
SLANG_UNIT_TEST(slangTestReporterReconcileHonorsExpectedFailure)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    const String command("slang-unit-test-tool/probeExpectedFail.internal");
    reporter.m_expectedFailureList.add(command);

    reporter.addTest(command, TestResult::PendingRetry);
    reporter.reconcilePendingRetries();

    SLANG_CHECK(reporter.m_expectedFailedTestCount == 1);
    SLANG_CHECK(reporter.m_failedTestCount == 0);
    SLANG_CHECK(reporter.didAllSucceed());
}

// The deferral and the final result that redeems it can be recorded by two different sub-reporters,
// because the retry pass runs on a fresh set of threads. After `consolidateWith()` unions both
// sets, the merged reporter must see the retry as redeemed rather than failed -- this is why the
// pending/final sets are tracked separately and reconciled only after consolidation.
SLANG_UNIT_TEST(slangTestReporterConsolidateThenReconcile)
{
    const String command("slang-unit-test-tool/probeCrossThread.internal");

    // Each reporter here reports a result, so each keeps its writes off
    // test-server's JSON-RPC channel.
    TestReporter firstPass;
    firstPass.m_suppressConsoleOutput = true;
    firstPass.addTest(command, TestResult::PendingRetry); // deferred on one worker

    TestReporter retryPass;
    retryPass.m_suppressConsoleOutput = true;
    retryPass.addTest(command, TestResult::Pass); // redeemed on another worker

    TestReporter main;
    main.m_suppressConsoleOutput = true;
    main.consolidateWith(&firstPass);
    main.consolidateWith(&retryPass);

    SLANG_CHECK(main.m_pendingRetryTests.contains(command));
    SLANG_CHECK(main.m_finalResultTests.contains(command));

    main.reconcilePendingRetries();

    SLANG_CHECK(main.m_failedTestCount == 0);
    SLANG_CHECK(main.m_passedTestCount == 1);
    SLANG_CHECK(main.didAllSucceed());
}

// A retry that runs and fails again reports a final `Fail`, which redeems the deferral. The failure
// must be counted exactly once: `reconcilePendingRetries()` must not also synthesize a second
// failure for the same test. This is the ordinary "flaky test stays broken" path.
SLANG_UNIT_TEST(slangTestReporterReconcileCountsRetriedFailureOnce)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    const String command("slang-unit-test-tool/probeRetriedStillFails.internal");

    reporter.addTest(command, TestResult::PendingRetry); // first pass: deferred
    reporter.addTest(command, TestResult::Fail);         // retry pass: ran and failed again

    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(reporter.m_finalResultTests.contains(command));

    // Already redeemed by the final Fail, so reconciliation must not count it a second time.
    reporter.reconcilePendingRetries();

    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(reporter.m_totalTestCount == 1);
    SLANG_CHECK(!reporter.didAllSucceed());
}

// The cross-sub-reporter counterpart of the unredeemed case: a deferral recorded by one worker that
// no worker ever redeems must, after `consolidateWith()`, still be reconciled into a failure.
SLANG_UNIT_TEST(slangTestReporterConsolidateUnredeemedReconcilesToFailure)
{
    const String command("slang-unit-test-tool/probeCrossThreadUnredeemed.internal");

    // Each reporter here reports a result, so each keeps its writes off
    // test-server's JSON-RPC channel.
    TestReporter firstPass;
    firstPass.m_suppressConsoleOutput = true;
    firstPass.addTest(command, TestResult::PendingRetry); // deferred on one worker, never redeemed

    TestReporter main;
    main.m_suppressConsoleOutput = true;
    main.consolidateWith(&firstPass);

    SLANG_CHECK(main.m_pendingRetryTests.contains(command));
    SLANG_CHECK(!main.m_finalResultTests.contains(command));

    main.reconcilePendingRetries();

    SLANG_CHECK(main.m_failedTestCount == 1);
    SLANG_CHECK(!main.didAllSucceed());
}

// A retry that skips the test does not redeem the deferral. The test was deferred because it ran
// and failed; an ignore produces no verdict and refutes nothing, so the first-pass failure has to
// survive to be counted.
//
// This is not hypothetical: gfx-unit-test-tool/computeTrivialD3D11 failed with an RPC failure (the
// test server died), was deferred, and the retry reported it ignored -- so it left the failure
// count and landed in the ignored tally, and the run reported only the two unrelated file-test
// failures. Under -hide-ignored it would not have been counted at all.
SLANG_UNIT_TEST(slangTestReporterIgnoredRetryDoesNotRedeemDeferral)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    const String command("gfx-unit-test-tool/probeIgnoredOnRetry.internal");

    reporter.addTest(command, TestResult::PendingRetry); // first pass: ran and failed
    reporter.addTest(command, TestResult::Ignored);      // retry: skipped, no verdict

    // The ignore is still reported as an ignore, but it does not redeem anything.
    SLANG_CHECK(reporter.m_ignoredTestCount == 1);
    SLANG_CHECK(!reporter.m_finalResultTests.contains(command));
    SLANG_CHECK(reporter.m_pendingRetryTests.contains(command));

    reporter.reconcilePendingRetries();

    // The first-pass failure survives instead of being laundered into a skip.
    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(!reporter.didAllSucceed());
}

// The same, with the ignore hidden from the output: -hide-ignored is what made the earlier
// behaviour leave no trace at all, since the redeeming ignore was never counted either.
SLANG_UNIT_TEST(slangTestReporterHiddenIgnoredRetryStillLeavesAFailure)
{
    TestReporter reporter;
    reporter.m_suppressConsoleOutput = true;
    reporter.m_hideIgnored = true;
    const String command("gfx-unit-test-tool/probeHiddenIgnoredOnRetry.internal");

    reporter.addTest(command, TestResult::PendingRetry);
    reporter.addTest(command, TestResult::Ignored);

    SLANG_CHECK(reporter.m_ignoredTestCount == 0); // hidden, so not counted
    SLANG_CHECK(!reporter.m_finalResultTests.contains(command));

    reporter.reconcilePendingRetries();

    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(!reporter.didAllSucceed());
}

// The retry-eligibility gate in `runUnitTestModule` keys off the reporter's test key -- the full
// command -- because that is what the expected-failure files are written in and what
// `adjustResult()` looks up. Keying off the bare test name instead never matches, so every
// known-failing unit test gets pushed through a retry that can only reach the same result. This
// pins the key so that regression cannot come back silently.
SLANG_UNIT_TEST(slangTestReporterExpectedFailureKeysOnCommand)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    const String command("slang-unit-test-tool/probeExpectedFail.internal");
    const String bareName("probeExpectedFail");
    reporter.m_expectedFailureList.add(command);

    // An expected failure is recognised by its command and must not be deferred.
    SLANG_CHECK(reporter.isExpectedFailure(command));

    // The bare name is not the key: looking up by it would answer "defer" for a test that is
    // already known to fail, which is the bug.
    SLANG_CHECK(!reporter.isExpectedFailure(bareName));

    // Anything not on the list is still eligible.
    SLANG_CHECK(!reporter.isExpectedFailure(String("slang-unit-test-tool/other.internal")));
}

// Reconciliation is terminal: it clears both halves of the pending/redeemed pair, so calling it a
// second time cannot synthesize the same failure twice. Without that, a reporter reused after
// reconciliation would carry stale redeemers.
SLANG_UNIT_TEST(slangTestReporterReconcileIsIdempotent)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    const String command("slang-unit-test-tool/probeUnredeemed.internal");

    reporter.addTest(command, TestResult::PendingRetry);
    reporter.reconcilePendingRetries();
    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(reporter.m_totalTestCount == 1);

    // Both sets are empty afterwards, so the reporter no longer claims a deferral or a redeemer.
    SLANG_CHECK(reporter.m_pendingRetryTests.getCount() == 0);
    SLANG_CHECK(reporter.m_finalResultTests.getCount() == 0);

    reporter.reconcilePendingRetries();
    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(reporter.m_totalTestCount == 1);
}

// The flag has to cover every write the reporter makes, not just the result lines: a test that
// opens a suite scope or asks for a summary is on the same JSON-RPC channel.
//
// Two tests because the two writes live in different output modes: TeamCity is what makes
// startSuite/endSuite emit their markers, and it is the Default summary that prints a pass count.
// Each uses a string the surrounding slang-test run cannot itself produce, so a leak is
// attributable to this reporter rather than to the harness reporting on the test.

SLANG_UNIT_TEST(slangTestReporterSuppressesSuiteMarkers)
{
    TestReporter reporter;
    reporter.m_suppressConsoleOutput = true;
    reporter.m_outputMode = TestOutputMode::TeamCity;

    {
        // "probeSuiteName" appears in no other output, so `##teamcity[...probeSuiteName...]`
        // on stdout would be this reporter's marker escaping.
        TestReporter::SuiteScope suite(&reporter, "probeSuiteName");
        SLANG_CHECK(reporter.m_suiteStack.getCount() == 1);
        reporter.addTest(String("slang-unit-test-tool/probeSuite.internal"), TestResult::Pass);
    }

    // Suppressing the markers must not stop the stack unwinding.
    SLANG_CHECK(reporter.m_suiteStack.getCount() == 0);
    SLANG_CHECK(reporter.m_passedTestCount == 1);
}

SLANG_UNIT_TEST(slangTestReporterSuppressesSummary)
{
    TestReporter reporter;
    reporter.m_suppressConsoleOutput = true;

    // Three passes, so the summary this reporter would print reads "(3/3)". The slang-test run
    // hosting this test reports its own counts and never produces that, which is what makes a
    // leak distinguishable from the harness's own summary.
    for (int i = 0; i < 3; ++i)
    {
        StringBuilder name;
        name << "slang-unit-test-tool/probeSummary" << i << ".internal";
        reporter.addTest(name.produceString(), TestResult::Pass);
    }
    SLANG_CHECK(reporter.m_passedTestCount == 3);

    reporter.outputSummary();

    SLANG_CHECK(reporter.m_totalTestCount == 3);
    SLANG_CHECK(reporter.didAllSucceed());
}

// A deferral whose first attempt never reached the test is answered by a retry that skips it. The
// two live instances of this were both `JSON RPC failure: sendCall()` on a test for an API the run
// had not enabled -- the connection died, the retry reached the test, and the test skipped itself
// because it was never applicable. Counting that as a test failure blames a dead connection on
// whichever test was next in the queue.
SLANG_UNIT_TEST(slangTestReporterIgnoredRedeemsDispatchFailure)
{
    TestReporter reporter;
    reporter.m_suppressConsoleOutput = true;
    const String command("gfx-unit-test-tool/probeDispatchFailure.internal");

    reporter.noteDispatchFailure(command);                // the call never reached the test
    reporter.addTest(command, TestResult::PendingRetry);  // deferred
    reporter.addTest(command, TestResult::Ignored);       // retry reached it; not applicable here

    SLANG_CHECK(reporter.m_finalResultTests.contains(command));

    reporter.reconcilePendingRetries();

    // No failure invented for a test that was never applicable...
    SLANG_CHECK(reporter.m_failedTestCount == 0);
    SLANG_CHECK(reporter.didAllSucceed());
    // ...but the connection dying is still on the record.
    SLANG_CHECK(reporter.m_dispatchFailureCount == 1);
}

// The same shape without a dispatch failure is still a failure: there the test ran and failed, so a
// retry that skips it refutes nothing.
SLANG_UNIT_TEST(slangTestReporterIgnoredDoesNotRedeemRealFailure)
{
    TestReporter reporter;
    reporter.m_suppressConsoleOutput = true;
    const String command("gfx-unit-test-tool/probeRanAndFailed.internal");

    reporter.addTest(command, TestResult::PendingRetry);
    reporter.addTest(command, TestResult::Ignored);

    SLANG_CHECK(!reporter.m_finalResultTests.contains(command));

    reporter.reconcilePendingRetries();

    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(!reporter.didAllSucceed());
}

// A dispatch failure that no retry ever resolves is still a failure -- the run must not go green
// having never obtained a result, which is the sibling bug #11751.
SLANG_UNIT_TEST(slangTestReporterUnresolvedDispatchFailureStillFails)
{
    TestReporter reporter;
    reporter.m_suppressConsoleOutput = true;
    const String command("gfx-unit-test-tool/probeDispatchNeverResolved.internal");

    reporter.noteDispatchFailure(command);
    reporter.addTest(command, TestResult::PendingRetry);

    reporter.reconcilePendingRetries();

    SLANG_CHECK(reporter.m_failedTestCount == 1);
    SLANG_CHECK(!reporter.didAllSucceed());
}
