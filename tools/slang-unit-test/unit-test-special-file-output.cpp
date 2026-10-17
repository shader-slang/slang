// unit-test-special-file-output.cpp
// Tests for writing outputs to FIFOs and the null device, and for outputs that cannot be written.

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-stream.h"
#include "slang-com-ptr.h"
#include "slang-record-replay/replay-stream.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#if SLANG_UNIX_FAMILY
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <string.h>

using namespace Slang;

namespace
{

#if SLANG_WINDOWS_FAMILY
const char kNullDevice[] = "NUL";
#else
const char kNullDevice[] = "/dev/null";
#endif

struct ScopedTempDir
{
    String path;

    ~ScopedTempDir()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

/// Creates an empty temporary directory. `File::generateTemporary` reserves a unique name by
/// creating a file, which we replace with a directory of the same name.
SlangResult makeTempDir(const char* prefix, ScopedTempDir& out)
{
    String base;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), base));
    SLANG_RETURN_ON_FAIL(File::remove(base));
    if (!Path::createDirectoryRecursive(base))
        return SLANG_FAIL;
    out.path = base;
    return SLANG_OK;
}

/// Runs slangc on the compute entry point `main` of `slangPath`, writing `target` output to
/// `outputPath`, the dependency file to `depfilePath` and, if `reflectionPath` is not empty, the
/// reflection JSON to `reflectionPath`. Returns failure only if slangc could not be launched; its
/// exit code is in `outResult`.
SlangResult runSlangc(
    UnitTestContext* context,
    const char* target,
    const String& slangPath,
    const String& outputPath,
    const String& depfilePath,
    ExecuteResult& outResult,
    const String& reflectionPath = String())
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, "slangc"));
    cmdLine.addArg("-target");
    cmdLine.addArg(target);
    cmdLine.addArg("-entry");
    cmdLine.addArg("main");
    cmdLine.addArg("-stage");
    cmdLine.addArg("compute");
    cmdLine.addArg("-o");
    cmdLine.addArg(outputPath);
    cmdLine.addArg("-depfile");
    cmdLine.addArg(depfilePath);
    if (reflectionPath.getLength())
    {
        cmdLine.addArg("-reflection-json");
        cmdLine.addArg(reflectionPath);
    }
    cmdLine.addArg(slangPath);
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, outResult));
    if (outResult.resultCode != 0)
        getTestReporter()->message(TestMessageType::Info, outResult.standardError.getBuffer());
    return SLANG_OK;
}

/// Compiles the compute entry point `main` through the compile-request API, with no diagnostic
/// writer, emitting reflection JSON to `reflectionPath`. Returns the compile result and stores the
/// diagnostics the API reports in `outDiagnostics`.
SlangResult compileWithReflectionJson(
    UnitTestContext* context,
    const String& reflectionPath,
    bool hasTarget,
    String& outDiagnostics)
{
    ComPtr<slang::ICompileRequest> request;
    SLANG_RETURN_ON_FAIL(context->slangGlobalSession->createCompileRequest(request.writeRef()));
    const char* args[] = {"-reflection-json", reflectionPath.getBuffer()};
    SLANG_RETURN_ON_FAIL(request->processCommandLineArguments(args, SLANG_COUNT_OF(args)));
    if (hasTarget)
        request->setCodeGenTarget(SLANG_HLSL);
    const int translationUnit = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "m");
    request->addTranslationUnitSourceString(
        translationUnit,
        "m.slang",
        "[shader(\"compute\")] void main() {}");
    request->addEntryPoint(translationUnit, "main", SLANG_STAGE_COMPUTE);

    const SlangResult result = request->compile();
    const char* diagnostics = request->getDiagnosticOutput();
    outDiagnostics = diagnostics ? String(diagnostics) : String();
    return result;
}

#if SLANG_UNIX_FAMILY
/// A FIFO whose read end we hold open without blocking. Holding a reader lets a writer open the
/// FIFO immediately, and the small outputs in these tests fit in the pipe buffer, so the writer can
/// finish before we drain the FIFO.
struct FifoReader
{
    int fd = -1;

    ~FifoReader()
    {
        if (fd >= 0)
            ::close(fd);
    }

    SlangResult init(const String& path)
    {
        if (::mkfifo(path.getBuffer(), 0600) != 0)
            return SLANG_FAIL;
        fd = ::open(path.getBuffer(), O_RDONLY | O_NONBLOCK);
        return fd >= 0 ? SLANG_OK : SLANG_FAIL;
    }

