// unit-test-parameter-usage-reflection.cpp

#include "core/slang-io.h"
#include "core/slang-process.h"
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

namespace
{
struct ParameterLocationExpectation
{
    SlangParameterCategory category;
    SlangUInt spaceIndex;
    SlangUInt registerIndex;
    bool expectedUsed;
};

// Check each expectation against the per-entry-point metadata of `main` in `source`. We query
// `getEntryPointMetadata` rather than `getTargetMetadata` because renderers build one pipeline
// layout per entry point from it, which is where a false "used" adds a phantom binding.
void checkEntryPointParameterLocationUsage(
    slang::IGlobalSession* globalSession,
    const char* source,
    SlangCompileTarget format,
    const char* profile,
    ParameterLocationExpectation const* expectations,
    Index expectationCount)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = format;
    targetDesc.profile = profile ? globalSession->findProfile(profile) : SLANG_PROFILE_UNKNOWN;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // Each step prints the compiler's diagnostics before checking it, so a failing case says why.
    ComPtr<slang::IBlob> diagnosticBlob;
    auto printDiagnostics = [&]()
    {
        if (diagnosticBlob && diagnosticBlob->getBufferSize())
            fprintf(stderr, "%s\n", (const char*)diagnosticBlob->getBufferPointer());
    };

    auto module = session->loadModuleFromSourceString(
        "isParameterLocationUsedSplitResources",
        "split-resources.slang",
        source,
        diagnosticBlob.writeRef());
    printDiagnostics();
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        module->findAndCheckEntryPoint(
            "main",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            components,
            2,
            compositeProgram.writeRef(),
            diagnosticBlob.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    auto linkResult = compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    printDiagnostics();
    SLANG_CHECK_ABORT(linkResult == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    auto metadataResult =
        linkedProgram->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnosticBlob.writeRef());
    printDiagnostics();
    SLANG_CHECK_ABORT(metadataResult == SLANG_OK);

    for (Index i = 0; i < expectationCount; ++i)
    {
        auto const& expectation = expectations[i];
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
} // namespace

// A constant buffer that holds both ordinary data and resources is split by type legalization
// into a data-only buffer plus one parameter per resource. A resource field that the shader never
// reads must be reported as unused, even though the data-only buffer is still live.
SLANG_UNIT_TEST(isParameterLocationUsedMixedConstantBuffer)
{
    auto globalSession = unitTestContext->slangGlobalSession;

    // The entry-point `uniform` parameters are collected into one implicit constant buffer.
    const char* entryPointSource = R"(
        RWStructuredBuffer<float4> Output;

        [shader("compute")]
        [numthreads(64, 1, 1)]
        void main(uniform float4 tint,
                  uniform StructuredBuffer<float4> usedBuf,
                  uniform StructuredBuffer<float4> unusedBuf,
                  uint3 tid : SV_DispatchThreadID)
        {
            Output[tid.x] = usedBuf[tid.x] * tint;
        }
    )";

    // On SPIR-V the entry-point constant buffer is a push-constant block with no binding, so
    // `usedBuf` and `unusedBuf` are at bindings 1 and 2.
    {
        ParameterLocationExpectation expectations[] = {
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 0, true},
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 1, true},
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 2, false},
        };
        checkEntryPointParameterLocationUsage(
            globalSession,
            entryPointSource,
            SLANG_SPIRV,
            "spirv_1_5",
            expectations,
            SLANG_COUNT_OF(expectations));
    }

    // On WGSL the entry-point constant buffer takes binding 1, so `usedBuf` and `unusedBuf` are at
    // bindings 2 and 3.
    {
        ParameterLocationExpectation expectations[] = {
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 0, true},
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 1, true},
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 2, true},
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 3, false},
        };
        checkEntryPointParameterLocationUsage(
            globalSession,
            entryPointSource,
            SLANG_WGSL,
            nullptr,
            expectations,
            SLANG_COUNT_OF(expectations));
    }

    // On HLSL `usedBuf` and `unusedBuf` are at t0 and t1, and the constant buffer is at b0.
    {
        ParameterLocationExpectation expectations[] = {
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 0, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 1, false},
            {SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER, 0, 0, true},
            {SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS, 0, 0, true},
        };
        checkEntryPointParameterLocationUsage(
            globalSession,
            entryPointSource,
            SLANG_HLSL,
            "sm_5_0",
            expectations,
            SLANG_COUNT_OF(expectations));
    }

    // A global `ConstantBuffer<P>` that mixes data and resources is split the same way.
    const char* globalSource = R"(
        struct P
        {
            float4 tint;
            StructuredBuffer<float4> usedBuf;
            StructuredBuffer<float4> unusedBuf;
        };

        RWStructuredBuffer<float4> Output;
        ConstantBuffer<P> gP;

        [shader("compute")]
        [numthreads(64, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
        {
            Output[tid.x] = gP.usedBuf[tid.x] * gP.tint;
        }
    )";

    // `gP` takes binding 1 for its own buffer, then `usedBuf` and `unusedBuf` take 2 and 3.
    {
        ParameterLocationExpectation expectations[] = {
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 0, true},
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 1, true},
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 2, true},
            {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 3, false},
        };
        checkEntryPointParameterLocationUsage(
            globalSession,
            globalSource,
            SLANG_SPIRV,
            "spirv_1_5",
            expectations,
            SLANG_COUNT_OF(expectations));
    }
}

