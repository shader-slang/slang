// unit-test-stdin-compile.cpp
//
// Tests that slangc can read shader source from stdin when '-' is passed as the input path.

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

    // Empty source with -entry main should emit the EntryPointFunctionNotFound diagnostic
    // (error 3782: "no function found matching entry point name 'main'").
    // Matching this specific phrase distinguishes a clean compiler error from a crash/assert.
    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (result.standardError.getUnownedSlice().indexOf(
            toSlice("no function found matching entry point name 'main'")) < 0)
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

    // Both '-' tokens go into the same Slang translation unit (m_slangTranslationUnitIndex
    // reuse), so the second read appends nothing meaningful. Compilation should still
    // succeed since the first read provided a valid entry point.
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
}
