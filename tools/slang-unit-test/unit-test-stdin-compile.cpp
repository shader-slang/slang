// unit-test-stdin-compile.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "slang-com-ptr.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

#ifndef SLANG_BUILD_STDIN_TEST_HOOKS
#define SLANG_BUILD_STDIN_TEST_HOOKS 0
#endif

static bool _contains(const String& text, const char* expected)
{
    return text.getUnownedSlice().indexOf(UnownedStringSlice(expected)) >= 0;
}

static void _addStdinCompileArgs(List<String>& args, const char* language)
{
    args.add("-lang");
    args.add(language);
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add("--");
    args.add("-");
}

static SlangResult _runSlangcWithStdin(
    UnitTestContext* context,
    const List<String>& args,
    const char* stdinSource,
    ExecuteResult& outResult,
    bool allowStdinWriteFailure = false)
{
    CommandLine cmdLine;
    ExecutableLocation slangcLocation(context->executableDirectory, "slangc");
    cmdLine.setExecutableLocation(slangcLocation);
    for (const auto& arg : args)
        cmdLine.addArg(arg);

    RefPtr<Process> process;
    SLANG_RETURN_ON_FAIL(Process::create(cmdLine, 0, process));

    Stream* stdinStream = process->getStream(StdStreamType::In);
    if (stdinSource)
    {
        const Index length = Index(strlen(stdinSource));
        if (length != 0)
        {
            const SlangResult writeResult = stdinStream->write(stdinSource, length);
            if (SLANG_FAILED(writeResult) && !allowStdinWriteFailure)
                return writeResult;
        }
    }
    stdinStream->close();

    return ProcessUtil::readUntilTermination(process, outResult);
}

static SlangResult _expectSlangcSuccess(
    UnitTestContext* context,
    const List<String>& args,
    const char* stdinSource)
{
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(context, args, stdinSource, result));

    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (!_contains(result.standardOutput, "OpEntryPoint"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testSlangStdin(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "slang");

    return _expectSlangcSuccess(context, args, "[shader(\"compute\")] void main() {}\n");
}

static SlangResult _testHlslStdin(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "hlsl");

    return _expectSlangcSuccess(context, args, "[numthreads(1, 1, 1)] void main() {}\n");
}

static SlangResult _testMissingLanguage(UnitTestContext* context)
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

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(context, args, nullptr, result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "can't deduce language for input file '<stdin>'"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testEmptyStdin(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "slang");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(context, args, "", result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "no function found matching entry point name 'main'"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testDuplicateStdin(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "slang");
    args.add("-");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(
        _runSlangcWithStdin(context, args, "[shader(\"compute\")] void main() {}\n", result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "standard input can only be used once"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testDiagnosticLocation(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "slang");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(
        context,
        args,
        "[shader(\"compute\")] void main() {\n"
        "    undeclaredIdentifier;\n"
        "}\n",
        result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "<stdin>:2:"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testCrlfDiagnosticLocation(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "slang");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(
        context,
        args,
        "[shader(\"compute\")] void main() {\r\n"
        "    undeclaredIdentifier;\r\n"
        "}\r\n",
        result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "<stdin>:2:"))
        return SLANG_FAIL;

    return SLANG_OK;
}

struct TempSlangFile
{
    String basePath;
    String slangPath;

    ~TempSlangFile()
    {
        File::remove(basePath);
        File::remove(slangPath);
    }
};

static SlangResult _createTempSlangFile(
    const char* prefix,
    const char* contents,
    TempSlangFile& out)
{
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), out.basePath));
    out.slangPath = out.basePath + ".slang";
    return File::writeAllText(out.slangPath, contents);
}

struct TempHlslFile
{
    String basePath;
    String hlslPath;

    ~TempHlslFile()
    {
        File::remove(basePath);
        File::remove(hlslPath);
    }
};

static SlangResult _createTempHlslFile(const char* prefix, const char* contents, TempHlslFile& out)
{
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), out.basePath));
    out.hlslPath = out.basePath + ".hlsl";
    return File::writeAllText(out.hlslPath, contents);
}

static SlangResult _testStdinAndFileShareSlangTranslationUnit(
    UnitTestContext* context,
    bool stdinFirst)
{
    TempSlangFile helper;
    SLANG_RETURN_ON_FAIL(_createTempSlangFile("slangc-stdin-helper", "void helper() {}\n", helper));

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
    if (stdinFirst)
    {
        args.add("-");
        args.add(helper.slangPath);
    }
    else
    {
        args.add(helper.slangPath);
        args.add("-");
    }

    return _expectSlangcSuccess(context, args, "[shader(\"compute\")] void main() { helper(); }\n");
}

