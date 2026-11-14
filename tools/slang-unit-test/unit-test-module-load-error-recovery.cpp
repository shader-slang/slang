#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// Failed module loads must return diagnostics and leave the session usable, including when Slang
// is built with SLANG_DISABLE_EXCEPTIONS. The import-error summary used to abort the host process.
SLANG_UNIT_TEST(moduleLoadErrorRecovery)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    const char* invalidSources[] = {
        "public float brokenHelper() { return someUndefinedSymbol; }",
        "public float brokenHelper( {",
    };

    for (auto invalidSource : invalidSources)
    {
        ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem());
        SLANG_CHECK_ABORT(
            fileSystem->saveFile("bad.slang", invalidSource, strlen(invalidSource)) == SLANG_OK);
        const char* middleSource = "import bad;";
        SLANG_CHECK_ABORT(
            fileSystem->saveFile("middle.slang", middleSource, strlen(middleSource)) == SLANG_OK);
        const char* mainSource = "import middle;";
        SLANG_CHECK_ABORT(
            fileSystem->saveFile("main.slang", mainSource, strlen(mainSource)) == SLANG_OK);

        // Exercise direct source loading, file discovery, and a transitive import. Each API starts
        // with a fresh session so a cached failed module cannot bypass the compilation path.
        for (int loadKind = 0; loadKind < 3; ++loadKind)
        {
            slang::TargetDesc target = {};
            target.format = SLANG_SPIRV;
            target.profile = globalSession->findProfile("spirv_1_5");
            slang::SessionDesc sessionDesc = {};
            sessionDesc.fileSystem = fileSystem;
            sessionDesc.targets = &target;
            sessionDesc.targetCount = 1;
            ComPtr<slang::ISession> session;
            SLANG_CHECK_ABORT(
                globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

            ComPtr<ISlangBlob> diagnostics;
            auto loadedModuleCount = session->getLoadedModuleCount();
            // Failed loads must not retain modules in the session's loaded-module list.
            for (int attempt = 0; attempt < 3; ++attempt)
            {
                diagnostics.setNull();
                slang::IModule* failedModule = nullptr;
                switch (loadKind)
                {
                case 0:
                    failedModule = session->loadModuleFromSourceString(
                        "bad",
                        "bad.slang",
                        invalidSource,
                        diagnostics.writeRef());
                    break;
                case 1:
                    failedModule = session->loadModule("main", diagnostics.writeRef());
                    break;
                case 2:
                    failedModule = session->loadModuleFromSourceString(
                        "main",
                        "main.slang",
                        mainSource,
                        diagnostics.writeRef());
                    break;
                }
                SLANG_CHECK(failedModule == nullptr);
                SLANG_CHECK_ABORT(diagnostics != nullptr);
                auto diagnosticText = (const char*)diagnostics->getBufferPointer();
                SLANG_CHECK(strstr(diagnosticText, "bad.slang") != nullptr);
                SLANG_CHECK(strstr(diagnosticText, "compilation ceased") != nullptr);
                SLANG_CHECK(strstr(diagnosticText, "cannot open file") == nullptr);
                if (invalidSource == invalidSources[0])
                    SLANG_CHECK(strstr(diagnosticText, "someUndefinedSymbol") != nullptr);
                if (loadKind != 0)
                    SLANG_CHECK(strstr(diagnosticText, "import failed") != nullptr);

                SLANG_CHECK(session->getLoadedModuleCount() == loadedModuleCount);
            }

            // Reload the failed dependency with valid source in the same session. This also proves
            // that failed loads did not leave an import in progress or a partially checked module
            // in the cache. The original consumers can then import the corrected dependency.
            diagnostics.setNull();
            auto goodModule = session->loadModuleFromSourceString(
                "bad",
                "bad.slang",
                "[shader(\"compute\")][numthreads(1,1,1)] void computeMain() {}",
                diagnostics.writeRef());
            SLANG_CHECK_ABORT(goodModule != nullptr);
            SLANG_CHECK(session->loadModule("bad") == goodModule);
            SLANG_CHECK(session->loadModule("main", diagnostics.writeRef()) != nullptr);
            ComPtr<slang::IEntryPoint> entryPoint;
            SLANG_CHECK_ABORT(
                goodModule->findEntryPointByName("computeMain", entryPoint.writeRef()) == SLANG_OK);
            ComPtr<slang::IComponentType> linked;
            SLANG_CHECK_ABORT(
                entryPoint->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);
            ComPtr<ISlangBlob> code;
            SLANG_CHECK(
                linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) ==
                SLANG_OK);
            SLANG_CHECK(code != nullptr);
        }
    }
}
