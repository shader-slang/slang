// unit-test-spirv-link-validation.cpp

// SPIR-V validation of a downstream-linked module inspects the linked result, not the pre-link
// module Slang emitted.

#include "scoped-env-var.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using SlangUnitTest::ScopedEnvVar;

namespace
{

// The generator word SPIRV-Tools' linker writes into a module header: tool id
// `SPV_GENERATOR_KHRONOS_LINKER` in the high half, per `(tool << 16) | misc`. Stored pre-shifted to
// match `kSPIRVSlangCompilerId` in `slang-emit-spirv.cpp`. Spelled out rather than included because
// the defining header is private to SPIRV-Tools, and the id is a registry allocation, so it cannot
// be derived by eye.
static const uint32_t kSpvGeneratorKhronosLinker = 17 << 16;

// The result code and whether any code came back are tracked separately so a compile that reports
// success without returning a module is not read as a pass.
struct LinkedSpirvOutcome
{
    SlangResult codeResult;
    bool producedCode;
    // Header word 2 of the returned module. Only meaningful when `producedCode` is true, since 0 is
    // itself a legal tool id.
    uint32_t generatorMagic;
    String diagnostics;
};

// Report a failed setup step, with whatever the compiler said about it, before the enclosing
// `SLANG_CHECK_ABORT` unwinds. The abort throws, so anything printed after it never runs, and the
// harness does not surface the assert message itself -- a failure would otherwise reach CI as a
// bare non-zero exit with both streams empty. See shader-slang/slang#12431.
static bool reportStep(const char* step, bool ok, slang::IBlob* diagnostics)
{
    if (!ok)
    {
        StringBuilder message;
        message << "setup step failed: " << step;
        if (diagnostics && diagnostics->getBufferSize())
        {
            message << "\ndiagnostics:\n"
                    << UnownedStringSlice(
                           (const char*)diagnostics->getBufferPointer(),
                           diagnostics->getBufferSize());
        }
        // Routed through the reporter rather than `stderr`: under `-use-test-server`, which CI
        // uses, the reported `stdError` is the reporter's own buffer and a write to the process's
        // stderr never reaches it.
        getTestReporter()->message(TestMessageType::TestFailure, message.getBuffer());
    }
    return ok;
}

// Compile an entry point that imports a module precompiled to SPIR-V, with SPIR-V validation forced
// on, and report what came back.
//
// Precompiling the imported module embeds its target IR, so the entry point's compile has two
// SPIR-V modules to combine and takes the downstream link path in `createArtifactFromIR`.
LinkedSpirvOutcome compileImportingModuleWithValidation()
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(reportStep(
        "createGlobalSession",
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK,
        nullptr));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // `precompileForTarget` is documented as not thread-safe because it mutates the module, so this
    // session and its modules stay private to this call.
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(reportStep(
        "createSession",
        globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK,
        nullptr));

    const char* librarySource = R"SLANG(
        public int addOne(int value)
        {
            return value + 1;
        }
    )SLANG";

    const char* entryPointSource = R"SLANG(
        import lib;

        RWStructuredBuffer<int> gOutputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutputBuffer[dispatchThreadID.x] = addOne(int(dispatchThreadID.x));
        }
    )SLANG";

    ComPtr<slang::IBlob> diagnostics;
    auto libraryModule = session->loadModuleFromSourceString(
        "lib",
        "lib.slang",
        librarySource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(reportStep("loadModule(library)", libraryModule != nullptr, diagnostics));

    // Embed SPIR-V for the library so the importing compile has a second module to link.
    // Precompilation lives on a separate experimental interface rather than on `IModule`.
    ComPtr<slang::IModulePrecompileService_Experimental> precompileService;
    SLANG_CHECK_ABORT(reportStep(
        "queryInterface(IModulePrecompileService_Experimental)",
        libraryModule->queryInterface(
            slang::IModulePrecompileService_Experimental::getTypeGuid(),
            (void**)precompileService.writeRef()) == SLANG_OK,
        nullptr));
    SLANG_CHECK_ABORT(
        reportStep("precompileService non-null", precompileService != nullptr, nullptr));

    // Force validation off across the precompile: the validation gate does not yet know that a
    // precompile-for-target is by construction not a final module, so an ambient
    // `SLANG_RUN_SPIRV_VALIDATION=1` -- which CI sets globally -- rejects the library for the
    // `Linkage` capability and `Export` decorations that make it linkable at all. See
    // shader-slang/slang#12385; once that gate is fixed this window can be removed.
    diagnostics.setNull();
    {
        ScopedEnvVar skipValidationWhilePrecompiling("SLANG_RUN_SPIRV_VALIDATION", "0");
        SLANG_CHECK_ABORT(reportStep(
            "precompileForTarget(SLANG_SPIRV)",
            precompileService->precompileForTarget(SLANG_SPIRV, diagnostics.writeRef()) == SLANG_OK,
            diagnostics));
    }

    diagnostics.setNull();
    auto module = session->loadModuleFromSourceString(
        "entry",
        "entry.slang",
        entryPointSource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(reportStep("loadModule(entry)", module != nullptr, diagnostics));

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(reportStep("findAndCheckEntryPoint", entryPoint != nullptr, diagnostics));

    slang::IComponentType* componentTypes[2] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composedProgram;
    SLANG_CHECK_ABORT(reportStep(
        "createCompositeComponentType",
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnostics.writeRef()) == SLANG_OK,
        diagnostics));

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(reportStep(
        "link",
        composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK,
        diagnostics));

    ComPtr<slang::IBlob> code;
    diagnostics.setNull();
    LinkedSpirvOutcome outcome;
    {
        ScopedEnvVar validateSpirv("SLANG_RUN_SPIRV_VALIDATION", "1");
        outcome.codeResult =
            linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    }
    outcome.producedCode = code && code->getBufferSize() != 0;

    outcome.generatorMagic = 0;
    if (code && code->getBufferSize() >= 3 * sizeof(uint32_t))
    {
        outcome.generatorMagic = ((const uint32_t*)code->getBufferPointer())[2];
    }

    if (diagnostics && diagnostics->getBufferSize())
    {
        outcome.diagnostics = String(
            (const char*)diagnostics->getBufferPointer(),
            (const char*)diagnostics->getBufferPointer() + diagnostics->getBufferSize());
    }
    return outcome;
}

} // namespace

// A compile whose SPIR-V is assembled by the downstream linker must pass validation: the linker
// resolves the imports and drops the `Linkage` capability, so the module handed to the caller
// satisfies the Vulkan rules that validation enforces.
SLANG_UNIT_TEST(spirvValidationAcceptsDownstreamLinkedModule)
{
    const LinkedSpirvOutcome outcome = compileImportingModuleWithValidation();

    // This test is designed to bite in CI, where nobody can attach a debugger, so surface the
    // compiler's own diagnostics rather than leaving a bare failed assertion in the log.
    if (outcome.codeResult != SLANG_OK && outcome.diagnostics.getLength())
    {
        fprintf(stderr, "compile diagnostics:\n%s\n", outcome.diagnostics.getBuffer());
    }

    SLANG_CHECK(outcome.codeResult == SLANG_OK);
    SLANG_CHECK(outcome.producedCode);

    // Without this the test would still pass if a change stopped the link from happening at all,
    // since a single-module compile also succeeds.
    SLANG_CHECK((outcome.generatorMagic & 0xFFFF0000u) == kSpvGeneratorKhronosLinker);
}
