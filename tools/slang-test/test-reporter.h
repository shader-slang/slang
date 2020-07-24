// test-reporter.h

#ifndef TEST_REPORTER_H_INCLUDED
#define TEST_REPORTER_H_INCLUDED

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-platform.h"
#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-dictionary.h"


#define SLANG_CHECK(x) TestReporter::get()->addResultWithLocation((x), #x, __FILE__, __LINE__);
#define SLANG_CHECK_ABORT(x)                                                                     \
    {                                                                                            \
        bool _slang_check_result = (x);                                                          \
        TestReporter::get()->addResultWithLocation(_slang_check_result, #x, __FILE__, __LINE__); \
        if (!_slang_check_result) return;                                                        \
    }

struct TestRegister
{
    typedef void (*TestFunc)();

    TestRegister(const char* name, TestFunc func):
        m_next(s_first),
        m_name(name),
        m_func(func)
    {
        s_first = this;
    }

    TestFunc m_func;
    const char* m_name;
    TestRegister* m_next;

    static TestRegister* s_first;
};

#define SLANG_UNIT_TEST(name, func) static TestRegister s_unitTest##__LINE__(name, func)

enum class TestOutputMode
{
    Default = 0,   ///< Default mode is to write test results to the console
    AppVeyor,      ///< For AppVeyor continuous integration 
    Travis,        ///< We currently don't specialize for Travis, but maybe we should.
    XUnit,         ///< xUnit original format  https://nose.readthedocs.io/en/latest/plugins/xunit.html
    XUnit2,        ///< https://xunit.github.io/docs/format-xml-v2
    TeamCity,      ///< Output suitable for teamcity
};

enum class TestResult
{
    // NOTE! Must keep in order such that combine is meaningful. That is larger values are higher precident - and a series of tests that has lots of passes
    // and a fail, is still a fail overall. 
    Ignored,
    Pass,
    Fail,
};

enum class TestMessageType
{
    Info,                   ///< General info (may not be shown depending on verbosity setting)
    TestFailure,           ///< Describes how a test failure took place
    RunError,              ///< Describes an error that caused a test not to actually correctly run
};

class TestReporter
{
    public:

    struct TestInfo
    {
        TestResult testResult = TestResult::Ignored;
        Slang::String name;
        Slang::String message;                  ///< Message that is specific for the testResult
        double executionTime = 0.0;             ///< <= 0.0 if not defined. Time is in seconds. 
    };
    
    class TestScope
    {
    public:
        TestScope(TestReporter* reporter, const Slang::String& testName) :
            m_reporter(reporter)
        {
            reporter->startTest(testName);
        }
        ~TestScope()
        {
            m_reporter->endTest();
        }

    protected:
        TestReporter* m_reporter;
    };

    class SuiteScope
    {
    public:
        SuiteScope(TestReporter* reporter, const Slang::String& suiteName) :
            m_reporter(reporter)
        {
            reporter->startSuite(suiteName);
        }
        ~SuiteScope()
        {
            m_reporter->endSuite();
        }

    protected:
        TestReporter* m_reporter;
    };

    void startSuite(const Slang::String& name);
    void endSuite();

    void startTest(const Slang::String& testName);
    void addResult(TestResult result);
    void addResultWithLocation(TestResult result, const char* testText, const char* file, int line);
    void addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line);
    void addExecutionTime(double time);
    void endTest();
    
        /// Runs start/endTest and outputs the result
    TestResult addTest(const Slang::String& testName, bool isPass);
        /// Effectively runs start/endTest (so cannot be called inside start/endTest). 
    void addTest(const Slang::String& testName, TestResult testResult);

        // Called for an error in the test-runner (not for an error involving a test itself).
    void message(TestMessageType type, const Slang::String& errorText);
    void messageFormat(TestMessageType type, char const* message, ...);

    void dumpOutputDifference(const Slang::String& expectedOutput, const Slang::String& actualOutput);

        /// True if can write output directly to stderr
    bool canWriteStdError() const;

    
        /// Returns true if all run tests succeeded
    bool didAllSucceed() const;

    void outputSummary();

    SlangResult init(TestOutputMode outputMode);

        /// Ctor
    TestReporter();
        /// Dtor
    ~TestReporter();

    static TestResult combine(TestResult a, TestResult b) { return (a > b) ? a : b; }

    static TestReporter* get() { return s_reporter; }
    static void set(TestReporter* reporter) { s_reporter = reporter; }

    Slang::List<TestInfo> m_testInfos;

    Slang::List<Slang::String> m_suiteStack;

    int m_totalTestCount;
    int m_passedTestCount;
    int m_failedTestCount;
    int m_ignoredTestCount;

    int m_maxFailTestResults;                   ///< Maximum amount of results per test. If 0 it's infinite.

    TestOutputMode m_outputMode = TestOutputMode::Default;
    bool m_dumpOutputOnFailure;
    bool m_isVerbose = false;
    bool m_hideIgnored = false;

protected:
    
    void _addResult(const TestInfo& info);
    
    Slang::StringBuilder m_currentMessage;
    TestInfo m_currentInfo;
    int m_numCurrentResults;
    int m_numFailResults;

    bool m_inTest;

    static TestReporter* s_reporter;
};

#endif // TEST_REPORTER_H_INCLUDED

