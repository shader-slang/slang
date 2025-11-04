#pragma once

#include "core/slang-render-api-util.h"
#include "slang.h"

enum class TestResult
{
    // NOTE! Must keep in order such that combine is meaningful. That is larger values are higher
    // precident - and a series of tests that has lots of passes and a fail, is still a fail
    // overall.
    // NOTE! Do NOT reorder or change values 0-3. The slang-llvm prebuilt library depends on these.

    // Special states with negative values (lowest precedence)
    Uninitialized = -2, ///< Test result not yet set - initial state before any results are added
    PendingRetry =
        -1, ///< Test failed and is scheduled for retry - not counted in any statistics yet

    // Normal test results (NOTE! Do NOT CHANGE - binary compatibility!)
    Ignored = 0,      ///< Test was explicitly skipped/ignored
    Pass = 1,         ///< Test passed
    ExpectedFail = 2, ///< Test expected to fail based on expected failure list
    Fail = 3,         ///< Test failed
};

enum class TestMessageType
{
    Info,        ///< General info (may not be shown depending on verbosity setting)
    TestFailure, ///< Describes how a test failure took place
    RunError,    ///< Describes an error that caused a test not to actually correctly run
};

class ITestReporter
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL startTest(const char* testName) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL addResult(TestResult result) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
    addResultWithLocation(TestResult result, const char* testText, const char* file, int line) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
    addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL addExecutionTime(double time) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL message(TestMessageType type, const char* message) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL endTest() = 0;
};

ITestReporter* getTestReporter();

namespace rhi
{
class IDebugCallback;
}

struct UnitTestContext
{
    slang::IGlobalSession* slangGlobalSession;
    const char* workDirectory;
    const char* executableDirectory;
    Slang::RenderApiFlags enabledApis;
    bool enableDebugLayers;
    rhi::IDebugCallback* debugCallback = nullptr;
};

typedef void (*UnitTestFunc)(UnitTestContext*);

class IUnitTestModule
{
public:
    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getTestCount() = 0;
    virtual SLANG_NO_THROW const char* SLANG_MCALL getTestName(SlangInt index) = 0;
    virtual SLANG_NO_THROW UnitTestFunc SLANG_MCALL getTestFunc(SlangInt index) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setTestReporter(ITestReporter* reporter) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL destroy() = 0;
};

class UnitTestRegisterHelper
{
public:
    UnitTestRegisterHelper(const char* name, UnitTestFunc testFunc);
};

class AbortTestException
{
};

typedef IUnitTestModule* (*UnitTestGetModuleFunc)();

#define SLANG_UNIT_TEST(name)                                    \
    void _##name##_impl(UnitTestContext* unitTestContext);       \
    void name(UnitTestContext* unitTestContext)                  \
    {                                                            \
        try                                                      \
        {                                                        \
            _##name##_impl(unitTestContext);                     \
        }                                                        \
        catch (AbortTestException&)                              \
        {                                                        \
        }                                                        \
    }                                                            \
    UnitTestRegisterHelper _##name##RegisterHelper(#name, name); \
    void _##name##_impl(UnitTestContext* unitTestContext)

#define SLANG_CHECK(x) getTestReporter()->addResultWithLocation((x), #x, __FILE__, __LINE__);
#define SLANG_CHECK_ABORT(x)                                                                   \
    {                                                                                          \
        bool _slang_check_result = (x);                                                        \
        getTestReporter()->addResultWithLocation(_slang_check_result, #x, __FILE__, __LINE__); \
        if (!_slang_check_result)                                                              \
            throw AbortTestException();                                                        \
    }
#define SLANG_IGNORE_TEST                              \
    getTestReporter()->addResult(TestResult::Ignored); \
    throw AbortTestException();
#define SLANG_CHECK_MSG(condition, message) \
    getTestReporter()                       \
        ->addResultWithLocation((condition), #condition " " message, __FILE__, __LINE__)
