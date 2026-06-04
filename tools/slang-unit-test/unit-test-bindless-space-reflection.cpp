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

static const char* _getTextureHandleSource()
{
    return R"(
        RWStructuredBuffer<float> gOutput;

        struct P
        {
            Texture2D.Handle t;
        };

        ParameterBlock<P> p1;

        [Shader("compute")]
        [NumThreads(1, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutput[0] = p1.t.Load(int3(0, 0, 0)).x;
        }
    )";
}

static const char* _getTextureSamplerHandleSource()
{
    return R"(
        RWStructuredBuffer<float> gOutput;

        struct P
        {
            Texture2D.Handle t;
            SamplerState.Handle s;
        };

        ParameterBlock<P> p1;

        [Shader("compute")]
        [NumThreads(2, 2, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutput[0] = p1.t.Sample(p1.s, float2(0.0f, 0.0f)).x;
        }
    )";
}

static const char* _getSamplerHandleSource()
{
    return R"(
        Texture2D gTexture;
        RWStructuredBuffer<float> gOutput;

        struct P
        {
            SamplerState.Handle s;
        };

        ParameterBlock<P> p1;

        [Shader("compute")]
        [NumThreads(2, 2, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutput[0] = gTexture.Sample(p1.s, float2(0.0f, 0.0f)).x;
        }
    )";
}

static void _checkBindlessSpaceReflection(
    const char* userSource,
    BindlessSpaceExpectation bindlessSpaceExpectation,
    bool expectedUsesBindlessResourceHeap,
    const slang::CompilerOptionEntry* compilerOptions = nullptr,
    uint32_t compilerOptionCount = 0,
    SlangCompileTarget targetFormat = SLANG_SPIRV,
    const char* profileName = "spirv_1_5",
    const char* targetCapabilityName = nullptr,
    bool queryMetadataBeforeLayout = false)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry targetCapabilityOption = {};

    slang::TargetDesc targetDesc = {};
    targetDesc.format = targetFormat;
    if (profileName)
        targetDesc.profile = globalSession->findProfile(profileName);
    if (targetCapabilityName)
    {
        auto capability = globalSession->findCapability(targetCapabilityName);
        SLANG_CHECK(capability != SLANG_CAPABILITY_UNKNOWN);
        targetCapabilityOption.name = slang::CompilerOptionName::Capability;
        targetCapabilityOption.value.kind = slang::CompilerOptionValueKind::Int;
        targetCapabilityOption.value.intValue0 = int32_t(capability);
        targetDesc.compilerOptionEntries = &targetCapabilityOption;
        targetDesc.compilerOptionEntryCount = 1;
    }

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = compilerOptions;
    sessionDesc.compilerOptionEntryCount = compilerOptionCount;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "test",
        "test.slang",
        userSource,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    auto checkBindlessSpaceLayout = [&]()
    {
        auto programLayout = linkedProgram->getLayout();
        SLANG_CHECK_ABORT(programLayout != nullptr);

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
    };

    if (!queryMetadataBeforeLayout)
        checkBindlessSpaceLayout();

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK_ABORT(
        linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnosticBlob.writeRef()) ==
        SLANG_OK);
    SLANG_CHECK_ABORT(metadata != nullptr);

    auto bindlessMetadata = static_cast<slang::IBindlessResourceMetadata*>(
        metadata->castAs(slang::IBindlessResourceMetadata::getTypeGuid()));
    SLANG_CHECK_ABORT(bindlessMetadata != nullptr);
    SLANG_CHECK(bindlessMetadata->usesBindlessResourceHeap() == expectedUsesBindlessResourceHeap);

    if (queryMetadataBeforeLayout)
        checkBindlessSpaceLayout();
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

SLANG_UNIT_TEST(bindlessSpaceMetadataWithUnboundedResourceArray)
{
    const char* userSource = R"(
        Texture2D<float4> gTextures[];
        RWStructuredBuffer<float> gOutput;

        [Shader("compute")]
        [NumThreads(1, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutput[0] = gTextures[0].Load(int3(0, 0, 0)).x;
        }
    )";

    _checkBindlessSpaceReflection(userSource, _expectAnyReservedBindlessSpace(), false);
}

