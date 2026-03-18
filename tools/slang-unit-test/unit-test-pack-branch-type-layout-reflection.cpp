// unit-test-pack-branch-type-layout-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Verify reflection for a terminating `__packBranch` recursion under scalar layout rules.
//
// `Params<1,2,3>` expands to three recursive layers. Each layer contributes:
// - one `uint` in ordinary/uniform data
// - one `Texture2D` in descriptor-slot usage
//
// The test checks both:
// - the standalone reflected type layout for `Params<1,2,3>`
// - the cumulative offsets seen through the linked program's global `gParams`
//   parameter layout, walking the `next.next` path
SLANG_UNIT_TEST(packBranchTypeLayoutReflection)
{
    const char* source = R"(
        struct EmptyNode
        {
        }

        struct Params<let Head : int, let each Tail : int>
        {
            Texture2D tex;
            uint value;
            __packBranch(Tail, EmptyNode, Params<__first(Tail), __trimHead(Tail)>) next;
        }

        ParameterBlock<Params<1, 2, 3>> gParams;
        RWStructuredBuffer<uint> gOutput;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain()
        {
            gOutput[0] = gParams.value + gParams.next.value + gParams.next.next.value;
        }

    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    targetDesc.forceGLSLScalarBufferLayout = true;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "packBranchTypeLayoutReflection",
        "packBranchTypeLayoutReflection.slang",
        source,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef())));
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> compositeProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef())));
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef())));
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    auto reflection = linkedProgram->getLayout();
    SLANG_CHECK_ABORT(reflection != nullptr);

    auto paramsType = reflection->findTypeByName("Params<1,2,3>");
    SLANG_CHECK_ABORT(paramsType != nullptr);

    auto paramsLayout = reflection->getTypeLayout(paramsType);
    SLANG_CHECK_ABORT(paramsLayout != nullptr);

    // Check the scalar-layout byte size of `Params<1,2,3>`.
    // There are three recursive layers, each contributing exactly one `uint`,
    // so the total ordinary data size should be 3 * 4 = 12 bytes.
    SLANG_CHECK(paramsLayout->getSize() == 12);

    // Check the descriptor-slot usage of `Params<1,2,3>`.
    // Each recursive layer contributes one `Texture2D`, so the total number of
    // texture descriptor slots should be 3.
    SLANG_CHECK(paramsLayout->getSize(slang::ParameterCategory::DescriptorTableSlot) == 3);

    slang::VariableLayoutReflection* gParamsLayout = nullptr;
    for (unsigned i = 0; i < reflection->getParameterCount(); ++i)
    {
        auto param = reflection->getParameterByIndex(i);
        if (param && param->getName() && String(param->getName()) == "gParams")
        {
            gParamsLayout = param;
            break;
        }
    }
    SLANG_CHECK_ABORT(gParamsLayout != nullptr);

    auto paramsValueLayout = gParamsLayout->getTypeLayout()->getElementTypeLayout();
    SLANG_CHECK_ABORT(paramsValueLayout != nullptr);
    auto nextIndex = paramsValueLayout->findFieldIndexByName("next");
    SLANG_CHECK_ABORT(nextIndex != -1);
    auto nextLayout = paramsValueLayout->getFieldByIndex(nextIndex);
    SLANG_CHECK_ABORT(nextLayout != nullptr);

    auto nextValueLayout = nextLayout->getTypeLayout();
    SLANG_CHECK_ABORT(nextValueLayout != nullptr);
    auto nextNextIndex = nextValueLayout->findFieldIndexByName("next");
    SLANG_CHECK_ABORT(nextNextIndex != -1);
    auto nextNextLayout = nextValueLayout->getFieldByIndex(nextNextIndex);
    SLANG_CHECK_ABORT(nextNextLayout != nullptr);

    auto nextNextByteOffset =
        gParamsLayout->getOffset() + nextLayout->getOffset() + nextNextLayout->getOffset();
    auto nextNextDescriptorOffset =
        gParamsLayout->getOffset(slang::ParameterCategory::DescriptorTableSlot) +
        nextLayout->getOffset(slang::ParameterCategory::DescriptorTableSlot) +
        nextNextLayout->getOffset(slang::ParameterCategory::DescriptorTableSlot);

    // Check the cumulative byte offset of `gParams.next.next`.
    // The top-level `gParams` field contributes 0 bytes, while each `next` field
    // comes after one `Texture2D` and one `uint`. Under scalar layout only the
    // `uint` contributes to byte size, so the path advances by 4 bytes per layer:
    // 0 + 4 + 4 = 8.
    SLANG_CHECK(nextNextByteOffset == 8);

    // Check the cumulative descriptor-slot offset of `gParams.next.next`.
    // `gParams` starts after the implicit constant buffer resource of the param block, contributing
    // 1 slot. Each recursive `next` then skips past exactly one `Texture2D`, contributing one
    // additional slot per layer, so the total is 1 + 1 + 1 = 3.
    SLANG_CHECK(nextNextDescriptorOffset == 3);
}
