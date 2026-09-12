// unit-test-link-time-minimum-optimization.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// `checkUnsupportedInst` runs at every optimization level, and the checks that minimum optimization
// is meant to skip read the *effective* option set on the `TargetProgram`. That distinction matters
// because `linkWithOptions` records options on the linked component, which override the target's
// own, so an option supplied only at link time is invisible on the `TargetRequest`.
//
// Only the API reaches that path, so this test drives it: `MinimumSlangOptimization` at link time
// must suppress the gated E56002 ("cannot get size of unsized array").
SLANG_UNIT_TEST(gatedCheckHonorsLinkTimeMinimumOptimization)
{
    // `getCount()` on an unsized array reaches `kIROp_GetArrayLength`, which E56002 rejects when
    // optimizations are enabled (see tests/bugs/gh-6698.slang).
    const char* userSourceBody = R"(
            int f(int b[]) { return b.getCount(); }
            int g(int b[]) { return f(b); }

            uniform int unsizedParam[];

            [shader("compute")]
            [numthreads(1, 1, 1)]
            void computeMain()
            {
                g(unsizedParam);
            }
        )";

    // Compile the same source twice: once plainly, once with `MinimumSlangOptimization` supplied at
    // link time only. The first run establishes that the diagnostic is reachable at all, so a
    // silent failure to trigger it cannot masquerade as the suppression being tested.
    auto compile = [&](bool minimumOptimizationAtLinkTime, String& outDiagnostics)
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HLSL;
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
            "computeMain",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK(entryPoint != nullptr);

        slang::IComponentType* componentTypes[2] = {module, entryPoint.get()};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK(composedProgram != nullptr);

        ComPtr<slang::IComponentType> linkedProgram;
        if (minimumOptimizationAtLinkTime)
        {
            slang::CompilerOptionEntry entry = {};
            entry.name = slang::CompilerOptionName::MinimumSlangOptimization;
            entry.value.kind = slang::CompilerOptionValueKind::Int;
            entry.value.intValue0 = 1;
            composedProgram
                ->linkWithOptions(linkedProgram.writeRef(), 1, &entry, diagnosticBlob.writeRef());
        }
        else
        {
            composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
        }
        SLANG_CHECK(linkedProgram != nullptr);

        ComPtr<slang::IBlob> code;
        diagnosticBlob = nullptr;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
        outDiagnostics =
            diagnosticBlob ? String((const char*)diagnosticBlob->getBufferPointer()) : String();
    };

    String baselineDiagnostics;
    compile(false, baselineDiagnostics);
    SLANG_CHECK(baselineDiagnostics.indexOf(toSlice("56002")) != -1);

    String minimumOptimizationDiagnostics;
    compile(true, minimumOptimizationDiagnostics);
    SLANG_CHECK(minimumOptimizationDiagnostics.indexOf(toSlice("56002")) == -1);
}
