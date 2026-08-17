// test-reporter.h

#ifndef TEST_REPORTER_H_INCLUDED
#define TEST_REPORTER_H_INCLUDED

#include "core/slang-dictionary.h"
#include "core/slang-platform.h"
#include "core/slang-std-writers.h"
#include "core/slang-string-util.h"
#include "unit-test/slang-unit-test.h"

#include <mutex>

// Forward declaration
enum class VerbosityLevel;
struct Options;

enum class TestOutputMode
{
    Default = 0, ///< Default mode is to write test results to the console
    AppVeyor,    ///< For AppVeyor continuous integration
    Travis,      ///< We currently don't specialize for Travis, but maybe we should.
    XUnit,    ///< xUnit original format  https://nose.readthedocs.io/en/latest/plugins/xunit.html
    XUnit2,   ///< https://xunit.github.io/docs/format-xml-v2
    TeamCity, ///< Output suitable for teamcity
};

/// Returns the reporter key for a unit test: `<moduleName>/<testName>.internal`.
///
/// This is the identity a unit test is reported, retried and looked up under -- the same string the
/// expected-failure files are written in and that `TestReporter::adjustResult()` matches against.
/// It lives here, rather than being spelled out where it happens to be needed, because keying off
/// the bare test name instead is a mistake that produces no visible symptom: the expected-failure
/// lookup silently never matches.
Slang::String makeUnitTestKey(
    const Slang::UnownedStringSlice& moduleName,
    const Slang::UnownedStringSlice& testName);

class TestReporter : public ITestReporter
{
public:
    struct TestInfo
    {
        TestResult testResult = TestResult::Uninitialized;
        Slang::String name;
        Slang::String message;      ///< Message that is specific for the testResult
        double executionTime = 0.0; ///< <= 0.0 if not defined. Time is in seconds.
    };

    class TestScope
    {
    public:
        TestScope(TestReporter* reporter, const Slang::String& testName)
            : m_reporter(reporter)
        {
            reporter->startTest(testName.getBuffer());
        }
        ~TestScope() { m_reporter->endTest(); }

    protected:
        TestReporter* m_reporter;
    };

    class SuiteScope
    {
    public:
        SuiteScope(TestReporter* reporter, const Slang::String& suiteName)
            : m_reporter(reporter)
        {
            reporter->startSuite(suiteName);
        }
        ~SuiteScope() { m_reporter->endSuite(); }

    protected:
        TestReporter* m_reporter;
    };

    void startSuite(const Slang::String& name);
    void endSuite();

    TestResult adjustResult(Slang::UnownedStringSlice testName, TestResult result);

    virtual SLANG_NO_THROW void SLANG_MCALL startTest(const char* testName) override;
    virtual SLANG_NO_THROW void SLANG_MCALL addResult(TestResult result) override;
    virtual SLANG_NO_THROW void SLANG_MCALL addResultWithLocation(
        TestResult result,
        const char* testText,
        const char* file,
        int line) override;
    virtual SLANG_NO_THROW void SLANG_MCALL addResultWithLocation(
        bool testSucceeded,
        const char* testText,
        const char* file,
        int line) override;
    virtual SLANG_NO_THROW void SLANG_MCALL addExecutionTime(double time) override;
    virtual SLANG_NO_THROW void SLANG_MCALL endTest() override;

    /// Runs start/endTest and outputs the result
    TestResult addTest(const Slang::String& testName, bool isPass);
    /// Effectively runs start/endTest (so cannot be called inside start/endTest).
    void addTest(const Slang::String& testName, TestResult testResult);

    // Called for an error in the test-runner (not for an error involving a test itself).
    void message(TestMessageType type, const Slang::String& errorText);
    SLANG_ATTR_PRINTF(3, 4)
    void messageFormat(TestMessageType type, char const* message, ...);
    virtual SLANG_NO_THROW void SLANG_MCALL
    message(TestMessageType type, char const* message) override;

    void dumpOutputDifference(
        const Slang::String& expectedOutput,
        const Slang::String& actualOutput);

    void consolidateWith(TestReporter* other);

    /// Record that a test server died under a test that then passed on a fresh one.
    ///
    /// Counted rather than charged to the test, because the test is demonstrably fine. But
    /// counted, and named, because the alternative -- absorbing it into the pass -- is how a
    /// loss rate grows without anyone noticing. Reported by outputSummary as a warning; it
    /// does not fail the run, since there is not yet a baseline to say what rate is normal.
    void recordTestServerLoss();

