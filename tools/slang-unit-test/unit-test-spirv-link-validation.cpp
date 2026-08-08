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
    // Whether `spirv-opt` could be loaded at all. Without it `createArtifactFromIR` skips the whole
    // link-and-validate block, so there is no downstream linker for this test to exercise.
    bool haveSpirvOpt;
    // Header word 2 of the returned module. Only meaningful when `producedCode` is true, since 0 is
    // itself a legal tool id.
    uint32_t generatorMagic;
    String diagnostics;
};

// Compile an entry point that imports a module precompiled to SPIR-V, with SPIR-V validation forced
// on, and report what came back.
//
// Precompiling the imported module embeds its target IR, so the entry point's compile has two
// SPIR-V modules to combine and takes the downstream link path in `createArtifactFromIR`.
LinkedSpirvOutcome compileImportingModuleWithValidation()
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Ask the dependency directly rather than inferring it from the output later: this attempts the
    // real load and reports `SLANG_E_NOT_FOUND` when the module is unusable.
    const bool haveSpirvOpt =
        SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_SPIRV_OPT));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // `precompileForTarget` is documented as not thread-safe because it mutates the module, so this
    // session and its modules stay private to this call.
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

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
    SLANG_CHECK_ABORT(libraryModule != nullptr);

    // Embed SPIR-V for the library so the importing compile has a second module to link.
    // Precompilation lives on a separate experimental interface rather than on `IModule`.
    ComPtr<slang::IModulePrecompileService_Experimental> precompileService;
    SLANG_CHECK_ABORT(
        libraryModule->queryInterface(
            slang::IModulePrecompileService_Experimental::getTypeGuid(),
            (void**)precompileService.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(precompileService != nullptr);

    // Force validation off across the precompile: the validation gate does not yet know that a
    // precompile-for-target is by construction not a final module, so an ambient
    // `SLANG_RUN_SPIRV_VALIDATION=1` -- which CI sets globally -- rejects the library for the
    // `Linkage` capability and `Export` decorations that make it linkable at all. See
    // shader-slang/slang#12385; once that gate is fixed this window can be removed.
    diagnostics.setNull();
    {
        ScopedEnvVar skipValidationWhilePrecompiling("SLANG_RUN_SPIRV_VALIDATION", "0");
        SLANG_CHECK_ABORT(
            precompileService->precompileForTarget(SLANG_SPIRV, diagnostics.writeRef()) ==
            SLANG_OK);
    }

    diagnostics.setNull();
    auto module = session->loadModuleFromSourceString(
        "entry",
        "entry.slang",
        entryPointSource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* componentTypes[2] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composedProgram;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> code;
    diagnostics.setNull();
    LinkedSpirvOutcome outcome;
    {
        ScopedEnvVar validateSpirv("SLANG_RUN_SPIRV_VALIDATION", "1");
        outcome.codeResult =
            linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    }
    outcome.haveSpirvOpt = haveSpirvOpt;
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

    // This test is designed to bite in CI, where nobody can attach a debugger, so report everything
    // needed to classify a failure from the log alone. Printing only on a failed `codeResult` is
    // not enough: the link is skipped silently when the downstream compiler is missing, which
    // leaves `codeResult` OK and the generator assertion below as the only symptom.
    if (outcome.diagnostics.getLength())
    {
        fprintf(stderr, "compile diagnostics:\n%s\n", outcome.diagnostics.getBuffer());
    }
    fprintf(
        stderr,
        "codeResult=0x%08x producedCode=%d generator=0x%08x\n",
        (unsigned)outcome.codeResult,
        (int)outcome.producedCode,
        outcome.generatorMagic);

    // `spirv-opt` (slang-glslang) is loaded at runtime, and `createArtifactFromIR` skips its whole
    // link-and-validate block when it is absent, so there is no downstream linker to exercise. Key
    // this on the dependency rather than on the emitted module's shape: a module carrying Slang's
    // own generator id is also exactly what a regression that stopped linking would produce, and
    // skipping on that would delete the coverage the assertion below exists to provide. The sibling
    // `unit-test-spirv-validation-unavailable.cpp` branches on the locator's result for the same
    // reason.
    if (!outcome.haveSpirvOpt)
    {
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK(outcome.codeResult == SLANG_OK);
    SLANG_CHECK(outcome.producedCode);

    // Without this the test would still pass if a change stopped the link from happening at all,
    // since a single-module compile also succeeds.
    SLANG_CHECK((outcome.generatorMagic & 0xFFFF0000u) == kSpvGeneratorKhronosLinker);
}
