// The module-format-not-frozen warning is gated on command-line compiler mode. This drives an
// ICompileRequest directly so it can toggle that mode and check the diagnostic on both settings.
// The sibling tests/diagnostics/command-line/module-format-not-frozen.slang runs only in
// command-line mode (through slangc-tool), so it covers only the positive case; this unit test
// adds the negative (programmatic) case.

#include "core/slang-io.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

namespace
{
struct ModuleCompileResult
{
    bool compiled;
    bool foundModuleFormatWarning;
};

ModuleCompileResult compileModuleContainer(const char* outputPath, bool commandLineMode)
{
    SlangSession* session = spCreateSession();
    slang::ICompileRequest* request = spCreateCompileRequest(session);

    if (commandLineMode)
        request->setCommandLineCompilerMode();

    // `-o *.slang-module` sets the emit-IR flag and the SlangModule container format that make
    // maybeCreateContainer() reach the diagnostic. The emit-IR flag has no public API setter, so
    // the test drives it through command-line argument processing.
    const char* args[] = {"-o", outputPath};
    const SlangResult processResult =
        spProcessCommandLineArguments(request, args, SLANG_COUNT_OF(args));

    int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "m");
    spAddTranslationUnitSourceString(
        request,
        translationUnitIndex,
        "m.slang",
        "public int add(int a, int b) { return a + b; }");

    const SlangResult compileResult = spCompile(request);
    const char* diagnostics = spGetDiagnosticOutput(request);

    ModuleCompileResult result;
    result.compiled = SLANG_SUCCEEDED(processResult) && SLANG_SUCCEEDED(compileResult);
    result.foundModuleFormatWarning = diagnostics && strstr(diagnostics, "E00088") != nullptr;

    spDestroyCompileRequest(request);
    spDestroySession(session);
    return result;
}
} // namespace

SLANG_UNIT_TEST(moduleFormatNotFrozenCliGate)
{
    String tempBase;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-module-format-test"), tempBase)));
    String outputPath = tempBase + ".slang-module";

    const ModuleCompileResult programmatic = compileModuleContainer(outputPath.getBuffer(), false);
    const ModuleCompileResult commandLine = compileModuleContainer(outputPath.getBuffer(), true);

    // Clean up before asserting so a failed expectation never leaves the produced container behind.
    File::remove(outputPath);
    File::remove(tempBase);

    SLANG_CHECK(programmatic.compiled);
    SLANG_CHECK(commandLine.compiled);
    SLANG_CHECK(!programmatic.foundModuleFormatWarning);
    SLANG_CHECK(commandLine.foundModuleFormatWarning);
}
