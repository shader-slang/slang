// test-context.h

#include "../../source/core/slang-string-util.h"

#define SLANG_CHECK(x) TestContext::get()->addTest(#x, (x)); 

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
    eFail,
    ePass,
    eIgnored,
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
    
    void startTest(const Slang::String& testName);
    TestResult endTest(TestResult result);
    
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

        /// Ctor
    TestContext(TestOutputMode outputMode);

    static TestContext* get() { return s_context; }
    static void set(TestContext* context) { s_context = context; }

    Slang::List<TestInfo> m_testInfos;

    int m_totalTestCount;
    int m_passedTestCount;
    int m_failedTestCount;
    int m_ignoredTestCount;

    TestOutputMode m_outputMode = TestOutputMode::eDefault;
    bool m_dumpOutputOnFailure;
    bool m_isVerbose;

protected:
    void _addResult(const TestInfo& info);

    Slang::StringBuilder m_currentMessage;

    TestInfo m_currentInfo;
    bool m_inTest;

    static TestContext* s_context;
};


