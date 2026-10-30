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
            "-target",
            "spirv",
            "-o",
            "out1.spv",
            "-profile",
            "sm_5_0",
            "-target",
            "dxil",
            "-o",
            "out2.dxil",
            "-profile",
            "sm_6_0",
            "--",
            "shader.slang"};
        int argc = SLANG_COUNT_OF(argv);

        slang::SessionDesc sessionDesc{};
        ComPtr<ISlangUnknown> allocation;

        SLANG_RETURN_ON_FAIL(slangSession->parseCommandLineArguments(
            argc,
            argv,
            &sessionDesc,
            allocation.writeRef()));

        const auto findCompilerOptionEntry =
            [](const slang::TargetDesc& targetDesc,
               slang::CompilerOptionName optionName) -> const slang::CompilerOptionEntry*
        {
            if (!targetDesc.compilerOptionEntries)
                return nullptr;

            for (SlangInt i = 0; i < targetDesc.compilerOptionEntryCount; ++i)
            {
                const auto& entry = targetDesc.compilerOptionEntries[i];
                if (entry.name == optionName)
                    return &entry;
            }
            return nullptr;
        };

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

        const auto* targetOption0 =
            findCompilerOptionEntry(target0, slang::CompilerOptionName::Target);
        SLANG_CHECK(targetOption0 != nullptr);
        if (!targetOption0)
            return SLANG_FAIL;
        SLANG_CHECK(targetOption0->value.intValue0 == static_cast<int32_t>(target0.format));
        if (targetOption0->value.intValue0 != static_cast<int32_t>(target0.format))
            return SLANG_FAIL;

        const auto* profileOption0 =
            findCompilerOptionEntry(target0, slang::CompilerOptionName::Profile);
        SLANG_CHECK(profileOption0 != nullptr);
        if (!profileOption0)
            return SLANG_FAIL;
        SLANG_CHECK(profileOption0->value.intValue0 == static_cast<int32_t>(profile_sm_5_0));
        if (profileOption0->value.intValue0 != static_cast<int32_t>(profile_sm_5_0))
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

        const auto* targetOption1 =
            findCompilerOptionEntry(target1, slang::CompilerOptionName::Target);
        SLANG_CHECK(targetOption1 != nullptr);
        if (!targetOption1)
            return SLANG_FAIL;
        SLANG_CHECK(targetOption1->value.intValue0 == static_cast<int32_t>(target1.format));
        if (targetOption1->value.intValue0 != static_cast<int32_t>(target1.format))
            return SLANG_FAIL;

        const auto* profileOption1 =
            findCompilerOptionEntry(target1, slang::CompilerOptionName::Profile);
        SLANG_CHECK(profileOption1 != nullptr);
        if (!profileOption1)
            return SLANG_FAIL;
        SLANG_CHECK(profileOption1->value.intValue0 == static_cast<int32_t>(profile_sm_6_0));
        if (profileOption1->value.intValue0 != static_cast<int32_t>(profile_sm_6_0))
            return SLANG_FAIL;

        return SLANG_OK;
    }

    UnitTestContext* m_unitTestContext;
};

// A failed parse preserves the caller's existing session description so it remains usable;
// the auxiliary allocation is cleared because no parsed data is returned.
void checkParseFails(slang::IGlobalSession* globalSession, int argc, const char* const* argv)
{
    // Non-default starting values let the checks below tell an untouched output from one that was
    // written. The allocation value is never dereferenced.
    const slang::TargetDesc callerTarget{};
    const slang::CompilerOptionEntry callerEntry{};
    slang::SessionDesc sessionDesc{};
    sessionDesc.targets = &callerTarget;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = &callerEntry;
    sessionDesc.compilerOptionEntryCount = 1;
    ISlangUnknown* allocation = reinterpret_cast<ISlangUnknown*>(uintptr_t(1));

    const SlangResult result =
        globalSession->parseCommandLineArguments(argc, argv, &sessionDesc, &allocation);
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(allocation == nullptr);
    SLANG_CHECK(sessionDesc.targets == &callerTarget);
    SLANG_CHECK(sessionDesc.targetCount == 1);
    SLANG_CHECK(sessionDesc.compilerOptionEntries == &callerEntry);
    SLANG_CHECK(sessionDesc.compilerOptionEntryCount == 1);

    if (SLANG_SUCCEEDED(result))
        allocation->release();
}

} // namespace

SLANG_UNIT_TEST(parseCommandLineArgs)
{
    ParseCommandLineArgsTestContext context(unitTestContext);

    const auto result = context.runTests();

    SLANG_CHECK(SLANG_SUCCEEDED(result));
}

SLANG_UNIT_TEST(parseCommandLineArgsReportsParseErrors)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* missingTargetName[] = {"-target"};
    checkParseFails(globalSession, SLANG_COUNT_OF(missingTargetName), missingTargetName);

    const char* unknownOption[] = {"-bogus-option"};
    checkParseFails(globalSession, SLANG_COUNT_OF(unknownOption), unknownOption);

    const char* unknownTarget[] = {"-target", "bogus"};
    checkParseFails(globalSession, SLANG_COUNT_OF(unknownTarget), unknownTarget);

    const char* missingProfileName[] = {"-profile"};
    checkParseFails(globalSession, SLANG_COUNT_OF(missingProfileName), missingProfileName);

    {
        slang::SessionDesc sessionDesc{};
        ComPtr<ISlangUnknown> allocation;
        SLANG_CHECK(SLANG_SUCCEEDED(
            globalSession
                ->parseCommandLineArguments(0, nullptr, &sessionDesc, allocation.writeRef())));
        SLANG_CHECK(sessionDesc.targetCount == 0);
    }
    {
        const char* argv[] = {"-target", "spirv"};
        slang::SessionDesc sessionDesc{};
        ComPtr<ISlangUnknown> allocation;
        SLANG_CHECK(SLANG_SUCCEEDED(globalSession->parseCommandLineArguments(
            SLANG_COUNT_OF(argv),
            argv,
            &sessionDesc,
            allocation.writeRef())));
        SLANG_CHECK(sessionDesc.targetCount == 1);
        if (sessionDesc.targetCount == 1)
            SLANG_CHECK(sessionDesc.targets[0].format == SLANG_SPIRV);
    }
}

// Target options given without `-target` are not an error for this API: the session desc carries
// them as session options, so they apply to every target the caller adds to the session.
SLANG_UNIT_TEST(parseCommandLineArgsKeepsTargetOptionsWithoutTarget)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* argv[] = {"-floating-point-mode", "precise"};
    slang::SessionDesc sessionDesc{};
    ComPtr<ISlangUnknown> allocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->parseCommandLineArguments(
        SLANG_COUNT_OF(argv),
        argv,
        &sessionDesc,
        allocation.writeRef())));
    SLANG_CHECK(sessionDesc.targetCount == 0);

    bool hasPreciseFloatingPointMode = false;
    for (uint32_t i = 0; i < sessionDesc.compilerOptionEntryCount; ++i)
    {
        const auto& entry = sessionDesc.compilerOptionEntries[i];
        if (entry.name == slang::CompilerOptionName::FloatingPointMode &&
            entry.value.intValue0 == SLANG_FLOATING_POINT_MODE_PRECISE)
            hasPreciseFloatingPointMode = true;
    }
    SLANG_CHECK(hasPreciseFloatingPointMode);
}
