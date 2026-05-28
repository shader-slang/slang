// unit-test-stdin-compile.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "../../source/slang/slang-internal.h"
#include "slang-com-ptr.h"
#include "unit-test/slang-unit-test.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif
#include <mutex>
#include <string.h>

using namespace Slang;

static bool _contains(const String& text, const char* expected)
{
    return text.getUnownedSlice().indexOf(UnownedStringSlice(expected)) >= 0;
}

static void _appendDiagnostic(char const* message, void* userData)
{
    StringBuilder* diagnostics = (StringBuilder*)userData;
    *diagnostics << message;
}

struct ScopedWriteOnlyStdin
{
    SlangResult redirect()
    {
#ifdef _WIN32
        const int stdinFd = _fileno(stdin);
        m_savedStdinFd = _dup(stdinFd);
        if (m_savedStdinFd == -1)
            return SLANG_FAIL;

        const int writeOnlyFd = _open("NUL", _O_WRONLY);
        if (writeOnlyFd == -1)
            return SLANG_FAIL;

        const int result = _dup2(writeOnlyFd, stdinFd);
        _close(writeOnlyFd);
#else
        const int stdinFd = fileno(stdin);
        m_savedStdinFd = dup(stdinFd);
        if (m_savedStdinFd == -1)
            return SLANG_FAIL;

        const int writeOnlyFd = open("/dev/null", O_WRONLY);
        if (writeOnlyFd == -1)
            return SLANG_FAIL;

        const int result = dup2(writeOnlyFd, stdinFd);
        close(writeOnlyFd);
#endif
        if (result == -1)
            return SLANG_FAIL;

        clearerr(stdin);
        return SLANG_OK;
    }

    ~ScopedWriteOnlyStdin()
    {
        if (m_savedStdinFd == -1)
            return;

#ifdef _WIN32
        _dup2(m_savedStdinFd, _fileno(stdin));
        _close(m_savedStdinFd);
#else
        dup2(m_savedStdinFd, fileno(stdin));
        close(m_savedStdinFd);
#endif
        clearerr(stdin);
    }

private:
    int m_savedStdinFd = -1;
};

static std::mutex g_stdinRedirectMutex;

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
    ExecuteResult& outResult)
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
            if (SLANG_FAILED(writeResult))
                return writeResult;
        }
    }
    stdinStream->close();

    return ProcessUtil::readUntilTermination(process, outResult);
}

