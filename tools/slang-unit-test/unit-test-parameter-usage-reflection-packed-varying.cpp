// unit-test-parameter-usage-reflection-packed-varying.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Test that the isParameterLocationUsed API works for targets that pack
// varying inputs into a struct (WGSL and Metal). See issue #9790.

static void testParameterLocationUsed(SlangCompileTarget format)
{
    const char* userSourceBody = R"(
        Texture2D g_tex;
        SamplerState g_sampler;

        struct VSInput
        {
            float2 pos : POSITION;
            float2 uv : TEXCOORD;
            float4 col : COLOR;
        };

        struct PSInput
        {
            float4 pos : SV_Position;
            float2 uv : TEXCOORD;
            float4 col : COLOR;
        };

        [shader("vertex")]
        PSInput vertexMain(VSInput vsIn)
        {
            PSInput psIn;
            psIn.pos = float4(vsIn.pos, 0.0, 1.0);
            psIn.uv = vsIn.uv;
            psIn.col = vsIn.col;
            return psIn;
        }

        [shader("fragment")]
        float4 fragmentMain(PSInput psIn) : SV_Target
        {
            return g_tex.Sample(g_sampler, psIn.uv) * psIn.col;
        }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = format;
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

    // Test vertex shader varying inputs.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "vertexMain",
            SLANG_STAGE_VERTEX,
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
        SLANG_CHECK(SLANG_SUCCEEDED(
            compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef())));
        SLANG_CHECK(linkedProgram != nullptr);

        ComPtr<slang::IMetadata> metadata;
        SLANG_CHECK(SLANG_SUCCEEDED(
            linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnosticBlob.writeRef())));
        SLANG_CHECK(metadata != nullptr);

        auto checkUsed = [&](SlangParameterCategory category, SlangUInt index, bool expected)
        {
            bool isUsed = !expected;
            SLANG_CHECK(
                SLANG_SUCCEEDED(metadata->isParameterLocationUsed(category, 0, index, isUsed)));
            SLANG_CHECK(isUsed == expected);
        };

        // All three vertex inputs (POSITION=0, TEXCOORD=1, COLOR=2) are used.
        checkUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, true);
        checkUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 1, true);
        checkUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 2, true);

        // Location 3 should not be used.
        checkUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 3, false);
    }

    // Test fragment shader varying inputs.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "fragmentMain",
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
        SLANG_CHECK(SLANG_SUCCEEDED(
            compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef())));
        SLANG_CHECK(linkedProgram != nullptr);

        ComPtr<slang::IMetadata> metadata;
        SLANG_CHECK(SLANG_SUCCEEDED(
            linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnosticBlob.writeRef())));
        SLANG_CHECK(metadata != nullptr);

        auto checkUsed = [&](SlangParameterCategory category, SlangUInt index, bool expected)
        {
            bool isUsed = !expected;
            SLANG_CHECK(
                SLANG_SUCCEEDED(metadata->isParameterLocationUsed(category, 0, index, isUsed)));
            SLANG_CHECK(isUsed == expected);
        };

        // Fragment varying inputs: TEXCOORD=0 and COLOR=1 are used.
        // SV_Position is a system value, not a user varying input.
        checkUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, true);
        checkUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 1, true);

        // Location 2 should not be used (only 2 user varying inputs).
        checkUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 2, false);

        // Verify resource bindings for the texture and sampler.
        // WGSL uses DescriptorTableSlot; Metal uses ShaderResource + SamplerState.
        if (format == SLANG_WGSL)
        {
            checkUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, true);
            checkUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 1, true);
        }
        else if (format == SLANG_METAL)
        {
            checkUsed(SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE, 0, true);
            checkUsed(SLANG_PARAMETER_CATEGORY_SAMPLER_STATE, 0, true);
        }
    }
}

SLANG_UNIT_TEST(isParameterLocationUsedReflectionWGSL)
{
    testParameterLocationUsed(SLANG_WGSL);
}

SLANG_UNIT_TEST(isParameterLocationUsedReflectionMetal)
{
    testParameterLocationUsed(SLANG_METAL);
}
