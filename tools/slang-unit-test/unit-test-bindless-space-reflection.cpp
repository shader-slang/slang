// unit-test-bindless-space-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that the reflection API correctly returns the bindless space index.

struct BindlessSpaceExpectation
{
    bool expectsReservedSpace;
    bool expectsExactSpaceIndex;
    SlangInt exactSpaceIndex;
};

static BindlessSpaceExpectation _expectReservedBindlessSpaceAt(SlangInt index)
{
    return {true, true, index};
}

static BindlessSpaceExpectation _expectAnyReservedBindlessSpace()
{
    return {true, false, 0};
}

static void _checkBindlessSpaceReflection(
    const char* userSource,
    BindlessSpaceExpectation bindlessSpaceExpectation,
    bool expectedUsesBindlessResourceHeap,
    const slang::CompilerOptionEntry* compilerOptions = nullptr,
    uint32_t compilerOptionCount = 0)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = compilerOptions;
    sessionDesc.compilerOptionEntryCount = compilerOptionCount;

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

    auto bindlessSpaceIndex = programLayout->getBindlessSpaceIndex();
    if (bindlessSpaceExpectation.expectsReservedSpace)
    {
        SLANG_CHECK(bindlessSpaceIndex >= 0);
        if (bindlessSpaceExpectation.expectsExactSpaceIndex)
            SLANG_CHECK(bindlessSpaceIndex == bindlessSpaceExpectation.exactSpaceIndex);
    }
    else
    {
        SLANG_CHECK(bindlessSpaceIndex == -1);
    }

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnosticBlob.writeRef()) ==
        SLANG_OK);
    SLANG_CHECK(metadata != nullptr);

    auto bindlessMetadata = static_cast<slang::IBindlessResourceMetadata*>(
        metadata->castAs(slang::IBindlessResourceMetadata::getTypeGuid()));
    SLANG_CHECK(bindlessMetadata != nullptr);
    SLANG_CHECK(bindlessMetadata->usesBindlessResourceHeap() == expectedUsesBindlessResourceHeap);
}

SLANG_UNIT_TEST(bindlessSpaceReflection)
{
    const char* userSource = R"(
        RWStructuredBuffer<float> gInput;
        RWStructuredBuffer<float> gOutput;

        struct P
        {
            Texture2D.Handle t;
        };

        ParameterBlock<P> p1;

        [Shader("compute")]
        [NumThreads(4, 1, 1)]
        void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
        {
            uint tid = dispatchThreadID.x;
            gOutput[tid] = gInput[tid] + p1.t.Load(int3(0,0,0)).x;
        }
    )";

    // The shader has:
    // - gInput and gOutput at space 0 (default)
    // - ParameterBlock<P> p1 at space 1
    // So the bindless space should be allocated at space 2.
    _checkBindlessSpaceReflection(userSource, _expectReservedBindlessSpaceAt(2), true);
}

SLANG_UNIT_TEST(bindlessSpaceMetadataWithoutDescriptorHandle)
{
    const char* userSource = R"(
        RWStructuredBuffer<float> gInput;
        RWStructuredBuffer<float> gOutput;

        [Shader("compute")]
        [NumThreads(4, 1, 1)]
        void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
        {
            uint tid = dispatchThreadID.x;
            gOutput[tid] = gInput[tid] + 1.0f;
        }
    )";

    // Reflection still reserves a stable bindless space for descriptor-handle-capable
    // targets, but post-emit metadata reports that no bindless heap path survived.
    _checkBindlessSpaceReflection(userSource, _expectAnyReservedBindlessSpace(), false);
}

SLANG_UNIT_TEST(bindlessSpaceMetadataWithStrippedNameHints)
{
    const char* userSource = R"(
        RWStructuredBuffer<float> gInput;
        RWStructuredBuffer<float> gOutput;

        struct P
        {
            Texture2D.Handle t;
        };

        ParameterBlock<P> p1;

        [Shader("compute")]
        [NumThreads(4, 1, 1)]
        void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
        {
            uint tid = dispatchThreadID.x;
            gOutput[tid] = gInput[tid] + p1.t.Load(int3(0,0,0)).x;
        }
    )";

    slang::CompilerOptionEntry obfuscationOption = {};
    obfuscationOption.name = slang::CompilerOptionName::Obfuscate;
    obfuscationOption.value.kind = slang::CompilerOptionValueKind::Int;
    obfuscationOption.value.intValue0 = 1;

    _checkBindlessSpaceReflection(
        userSource,
        _expectReservedBindlessSpaceAt(2),
        true,
        &obfuscationOption,
        1);
}

SLANG_UNIT_TEST(bindlessSpaceMetadataWithTexelPointerHeap)
{
    // Use format id 37 (`r32ui`) so the SPIR-V texel-pointer path validates.
    const char* userSource = R"(
        RWStructuredBuffer<uint> gOutput;

        struct P
        {
            RWTexture2D<uint, 0, 37>.Handle t;
        };

        ParameterBlock<P> p1;

        [Shader("compute")]
        [NumThreads(1, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            uint previousValue;
            InterlockedAdd(p1.t[dispatchThreadID.xy], 1, previousValue);
            gOutput[0] = previousValue;
        }
    )";

    _checkBindlessSpaceReflection(userSource, _expectReservedBindlessSpaceAt(2), true);
}
