// unit-test-stdin-compile.cpp
//
// Tests that slangc can read shader source from stdin when '-' is passed as the input path.

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

static SlangResult _spawnSlangcWithStdin(
    UnitTestContext* context,
    const List<String>& args,
    const char* stdinSource,
    ExecuteResult& outResult)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, "slangc"));
    for (const auto& arg : args)
        cmdLine.addArg(arg);

    RefPtr<Process> process;
    SLANG_RETURN_ON_FAIL(Process::create(cmdLine, 0, process));

    // Write source to slangc's stdin and close the stream so it sees EOF.
    if (stdinSource && stdinSource[0] != '\0')
    {
        Stream* inStream = process->getStream(StdStreamType::In);
        const Index len = Index(strlen(stdinSource));
        SLANG_RETURN_ON_FAIL(inStream->write(stdinSource, len));
    }
    process->getStream(StdStreamType::In)->close();

    SLANG_RETURN_ON_FAIL(ProcessUtil::readUntilTermination(process, outResult));
    return SLANG_OK;
}

// Case 1: valid Slang shader piped via stdin with -lang slang should succeed.
// Exercises the m_slangTranslationUnitIndex (lazy-init) branch.
static SlangResult _testStdinWithLangSlang(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("slang");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");

    const char* source = "[shader(\"compute\")] void main() {}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    // Should compile successfully and produce SPIR-V assembly output.
    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (result.standardOutput.getUnownedSlice().indexOf(toSlice("OpEntryPoint")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Case 2: valid HLSL shader piped via stdin with -lang hlsl should succeed.
// Exercises the foreign-language else branch (addTranslationUnit with non-Slang language).
static SlangResult _testStdinWithLangHlsl(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("hlsl");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");

    const char* source = "[numthreads(1,1,1)] void main() {}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    // Should compile successfully and produce SPIR-V assembly output.
    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (result.standardOutput.getUnownedSlice().indexOf(toSlice("OpEntryPoint")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Case 3: omitting -lang should produce a clean diagnostic, not a crash.
static SlangResult _testStdinWithoutLang(UnitTestContext* context)
{
    List<String> args;
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");

    const char* source = "[shader(\"compute\")] void main() {}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    // Should fail with the CannotDeduceSourceLanguage diagnostic (error 12).
    // Match the exact message text so a generic "<stdin>" mention from a different
    // diagnostic does not mask a regression on this specific code path.
    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (result.standardError.getUnownedSlice().indexOf(
            toSlice("can't deduce language for input file '<stdin>'")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Case 4: empty stdin should fail cleanly with an explicit entry/stage.
static SlangResult _testEmptyStdin(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("slang");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, "", result));

    // Empty stdin should emit the EmptySourceInput diagnostic (error 106:
    // "no source code found in '<stdin>'") and fail, not silently create an
    // empty translation unit or crash.
    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (result.standardError.getUnownedSlice().indexOf(
            toSlice("no source code found in '<stdin>'")) < 0)
        return SLANG_FAIL;
    return SLANG_OK;
}

// Case 5: passing '-' twice consumes stdin on the first read; the second '-' sees EOF.
// This pins the current behavior so any future change is intentional.
static SlangResult _testStdinTwice(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("slang");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");
    args.add("-");

    const char* source = "[shader(\"compute\")] void main() {}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    // The second '-' is a no-op (m_stdinConsumed guard short-circuits before re-reading
    // the drained pipe). The source from the first '-' should compile to exactly one
    // entry point — if the second '-' were incorrectly processed it could produce two.
    if (result.resultCode != 0)
        return SLANG_FAIL;

    // Count OpEntryPoint occurrences to confirm only one entry point was compiled.
    Index count = 0;
    const UnownedStringSlice marker = toSlice("OpEntryPoint");
    UnownedStringSlice remaining = result.standardOutput.getUnownedSlice();
    while (true)
    {
        Index found = remaining.indexOf(marker);
        if (found < 0)
            break;
        count++;
        remaining = remaining.tail(found + marker.getLength());
    }
    if (count != 1)
        return SLANG_FAIL;

    // Silent no-op contract: the second '-' must produce no diagnostic.
    // Any future change that emits a warning or error on the second '-' must
    // update this test to reflect the new intentional behavior.
    UnownedStringSlice err = result.standardError.getUnownedSlice();
    if (err.indexOf(toSlice("no source code found")) >= 0)
        return SLANG_FAIL;
    if (err.indexOf(toSlice("can't deduce language")) >= 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Case 6: a deliberate syntax error in stdin source should produce a diagnostic that
// includes both "<stdin>" as the path and a line number, confirming the SourceFile
// path-info flows correctly through the diagnostic sink for tooling integrations.
static SlangResult _testStdinDiagnosticLocation(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("slang");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");

    // Line 1 is a valid declaration; line 2 references an undeclared identifier.
    // This pins that the line number in the diagnostic refers to the stdin source.
    const char* source =
        "[shader(\"compute\")] void main() {\n"
        "    undeclared_identifier;\n"
        "}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    if (result.resultCode == 0)
        return SLANG_FAIL;

    // Diagnostic format is: <stdin>(line): error N: message
    // Assert the exact line number so a BOM or off-by-one regression in SourceLoc
    // tracking surfaces here rather than silently passing.
    // undeclared_identifier; is on line 2 of the stdin source.
    UnownedStringSlice err = result.standardError.getUnownedSlice();
    if (err.indexOf(toSlice("<stdin>(2):")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Case 7a: stdin source with a UTF-8 BOM should compile correctly.
// SourceFile does BOM detection; verify it works for the stdin blob path too.
static SlangResult _testStdinWithBom(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("slang");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");

    // UTF-8 BOM (\xEF\xBB\xBF) prepended to a valid shader.
    const char* source = "\xEF\xBB\xBF[shader(\"compute\")] void main() {}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (result.standardOutput.getUnownedSlice().indexOf(toSlice("OpEntryPoint")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Case 7b: BOM + deliberate error on line 2 should report <stdin>(2): so a
// BOM-handling regression that treats the BOM as a newline would shift line
// numbers and fail this assertion.
static SlangResult _testStdinBomDiagnosticLocation(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("slang");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");

    // BOM on byte 0, valid line 1, error on line 2.
    const char* source =
        "\xEF\xBB\xBF[shader(\"compute\")] void main() {\n"
        "    undeclared_identifier;\n"
        "}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    if (result.resultCode == 0)
        return SLANG_FAIL;

    // If the BOM were consumed as a newline, the error would appear on line 3.
    if (result.standardError.getUnownedSlice().indexOf(toSlice("<stdin>(2):")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Case 8: GLSL shader piped via stdin with -lang glsl and -stage compute.
// For GLSL the stage is normally inferred from the file extension (.comp, .vert, etc.);
// stdin has no extension so -stage is always required. This pins that the path works
// and that the documented requirement is enforced.
static SlangResult _testStdinWithLangGlsl(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("glsl");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");

    const char* source =
        "#version 450\n"
        "layout(local_size_x = 1) in;\n"
        "void main() {}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (result.standardOutput.getUnownedSlice().indexOf(toSlice("OpEntryPoint")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Case 9: stdin combined with an on-disk .slang file.
// Both inputs go into the same Slang translation unit (m_slangTranslationUnitIndex reuse),
// so the stdin entry point can call a function defined in the on-disk file.
// A future refactor that allocates a fresh TU for stdin would break cross-file visibility
// and fail this test.
static SlangResult _testStdinMixedWithFile(UnitTestContext* context)
{
    // Generate a uniquely-named temp file in the OS temp directory so the test
    // is safe in read-only cwds and parallel/re-run CI environments.
    String helperPath;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slangc-stdin-helper"), helperPath));
    helperPath = helperPath + ".slang";

    // RAII guard: removes the file on any return path, including early-out failures.
    struct FileGuard
    {
        String path;
        ~FileGuard() { File::remove(path); }
    } guard{helperPath};

    SLANG_RETURN_ON_FAIL(File::writeAllText(helperPath, "void foo() {}\n"));

    List<String> args;
    args.add("-lang");
    args.add("slang");
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add(helperPath);
    args.add("--");
    args.add("-");

    // Entry point in stdin calls foo() defined in the on-disk file.
    const char* source = "[shader(\"compute\")] void main() { foo(); }\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (result.standardOutput.getUnownedSlice().indexOf(toSlice("OpEntryPoint")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

SLANG_UNIT_TEST(SlangcReadFromStdin)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinWithLangSlang(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinWithLangHlsl(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinWithoutLang(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testEmptyStdin(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinTwice(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinDiagnosticLocation(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinWithBom(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinBomDiagnosticLocation(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinWithLangGlsl(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinMixedWithFile(unitTestContext)));
}
