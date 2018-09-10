// test-context.h

#include "../../source/core/slang-string-util.h"

enum OutputMode
{
    // Default mode is to write test results to the console
    kOutputMode_Default = 0,

    // When running under AppVeyor continuous integration, we
    // need to output test results in a way that the AppVeyor
    // environment can pick up and display.
    kOutputMode_AppVeyor,

    // We currently don't specialize for Travis, but maybe
    // we should.
    kOutputMode_Travis,

    // xUnit original format
    // https://nose.readthedocs.io/en/latest/plugins/xunit.html
    kOutputMode_xUnit,

    // https://xunit.github.io/docs/format-xml-v2
    kOutputMode_xUnit2,
};

enum TestResult
{
    kTestResult_Fail,
    kTestResult_Pass,
    kTestResult_Ignored,
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
        TestResult testResult = TestResult::kTestResult_Ignored;
        Slang::String name;
        Slang::String message;                 ///< Message that is specific for the testResult
    };

    void addResult(const Slang::String& testName, TestResult testResult);

    void startTest(const Slang::String& testName);
    TestResult endTest(TestResult result);

        // Called for an error in the test-runner (not for an error involving a test itself).
    void message(TestMessageType type, const Slang::String& errorText);
    void messageFormat(TestMessageType type, char const* message, ...);

    void dumpOutputDifference(const Slang::String& expectedOutput, const Slang::String& actualOutput);

        /// True if can write output directly to stderr
    bool canWriteStdError() const;

        /// Ctor
    TestContext(OutputMode outputMode);

    Slang::List<TestInfo> m_testInfos;

    int m_totalTestCount;
    int m_passedTestCount;
    int m_failedTestCount;
    int m_ignoredTestCount;

    OutputMode m_outputMode = kOutputMode_Default;
    bool m_dumpOutputOnFailure;
    bool m_isVerbose;

protected:
    void _addResult(const TestInfo& info);

    Slang::StringBuilder m_currentMessage;

    TestInfo m_currentInfo;
    bool m_inTest;
};


