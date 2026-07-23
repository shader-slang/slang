// unit-test-parameter-usage-reflection.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Slang;

// Tests the isParameterLocationUsed and IParameterByteRangeUsageInfo reflection APIs.

SLANG_UNIT_TEST(isParameterLocationUsedReflection)
{
    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        Texture2D g_tex : register(t0);
        struct Params {
            Texture2D tex2;
            Texture2D tex3;
        };
        struct Material
        {
            float2 uvScale;
            float2 uvBias;
        }
        ParameterBlock<Params> gParams;
        ConstantBuffer<Material> gcMaterial;
        ParameterBlock<Material> gMaterial;
        [shader("fragment")]
        float4 fragMain(float4 pos:SV_Position, float unused:COLOR0, float4 used:COLOR1) : SV_Target
        {
            return g_tex.Load(int3(0, 0, 0)) + gParams.tex3.Load(int3(0)) + used + gMaterial.uvScale.x + gcMaterial.uvBias.x;
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
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
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "fragMain",
        SLANG_STAGE_FRAGMENT,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
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
    compositeProgram->link(linkedProgram.writeRef(), nullptr);

    ComPtr<slang::IMetadata> metadata;
    linkedProgram->getTargetMetadata(0, metadata.writeRef(), nullptr);

    bool isUsed = false;
    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 0, isUsed);
    SLANG_CHECK(isUsed);

    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 1, isUsed);
    SLANG_CHECK(isUsed);

    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 2, isUsed);
    SLANG_CHECK(!isUsed);

    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 1, 0, isUsed);
    SLANG_CHECK(!isUsed);

    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 1, 1, isUsed);
    SLANG_CHECK(isUsed);

    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 2, 0, isUsed);
    SLANG_CHECK(isUsed);

    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, 0, isUsed);
    SLANG_CHECK(!isUsed);

    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, 1, isUsed);
    SLANG_CHECK(isUsed);
}

