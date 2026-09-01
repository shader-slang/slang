// unit-test-precompile-exception-boundary.cpp

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// `Module::precompileForTarget` is `SLANG_NO_THROW` and reachable across the public C ABI through
// `IModulePrecompileService_Experimental`, so an internal exception must be turned into a
// `SlangResult` rather than allowed to cross the boundary and terminate the host process. This test
// pins that contract: `precompileForTarget` must return a failing result with a populated
// diagnostics blob even when emission throws.
//
// The module below reaches the failure without a GPU or SPIR-V validation. `readIt` returns `uint4`
// and takes no parameters, so it survives `attemptPrecompiledExport` and precompilation proceeds to
// SPIR-V emission, where the global `ParameterBlock` instruction is unhandled and
// `emitPrecompiledDownstreamIR` raises `InternalError` via `SLANG_UNIMPLEMENTED_X`.
//
// Gated on `SLANG_HAS_EXCEPTIONS`: with exceptions disabled `SLANG_UNIMPLEMENTED_X` panics and
// exits the process instead of throwing, so there is no exception for the boundary to contain.
#if SLANG_HAS_EXCEPTIONS
SLANG_UNIT_TEST(precompileForTargetContainsEmissionException)
{
    const char* moduleSource = R"SLANG(
        module nested;
        public struct CB { public uint4 value; }
        public struct MaterialSystem { public CB cb; public StructuredBuffer<uint4> data; }
        public struct Scene { public CB sceneCb; public ParameterBlock<MaterialSystem> material; }
        public ParameterBlock<Scene> scene;
        public uint4 readIt() { return scene.material.cb.value + scene.sceneCb.value; }
    )SLANG";

    ComPtr<ISlangFileSystemExt> fs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto& memoryFS = *static_cast<MemoryFileSystem*>(fs.get());
    memoryFS.saveFile("nested.slang", moduleSource, strlen(moduleSource));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.fileSystem = fs;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModule("nested", diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    // Precompilation lives on a separate experimental interface rather than on `IModule` itself.
    ComPtr<slang::IModulePrecompileService_Experimental> precompileService;
    SLANG_CHECK_ABORT(
        module->queryInterface(
            slang::IModulePrecompileService_Experimental::getTypeGuid(),
            (void**)precompileService.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(precompileService != nullptr);

    diagnostics.setNull();
    const SlangResult result =
        precompileService->precompileForTarget(SLANG_SPIRV, diagnostics.writeRef());

    // Reaching this line at all proves the `SLANG_NO_THROW` boundary contained the exception rather
    // than letting it terminate the process.
    SLANG_CHECK(SLANG_FAILED(result));

    // The caller must also learn why, rather than getting a bare failure with the message lost.
    SLANG_CHECK(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getBufferSize() != 0);
}
#endif // SLANG_HAS_EXCEPTIONS
