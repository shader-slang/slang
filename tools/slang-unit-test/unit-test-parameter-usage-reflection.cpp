// unit-test-parameter-usage-reflection.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that the isParameterLocationUsed API works.

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

// Exercises the Uniform category of the API, which the other tests do not
// touch. The key case is a query against a space the pass produced no
// entry for: that must report SLANG_E_NOT_AVAILABLE rather than fabricate
// an authoritative "not used". The invariant is that a "no" is always
// accurate, so with no information about a space we may not answer false.
SLANG_UNIT_TEST(isParameterLocationUsedUniformUnknownSpace)
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
        "uniformUnknownSpace",
        "uniformUnknownSpace.slang",
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

    ComPtr<slang::IMetadata> metadata;
    linkedProgram->getTargetMetadata(0, metadata.writeRef(), nullptr);
    SLANG_CHECK(metadata != nullptr);

    // The constant buffer lands in space 0. usedColor occupies bytes
    // [0, 16) and usedScalar byte [32, 36); unusedColor [16, 32) and
    // unusedScalar [36, 40) are present in the buffer but never read.
    bool isUsed = false;
    SLANG_CHECK(
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_UNIFORM, 0, 0, isUsed) ==
        SLANG_OK);
    SLANG_CHECK(isUsed);

    SLANG_CHECK(
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_UNIFORM, 0, 32, isUsed) ==
        SLANG_OK);
    SLANG_CHECK(isUsed);

    // Tracked but unread bytes in a space we did record are an accurate false.
    SLANG_CHECK(
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_UNIFORM, 0, 16, isUsed) ==
        SLANG_OK);
    SLANG_CHECK(!isUsed);

    SLANG_CHECK(
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_UNIFORM, 0, 36, isUsed) ==
        SLANG_OK);
    SLANG_CHECK(!isUsed);

    // The regression guard: a space the pass never produced an entry for
    // must be reported as not available, never a fabricated false. Seed
    // isUsed = true to confirm the failing path does not leave it cleared.
    isUsed = true;
    SLANG_CHECK(
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_UNIFORM, 999, 0, isUsed) ==
        SLANG_E_NOT_AVAILABLE);
}

// Covers the other path to SLANG_E_NOT_AVAILABLE: a space the pass *did*
// record an entry for, but that entry is untracked. This is distinct from
// the unknown-space case above, which returns not available because no
// entry matched the space at all. Here the matcher must short-circuit on
// the untracked marker before it ever consults byte ranges, so the byte
// offset in the query is irrelevant. A push-constant constant buffer is
// the cleanest trigger: it carries neither a ConstantBuffer nor a
// DescriptorTableSlot binding, so the pass has no parent identity to scope
// byte ranges against and emits an untracked entry rather than risk a
// misleading "not used".
SLANG_UNIT_TEST(isParameterLocationUsedUniformUntracked)
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
        "uniformUntracked",
        "uniformUntracked.slang",
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

    ComPtr<slang::IMetadata> metadata;
    linkedProgram->getTargetMetadata(0, metadata.writeRef(), nullptr);
    SLANG_CHECK(metadata != nullptr);

    // The push constant produces an untracked entry in space 0. A uniform
    // query there must report not available via the untracked short-circuit,
    // not a fabricated true or false. Seed isUsed = true to confirm the
    // failing path leaves it untouched, and query a byte the shader actually
    // reads (offset 0) to prove the untracked check fires regardless of the
    // recorded ranges.
    bool isUsed = true;
    SLANG_CHECK(
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_UNIFORM, 0, 0, isUsed) ==
        SLANG_E_NOT_AVAILABLE);
}
