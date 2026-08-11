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

// A final `Ignored` result under `-hide-ignored` never reaches the statistics or `m_testInfos`, but
// it must still redeem a deferral -- which is why `_addResult` records `m_finalResultTests` before
// the hide-ignored early return. Without that ordering, an ignored-on-retry test would be counted a
// spurious failure by reconciliation.
SLANG_UNIT_TEST(slangTestReporterReconcileRedeemedByHiddenIgnored)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    reporter.m_hideIgnored = true;
    const String command("slang-unit-test-tool/probeHiddenIgnored.internal");

    reporter.addTest(command, TestResult::PendingRetry); // first pass: deferred
    reporter.addTest(command, TestResult::Ignored);      // retry pass: ignored, hidden from output

    // The ignored result is hidden (not counted), but it still redeems the deferral.
    SLANG_CHECK(reporter.m_ignoredTestCount == 0);
    SLANG_CHECK(reporter.m_finalResultTests.contains(command));

    reporter.reconcilePendingRetries();

    SLANG_CHECK(reporter.m_failedTestCount == 0);
    SLANG_CHECK(reporter.didAllSucceed());
}

// The retry-eligibility gate in `runUnitTestModule` keys off the reporter's test key -- the full
// command -- because that is what the expected-failure files are written in and what
// `adjustResult()` looks up. Keying off the bare test name instead never matches, so every
// known-failing unit test gets pushed through a retry that can only reach the same result. This
// pins the key so that regression cannot come back silently.
SLANG_UNIT_TEST(slangTestReporterShouldDeferForRetryKeysOnCommand)
{
    TestReporter reporter;
    // Keeps the reporter's writes off test-server's JSON-RPC channel.
    reporter.m_suppressConsoleOutput = true;
    const String command("slang-unit-test-tool/probeExpectedFail.internal");
    const String bareName("probeExpectedFail");
    reporter.m_expectedFailureList.add(command);

    // An expected failure is recognised by its command and must not be deferred.
    SLANG_CHECK(!reporter.shouldDeferForRetry(command));

    // The bare name is not the key: looking up by it would answer "defer" for a test that is
    // already known to fail, which is the bug.
    SLANG_CHECK(reporter.shouldDeferForRetry(bareName));

    // Anything not on the list is still eligible.
    SLANG_CHECK(reporter.shouldDeferForRetry(String("slang-unit-test-tool/other.internal")));
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