static SlangResult _runSlangcWithRepeatedStdin(
    UnitTestContext* context,
    const List<String>& args,
    size_t byteCount,
    ExecuteResult& outResult)
{
    CommandLine cmdLine;
    ExecutableLocation slangcLocation(context->executableDirectory, "slangc");
    cmdLine.setExecutableLocation(slangcLocation);
    for (const auto& arg : args)
        cmdLine.addArg(arg);

    RefPtr<Process> process;
    SLANG_RETURN_ON_FAIL(Process::create(cmdLine, 0, process));

    char chunk[16 * 1024];
    memset(chunk, ' ', sizeof(chunk));

    Stream* stdinStream = process->getStream(StdStreamType::In);
    size_t bytesRemaining = byteCount;
    while (bytesRemaining != 0)
    {
        const size_t writeSize = bytesRemaining < sizeof(chunk) ? bytesRemaining : sizeof(chunk);
        const SlangResult writeResult = stdinStream->write(chunk, writeSize);
        if (SLANG_FAILED(writeResult))
            break;
        bytesRemaining -= writeSize;
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

static SlangResult _testInputTooLargeDiagnostic(UnitTestContext* context)
{
    List<String> args;
    _addStdinCompileArgs(args, "slang");

    const size_t maxStdinBytes = size_t(256) << 20;
    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangcWithRepeatedStdin(context, args, maxStdinBytes + 1, result));

    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "stdin input exceeds the maximum allowed size"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testCannotReadFromStdinDiagnostic(UnitTestContext* context)
{
    std::lock_guard<std::mutex> lock(g_stdinRedirectMutex);

    ScopedWriteOnlyStdin stdinRedirect;
    SLANG_RETURN_ON_FAIL(stdinRedirect.redirect());

    SlangCompileRequest* compileRequest = spCreateCompileRequest(context->slangGlobalSession);
    if (!compileRequest)
        return SLANG_FAIL;

    StringBuilder diagnostics;
    spSetDiagnosticCallback(compileRequest, _appendDiagnostic, &diagnostics);
    spSetCommandLineCompilerMode(compileRequest);

    const char* args[] = {
        "-lang",
        "slang",
        "-target",
        "spirv-asm",
        "-entry",
        "main",
        "-stage",
        "compute",
        "--",
        "-",
    };
    const SlangResult result =
        spProcessCommandLineArguments(compileRequest, args, SLANG_COUNT_OF(args));
    spDestroyCompileRequest(compileRequest);

    if (SLANG_SUCCEEDED(result))
        return SLANG_FAIL;

    const String diagnosticText = diagnostics.produceString();
    if (!_contains(diagnosticText, "failed to read source from stdin"))
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

static const char* kCoverageCliShader = R"(
RWStructuredBuffer<uint> outputBuffer;

void helper(inout uint value)
{
    value += 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint value = tid.x;
    helper(value);
    if ((value & 1u) == 0u)
        value += 2;
    else
        value += 3;
    outputBuffer[0] = value;
}
)";

struct TempCoverageCliFiles
{
    String basePath;
    String sourcePath;
    String outputPath;
    String autoManifestPath;
    String disassemblyOutputPath;
    String disassemblyManifestPath;
    String explicitManifestPath;

    ~TempCoverageCliFiles()
    {
        File::remove(basePath);
        File::remove(sourcePath);
        File::remove(outputPath);
        File::remove(autoManifestPath);
        File::remove(disassemblyOutputPath);
        File::remove(disassemblyManifestPath);
        File::remove(explicitManifestPath);
    }
};

static SlangResult _createTempCoverageCliFiles(TempCoverageCliFiles& out)
{
    SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slangc-coverage-cli"), out.basePath));
    out.sourcePath = out.basePath + ".slang";
    out.outputPath = out.basePath + ".spv";
    out.autoManifestPath = out.outputPath + ".coverage-mapping.json";
    out.disassemblyOutputPath = out.basePath + ".spvasm";
    out.disassemblyManifestPath = out.disassemblyOutputPath + ".coverage-mapping.json";
    out.explicitManifestPath = out.basePath + ".coverage-mapping.json";
    return File::writeAllText(out.sourcePath, kCoverageCliShader);
}

static void _addCoverageCliCompileArgs(
    List<String>& args,
    const String& sourcePath,
    bool enableCoverage,
    const char* target = "spirv")
{
    args.add(sourcePath);
    args.add("-target");
    args.add(target);
    args.add("-entry");
    args.add("main");
    args.add("-stage");
    args.add("compute");
    if (enableCoverage)
    {
        args.add("-trace-coverage");
        args.add("-trace-function-coverage");
        args.add("-trace-branch-coverage");
    }
}

static SlangResult _runSlangc(
    UnitTestContext* context,
    const List<String>& args,
    ExecuteResult& out)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, "slangc"));
    for (const auto& arg : args)
        cmdLine.addArg(arg);

    return ProcessUtil::execute(cmdLine, out);
}

static SlangResult _checkCoverageManifest(const String& path)
{
    if (!File::exists(path))
        return SLANG_FAIL;

    String manifest;
    SLANG_RETURN_ON_FAIL(File::readAllText(path, manifest));
    if (!_contains(manifest, "\"version\"") || !_contains(manifest, "\"counter_count\"") ||
        !_contains(manifest, "\"entries\"") || !_contains(manifest, "\"buffer\""))
        return SLANG_FAIL;
    if (!_contains(manifest, "\"line\"") || !_contains(manifest, "\"function\"") ||
        !_contains(manifest, "\"branch\""))
        return SLANG_FAIL;
    if (!_contains(manifest, "__slang_coverage"))
        return SLANG_FAIL;

    return SLANG_OK;
}

