#include "slang-unit-test.h"
#include "slang.h"
#include "source/core/slang-basic.h"

struct SlangUnitTest
{
    const char* name;
    slang::UnitTestFunc func;
};

class SlangUnitTestModule : public slang::IUnitTestModule
{
public:
    Slang::List<SlangUnitTest> tests;

    virtual SlangInt getTestCount() override
    {
        return tests.getCount();
    }
    virtual const char* getTestName(SlangInt index) override
    {
        return tests[index].name;
    }

    virtual slang::UnitTestFunc getTestFunc(SlangInt index) override
    {
        return tests[index].func;
    }
};

SlangUnitTestModule* _getTestModule()
{
    static SlangUnitTestModule testModule;
    return &testModule;
}

extern "C"
{
SLANG_DLL_EXPORT slang::IUnitTestModule* slangUnitTestGetModule()
{
    return _getTestModule();
}
}

slang::UnitTestRegisterHelper::UnitTestRegisterHelper(const char* name, UnitTestFunc testFunc)
{
    _getTestModule()->tests.add(SlangUnitTest{ name, testFunc });
}