SLANG_UNIT_TEST(isParameterLocationUsedParameterBlockConstantBuffer)
{
    const char* shaderSource = R"(
        struct Foo {
            float a;
            Texture2D<float> b;
        };

        struct Bar {
            Texture2D<float4> c;
        };

        [[vk::binding(0, 5)]]
        ParameterBlock<Foo> foo;

        [[vk::binding(0, 10)]]
        ParameterBlock<Bar> bar;

        [shader("fragment")]
        float main(): SV_Target {
            return foo.a;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_3");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "isParameterLocationUsedParameterBlockConstantBuffer",
        "pb.slang",
        shaderSource,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK(
        module->findAndCheckEntryPoint(
            "main",
            SLANG_STAGE_FRAGMENT,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            2,
            compositeProgram.writeRef(),
            diagnosticBlob.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK(compositeProgram->link(linkedProgram.writeRef(), nullptr) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(linkedProgram->getTargetMetadata(0, metadata.writeRef(), nullptr) == SLANG_OK);

    struct Expectation
    {
        SlangParameterCategory category;
        SlangUInt spaceIndex;
        SlangUInt registerIndex;
        bool expectedUsed;
    };

    Expectation expectations[] = {
        {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 5, 0, true},
        {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 5, 1, false},
        {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 10, 0, false},
    };

    for (auto expectation : expectations)
    {
        bool isUsed = false;
        SLANG_CHECK(
            metadata->isParameterLocationUsed(
                expectation.category,
                expectation.spaceIndex,
                expectation.registerIndex,
                isUsed) == SLANG_OK);
        SLANG_CHECK(isUsed == expectation.expectedUsed);
    }
}

// Resolve a variable layout to its parent binding location, the correlation
// a host performs with the main reflection API.
static void getParentBindingLocation(
    slang::VariableLayoutReflection* var,
    SlangUInt& outSpaceIndex,
    SlangUInt& outRegisterIndex)
{
    bool hasConstantBuffer = false;
    bool hasDescriptorTableSlot = false;
    for (unsigned int i = 0, n = var->getCategoryCount(); i < n; ++i)
    {
        switch (var->getCategoryByIndex(i))
        {
        case slang::ParameterCategory::ConstantBuffer:
            hasConstantBuffer = true;
            break;
        case slang::ParameterCategory::DescriptorTableSlot:
            hasDescriptorTableSlot = true;
            break;
        default:
            break;
        }
    }
    if (hasConstantBuffer)
    {
        outSpaceIndex = var->getBindingSpace(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER);
        outRegisterIndex = var->getOffset(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER);
    }
    else if (hasDescriptorTableSlot)
    {
        outSpaceIndex = var->getBindingSpace(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
        outRegisterIndex = var->getOffset(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
    }
    else
    {
        outSpaceIndex = var->getBindingSpace(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER);
        outRegisterIndex = 0;
    }
}

// Exercises the uniform byte range API IParameterByteRangeUsageInfo, which the
// isParameterLocationUsed tests do not touch. The constant buffer reads
// usedColor (bytes [0, 16)) and usedScalar (bytes [32, 36)) and leaves
// unusedColor [16, 32) and unusedScalar [36, 40) untouched, so the
// reported ranges must be exactly those two reads and must exclude the
// unread bytes. The order is deterministic, ascending by offset.
SLANG_UNIT_TEST(parameterUsageUniformByteRanges)
{
    const char* userSourceBody = R"(
        cbuffer Params
        {
            float4 usedColor;
            float4 unusedColor;
            float  usedScalar;
            float  unusedScalar;
        }
        RWStructuredBuffer<float4> outBuf;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint tid : SV_DispatchThreadID)
        {
            outBuf[tid] = usedColor * usedScalar;
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
        "uniformByteRanges",
        "uniformByteRanges.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "main",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
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
    compositeProgram->link(linkedProgram.writeRef(), nullptr);
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IParameterByteRangeUsageInfo> parameterUsage;
    SLANG_CHECK(
        linkedProgram->queryInterface(
            slang::IParameterByteRangeUsageInfo::getTypeGuid(),
            (void**)parameterUsage.writeRef()) == SLANG_OK);
    SLANG_CHECK(parameterUsage != nullptr);

    auto programLayout = linkedProgram->getLayout(0);
    SLANG_CHECK(programLayout != nullptr);

    slang::VariableLayoutReflection* paramsVar = nullptr;
    for (unsigned int i = 0, n = programLayout->getParameterCount(); i < n; ++i)
    {
        auto p = programLayout->getParameterByIndex(i);
        if (strcmp(p->getName(), "Params") == 0)
        {
            paramsVar = p;
            break;
        }
    }
    SLANG_CHECK(paramsVar != nullptr);

    SlangUInt paramsSpace = 0;
    SlangUInt paramsRegister = 0;
    getParentBindingLocation(paramsVar, paramsSpace, paramsRegister);

    SLANG_CHECK(parameterUsage->getUsedByteRangeCount(0, 0, paramsSpace, paramsRegister) == 2);

    slang::ByteRange range0 = {};
    SLANG_CHECK(
        parameterUsage->getUsedByteRange(0, 0, paramsSpace, paramsRegister, 0, &range0) ==
        SLANG_OK);
    SLANG_CHECK(range0.offset == 0);
    SLANG_CHECK(range0.size == 16);

    slang::ByteRange range1 = {};
    SLANG_CHECK(
        parameterUsage->getUsedByteRange(0, 0, paramsSpace, paramsRegister, 1, &range1) ==
        SLANG_OK);
    SLANG_CHECK(range1.offset == 32);
    SLANG_CHECK(range1.size == 4);

    // An out of range index is rejected rather than returning a stale range.
    slang::ByteRange ignored = {};
    SLANG_CHECK(
        parameterUsage->getUsedByteRange(0, 0, paramsSpace, paramsRegister, 2, &ignored) ==
        SLANG_E_INVALID_ARG);
}

// A push constant carries neither a ConstantBuffer nor a
// DescriptorTableSlot binding, so the IR pass has no parent binding
// identity to scope byte ranges against and emits no record. With no
// record IParameterByteRangeUsageInfo reports the whole parameter as used: a single
// range at offset 0 whose size is SLANG_UNBOUNDED_SIZE, so a host binds
// the whole block rather than stripping bytes it cannot prove are unread.
SLANG_UNIT_TEST(parameterUsageUniformWholeParameterFallback)
{
    const char* userSourceBody = R"(
        struct PushData
        {
            float used;
            float unused;
        }
        [[vk::push_constant]] ConstantBuffer<PushData> pc;
        RWStructuredBuffer<float> outBuf;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint tid : SV_DispatchThreadID)
        {
            outBuf[tid] = pc.used;
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
        "uniformWholeParameterFallback",
        "uniformWholeParameterFallback.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "main",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
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
    compositeProgram->link(linkedProgram.writeRef(), nullptr);
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IParameterByteRangeUsageInfo> parameterUsage;
    SLANG_CHECK(
        linkedProgram->queryInterface(
            slang::IParameterByteRangeUsageInfo::getTypeGuid(),
            (void**)parameterUsage.writeRef()) == SLANG_OK);
    SLANG_CHECK(parameterUsage != nullptr);

    auto programLayout = linkedProgram->getLayout(0);
    SLANG_CHECK(programLayout != nullptr);

    slang::VariableLayoutReflection* pcVar = nullptr;
    for (unsigned int i = 0, n = programLayout->getParameterCount(); i < n; ++i)
    {
        auto p = programLayout->getParameterByIndex(i);
        if (strcmp(p->getName(), "pc") == 0)
        {
            pcVar = p;
            break;
        }
    }
    SLANG_CHECK(pcVar != nullptr);

    SlangUInt pcSpace = 0;
    SlangUInt pcRegister = 0;
    getParentBindingLocation(pcVar, pcSpace, pcRegister);

    SLANG_CHECK(parameterUsage->getUsedByteRangeCount(0, 0, pcSpace, pcRegister) == 1);

    slang::ByteRange whole = {};
    SLANG_CHECK(parameterUsage->getUsedByteRange(0, 0, pcSpace, pcRegister, 0, &whole) == SLANG_OK);
    SLANG_CHECK(whole.offset == 0);
    SLANG_CHECK(whole.size == SLANG_UNBOUNDED_SIZE);
}

// IParameterByteRangeUsageInfo is a distinct COM interface reached through
// queryInterface, not an extension of the IComponentType vtable, so
// existing callers keep their ABI. This probe checks the interface is
// reachable, that it shares one COM identity with the component type (a
// round trip through ISlangUnknown returns the same pointer), and that a
// base IComponentType method still works through the queried interface.
SLANG_UNIT_TEST(parameterUsageInterfaceIdentity)
{
    const char* userSourceBody = R"(
        cbuffer Params { float4 color; }
        RWStructuredBuffer<float4> outBuf;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint tid : SV_DispatchThreadID)
        {
            outBuf[tid] = color;
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
        "parameterUsageIdentity",
        "parameterUsageIdentity.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "main",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
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
    compositeProgram->link(linkedProgram.writeRef(), nullptr);
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IParameterByteRangeUsageInfo> parameterUsage;
    SLANG_CHECK(
        linkedProgram->queryInterface(
            slang::IParameterByteRangeUsageInfo::getTypeGuid(),
            (void**)parameterUsage.writeRef()) == SLANG_OK);
    SLANG_CHECK(parameterUsage != nullptr);

    ComPtr<ISlangUnknown> unknownFromProgram;
    SLANG_CHECK(
        linkedProgram->queryInterface(
            ISlangUnknown::getTypeGuid(),
            (void**)unknownFromProgram.writeRef()) == SLANG_OK);
    ComPtr<ISlangUnknown> unknownFromUsage;
    SLANG_CHECK(
        parameterUsage->queryInterface(
            ISlangUnknown::getTypeGuid(),
            (void**)unknownFromUsage.writeRef()) == SLANG_OK);
    SLANG_CHECK(unknownFromProgram.get() == unknownFromUsage.get());

    ComPtr<slang::IComponentType> programAgain;
    SLANG_CHECK(
        parameterUsage->queryInterface(
            slang::IComponentType::getTypeGuid(),
            (void**)programAgain.writeRef()) == SLANG_OK);
    SLANG_CHECK(programAgain != nullptr);
    SLANG_CHECK(programAgain->getLayout(0) != nullptr);
}

// Loose global uniforms report usage on the implicit $Globals constant
// buffer reached via getGlobalParamsVarLayout. $Globals shares a space with
// sibling constant buffers, so records must stay distinct: the shader reads
// usedScalar ([0, 4) of $Globals) and UsedCB.Extra.x ([8, 12) of UsedCB),
// and the $Globals query must return only [0, 4).
SLANG_UNIT_TEST(parameterUsageLooseGlobalUniforms)
{
    const char* userSourceBody = R"(
        struct S { uint2 Size; uint2 Extra; };
        ConstantBuffer<S> UsedCB;
        ConstantBuffer<S> UnusedCB;
        uniform float usedScalar;
        uniform float unusedScalar;
        RWStructuredBuffer<float> outBuf;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint tid : SV_DispatchThreadID)
        {
            outBuf[tid] = usedScalar + float(UsedCB.Extra.x);
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
        "looseGlobalUniforms",
        "looseGlobalUniforms.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "main",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
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
    compositeProgram->link(linkedProgram.writeRef(), nullptr);
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IParameterByteRangeUsageInfo> parameterUsage;
    SLANG_CHECK(
        linkedProgram->queryInterface(
            slang::IParameterByteRangeUsageInfo::getTypeGuid(),
            (void**)parameterUsage.writeRef()) == SLANG_OK);

    auto programLayout = linkedProgram->getLayout(0);
    SLANG_CHECK(programLayout != nullptr);

    slang::VariableLayoutReflection* usedCBVar = nullptr;
    for (unsigned int i = 0, n = programLayout->getParameterCount(); i < n; ++i)
    {
        auto p = programLayout->getParameterByIndex(i);
        if (strcmp(p->getName(), "UsedCB") == 0)
            usedCBVar = p;
    }
    SLANG_CHECK(usedCBVar != nullptr);

    SlangUInt usedCBSpace = 0;
    SlangUInt usedCBRegister = 0;
    getParentBindingLocation(usedCBVar, usedCBSpace, usedCBRegister);
    SLANG_CHECK(parameterUsage->getUsedByteRangeCount(0, 0, usedCBSpace, usedCBRegister) == 1);
    slang::ByteRange cbRange = {};
    SLANG_CHECK(
        parameterUsage->getUsedByteRange(0, 0, usedCBSpace, usedCBRegister, 0, &cbRange) ==
        SLANG_OK);
    SLANG_CHECK(cbRange.offset == 8);
    SLANG_CHECK(cbRange.size == 4);

    auto globalVar = programLayout->getGlobalParamsVarLayout();
    SLANG_CHECK(globalVar != nullptr);
    SLANG_CHECK(
        globalVar->getTypeLayout()->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
    SlangUInt globalSpace = 0;
    SlangUInt globalRegister = 0;
    getParentBindingLocation(globalVar, globalSpace, globalRegister);
    SLANG_CHECK(parameterUsage->getUsedByteRangeCount(0, 0, globalSpace, globalRegister) == 1);
    slang::ByteRange globalRange = {};
    SLANG_CHECK(
        parameterUsage->getUsedByteRange(0, 0, globalSpace, globalRegister, 0, &globalRange) ==
        SLANG_OK);
    SLANG_CHECK(globalRange.offset == 0);
    SLANG_CHECK(globalRange.size == 4);
}
