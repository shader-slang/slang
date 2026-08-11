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

    /// Names of every test that reached a *redeeming verdict* -- Pass, Fail or ExpectedFail.
    ///
    /// `Ignored` is excluded even though it is also a final, non-deferred result. A test is
    /// deferred because it ran and failed, and a retry that skips it produces no verdict and so
    /// refutes nothing; letting an ignore redeem the deferral is how a real failure went missing
    /// (see the recording site in `_addResult`).
    ///
    /// Every other result lands here, so this holds roughly one entry per test in the run rather
    /// than just the handful that redeem something. Reconciliation reads only the intersection with
    /// `m_pendingRetryTests`; recording unconditionally is what makes that order-independent across
    /// sub-reporters.
    ///
    /// This is what redeems an entry in `m_pendingRetryTests`, and it is tracked separately rather
    /// than read back out of `m_testInfos` for two reasons: a test can be deferred by one
    /// sub-reporter and resolved by another (the retry pass runs on different threads than the
    /// first pass), so the two sets have to survive `consolidateWith` independently; and a final
    /// `Ignored` result under `-hide-ignored` never reaches `m_testInfos` at all.
    Slang::HashSet<Slang::String> m_finalResultTests;

    Slang::List<Slang::String> m_suiteStack;

    int m_totalTestCount;
    int m_passedTestCount;
    int m_failedTestCount;
    int m_ignoredTestCount;
    int m_expectedFailedTestCount;

    int m_maxFailTestResults; ///< Maximum amount of results per test. If 0 it's infinite.

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
