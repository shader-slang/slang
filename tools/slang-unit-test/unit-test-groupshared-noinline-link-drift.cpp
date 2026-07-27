// unit-test-groupshared-noinline-link-drift.cpp

// A bare `groupshared` parameter on a `[noinline]` function is legal on direct SPIR-V (the boundary
// is kept and emit declares `SPV_KHR_variable_pointers`) but not on SPIR-V-via-GLSL, which requires
// the callee to be inlined away. The front end diagnoses the illegal combination using the target
// options present at semantic-checking time. `linkWithOptions()` can select `EmitSpirvViaGLSL`
// *after* that check, so a module the front end accepted under direct SPIR-V can reach codegen with
// an illegal boundary. This must produce a clean diagnostic (error 30710) rather than an internal
// compiler error / crash. This test reproduces exactly that drift path.

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(groupSharedNoInlineLinkDrift)
{
    const char* testSource = R"(
        RWStructuredBuffer<uint> outputBuffer;
        static const uint N = 8;
        groupshared uint groupSharedData[N];

        [noinline]
        void writeSlot(uint tid, groupshared uint scratch[N])
        {
            scratch[tid] = tid + 1;
        }

        [shader("compute")]
        [numthreads(8, 1, 1)]
        void computeMain(uint3 tid : SV_GroupThreadID)
        {
            writeSlot(tid.x, groupSharedData);
            GroupMemoryBarrierWithGroupSync();
            outputBuffer[tid.x] = groupSharedData[(tid.x + 1) % N];
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Session targets direct SPIR-V (no EmitSpirvViaGLSL), so the front-end check accepts the
    // `[noinline]` groupshared parameter.
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", testSource, diagnosticBlob.writeRef());
    // The module is accepted: the front end saw only the direct-SPIR-V target.
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    // Now flip the emit method to SPIR-V-via-GLSL at link time -- the drift the front end could not
    // have seen.
    slang::CompilerOptionEntry viaGlslOption = {};
    viaGlslOption.name = slang::CompilerOptionName::EmitSpirvViaGLSL;
    viaGlslOption.value.kind = slang::CompilerOptionValueKind::Int;
    viaGlslOption.value.intValue0 = 1;

    ComPtr<slang::IComponentType> linkedProgram;
    ComPtr<slang::IBlob> linkDiagnostics;
    SlangResult linkResult = entryPoint->linkWithOptions(
        linkedProgram.writeRef(),
        1,
        &viaGlslOption,
        linkDiagnostics.writeRef());
    SLANG_CHECK_ABORT(linkResult == SLANG_OK && linkedProgram != nullptr);

    // Requesting code must fail with a clean diagnostic, not crash. The IR backstop reports error
    // 30710 for the now-illegal `[noinline]` groupshared boundary.
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> codeDiagnostics;
    SlangResult codeResult =
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), codeDiagnostics.writeRef());
    SLANG_CHECK(codeResult != SLANG_OK);
    SLANG_CHECK(codeDiagnostics != nullptr);
    auto diagText = UnownedStringSlice(
        (const char*)codeDiagnostics->getBufferPointer(),
        codeDiagnostics->getBufferSize());
    SLANG_CHECK(diagText.indexOf(toSlice("30710")) != -1);
}
