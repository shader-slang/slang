// unit-test-parse-command-line-args.cpp

#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

namespace
{

using namespace Slang;

struct ParseCommandLineArgsTestContext
{
    ParseCommandLineArgsTestContext(UnitTestContext* context)
        : m_unitTestContext(context)
    {
    }

    SlangResult runTests()
    {
        slang::IGlobalSession* slangSession = m_unitTestContext->slangGlobalSession;
        
        // Test multiple targets with different formats and profiles
        // Note: parseCommandLineArguments expects arguments WITHOUT the program name
        const char* argv[] = {
            "-target", "spirv",
            "-o", "out1.spv",
            "-profile", "sm_5_0",
            "-target", "dxil",
            "-o", "out2.dxil",
            "-profile", "sm_6_0",
            "--",
            "shader.slang"
        };
        int argc = SLANG_COUNT_OF(argv);

        slang::SessionDesc sessionDesc{};
        ComPtr<ISlangUnknown> allocation;
        
        SLANG_RETURN_ON_FAIL(slangSession->parseCommandLineArguments(
            argc, argv, &sessionDesc, allocation.writeRef()));

        // Verify we have 2 targets
        SLANG_CHECK(sessionDesc.targetCount == 2);
        if (sessionDesc.targetCount != 2)
            return SLANG_FAIL;

        // Verify first target is SPIRV with sm_5_0
        const auto& target0 = sessionDesc.targets[0];
        SLANG_CHECK(target0.format == SLANG_SPIRV);
        if (target0.format != SLANG_SPIRV)
            return SLANG_FAIL;

        auto profile_sm_5_0 = slangSession->findProfile("sm_5_0");
        SLANG_CHECK(target0.profile == profile_sm_5_0);
        if (target0.profile != profile_sm_5_0)
            return SLANG_FAIL;

        // Verify second target is DXIL with sm_6_0
        const auto& target1 = sessionDesc.targets[1];
        SLANG_CHECK(target1.format == SLANG_DXIL);
        if (target1.format != SLANG_DXIL)
            return SLANG_FAIL;

        auto profile_sm_6_0 = slangSession->findProfile("sm_6_0");
        SLANG_CHECK(target1.profile == profile_sm_6_0);
        if (target1.profile != profile_sm_6_0)
            return SLANG_FAIL;

        return SLANG_OK;
    }

    UnitTestContext* m_unitTestContext;
};

} // namespace

SLANG_UNIT_TEST(parseCommandLineArgs)
{
    ParseCommandLineArgsTestContext context(unitTestContext);

    const auto result = context.runTests();

    SLANG_CHECK(SLANG_SUCCEEDED(result));
}