static SlangResult _testStdinAndFileSeparateHlslTranslationUnit(
    UnitTestContext* context,
    bool stdinFirst)
{
    TempHlslFile helper;
    SLANG_RETURN_ON_FAIL(
        _createTempHlslFile("slangc-stdin-hlsl-helper", "void helper() {}\n", helper));

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
    if (stdinFirst)
    {
        args.add("-");
        args.add(helper.hlslPath);
    }
    else
    {
        args.add(helper.hlslPath);
        args.add("-");
    }

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(
        context,
        args,
        "[numthreads(1, 1, 1)] void main() { helper(); }\n",
        result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "undefined identifier 'helper'"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testLanguageSwitchAppliesToStdinAfterSlangInput(UnitTestContext* context)
{
    TempSlangFile helper;
    SLANG_RETURN_ON_FAIL(_createTempSlangFile("slangc-stdin-helper", "void helper() {}\n", helper));

    List<String> args;
    args.add("-target");
    args.add("spirv-asm");
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    args.add(helper.slangPath);
    args.add("-lang");
    args.add("hlsl");
    args.add("--");
    args.add("-");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(
        context,
        args,
        "[numthreads(1, 1, 1)] void main() { helper(); }\n",
        result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "undefined identifier 'helper'"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testCtrlZIsNotEndOfFile(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "slang");

    return _expectSlangcSuccess(
        context,
        args,
        "void helper() { /* \x1A */ }\n"
        "[shader(\"compute\")] void main() { helper(); }\n");
}

static SlangResult _testInputLargerThanReadBuffer(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "slang");

    StringBuilder source;
    for (int i = 0; i < 900; ++i)
        source << "void helper" << i << "() {}\n";
    source << "void tail() {}\n";
    source << "[shader(\"compute\")] void main() { tail(); }\n";

    const String sourceString = source.produceString();
    return _expectSlangcSuccess(context, args, sourceString.getBuffer());
}

#if SLANG_BUILD_STDIN_TEST_HOOKS
static void _addTestStdinMaxBytesArgs(List<String>& args, size_t maxBytes)
{
    args.add("-test-stdin-max-bytes");
    StringBuilder maxBytesText;
    maxBytesText << UInt64(maxBytes);
    args.add(maxBytesText.produceString());
}

static SlangResult _testInputExactFitDiagnosticPath(UnitTestContext* context)
{
    const char* source = "[shader(\"compute\")] void main() {}\n";
    List<String> args;
    _addTestStdinMaxBytesArgs(args, strlen(source));
    _addStdinCompileArgs(args, "slang");

    return _expectSlangcSuccess(context, args, source);
}

static SlangResult _testInputTooLargeDiagnostic(UnitTestContext* context)
{
    List<String> args;
    _addTestStdinMaxBytesArgs(args, 4);
    _addStdinCompileArgs(args, "slang");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(context, args, "abcde", result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "stdin input exceeds the maximum allowed size"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testCannotReadFromStdinDiagnostic(UnitTestContext* context)
{
    List<String> args;
    args.add("-test-stdin-force-cannot-read");
    _addStdinCompileArgs(args, "slang");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(
        _runSlangcWithStdin(context, args, "[shader(\"compute\")] void main() {}\n", result, true));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "failed to read source from stdin"))
        return SLANG_FAIL;

    return SLANG_OK;
}
#endif

static SlangResult _testHelpMentionsStdin(UnitTestContext* context)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, "slangc"));
    cmdLine.addArg("-h");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, result));

    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (!_contains(result.standardOutput, "Use '-' once to read from standard input"))
        return SLANG_FAIL;

    return SLANG_OK;
}

SLANG_UNIT_TEST(SlangcReadFromStdin)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_testSlangStdin(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testHlslStdin(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testMissingLanguage(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testEmptyStdin(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testDuplicateStdin(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testDiagnosticLocation(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCrlfDiagnosticLocation(unitTestContext)));
    SLANG_CHECK(
        SLANG_SUCCEEDED(_testStdinAndFileShareSlangTranslationUnit(unitTestContext, false)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinAndFileShareSlangTranslationUnit(unitTestContext, true)));
    SLANG_CHECK(
        SLANG_SUCCEEDED(_testStdinAndFileSeparateHlslTranslationUnit(unitTestContext, false)));
    SLANG_CHECK(
        SLANG_SUCCEEDED(_testStdinAndFileSeparateHlslTranslationUnit(unitTestContext, true)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testLanguageSwitchAppliesToStdinAfterSlangInput(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCtrlZIsNotEndOfFile(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testInputLargerThanReadBuffer(unitTestContext)));
#if SLANG_BUILD_STDIN_TEST_HOOKS
    SLANG_CHECK(SLANG_SUCCEEDED(_testInputExactFitDiagnosticPath(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testInputTooLargeDiagnostic(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCannotReadFromStdinDiagnostic(unitTestContext)));
#endif
    SLANG_CHECK(SLANG_SUCCEEDED(_testHelpMentionsStdin(unitTestContext)));
}
