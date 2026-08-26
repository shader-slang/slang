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

static const slang::CompilerOptionEntry* findCompilerOptionEntry(
    const slang::TargetDesc& targetDesc,
    slang::CompilerOptionName optionName)
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
}

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

} // namespace

SLANG_UNIT_TEST(parseCommandLineArgs)
{
    ParseCommandLineArgsTestContext context(unitTestContext);

    const auto result = context.runTests();

    SLANG_CHECK(SLANG_SUCCEEDED(result));
}

SLANG_UNIT_TEST(parseCUDAEmissionMethods)
{
    slang::IGlobalSession* slangSession = unitTestContext->slangGlobalSession;

    // Consider three distinct outputs in one request. Repeating an identical target format is
    // intentionally coalesced by the command-line parser, so PTX, DXIL, and CUDA source give us
    // three independently observable target option sets. The first two flags switch back to
    // NVRTC, the middle target makes no selection, and the final two flags switch to NVVM. This
    // checks both flag orders while also proving that a selection belongs only to its current
    // target.
    const char* argv[] = {
        "-target",
        "ptx",
        "-emit-cuda-via-nvvm",
        "-emit-cuda-via-nvrtc",
        "-o",
        "nvrtc.ptx",
        "-target",
        "dxil",
        "-o",
        "default.dxil",
        "-target",
        "cuda",
        "-emit-cuda-via-nvrtc",
        "-emit-cuda-via-nvvm",
        "-o",
        "nvvm.cu",
        "--",
        "shader.slang",
    };

    slang::SessionDesc sessionDesc = {};
    ComPtr<ISlangUnknown> allocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slangSession->parseCommandLineArguments(
        SLANG_COUNT_OF(argv),
        argv,
        &sessionDesc,
        allocation.writeRef())));
    SLANG_CHECK_ABORT(sessionDesc.targetCount == 3);

    SLANG_CHECK(sessionDesc.targets[0].format == SLANG_PTX);
    SLANG_CHECK(sessionDesc.targets[1].format == SLANG_DXIL);
    SLANG_CHECK(sessionDesc.targets[2].format == SLANG_CUDA_SOURCE);

    const auto* nvrtcMethod =
        findCompilerOptionEntry(sessionDesc.targets[0], slang::CompilerOptionName::EmitCUDAMethod);
    SLANG_CHECK_ABORT(nvrtcMethod != nullptr);
    SLANG_CHECK(nvrtcMethod->value.kind == slang::CompilerOptionValueKind::Int);
    SLANG_CHECK(nvrtcMethod->value.intValue0 == SLANG_EMIT_CUDA_VIA_NVRTC);

    // The default remains represented by an absent option. Materializing a default here would make
    // a flag on the preceding target leak into this otherwise independent target.
    SLANG_CHECK(
        findCompilerOptionEntry(
            sessionDesc.targets[1],
            slang::CompilerOptionName::EmitCUDAMethod) == nullptr);

    const auto* nvvmMethod =
        findCompilerOptionEntry(sessionDesc.targets[2], slang::CompilerOptionName::EmitCUDAMethod);
    SLANG_CHECK_ABORT(nvvmMethod != nullptr);
    SLANG_CHECK(nvvmMethod->value.kind == slang::CompilerOptionValueKind::Int);
    SLANG_CHECK(nvvmMethod->value.intValue0 == SLANG_EMIT_CUDA_VIA_NVVM);

    // The command-line spellings are selectors, not independent stored booleans. Keeping one
    // canonical option prevents routing, serialization, and cache identity from disagreeing.
    for (SlangInt i = 0; i < sessionDesc.targetCount; ++i)
    {
        SLANG_CHECK(
            findCompilerOptionEntry(
                sessionDesc.targets[i],
                slang::CompilerOptionName::EmitCUDAViaNVRTC) == nullptr);
        SLANG_CHECK(
            findCompilerOptionEntry(
                sessionDesc.targets[i],
                slang::CompilerOptionName::EmitCUDAViaNVVM) == nullptr);
    }
}