    /// Record that a test server returned an unreadable reply under a test that then passed
    /// on a fresh one.
    ///
    /// Kept apart from a loss rather than folded into it: the server did not die here, it
    /// answered with bytes the client could not parse, and the two have different causes. The
    /// rates move independently -- a run can double its crash count while its malformed-reply
    /// count stays flat -- so a single combined number would hide exactly the signal that
    /// tells the two apart.
    void recordTestServerProtocolError();

    /// True if can write output directly to stderr
    bool canWriteStdError() const;

    /// Returns whether `testKey` is on the expected-failure list.
    ///
    /// Named for the single proposition it computes rather than for its caller's decision: the
    /// retry-eligibility gate in `runUnitTestModule` also weighs whether the test actually failed,
    /// whether this is already the retry pass, and whether retries are disabled, and those stay at
    /// the call site.
    ///
    /// `testKey` is the reporter's test key -- for the unit-test path the full command string
    /// (`moduleName/testName.internal`), the same key `adjustResult()` looks up. Passing the bare
    /// test name here answers false for every expected failure, because the expected-failure files
    /// are written in command form; that mismatch is what sent known-failing unit tests round a
    /// pointless retry.
    bool isExpectedFailure(const Slang::String& testKey) const;

    /// Records that the harness never reached `testKey` -- the call failed in transport, so no
    /// verdict exists for it.
    ///
    /// This is not a test result. A dispatch failure says the connection died, and carries no
    /// information about the test that happened to be in flight: on a run configured for one API,
    /// the test in question is often one that would have skipped itself anyway. What it does change
    /// is what may redeem the retry -- see `m_dispatchFailures`.
    void noteDispatchFailure(const Slang::String& testKey);

    /// Returns the diagnostic printed for a deferral that no result ever redeemed.
    ///
    /// Separate from the printing so a test can assert the text: every reporter test suppresses
    /// console output, so this string is otherwise never produced under test.
    static Slang::String describeUnredeemedRetry(const Slang::String& testKey);

    /// Counts every test whose result is still deferred as a failure, and reports it.
    ///
    /// Reporting `TestResult::PendingRetry` for a test defers its result: it is left out of the
    /// statistics on the promise that a retry will report a real result for the same test later.
    /// Nothing guarantees the retry actually happens: it can be skipped wholesale (see the
    /// `stopSchedulingTests` early-abort) or fail to re-discover the test. A deferral that is
    /// never redeemed would otherwise vanish, letting a run that contained real failures report
    /// 100% passed. Call this once after the last retry pass and before `outputSummary()` /
    /// `didAllSucceed()`.
    void reconcilePendingRetries();

    /// Returns true if all run tests succeeded
    bool didAllSucceed() const;

    /// Returns a result from the current test
    TestResult getResult() const;

    void outputSummary();

    /// Configure the reporter from the parsed command-line options. Both the single-run and the
    /// per-worker parallel reporters go through this one path so that all reporter configuration
    /// derived from `Options` (output mode, verbosity, dump-on-failure, hide-ignored, and the
    /// expected-failure list) can never drift between them.
    SlangResult init(const Options& options, bool isSubReporter = false);

    /// Ctor
    TestReporter();
    /// Dtor
    ~TestReporter();

    static TestResult combine(TestResult a, TestResult b) { return (a > b) ? a : b; }

    static TestReporter* get() { return s_reporter; }
    static void set(TestReporter* reporter) { s_reporter = reporter; }

    Slang::List<TestInfo> m_testInfos;

    /// Names of tests whose result was deferred by a `TestResult::PendingRetry` report.
    ///
    /// The key is whatever `info.name` the deferring `addResult()` observed, and the two retry
    /// paths spell that differently: a unit test defers under its full command
    /// (`moduleName/testName.internal`), a file test under its `testName`. So these sets hold keys
    /// from both namespaces at once.
    ///
    /// That is fine, and the invariant the whole mechanism rests on is why: deferral and
    /// redemption use the identical name *within* a path, so an entry can only ever be matched by
    /// the retry that corresponds to it. What must not happen is a lookup in one namespace against
    /// a key written in the other -- the retry-eligibility gate in `runUnitTestModule` reads the
    /// expected-failure list, which is written in command form, and keying that off the bare test
    /// name is exactly the mismatch this fix corrects.
    Slang::HashSet<Slang::String> m_pendingRetryTests;

