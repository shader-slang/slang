// unit-test-diagnostic-color-target.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Compile a shader whose only diagnostic is emitted at the target/back-end stage and return the
// diagnostics blob produced by getTargetCode(), as a string. When `setColorOption` is true the
// session's DiagnosticColor option is set to `colorMode`; when false the option is left unset (a
// plain default compile), and `colorMode` is ignored.
//
// A global variable is implicitly a global shader parameter; the resulting E39019
// "global-uniform-not-expected" warning is reported during parameter binding, i.e. at the target
// stage reached through getTargetCode -> ComponentType::getTargetArtifact. Only the getTargetCode
// diagnostics blob is inspected: the module-load diagnostics blob is colored even without the fix,
// so asserting on it would give a false pass.
static String getTargetStageDiagnostics(
    slang::IGlobalSession* globalSession,
    bool setColorOption,
    SlangDiagnosticColor colorMode)
{
    const char* userSource = R"(
        int a;
        [shader("compute")]
        [numthreads(1,1,1)]
        void main() {}
        )";

    slang::TargetDesc targetDesc = {};
    // SPIR-V disassembly keeps this test GPU-free / headless.
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry colorOption = {};
    colorOption.name = slang::CompilerOptionName::DiagnosticColor;
    colorOption.value.kind = slang::CompilerOptionValueKind::Int;
    colorOption.value.intValue0 = colorMode;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    if (setColorOption)
    {
        sessionDesc.compilerOptionEntries = &colorOption;
        sessionDesc.compilerOptionEntryCount = 1;
    }

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> loadDiagnostics;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", userSource, loadDiagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    module->link(linkedProgram.writeRef(), loadDiagnostics.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> targetDiagnostics;
    linkedProgram->getTargetCode(0, code.writeRef(), targetDiagnostics.writeRef());

    if (!targetDiagnostics || targetDiagnostics->getBufferSize() == 0)
        return String();
    return String(UnownedStringSlice(
        (const char*)targetDiagnostics->getBufferPointer(),
        targetDiagnostics->getBufferSize()));
}

// Setting CompilerOptionName::DiagnosticColor through the API must color diagnostics emitted at the
// target/back-end stage (surfaced via getTargetCode), not only diagnostics emitted before target
// codegen. Regression test for the unconditional color-mode apply in applySettingsToDiagnosticSink,
// where the second apply from ComponentType's (empty) option set overwrote the ALWAYS mode set by
// the linkage option set with the AUTO default.
SLANG_UNIT_TEST(diagnosticColorTargetStage)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    const UnownedStringSlice ansiEscape = toSlice("\x1B[");

    // ALWAYS: the target-stage diagnostic must be colored.
    {
        String diagnostics =
            getTargetStageDiagnostics(globalSession, true, SLANG_DIAGNOSTIC_COLOR_ALWAYS);
        // The expected target-stage diagnostic (E39019) must actually be present, so an unrelated
        // diagnostic cannot vacuously satisfy the color assertion.
        SLANG_CHECK(diagnostics.indexOf(toSlice("E39019")) != -1);
        SLANG_CHECK(diagnostics.indexOf(ansiEscape) != -1);
    }

    // NEVER: the same diagnostic must not be colored (guards against a naive always-color fix).
    {
        String diagnostics =
            getTargetStageDiagnostics(globalSession, true, SLANG_DIAGNOSTIC_COLOR_NEVER);
        SLANG_CHECK(diagnostics.indexOf(toSlice("E39019")) != -1);
        SLANG_CHECK(diagnostics.indexOf(ansiEscape) == -1);
    }

    // Default: with DiagnosticColor unset, a plain compile must not color target-stage diagnostics
    // (the sink stays at its AUTO default and the blob sink has no console writer). This pins the
    // default-compile invariant and is a tripwire against any future change that starts
    // force-materializing DiagnosticColor into the target-stage option set.
    {
        String diagnostics =
            getTargetStageDiagnostics(globalSession, false, SLANG_DIAGNOSTIC_COLOR_AUTO);
        SLANG_CHECK(diagnostics.indexOf(toSlice("E39019")) != -1);
        SLANG_CHECK(diagnostics.indexOf(ansiEscape) == -1);
    }
}
