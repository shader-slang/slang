// test-context.h

#include "../../source/core/slang-string-util.h"

#define SLANG_CHECK(x) TestContext::get()->addResultWithLocation((x), #x, __FILE__, __LINE__); 

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
    eDefault = 0,   ///< Default mode is to write test results to the console
    eAppVeyor,      ///< For AppVeyor continuous integration 
    eTravis,        ///< We currently don't specialize for Travis, but maybe we should.
    eXUnit,         ///< xUnit original format  https://nose.readthedocs.io/en/latest/plugins/xunit.html
    eXUnit2,        ///< https://xunit.github.io/docs/format-xml-v2
};

enum class TestResult
{
    eIgnored,
    ePass,
    eFail,
};

enum class TestMessageType
{
    eInfo,                   ///< General info (may not be shown depending on verbosity setting)
    eTestFailure,           ///< Describes how a test failure took place
    eRunError,              ///< Describes an error that caused a test not to actually correctly run
};

class TestContext
{
    public:

    struct TestInfo
    {
        TestResult testResult = TestResult::eIgnored;
        Slang::String name;
        Slang::String message;                 ///< Message that is specific for the testResult
    };
    
    class Scope
    {
    public:
        Scope(TestContext* context, const Slang::String& testName) :
            m_context(context)
        {
            context->startTest(testName);
        }
        ~Scope()
        {
            m_context->endTest();
        }

    protected:
        TestContext* m_context;
    };

    void startTest(const Slang::String& testName);
    void addResult(TestResult result);
    void addResultWithLocation(TestResult result, const char* testText, const char* file, int line);
    void addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line);

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

        /// Call at end of tests 
    void outputSummary();

        /// Returns true if all run tests succeeded
    bool didAllSucceed() const;

        /// Ctor
    TestContext(TestOutputMode outputMode);

    static TestResult combine(TestResult a, TestResult b) { return (a > b) ? a : b; }

    static TestContext* get() { return s_context; }
    static void set(TestContext* context) { s_context = context; }

    Slang::List<TestInfo> m_testInfos;

    int m_totalTestCount;
    int m_passedTestCount;
    int m_failedTestCount;
    int m_ignoredTestCount;

    int m_maxFailTestResults;                   ///< Maximum amount of results per test. If 0 it's infinite.

    TestOutputMode m_outputMode = TestOutputMode::eDefault;
    bool m_dumpOutputOnFailure;
    bool m_isVerbose;

protected:
    void _addResult(const TestInfo& info);

    Slang::StringBuilder m_currentMessage;
    TestInfo m_currentInfo;
    int m_numCurrentResults;
    int m_numFailResults;

    bool m_inTest;
    
    static TestContext* s_context;
};