    /// Returns the bytes buffered in the FIFO. Call only after every writer has closed it, because
    /// the non-blocking read stops at the first empty read.
    List<uint8_t> readAll()
    {
        List<uint8_t> bytes;
        uint8_t buffer[4096];
        ssize_t count;
        while ((count = ::read(fd, buffer, sizeof(buffer))) > 0)
            bytes.addRange(buffer, Index(count));
        return bytes;
    }
};
#endif

} // namespace

SLANG_UNIT_TEST(fileStreamOpenSpecialFiles)
{
    ScopedTempDir dir;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(makeTempDir("slang-special-file", dir)));

    {
        FileStream stream;
        SLANG_CHECK(SLANG_FAILED(stream.init(dir.path, FileMode::Open)));
        SLANG_CHECK(SLANG_FAILED(stream.init(dir.path, FileMode::Create)));
    }

    {
        FileStream stream;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(stream.init(kNullDevice, FileMode::Create)));
        SLANG_CHECK(SLANG_SUCCEEDED(stream.write("x", 1)));
    }
    {
        FileStream stream;
        SLANG_CHECK(SLANG_SUCCEEDED(
            stream.init(kNullDevice, FileMode::Append, FileAccess::Write, FileShare::ReadWrite)));
    }

#if SLANG_UNIX_FAMILY
    // Reading still requires a regular file, because the read helpers size a file by seeking to
    // its end. POSIX `stat` reports `/dev/null` as a character device.
    {
        FileStream stream;
        SLANG_CHECK(SLANG_FAILED(stream.init(kNullDevice, FileMode::Open)));
    }

    {
        const String fifoPath = Path::combine(dir.path, "fifo");
        FifoReader reader;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(reader.init(fifoPath)));
        {
            FileStream stream;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(stream.init(fifoPath, FileMode::Create)));
            SLANG_CHECK(SLANG_SUCCEEDED(stream.write("hello", 5)));
        }
        const List<uint8_t> bytes = reader.readAll();
        SLANG_CHECK(bytes.getCount() == 5 && ::memcmp(bytes.getBuffer(), "hello", 5) == 0);

        // We hold a writer open so that, if `Open` ever accepted a FIFO, `init` would return
        // instead of blocking until a writer appears.
        const int writerFd = ::open(fifoPath.getBuffer(), O_WRONLY | O_NONBLOCK);
        SLANG_CHECK_ABORT(writerFd >= 0);
        FileStream readStream;
        SLANG_CHECK(SLANG_FAILED(readStream.init(fifoPath, FileMode::Open)));
        ::close(writerFd);
    }
#endif
}

SLANG_UNIT_TEST(slangcOutputToSpecialFiles)
{
    ScopedTempDir dir;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(makeTempDir("slangc-special-file", dir)));
    const String slangPath = Path::combine(dir.path, "shader.slang");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(slangPath, "[shader(\"compute\")] void main() {}\n")));

    {
        ExecuteResult result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(runSlangc(
            unitTestContext,
            "spirv",
            slangPath,
            kNullDevice,
            kNullDevice,
            result,
            kNullDevice)));
        SLANG_CHECK(result.resultCode == 0);
    }

    {
        const String spirvPath = Path::combine(dir.path, "regular.spv");
        const String depfilePath = Path::combine(Path::combine(dir.path, "missing-dir"), "deps.d");
        ExecuteResult result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            runSlangc(unitTestContext, "spirv", slangPath, spirvPath, depfilePath, result)));
        SLANG_CHECK(result.resultCode != 0);
        const UnownedStringSlice diagnostics = result.standardError.getUnownedSlice();
        SLANG_CHECK(diagnostics.indexOf(toSlice("E00004")) >= 0);
        SLANG_CHECK(diagnostics.indexOf(toSlice("deps.d")) >= 0);
    }

