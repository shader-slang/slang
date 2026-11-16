// unit-test-gh14474.cpp

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// `ISession` module-load boundaries are `SLANG_NO_THROW`, so a fatal diagnostic raised while
// compiling an imported module — an ordinary error inside the import followed by a fatal
// "compilation ceased" — must come back as a null module plus a populated diagnostics blob, in
// every build configuration. In a build with SLANG_DISABLE_EXCEPTIONS there is no exception to
// unwind with, so without the AbortCompilationEscape installed at the loadModule /
// loadModuleFromBlob boundaries the same call terminates the host process instead of returning.
// This test is deliberately NOT gated on SLANG_HAS_EXCEPTIONS: the recovery contract it pins is
// the same in both configurations.
//
// Covered here: the loadModule and loadModuleFromBlob (via loadModuleFromSourceString) abort
// paths, a repeat failing load exercising the bookkeeping state an aborted import leaves behind,
// and a clean load on the same session. Not covered: the link()/getTargetArtifact() recovery
// branches and compile()'s reported-nothing (SLANG_E_ABORT) flavor — no minimal user input is
// known to reach them; a fatal diagnostic during linking needs an internal-error condition.
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

    // Reaching the checks below at all proves the boundaries recovered instead of terminating
    // the process.
    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModule("main", diagnostics.writeRef());
    SLANG_CHECK_ABORT(module == nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(diagnostics->getBufferSize() != 0);
    UnownedStringSlice diagnosticText(
        (const char*)diagnostics->getBufferPointer(), (Int)diagnostics->getBufferSize());
    SLANG_CHECK_ABORT(diagnosticText.indexOf(toSlice("undefined identifier")) != -1);
    SLANG_CHECK_ABORT(diagnosticText.indexOf(toSlice("compilation ceased")) != -1);

    // The same failure through the blob boundary (loadModuleFromSourceString delegates to
    // loadModuleFromBlob) must recover the same way.
    const char* blobMainSource = "import broken;\nfloat4 unused() { return float4(0, 0, 0, 1); }\n";
    ComPtr<slang::IBlob> blobDiagnostics;
    slang::IModule* blobMain = session->loadModuleFromSourceString(
        "blobmain", "blobmain.slang", blobMainSource, blobDiagnostics.writeRef());
    SLANG_CHECK_ABORT(blobMain == nullptr);
    SLANG_CHECK_ABORT(blobDiagnostics != nullptr);
    SLANG_CHECK_ABORT(blobDiagnostics->getBufferSize() != 0);
    UnownedStringSlice blobDiagnosticText(
        (const char*)blobDiagnostics->getBufferPointer(), (Int)blobDiagnostics->getBufferSize());
    SLANG_CHECK_ABORT(blobDiagnosticText.indexOf(toSlice("compilation ceased")) != -1);

    // A second failing load exercises the bookkeeping state the aborted imports left behind
    // (a recovery branch drops the in-progress import list that the longjmp stranding skipped
    // the cleanup of): it must fail cleanly with fresh diagnostics rather than reading dead
    // bookkeeping.
    ComPtr<slang::IBlob> secondDiagnostics;
    slang::IModule* secondMain = session->loadModule("main", secondDiagnostics.writeRef());
    SLANG_CHECK_ABORT(secondMain == nullptr);
    SLANG_CHECK_ABORT(secondDiagnostics != nullptr);
    SLANG_CHECK_ABORT(secondDiagnostics->getBufferSize() != 0);
    UnownedStringSlice secondDiagnosticText(
        (const char*)secondDiagnostics->getBufferPointer(), (Int)secondDiagnostics->getBufferSize());
    SLANG_CHECK_ABORT(secondDiagnosticText.indexOf(toSlice("compilation ceased")) != -1);

    // A subsequent load with no errors still succeeds on the same session.
    const char* goodModuleSource = R"SLANG(
        float4 goodHelper() { return float4(1, 1, 1, 1); }
    )SLANG";
    ComPtr<slang::IBlob> goodDiagnostics;
    slang::IModule* goodModule = session->loadModuleFromSourceString(
        "good", "good.slang", goodModuleSource, goodDiagnostics.writeRef());
    SLANG_CHECK_ABORT(goodModule != nullptr);
    SLANG_CHECK_ABORT(goodModule->getDefinedEntryPointCount() == 0);
}