// An array of structs that holds resources is split into one array per resource field. On D3D
// each such array uses one register per element, so every register of the array must be
// reported as used.
SLANG_UNIT_TEST(isParameterLocationUsedArrayOfStructsWithResources)
{
    auto globalSession = unitTestContext->slangGlobalSession;

    // `gS.arr.t` becomes `Texture2D[2]` at t0..t1, and `gS.arr.u` becomes `Texture2D[2]` at
    // t2..t3.
    const char* globalSource = R"(
        struct Inner
        {
            Texture2D t;
            Texture2D u;
        };
        struct S
        {
            Inner arr[2];
        };

        RWStructuredBuffer<float4> Output;
        S gS;

        [shader("compute")]
        [numthreads(64, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
        {
            Output[tid.x] = gS.arr[1].t.Load(int3(0)) + gS.arr[1].u.Load(int3(0));
        }
    )";
    {
        ParameterLocationExpectation expectations[] = {
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 0, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 1, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 2, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 3, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 4, false},
        };
        checkEntryPointParameterLocationUsage(
            globalSession,
            globalSource,
            SLANG_HLSL,
            "sm_5_0",
            expectations,
            SLANG_COUNT_OF(expectations));
    }

    // A `ConstantBuffer<Texture2D>` field in an array of structs becomes a plain texture array,
    // `gS.cb` at t0..t1, and `gS.u` is at t2..t3.
    const char* constantBufferFieldSource = R"(
        struct S
        {
            ConstantBuffer<Texture2D> cb;
            Texture2D u;
        };

        RWStructuredBuffer<float4> Output;
        S gS[2];

        [shader("compute")]
        [numthreads(64, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
        {
            Output[tid.x] = gS[1].cb.Load(int3(0)) + gS[1].u.Load(int3(0));
        }
    )";
    {
        ParameterLocationExpectation expectations[] = {
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 0, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 1, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 2, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 3, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, 4, false},
        };
        checkEntryPointParameterLocationUsage(
            globalSession,
            constantBufferFieldSource,
            SLANG_HLSL,
            "sm_5_0",
            expectations,
            SLANG_COUNT_OF(expectations));
    }

    // An unbounded array of structs gives each resource field its own register space, so the
    // texture array `gS.arr.t` fills space 1 the way a plain `Texture2D textures[]` does.
    const char* unboundedSource = R"(
        struct Inner
        {
            Texture2D t;
        };
        struct S
        {
            Inner arr[];
        };

        RWStructuredBuffer<float4> Output;
        S gS;

        [shader("compute")]
        [numthreads(64, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
        {
            Output[tid.x] = gS.arr[tid.x].t.Load(int3(0));
        }
    )";
    {
        ParameterLocationExpectation expectations[] = {
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 1, 0, true},
            {SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 1, 5, true},
        };
        checkEntryPointParameterLocationUsage(
            globalSession,
            unboundedSource,
            SLANG_HLSL,
            "sm_5_1",
            expectations,
            SLANG_COUNT_OF(expectations));
    }
}
