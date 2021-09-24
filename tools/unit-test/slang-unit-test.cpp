#include "slang-unit-test.h"
#include "slang.h"
#include "source/core/slang-basic.h"

struct SlangUnitTest
{
    const char* name;
    UnitTestFunc func;
};

class SlangUnitTestModule : public IUnitTestModule
{
public:
    Slang::List<SlangUnitTest> tests;
    ITestReporter* testReporter = nullptr;

    virtual SlangInt getTestCount() override
    {
        return tests.getCount();
    }
    virtual const char* getTestName(SlangInt index) override
    {
        return tests[index].name;
    }

    virtual UnitTestFunc getTestFunc(SlangInt index) override
    {
        return tests[index].func;
    }

    virtual void setTestReporter(ITestReporter* reporter) override
    {
        testReporter = reporter;
    }

    virtual void destroy() override
    {
        tests = decltype(tests)();
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

UnitTestRegisterHelper::UnitTestRegisterHelper(const char* name, UnitTestFunc testFunc)
{
    _getTestModule()->tests.add(SlangUnitTest{ name, testFunc });
}
