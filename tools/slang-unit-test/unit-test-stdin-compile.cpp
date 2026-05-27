// unit-test-stdin-compile.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "../../source/slang/slang-stdin-source.h"
#include "slang-com-ptr.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Slang;

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

static int _writeEnvironmentVariable(const char* key, const char* value)
{
#ifdef _WIN32
    String variable = String(key) + "=" + value;
    return _putenv(variable.getBuffer());
#else
    return setenv(key, value, 1);
#endif
}

static int _unsetEnvironmentVariable(const char* key)
{
#ifdef _WIN32
    String variable = String(key) + "=";
    return _putenv(variable.getBuffer());
#else
    return unsetenv(key);
#endif
}

struct ScopedEnvVar
{
    const char* key;
    bool hadOldValue = false;
    bool isSet = false;
    String oldValue;

    ScopedEnvVar(const char* inKey, const char* value)
        : key(inKey)
    {
#ifdef _WIN32
        char* oldValueBuffer = nullptr;
        size_t oldValueLength = 0;
        if (_dupenv_s(&oldValueBuffer, &oldValueLength, key) == 0 && oldValueBuffer)
        {
            hadOldValue = true;
            oldValue = oldValueBuffer;
            free(oldValueBuffer);
        }
#else
        if (const char* oldValueBuffer = getenv(key))
        {
            hadOldValue = true;
            oldValue = oldValueBuffer;
        }
#endif
        isSet = _writeEnvironmentVariable(key, value) == 0;
    }

    ~ScopedEnvVar()
    {
        if (hadOldValue)
            _writeEnvironmentVariable(key, oldValue.getBuffer());
        else
            _unsetEnvironmentVariable(key);
    }
};

static SlangResult _runSlangcWithStdin(
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

    Stream* stdinStream = process->getStream(StdStreamType::In);
    if (stdinSource)
    {
        const Index length = Index(strlen(stdinSource));
        if (length != 0)
            SLANG_RETURN_ON_FAIL(stdinStream->write(stdinSource, length));
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

struct ScopedFile
{
    FILE* file = nullptr;

    ~ScopedFile()
    {
        if (file)
            fclose(file);
    }
};

struct TempFile
{
    String path;

    ~TempFile()
    {
        if (path.getLength())
            File::remove(path);
    }
};

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

static SlangResult _readStdinSourceForTest(
    const char* sourceText,
    Index maxBytes,
    List<Byte>& outSource,
    StdinSourceReadResult& outResult)
{
    outResult = StdinSourceReadResult::CannotRead;

    ScopedFile stream;
    stream.file = tmpfile();
    if (!stream.file)
        return SLANG_FAIL;

    const size_t sourceLength = strlen(sourceText);
    if (fwrite(sourceText, 1, sourceLength, stream.file) != sourceLength)
        return SLANG_FAIL;
    if (fseek(stream.file, 0, SEEK_SET) != 0)
        return SLANG_FAIL;

    outResult = readStdinSource(stream.file, maxBytes, outSource);
    return SLANG_OK;
}

static SlangResult _testInputExactFitReadResult()
{
    List<Byte> source;
    StdinSourceReadResult result;

    SLANG_RETURN_ON_FAIL(_readStdinSourceForTest("abcd", 4, source, result));

    if (result != StdinSourceReadResult::Success)
        return SLANG_FAIL;
    if (source.getCount() != 4)
        return SLANG_FAIL;
    if (memcmp(source.getBuffer(), "abcd", 4) != 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testInputTooLargeReadResult()
{
    StringBuilder sourceText;
    for (Index i = 0; i < 16 * 1024 + 5; ++i)
        sourceText.appendChar('a');

    List<Byte> source;
    StdinSourceReadResult result;
    const String sourceString = sourceText.produceString();

    SLANG_RETURN_ON_FAIL(
        _readStdinSourceForTest(sourceString.getBuffer(), 16 * 1024 + 4, source, result));

    if (result != StdinSourceReadResult::TooLarge)
        return SLANG_FAIL;
    if (source.getCount() != 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testInputTooLargeDiagnostic(UnitTestContext* context)
{
    ScopedEnvVar maxBytes("SLANG_TEST_STDIN_MAX_BYTES", "4");
    if (!maxBytes.isSet)
        return SLANG_FAIL;

    List<String> args;
    _addStdinCompileArgs(args, "slang");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithStdin(context, args, "abcde", result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "stdin input exceeds the maximum allowed size"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testCannotReadFromStdinReadResult()
{
    TempFile temp;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slangc-stdin-error"), temp.path));

    ScopedFile stream;
    stream.file = fopen(temp.path.getBuffer(), "w");
    if (!stream.file)
        return SLANG_FAIL;

    List<Byte> source;
    const StdinSourceReadResult result = readStdinSource(stream.file, 64, source);
    if (result != StdinSourceReadResult::CannotRead)
        return SLANG_FAIL;
    if (source.getCount() != 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testCannotReadFromStdinDiagnostic(UnitTestContext* context)
{
    ScopedEnvVar forceCannotRead("SLANG_TEST_STDIN_FORCE_CANNOT_READ", "1");
    if (!forceCannotRead.isSet)
        return SLANG_FAIL;

    List<String> args;
    _addStdinCompileArgs(args, "slang");

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(
        _runSlangcWithStdin(context, args, "[shader(\"compute\")] void main() {}\n", result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "failed to read source from stdin"))
        return SLANG_FAIL;

    return SLANG_OK;
}

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
    SLANG_CHECK(SLANG_SUCCEEDED(_testCtrlZIsNotEndOfFile(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testInputLargerThanReadBuffer(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testInputExactFitReadResult()));
    SLANG_CHECK(SLANG_SUCCEEDED(_testInputTooLargeReadResult()));
    SLANG_CHECK(SLANG_SUCCEEDED(_testInputTooLargeDiagnostic(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCannotReadFromStdinReadResult()));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCannotReadFromStdinDiagnostic(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testHelpMentionsStdin(unitTestContext)));
}
