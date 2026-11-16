// unit-test-gh14474.cpp

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// `ISession::loadModule` is `SLANG_NO_THROW`, so a fatal diagnostic raised while compiling an
// imported module — an ordinary error inside the import followed by a fatal "compilation
// ceased" — must come back as a null module plus a populated diagnostics blob, in every build
// configuration. In a build with SLANG_DISABLE_EXCEPTIONS there is no exception to unwind with,
// so without the AbortCompilationEscape installed at the loadModule boundary the same call
// terminates the host process instead of returning. This test is deliberately NOT gated on
// SLANG_HAS_EXCEPTIONS: the recovery contract it pins is the same in both configurations.
SLANG_UNIT_TEST(gh14474LoadModuleAbortRecovery)
{
    const char* brokenModuleSource = R"SLANG(
        float4 brokenHelper() { return someUndefinedSymbol; }
    )SLANG";
    const char* mainModuleSource = R"SLANG(
        import broken;
        float4 unused() { return float4(0, 0, 0, 1); }
    )SLANG";

    ComPtr<ISlangFileSystemExt> fs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto& memoryFS = *static_cast<MemoryFileSystem*>(fs.get());
    memoryFS.saveFile("broken.slang", brokenModuleSource, strlen(brokenModuleSource));
    memoryFS.saveFile("main.slang", mainModuleSource, strlen(mainModuleSource));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.fileSystem = fs;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModule("main", diagnostics.writeRef());

    // Reaching these checks at all proves the boundary recovered instead of terminating the
    // process.
    SLANG_CHECK_ABORT(module == nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(diagnostics->getBufferSize() != 0);

    // The caller must learn why the module did not load: the error from inside the imported
    // module and the fatal "compilation ceased" that followed it.
    UnownedStringSlice diagnosticText(
        (const char*)diagnostics->getBufferPointer(), (Int)diagnostics->getBufferSize());
    SLANG_CHECK_ABORT(diagnosticText.indexOf(toSlice("undefined identifier")) != -1);
    SLANG_CHECK_ABORT(diagnosticText.indexOf(toSlice("compilation ceased")) != -1);

    // The session stays usable after a recovered abort: a subsequent module load with no errors
    // succeeds on the same session.
    const char* goodModuleSource = R"SLANG(
        float4 goodHelper() { return float4(1, 1, 1, 1); }
    )SLANG";
    ComPtr<slang::IBlob> goodDiagnostics;
    ComPtr<slang::IModule> goodModule;
    goodModule = session->loadModuleFromSourceString(
        "good", "good.slang", goodModuleSource, goodDiagnostics.writeRef());
    SLANG_CHECK_ABORT(goodModule != nullptr);
    SLANG_CHECK_ABORT(goodModule->getDefinedEntryPointCount() == 0);
}
