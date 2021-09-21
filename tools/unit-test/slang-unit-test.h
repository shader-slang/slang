#pragma once

#include "slang.h"

namespace slang
{
    struct UnitTestContext
    {
        slang::IGlobalSession* slangGlobalSession;
        const char* workDirectory;
        ISlangWriter* outputWriter;
    };

    typedef SlangResult (*UnitTestFunc)(UnitTestContext*);

    class IUnitTestModule
    {
    public:
        virtual SlangInt getTestCount() = 0;
        virtual const char* getTestName(SlangInt index) = 0;
        virtual UnitTestFunc getTestFunc(SlangInt index) = 0;
    };

    class UnitTestRegisterHelper
    {
    public:
        UnitTestRegisterHelper(const char* name, UnitTestFunc testFunc);
    };

    typedef slang::IUnitTestModule* (*UnitTestGetModuleFunc)();

#define SLANG_UNIT_TEST(name) \
    SlangResult name(slang::UnitTestContext* context); \
    slang::UnitTestRegisterHelper _##name##RegisterHelper(#name, name); \
    SlangResult name(slang::UnitTestContext* context)
}
