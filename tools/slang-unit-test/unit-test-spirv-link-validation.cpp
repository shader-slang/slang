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

// `SPV_GENERATOR_KHRONOS_LINKER` from SPIRV-Tools' `source/spirv_constant.h`, which the linker
// writes into the module header. Spelled out rather than included: that header is private to
// SPIRV-Tools, so reaching it would mean adding an include path into another project's internals.
// The value is a registry allocation, not a sequence position, so it cannot be derived by eye.
static const uint32_t kSpvGeneratorKhronosLinker = 17;

// The result code and whether any code came back are tracked separately so a compile that reports
// success without returning a module is not read as a pass.
struct LinkedSpirvOutcome
{
    SlangResult codeResult;
    bool producedCode;
    // Generator magic of the returned module, or 0 when no code came back. SPIRV-Tools' linker
    // stamps its own tool id, so this distinguishes a linked module from one Slang emitted alone.
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

    // Force validation off across the precompile, rather than merely leaving it unset: CI exports
    // `SLANG_RUN_SPIRV_VALIDATION=1` globally, and validating a precompiled library rejects it for
    // carrying the `Linkage` capability and `Export` decorations that make it linkable at all.
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

    SLANG_CHECK(outcome.codeResult == SLANG_OK);
    SLANG_CHECK(outcome.producedCode);

    // Without this the test would still pass if a change stopped the link from happening at all,
    // since a single-module compile also succeeds. The shift recovers the tool half of the
    // generator word, whose layout is `(tool << 16) | misc`.
    SLANG_CHECK((outcome.generatorMagic >> 16) == kSpvGeneratorKhronosLinker);
}
