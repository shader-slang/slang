// test-context.h

#include "../../source/core/slang-string-util.h"
#include "../../source/core/platform.h"
#include "../../source/core/slang-app-context.h"
#include "../../source/core/dictionary.h"


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

class TestContext
{
    public:
    typedef SlangResult(*InnerMainFunc)(Slang::AppContext* appContext, SlangSession* session, int argc, const char*const* argv);

    struct TestInfo
    {
        TestResult testResult = TestResult::Ignored;
        Slang::String name;
        Slang::String message;                 ///< Message that is specific for the testResult
    };
    
    class TestScope
    {
    public:
        TestScope(TestContext* context, const Slang::String& testName) :
            m_context(context)
        {
            context->startTest(testName);
        }
        ~TestScope()
        {
            m_context->endTest();
        }

    protected:
        TestContext* m_context;
    };

    class SuiteScope
    {
    public:
        SuiteScope(TestContext* context, const Slang::String& suiteName) :
            m_context(context)
        {
            context->startSuite(suiteName);
        }
        ~SuiteScope()
        {
            m_context->endSuite();
        }

    protected:
        TestContext* m_context;
    };

    void startSuite(const Slang::String& name);
    void endSuite();

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

    
        /// Returns true if all run tests succeeded
    bool didAllSucceed() const;

        /// Get the InnerMain function from a shared library tool
    InnerMainFunc getInnerMainFunc(const Slang::String& dirPath, const Slang::String& name);

        /// Get the slang session
    SlangSession* getSession() const { return m_session;  }

    SlangResult init(TestOutputMode outputMode);

        /// Ctor
    TestContext();
        /// Dtor
    ~TestContext();

    static TestResult combine(TestResult a, TestResult b) { return (a > b) ? a : b; }

    static TestContext* get() { return s_context; }
    static void set(TestContext* context) { s_context = context; }

    Slang::List<TestInfo> m_testInfos;

    Slang::List<Slang::String> m_suiteStack;

    int m_totalTestCount;
    int m_passedTestCount;
    int m_failedTestCount;
    int m_ignoredTestCount;

    int m_maxFailTestResults;                   ///< Maximum amount of results per test. If 0 it's infinite.

    TestOutputMode m_outputMode = TestOutputMode::Default;
    bool m_dumpOutputOnFailure;
    bool m_isVerbose;

    bool m_useExes;

    void outputSummary();

protected:
    struct SharedLibraryTool
    {
        Slang::SharedLibrary::Handle m_sharedLibrary;
        InnerMainFunc m_func;
    };

    void _addResult(const TestInfo& info);
    
    Slang::StringBuilder m_currentMessage;
    TestInfo m_currentInfo;
    int m_numCurrentResults;
    int m_numFailResults;

    bool m_inTest;

    SlangSession* m_session;

    Slang::Dictionary<Slang::String, SharedLibraryTool> m_sharedLibTools;

    static TestContext* s_context;
};


