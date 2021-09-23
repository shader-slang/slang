#pragma once

#include "slang.h"
#include "source/core/slang-render-api-util.h"

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

class ITestReporter
{
public:
    virtual void startTest(const char* testName) = 0;
    virtual void addResult(TestResult result) = 0;
    virtual void addResultWithLocation(TestResult result, const char* testText, const char* file, int line) = 0;
    virtual void addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line) = 0;
    virtual void addExecutionTime(double time) = 0;
    virtual void message(TestMessageType type, const char* message) = 0;
    virtual void endTest() = 0;
};

ITestReporter* getTestReporter();

struct UnitTestContext
{
    slang::IGlobalSession* slangGlobalSession;
    const char* workDirectory;
    Slang::RenderApiFlags enabledApis;
};

typedef void (*UnitTestFunc)(UnitTestContext*);

class IUnitTestModule
{
public:
    virtual SlangInt getTestCount() = 0;
    virtual const char* getTestName(SlangInt index) = 0;
    virtual UnitTestFunc getTestFunc(SlangInt index) = 0;
    virtual void setTestReporter(ITestReporter* reporter) = 0;
    virtual void destroy() = 0;
};

class UnitTestRegisterHelper
{
public:
    UnitTestRegisterHelper(const char* name, UnitTestFunc testFunc);
};

typedef IUnitTestModule* (*UnitTestGetModuleFunc)();

#define SLANG_UNIT_TEST(name) \
void name(UnitTestContext* unitTestContext); \
UnitTestRegisterHelper _##name##RegisterHelper(#name, name); \
void name(UnitTestContext* unitTestContext)

#define SLANG_CHECK(x) getTestReporter()->addResultWithLocation((x), #x, __FILE__, __LINE__);
#define SLANG_CHECK_ABORT(x)                                                                     \
    {                                                                                            \
        bool _slang_check_result = (x);                                                          \
        getTestReporter()->addResultWithLocation(_slang_check_result, #x, __FILE__, __LINE__); \
        if (!_slang_check_result) return;                                                        \
    }
#define SLANG_IGNORE_TEST getTestReporter()->addResult(TestResult::Ignored); return;