    /// Names of every test that reached a verdict -- Pass, Fail or ExpectedFail.
    ///
    /// A verdict always redeems a deferral. `Ignored` is not a verdict and is tracked separately in
    /// `m_ignoredTests`, because whether it redeems depends on something this set cannot express:
    /// see `reconcilePendingRetries()`.
    Slang::HashSet<Slang::String> m_redeemingResultTests;

    /// Names of every test that reported `Ignored`.
    ///
    /// Kept apart from the verdicts because an ignore redeems a deferral only when the harness
    /// never reached the test on the first attempt (`m_dispatchFailures`). That pairing cannot be
    /// evaluated when the result is recorded -- the deferral and the retry are reported by
    /// different sub-reporters -- so both sets are carried through `consolidateWith()` and matched
    /// once, in `reconcilePendingRetries()`.
    Slang::HashSet<Slang::String> m_ignoredTests;

    /// Names deferred after the harness failed to reach the test at all, rather than after the test
    /// ran and failed.
    ///
    /// The distinction decides what may redeem the deferral. A test that ran and failed is not
    /// answered by a retry that skips it, so `Ignored` must not redeem it. A test that was never
    /// reached has no failure to refute, and a retry reporting it skipped *is* the answer -- that
    /// is what a run configured for one API says about a test for another. Treating those alike
    /// blames a dead connection on whichever test was next in the queue.
    Slang::HashSet<Slang::String> m_dispatchFailures;

    /// How many dispatch failures were seen, including ones a retry went on to resolve. Reported in
    /// the summary so a connection dying is never merely a line in the middle of the log.
    ///
    /// Unlike the tracking sets, this is not cleared by `reconcilePendingRetries()`: it is a tally
    /// of the run, and `outputSummary()` reads it afterwards.
    int m_dispatchFailureCount = 0;

    Slang::List<Slang::String> m_suiteStack;

    int m_totalTestCount;
    int m_passedTestCount;
    int m_failedTestCount;
    int m_ignoredTestCount;
    int m_expectedFailedTestCount;

    int m_maxFailTestResults; ///< Maximum amount of results per test. If 0 it's infinite.

    /// Test-server deaths that a retry on a fresh server proved were not the test's fault.
    /// Separate from the pass/fail counts on purpose: these tests DID pass, so folding the
    /// losses into m_failedTestCount would re-introduce the false verdict this exists to
    /// remove, while folding them into m_passedTestCount alone would erase the signal.
    /// The COUNT is authoritative: every loss increments it, including one that happens
    /// outside a test scope and therefore contributes no name. The NAME LIST is best-effort
    /// and frequency-weighted -- deliberately a List rather than a set, because a test that
    /// loses several servers is the signal worth seeing, and consolidateWith concatenates
    /// rather than unions for the same reason. So the two can legitimately disagree: more
    /// losses than names, and a name repeated. Neither is a bug.
    int m_testServerLossCount = 0;
    Slang::List<Slang::String> m_testServerLossTests;

    /// Malformed-reply counterpart of the two above, with the same count-vs-names contract.
    int m_testServerProtocolErrorCount = 0;
    Slang::List<Slang::String> m_testServerProtocolErrorTests;

    TestOutputMode m_outputMode = TestOutputMode::Default;
    bool m_dumpOutputOnFailure;
    VerbosityLevel m_verbosity;
    /// Suppresses every write this reporter makes to stdout -- result lines, the pending-retry
    /// notice, the TeamCity suite markers, and the summary -- leaving the accounting (counters,
    /// m_testInfos, the suite stack, the pending/final sets) untouched.
    ///
    /// Set by a unit test that drives a TestReporter directly. Those run inside test-server, which
    /// answers the harness over a JSON-RPC channel carried on its stdout, so a stray
    /// "failed(pending retry) ..." lands in the middle of a JSON message and kills the connection.
    /// Redirecting the file descriptor instead was tried and is not portable: it worked on Linux
    /// and macOS and silently did not on Windows, where the tests then wrote to the live channel.
    bool m_suppressConsoleOutput = false;

    bool m_hideIgnored = false;
    bool m_isSubReporter = false;
    Slang::HashSet<Slang::String> m_expectedFailureList;

protected:
    void _addResult(TestInfo info);

    Slang::StringBuilder m_currentMessage;
    TestInfo m_currentInfo;
    int m_numCurrentResults;
    int m_numFailResults;

    bool m_inTest;

    std::recursive_mutex m_mutex;

    static TestReporter* s_reporter;
};

#endif // TEST_REPORTER_H_INCLUDED
