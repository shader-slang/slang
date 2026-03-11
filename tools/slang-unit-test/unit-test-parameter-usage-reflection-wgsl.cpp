// unit-test-parameter-usage-reflection-wgsl.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that the isParameterLocationUsed API works for WGSL target
// with vertex shader varying inputs (issue #9790).

SLANG_UNIT_TEST(isParameterLocationUsedReflectionWGSL)
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
    targetDesc.format = SLANG_WGSL;
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
        compositeProgram->link(linkedProgram.writeRef(), nullptr);

        ComPtr<slang::IMetadata> metadata;
        linkedProgram->getTargetMetadata(0, metadata.writeRef(), nullptr);

        // All three vertex inputs (POSITION=0, TEXCOORD=1, COLOR=2) are used.
        bool isUsed = false;
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, 0, isUsed);
        SLANG_CHECK(isUsed);

        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, 1, isUsed);
        SLANG_CHECK(isUsed);

        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, 2, isUsed);
        SLANG_CHECK(isUsed);

        // Location 3 should not be used.
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, 3, isUsed);
        SLANG_CHECK(!isUsed);
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
        compositeProgram->link(linkedProgram.writeRef(), nullptr);

        ComPtr<slang::IMetadata> metadata;
        linkedProgram->getTargetMetadata(0, metadata.writeRef(), nullptr);

        // Fragment varying inputs: TEXCOORD=0 and COLOR=1 are used,
        // SV_Position is a system value, not a user varying input.
        bool isUsed = false;
        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, 0, isUsed);
        SLANG_CHECK(isUsed);

        metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_VARYING_INPUT, 0, 1, isUsed);
        SLANG_CHECK(isUsed);

        // Descriptor table slots for the texture and sampler should also be used.
        metadata
            ->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 0, isUsed);
        SLANG_CHECK(isUsed);

        metadata
            ->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 1, isUsed);
        SLANG_CHECK(isUsed);
    }
}