#if SLANG_UNIX_FAMILY
    {
        const String spirvFifoPath = Path::combine(dir.path, "shader.spv");
        const String depfileFifoPath = Path::combine(dir.path, "shader.d");
        const String reflectionFifoPath = Path::combine(dir.path, "shader.json");
        FifoReader spirvReader;
        FifoReader depfileReader;
        FifoReader reflectionReader;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(spirvReader.init(spirvFifoPath)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(depfileReader.init(depfileFifoPath)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(reflectionReader.init(reflectionFifoPath)));

        ExecuteResult result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(runSlangc(
            unitTestContext,
            "spirv",
            slangPath,
            spirvFifoPath,
            depfileFifoPath,
            result,
            reflectionFifoPath)));
        SLANG_CHECK(result.resultCode == 0);

        const List<uint8_t> spirv = spirvReader.readAll();
        uint32_t magic = 0;
        if (spirv.getCount() >= Index(sizeof(magic)))
            ::memcpy(&magic, spirv.getBuffer(), sizeof(magic));
        SLANG_CHECK_MSG(magic == 0x07230203, "FIFO did not receive a SPIR-V module");

        const List<uint8_t> depfile = depfileReader.readAll();
        const UnownedStringSlice depfileText(
            (const char*)depfile.getBuffer(),
            (const char*)depfile.getBuffer() + depfile.getCount());
        SLANG_CHECK_MSG(
            depfileText.indexOf(toSlice("shader.slang")) >= 0,
            "FIFO did not receive the dependency file");

        const List<uint8_t> reflection = reflectionReader.readAll();
        SLANG_CHECK_MSG(
            reflection.getCount() > 0 && reflection[0] == '{',
            "FIFO did not receive the reflection JSON");
    }

    // Text targets take a different write path, which first tries to read the target back. That
    // read-back must be refused for a FIFO; if it were not, slangc would block here.
    {
        const String glslFifoPath = Path::combine(dir.path, "shader.glsl");
        FifoReader glslReader;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(glslReader.init(glslFifoPath)));

        ExecuteResult result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            runSlangc(unitTestContext, "glsl", slangPath, glslFifoPath, kNullDevice, result)));
        SLANG_CHECK(result.resultCode == 0);

        const List<uint8_t> glsl = glslReader.readAll();
        const UnownedStringSlice glslText(
            (const char*)glsl.getBuffer(),
            (const char*)glsl.getBuffer() + glsl.getCount());
        SLANG_CHECK_MSG(glslText.startsWith(toSlice("#version")), "FIFO did not receive GLSL");
    }
#endif
}

SLANG_UNIT_TEST(reflectionJsonFailureDiagnostics)
{
    ScopedTempDir dir;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(makeTempDir("slang-reflection-json", dir)));
    const String reflectionPath =
        Path::combine(Path::combine(dir.path, "missing-dir"), "reflection.json");

    // Without a diagnostic writer, API callers see diagnostics only through
    // `getDiagnosticOutput`.
    String diagnostics;
    SLANG_CHECK(SLANG_FAILED(
        compileWithReflectionJson(unitTestContext, reflectionPath, true, diagnostics)));
    SLANG_CHECK(diagnostics.indexOf(toSlice("E52004")) >= 0);
    SLANG_CHECK(diagnostics.indexOf(toSlice("reflection.json")) >= 0);

    SLANG_CHECK(SLANG_FAILED(
        compileWithReflectionJson(unitTestContext, reflectionPath, false, diagnostics)));
    SLANG_CHECK(diagnostics.indexOf(toSlice("E52009")) >= 0);
}

SLANG_UNIT_TEST(replayMirrorRefusesSpecialFiles)
{
#if SLANG_UNIX_FAMILY && SLANG_HAS_EXCEPTIONS
    ScopedTempDir dir;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(makeTempDir("slang-replay-mirror", dir)));

    // We hold a reader on the FIFO so that, if the mirror accepted it, opening it would not block.
    const String fifoPath = Path::combine(dir.path, "stream.bin");
    FifoReader reader;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(reader.init(fifoPath)));

    const String mirrorPaths[] = {String(kNullDevice), fifoPath};
    for (const auto& mirrorPath : mirrorPaths)
    {
        SlangRecord::ReplayStream stream;
        bool refused = false;
        try
        {
            stream.setMirrorFile(mirrorPath.getBuffer());
        }
        catch (const Exception&)
        {
            refused = true;
        }
        SLANG_CHECK(refused);
        SLANG_CHECK(!stream.hasMirrorFile());

        // Recording carries on without a mirror.
        const uint32_t value = 1;
        stream.write(&value, sizeof(value));
        SLANG_CHECK(stream.getSize() == sizeof(value));
    }
#else
    SLANG_IGNORE_TEST;
#endif
}