SLANG_UNIT_TEST(bindlessSpaceMetadataWithUnboundedResourceArrayInRequestedSpace)
{
    const char* userSource = R"(
        Texture2D<float4> gTextures[] : register(t0, space0);
        RWStructuredBuffer<float> gOutput : register(u0, space1);

        [Shader("compute")]
        [NumThreads(1, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutput[0] = gTextures[0].Load(int3(0, 0, 0)).x;
        }
    )";

    slang::CompilerOptionEntry bindlessSpaceOption = {};
    bindlessSpaceOption.name = slang::CompilerOptionName::BindlessSpaceIndex;
    bindlessSpaceOption.value.kind = slang::CompilerOptionValueKind::Int;
    bindlessSpaceOption.value.intValue0 = 0;

    // The user array occupies the requested bindless space, so layout reservation moves the
    // system bindless space to the next free space instead of treating the user array as a heap.
    _checkBindlessSpaceReflection(
        userSource,
        _expectReservedBindlessSpaceAt(2),
        false,
        &bindlessSpaceOption,
        1,
        SLANG_SPIRV,
        "spirv_1_5",
        nullptr,
        true);
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

SLANG_UNIT_TEST(bindlessSpaceMetadataWithHelperFunctionHandleUse)
{
    const char* userSource = R"(
        RWStructuredBuffer<float> gOutput;

        struct P
        {
            Texture2D.Handle t;
        };

        ParameterBlock<P> p1;

        float readTexture(Texture2D.Handle textureHandle)
        {
            return textureHandle.Load(int3(0, 0, 0)).x;
        }

        [Shader("compute")]
        [NumThreads(1, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutput[0] = readTexture(p1.t);
        }
    )";

    _checkBindlessSpaceReflection(userSource, _expectReservedBindlessSpaceAt(2), true);
}

SLANG_UNIT_TEST(bindlessSpaceMetadataHLSLResourceAndSamplerHandles)
{
    _checkBindlessSpaceReflection(
        _getTextureSamplerHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_HLSL,
        "sm_6_6");
}

SLANG_UNIT_TEST(bindlessSpaceMetadataSPIRVDescriptorHeapResourceAndSamplerHandles)
{
    _checkBindlessSpaceReflection(
        _getTextureSamplerHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_SPIRV,
        "spirv_1_5",
        "spvDescriptorHeapEXT");
}

SLANG_UNIT_TEST(bindlessSpaceMetadataSPIRVDescriptorHeapResourceHandle)
{
    _checkBindlessSpaceReflection(
        _getTextureHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_SPIRV,
        "spirv_1_5",
        "spvDescriptorHeapEXT");
}

SLANG_UNIT_TEST(bindlessSpaceMetadataSPIRVDescriptorHeapSamplerHandle)
{
    _checkBindlessSpaceReflection(
        _getSamplerHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_SPIRV,
        "spirv_1_5",
        "spvDescriptorHeapEXT");
}

SLANG_UNIT_TEST(bindlessSpaceMetadataSPIRVNVResourceHandle)
{
    _checkBindlessSpaceReflection(
        _getTextureHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_SPIRV,
        "spirv_1_5",
        "spvBindlessTextureNV");
}

SLANG_UNIT_TEST(bindlessSpaceMetadataWGSLDynamicResourceHeap)
{
    _checkBindlessSpaceReflection(
        _getTextureHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_WGSL,
        nullptr);
}

SLANG_UNIT_TEST(bindlessSpaceMetadataNativeHandleCasts)
{
    _checkBindlessSpaceReflection(
        _getTextureHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_METAL,
        "metal");
    _checkBindlessSpaceReflection(
        _getTextureHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_CUDA_SOURCE,
        nullptr);
    _checkBindlessSpaceReflection(
        _getTextureHandleSource(),
        _expectAnyReservedBindlessSpace(),
        true,
        nullptr,
        0,
        SLANG_CPP_SOURCE,
        "sm_5_0");
}
