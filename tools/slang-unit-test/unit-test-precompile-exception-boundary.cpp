// unit-test-precompile-exception-boundary.cpp

#include "core/slang-io.h"
#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-internal.h"
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
/// Precompile the module described above for SPIR-V, which raises `InternalError` during
/// emission. Return the result of `precompileForTarget` and its diagnostics.
static SlangResult _precompileModuleThatRaisesInternalError(ComPtr<slang::IBlob>& outDiagnostics)
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

    outDiagnostics.setNull();
    return precompileService->precompileForTarget(SLANG_SPIRV, outDiagnostics.writeRef());
}

SLANG_UNIT_TEST(precompileForTargetContainsEmissionException, NoRenderApi)
{
    ComPtr<slang::IBlob> diagnostics;
    const SlangResult result = _precompileModuleThatRaisesInternalError(diagnostics);

    // Reaching this line at all proves the `SLANG_NO_THROW` boundary contained the exception rather
    // than letting it terminate the process.
    SLANG_CHECK(SLANG_FAILED(result));

    // The caller must also learn why, rather than getting a bare failure with the message lost.
    SLANG_CHECK(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getBufferSize() != 0);
}

/// Return the text of `diagnostics`, or an empty string if there is none.
static String _getText(slang::IBlob* diagnostics)
{
    if (!diagnostics)
        return String();
    return String(UnownedStringSlice(
        static_cast<const char*>(diagnostics->getBufferPointer()),
        diagnostics->getBufferSize()));
}

// A host application that loads Slang gets no crash report unless it asks for one, so the
// diagnostics of a failed call are what they were before crash reports existed.
SLANG_UNIT_TEST(internalErrorHasNoCrashReportByDefault, NoRenderApi)
{
    ComPtr<slang::IBlob> diagnostics;
    SLANG_CHECK(SLANG_FAILED(_precompileModuleThatRaisesInternalError(diagnostics)));
    SLANG_CHECK(_getText(diagnostics).indexOf(toSlice("Slang crash report")) < 0);
}

/// Crash reports are on for the lifetime of this object. Reports are a setting of the whole
/// process, so a test that aborts must not leave them on for the tests that run after it.
struct ScopedCrashReports
{
    ScopedCrashReports(const char* frontEndName) { slang_enableCrashReports(frontEndName); }
    ~ScopedCrashReports() { slang_enableCrashReports(nullptr); }
};

// Once a program turns crash reports on, the diagnostics of a call that fails on an internal
// error end with the report, and the report says what was raised and where.
SLANG_UNIT_TEST(internalErrorEndsWithCrashReportWhenEnabled, NoRenderApi)
{
    ComPtr<slang::IBlob> diagnostics;
    SlangResult result = SLANG_OK;
    {
        ScopedCrashReports crashReports("unit-test");
        result = _precompileModuleThatRaisesInternalError(diagnostics);
    }

    SLANG_CHECK(SLANG_FAILED(result));
    const String text = _getText(diagnostics);

    const Index reportStart = text.indexOf(toSlice("==== Slang crash report (format 1) ===="));
    SLANG_CHECK_ABORT(reportStart >= 0);

    // The message that the call reported before is still there, ahead of the report.
    SLANG_CHECK(reportStart > 0);

    // `SLANG_UNIMPLEMENTED_X` raised the error from a Slang source file, and the program that
    // asked for reports is named.
    const UnownedStringSlice report = text.getUnownedSlice().tail(reportStart);
    SLANG_CHECK(report.indexOf(toSlice("\nkind:        internal-error\n")) >= 0);
    SLANG_CHECK(report.indexOf(toSlice("\nsignal-type: unimplemented\n")) >= 0);
    SLANG_CHECK(report.indexOf(toSlice("\nlocation:    slang-")) >= 0);
    SLANG_CHECK(report.indexOf(toSlice("\nfront-end:   unit-test\n")) >= 0);

    // The error was raised while the compiler generated code for SPIR-V, and the report says
    // so, though the code that set the breadcrumb had already ended when the error was caught.
    SLANG_CHECK(report.indexOf(toSlice("\ntarget:      spirv\n")) >= 0);
    SLANG_CHECK(report.indexOf(toSlice("\nsignature:   ice-")) >= 0);

    // The message can hold names from the user's code, so it appears in the `[details]` part.
    const Index detailsStart = report.indexOf(toSlice("\n[details]\n"));
    SLANG_CHECK_ABORT(detailsStart >= 0);
    SLANG_CHECK(report.tail(detailsStart).indexOf(toSlice("\nmessage:     unimplemented")) >= 0);

    // Turning reports off again restores the default.
    SLANG_CHECK(SLANG_FAILED(_precompileModuleThatRaisesInternalError(diagnostics)));
    SLANG_CHECK(_getText(diagnostics).indexOf(toSlice("Slang crash report")) < 0);
}
// With a report directory set, the report is also saved as a file that is named after its
// signature, and the diagnostics say where.
SLANG_UNIT_TEST(internalErrorSavesCrashReportFile, NoRenderApi)
{
    // `generateTemporary` gives a name that no other test uses. We turn it into a directory.
    String directory;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-crash-report-"), directory)));
    File::remove(directory);
    SLANG_CHECK_ABORT(Path::createDirectory(directory));

    ComPtr<slang::IBlob> diagnostics;
    {
        ScopedCrashReports crashReports("unit-test");
        slang_setCrashReportDirectory(directory.getBuffer());
        _precompileModuleThatRaisesInternalError(diagnostics);
        slang_setCrashReportDirectory(nullptr);
    }
    const String text = _getText(diagnostics);

    // The line after the report names the file.
    static const char kSavedAs[] = "\nSaved as:    ";
    const Index reportStart = text.indexOf(toSlice("==== Slang crash report (format 1) ===="));
    const Index savedAsStart = text.indexOf(toSlice(kSavedAs));
    SLANG_CHECK_ABORT(reportStart >= 0 && savedAsStart > reportStart);

    const UnownedStringSlice afterSavedAs =
        text.getUnownedSlice().tail(savedAsStart + Index(strlen(kSavedAs)));
    const Index pathEnd = afterSavedAs.indexOf('\n');
    SLANG_CHECK_ABORT(pathEnd > 0);
    const String path = afterSavedAs.head(pathEnd);

    // The file is in the directory that we set, and its name holds the signature.
    SLANG_CHECK(path.startsWith(directory));
    SLANG_CHECK(path.indexOf(toSlice("slang-report-ice-")) >= 0);
    SLANG_CHECK(path.endsWith(".txt"));

    // The file holds the report exactly as the diagnostics show it, without the line that
    // names the file.
    String content;
    SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(path, content)));
    const UnownedStringSlice report =
        text.getUnownedSlice().subString(reportStart, savedAsStart + 1 - reportStart);
    SLANG_CHECK(content.getUnownedSlice() == report);

    File::remove(path);
    Path::remove(directory);
}
#endif // SLANG_HAS_EXCEPTIONS
