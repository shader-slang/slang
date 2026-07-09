// unit-test-descriptor-set-space-offset-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Verify that `TypeLayoutReflection::getDescriptorSetSpaceOffset`
// returns the user's actual `space=N` for HLSL `register(_, spaceN)`
// declarations (and equivalently for Vulkan `[[vk::binding(_, N)]]`).
//
// A previous bug left `DescriptorSetInfo::spaceOffset` default-
// initialized to 0 when descriptor sets were created in
// `_findOrAddDescriptorSet`. `getDescriptorSetSpaceOffset` therefore
// returned 0 for every set regardless of the user's declaration,
// causing slang-rhi backends (D3D12 root-signature builder, Vulkan
// descriptor-set allocator) to lay out their descriptor tables wrong
// for any non-zero space — D3D12 rejected the layout at root-
// signature creation; Vulkan silently mis-bound the buffer.
//
// The high-level reflection JSON path (`parameters[].binding.space`)
// goes through `VariableLayoutReflection::getOffset` and was already
// correct, so this regression specifically targets the descriptor-set
// path used by slang-rhi.
//
// See: https://github.com/shader-slang/slang/issues/10959
SLANG_UNIT_TEST(descriptorSetSpaceOffsetReflection)
{
    const char* userSource = R"(
        RWStructuredBuffer<uint> A : register(u0);
        RWStructuredBuffer<uint> B : register(u7, space3);

        [Shader("compute")]
        [NumThreads(64, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            A[tid.x] = A[tid.x] + 1;
            B[tid.x] = B[tid.x] + 1;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "test",
        "test.slang",
        userSource,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK(programLayout != nullptr);

    auto globalParamsLayout = programLayout->getGlobalParamsVarLayout()->getTypeLayout();
    SLANG_CHECK(globalParamsLayout != nullptr);

    // Two distinct logical descriptor sets are expected: one for A
    // (space=0) and one for B (space=3).
    SlangInt setCount = globalParamsLayout->getDescriptorSetCount();
    SLANG_CHECK(setCount == 2);

    bool seenSpace0 = false;
    bool seenSpace3 = false;
    for (SlangInt i = 0; i < setCount; ++i)
    {
        SlangInt spaceOffset = globalParamsLayout->getDescriptorSetSpaceOffset(i);
        if (spaceOffset == 0)
            seenSpace0 = true;
        if (spaceOffset == 3)
            seenSpace3 = true;
    }
    SLANG_CHECK(seenSpace0);
    SLANG_CHECK(seenSpace3);
}
