#include "slang-unit-test.h"

#include "core/slang-basic.h"
#include "slang.h"

struct SlangUnitTest
{
    const char* name;
    UnitTestFunc func;
    Slang::RenderApiFlags usedApis;
};

class SlangUnitTestModule : public IUnitTestModule
{
public:
    Slang::List<SlangUnitTest> tests;
    ITestReporter* testReporter = nullptr;

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getTestCount() override { return tests.getCount(); }
    virtual SLANG_NO_THROW const char* SLANG_MCALL getTestName(SlangInt index) override
    {
        return tests[index].name;
    }

    virtual SLANG_NO_THROW UnitTestFunc SLANG_MCALL getTestFunc(SlangInt index) override
    {
        return tests[index].func;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setTestReporter(ITestReporter* reporter) override
    {
        testReporter = reporter;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL destroy() override { tests = decltype(tests)(); }

    virtual SLANG_NO_THROW Slang::RenderApiFlags SLANG_MCALL
    getTestUsedApis(SlangInt index) override
    {
        return tests[index].usedApis;
    }
};

SlangUnitTestModule* _getTestModule()
{
    static SlangUnitTestModule testModule;
    return &testModule;
}

ITestReporter* getTestReporter()
{
    return _getTestModule()->testReporter;
}

extern "C"
{
    SLANG_DLL_EXPORT IUnitTestModule* slangUnitTestGetModule()
    {
        return _getTestModule();
    }
}

UnitTestRegisterHelper::UnitTestRegisterHelper(
    const char* name,
    UnitTestFunc testFunc,
    Slang::RenderApiFlags usedApis)
{
    _getTestModule()->tests.add(SlangUnitTest{name, testFunc, usedApis});
}