static SlangResult _testCoverageAutoSidecar(UnitTestContext* context)
{
    TempCoverageCliFiles files;
    SLANG_RETURN_ON_FAIL(_createTempCoverageCliFiles(files));

    List<String> args;
    _addCoverageCliCompileArgs(args, files.sourcePath, true);
    args.add("-o");
    args.add(files.outputPath);

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangc(context, args, result));
    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (!File::exists(files.outputPath))
        return SLANG_FAIL;
    return _checkCoverageManifest(files.autoManifestPath);
}

static SlangResult _testCoverageAutoSidecarForDisassembly(UnitTestContext* context)
{
    TempCoverageCliFiles files;
    SLANG_RETURN_ON_FAIL(_createTempCoverageCliFiles(files));

    List<String> args;
    _addCoverageCliCompileArgs(args, files.sourcePath, true, "spirv-asm");
    args.add("-o");
    args.add(files.disassemblyOutputPath);

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangc(context, args, result));
    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (!File::exists(files.disassemblyOutputPath))
        return SLANG_FAIL;
    return _checkCoverageManifest(files.disassemblyManifestPath);
}

static SlangResult _testCoverageExplicitSidecar(UnitTestContext* context)
{
    TempCoverageCliFiles files;
    SLANG_RETURN_ON_FAIL(_createTempCoverageCliFiles(files));

    List<String> args;
    _addCoverageCliCompileArgs(args, files.sourcePath, true);
    args.add("-coverage-mapping-output");
    args.add(files.explicitManifestPath);
    args.add("-o");
    args.add(files.outputPath);

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangc(context, args, result));
    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (!File::exists(files.outputPath))
        return SLANG_FAIL;
    if (File::exists(files.autoManifestPath))
        return SLANG_FAIL;
    return _checkCoverageManifest(files.explicitManifestPath);
}

static SlangResult _testCoverageExplicitSidecarWithStdoutArtifact(UnitTestContext* context)
{
    TempCoverageCliFiles files;
    SLANG_RETURN_ON_FAIL(_createTempCoverageCliFiles(files));

    List<String> args;
    _addCoverageCliCompileArgs(args, files.sourcePath, true);
    args.add("-coverage-mapping-output");
    args.add(files.explicitManifestPath);

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangc(context, args, result));
    if (result.resultCode != 0)
        return SLANG_FAIL;
    if (result.standardOutput.getLength() == 0)
        return SLANG_FAIL;
    return _checkCoverageManifest(files.explicitManifestPath);
}

static SlangResult _testCoverageExplicitSidecarRequiresCoverage(UnitTestContext* context)
{
    TempCoverageCliFiles files;
    SLANG_RETURN_ON_FAIL(_createTempCoverageCliFiles(files));

    List<String> args;
    _addCoverageCliCompileArgs(args, files.sourcePath, false);
    args.add("-coverage-mapping-output");
    args.add(files.explicitManifestPath);

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_runSlangc(context, args, result));
    if (result.resultCode == 0)
        return SLANG_FAIL;
    if (!_contains(result.standardError, "`-coverage-mapping-output` requires"))
        return SLANG_FAIL;
    if (File::exists(files.explicitManifestPath))
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
    SLANG_CHECK(SLANG_SUCCEEDED(_testInputTooLargeDiagnostic(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCannotReadFromStdinDiagnostic(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testHelpMentionsStdin(unitTestContext)));
}

SLANG_UNIT_TEST(SlangcCoverageMappingOutput)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_testCoverageAutoSidecar(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCoverageAutoSidecarForDisassembly(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCoverageExplicitSidecar(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCoverageExplicitSidecarWithStdoutArtifact(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testCoverageExplicitSidecarRequiresCoverage(unitTestContext)));
}
