// unit-test-stdin-compile.cpp
//
// Tests that slangc can read shader source from stdin when '-' is passed as the input path.

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

#if SLANG_UNIX_FAMILY
// For Case 11: fork+exec with a custom stdin fd.
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#elif SLANG_WINDOWS_FAMILY
// For Case 11: CreateProcess with a write-only pipe handle as stdin.
#include <windows.h>
#endif

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
// Both inputs must have the .slang extension (or be recognised as Slang by extension) so
// that addInputPath routes them through addInputSlangPath, which reuses
// m_slangTranslationUnitIndex and places both sources into the same translation unit.
// A future refactor that allocates a fresh TU for stdin would break cross-file visibility
// and fail this test.
static SlangResult _testStdinMixedWithFile(UnitTestContext* context)
{
    // Generate a uniquely-named temp file in the OS temp directory so the test
    // is safe in read-only cwds and parallel/re-run CI environments.
    // We append ".slang" so the file is routed through addInputSlangPath, which is
    // the path that sets/reuses m_slangTranslationUnitIndex.  Both the base path
    // (created by generateTemporary) and the ".slang" path must be removed on exit.
    String basePath;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slangc-stdin-helper"), basePath));
    const String helperPath = basePath + ".slang";

    // RAII guard: removes both the base inode (created by generateTemporary) and
    // the ".slang" file written below on any return path, including early-out failures.
    struct FileGuard
    {
        String base, slang;
        ~FileGuard()
        {
            File::remove(base);
            File::remove(slang);
        }
    } guard{basePath, helperPath};

    SLANG_RETURN_ON_FAIL(File::writeAllText(helperPath, "void foo() {}\n"));

    List<String> args;
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

// Case 11: pass a write-only fd/handle as slangc's stdin to trigger diagnostic 107.
// The stdin read in addInputPath uses fread/ferror directly (not PipeStream/readAll).
// POSIX: fread on an O_WRONLY fd calls read(2), which returns EBADF; fread returns 0
//        and sets ferror(stdin), satisfying the if (ferror(stdin)) guard → diagnostic 107.
// Windows: fread on a write-only pipe handle goes through the CRT, which calls ReadFile;
//          ReadFile fails on a write-only handle, the CRT sets ferror(stdin) → diagnostic 107.
static SlangResult _testStdinReadError(UnitTestContext* context)
{
#if SLANG_UNIX_FAMILY
    // Create a temp file, close it, and reopen it write-only.
    char tmpPath[] = "/tmp/slangc-stdin-err-XXXXXX";
    const int tmpRW = mkstemp(tmpPath);
    if (tmpRW < 0)
        return SLANG_FAIL;
    close(tmpRW);

    const int wfd = ::open(tmpPath, O_WRONLY);
    ::unlink(tmpPath); // delete the path; wfd keeps the inode alive until closed
    if (wfd < 0)
        return SLANG_FAIL;

    // Pipe to capture slangc's stderr so we can inspect the diagnostic text.
    int stderrPipe[2];
    if (::pipe(stderrPipe) != 0)
    {
        ::close(wfd);
        return SLANG_FAIL;
    }

    const String slangcPath = String(context->executableDirectory) + "/slangc";

    const pid_t pid = ::fork();
    if (pid < 0)
    {
        ::close(wfd);
        ::close(stderrPipe[0]);
        ::close(stderrPipe[1]);
        return SLANG_FAIL;
    }

    if (pid == 0)
    {
        // Child: wire fds, then exec slangc.
        // fd 0 (stdin) = write-only file fd → read(0,...) returns EBADF → diagnostic 107.
        ::dup2(wfd, STDIN_FILENO);
        ::dup2(stderrPipe[1], STDERR_FILENO);
        const int devNull = ::open("/dev/null", O_WRONLY);
        if (devNull >= 0)
        {
            ::dup2(devNull, STDOUT_FILENO);
            ::close(devNull);
        }
        ::close(wfd);
        ::close(stderrPipe[0]);
        ::close(stderrPipe[1]);

        ::execl(
            slangcPath.getBuffer(),
            slangcPath.getBuffer(),
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
            (char*)nullptr);
        ::_exit(127); // execl failed
    }

    // Parent: close the fds the child inherited.
    ::close(wfd);
    ::close(stderrPipe[1]);

    // Drain slangc's stderr.
    List<Byte> stderrBytes;
    {
        char buf[256];
        ssize_t n;
        while ((n = ::read(stderrPipe[0], buf, sizeof(buf))) > 0)
            for (ssize_t i = 0; i < n; ++i)
                stderrBytes.add(Byte(buf[i]));
    }
    ::close(stderrPipe[0]);

    int status = 0;
    ::waitpid(pid, &status, 0);
    const int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    if (exitCode == 0)
        return SLANG_FAIL;

    const UnownedStringSlice stderrSlice(
        (const char*)stderrBytes.getBuffer(), size_t(stderrBytes.getCount()));
    if (stderrSlice.indexOf(toSlice("failed to read source from stdin")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
#elif SLANG_WINDOWS_FAMILY
    // Create an anonymous pipe and pass the write end as the child's stdin.
    // fread in slangc calls ReadFile via the CRT; ReadFile fails on a write-only
    // pipe handle, the CRT sets the stdio error indicator, and ferror(stdin)
    // fires in addInputPath → diagnostic 107.
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE stdinRead = INVALID_HANDLE_VALUE;
    HANDLE stdinWrite = INVALID_HANDLE_VALUE;
    if (!CreatePipe(&stdinRead, &stdinWrite, &sa, 0))
        return SLANG_FAIL;
    CloseHandle(stdinRead); // only the write end is passed to the child

    // Pipe to capture slangc's stderr.
    HANDLE stderrRead = INVALID_HANDLE_VALUE;
    HANDLE stderrWrite = INVALID_HANDLE_VALUE;
    if (!CreatePipe(&stderrRead, &stderrWrite, &sa, 0))
    {
        CloseHandle(stdinWrite);
        return SLANG_FAIL;
    }
    SetHandleInformation(stderrRead, HANDLE_FLAG_INHERIT, 0); // parent-only

    HANDLE devNull =
        CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, nullptr);

    const String slangcPath = String(context->executableDirectory) + "/slangc.exe";

    // Build a mutable command-line buffer; CreateProcessA requires non-const lpCommandLine.
    const String cmdStr = String("\"") + slangcPath +
                          "\" -lang slang -target spirv-asm -entry main -stage compute -- -";
    List<char> cmdBuf;
    for (Index i = 0; i < cmdStr.getLength(); ++i)
        cmdBuf.add(cmdStr[i]);
    cmdBuf.add('\0');

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = stdinWrite; // write-only → PeekNamedPipe fails → diagnostic 107
    si.hStdOutput = devNull != INVALID_HANDLE_VALUE ? devNull : GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = stderrWrite;

    PROCESS_INFORMATION pi = {};
    const BOOL ok =
        CreateProcessA(nullptr, cmdBuf.getBuffer(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);

    CloseHandle(stdinWrite);
    CloseHandle(stderrWrite);
    if (devNull != INVALID_HANDLE_VALUE)
        CloseHandle(devNull);

    if (!ok)
    {
        CloseHandle(stderrRead);
        return SLANG_FAIL;
    }

    List<Byte> stderrBytes;
    {
        char buf[256];
        DWORD bytesRead = 0;
        while (ReadFile(stderrRead, buf, sizeof(buf), &bytesRead, nullptr) && bytesRead > 0)
            for (DWORD i = 0; i < bytesRead; ++i)
                stderrBytes.add(Byte(buf[i]));
    }
    CloseHandle(stderrRead);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode == 0)
        return SLANG_FAIL;

    const UnownedStringSlice stderrSlice(
        (const char*)stderrBytes.getBuffer(), size_t(stderrBytes.getCount()));
    if (stderrSlice.indexOf(toSlice("failed to read source from stdin")) < 0)
        return SLANG_FAIL;

    return SLANG_OK;
#else
    SLANG_UNUSED(context);
    return SLANG_OK;
#endif
}

// Case 12: GLSL via stdin without -stage fails. The shader stage cannot be inferred
// from a file extension when reading stdin, so pass-through GLSL compilation requires
// an explicit -stage flag. This pins that omitting it does NOT silently succeed — a
// future change that adds an implicit default stage would be caught here.
static SlangResult _testStdinGlslWithoutStage(UnitTestContext* context)
{
    List<String> args;
    args.add("-lang");
    args.add("glsl");
    args.add("-target");
    args.add("spirv-asm");
    args.add("--");
    args.add("-");

    // Use the same source that Case 8 proves compiles when -stage compute is given.
    // This ensures the failure here is due to the missing stage, not invalid source.
    const char* source =
        "#version 450\n"
        "layout(local_size_x = 1) in;\n"
        "void main() {}\n";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    if (result.resultCode == 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Helper: set an environment variable for the duration of a scope, then restore it.
struct ScopedEnvVar
{
    const char* key;
    bool hadOld;
    String oldVal;

    ScopedEnvVar(const char* k, const char* v) : key(k), hadOld(false)
    {
        if (const char* cur = getenv(k))
        {
            hadOld = true;
            oldVal = cur;
        }
#ifdef _WIN32
        String s = String(k) + "=" + v;
        _putenv(s.getBuffer());
#else
        setenv(k, v, 1);
#endif
    }
    ~ScopedEnvVar()
    {
#ifdef _WIN32
        String s = String(key) + "=" + (hadOld ? oldVal.getBuffer() : "");
        _putenv(s.getBuffer());
#else
        if (hadOld)
            setenv(key, oldVal.getBuffer(), 1);
        else
            unsetenv(key);
#endif
    }
};

// Case 13: stdin input exceeds the cap → StdinInputTooLarge diagnostic (error 108).
// The production cap is 256 MiB; we lower it to 10 bytes via SLANG_TEST_MAX_STDIN_BYTES
// so the test can trigger the diagnostic without allocating hundreds of megabytes.
static SlangResult _testStdinTooLarge(UnitTestContext* context)
{
    ScopedEnvVar capEnv("SLANG_TEST_MAX_STDIN_BYTES", "10");

    List<String> args;
    args.add("-lang");
    args.add("slang");
    args.add("-target");
    args.add("spirv-asm");
    args.add("--");
    args.add("-");

    // 11 bytes — one more than the cap set above.
    const char* source = "12345678901";

    ExecuteResult result;
    SLANG_RETURN_ON_FAIL(_spawnSlangcWithStdin(context, args, source, result));

    // Must fail with a non-zero exit code.
    if (result.resultCode == 0)
        return SLANG_FAIL;

    // The StdinInputTooLarge message must appear in the output.
    if (result.standardError.getUnownedSlice().indexOf(
            toSlice("stdin input exceeds the maximum allowed size")) < 0)
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
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinReadError(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinGlslWithoutStage(unitTestContext)));
    SLANG_CHECK(SLANG_SUCCEEDED(_testStdinTooLarge(unitTestContext)));
}
