// unit-test-special-file-output.cpp
// Tests for writing outputs to FIFOs and the null device.

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-stream.h"
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

SlangResult runSlangc(
    UnitTestContext* context,
    const char* target,
    const String& slangPath,
    const String& outputPath,
    const String& depfilePath,
    ExecuteResult& outResult)
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
    cmdLine.addArg(slangPath);
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, outResult));
    if (outResult.resultCode != 0)
        getTestReporter()->message(TestMessageType::Info, outResult.standardError.getBuffer());
    return SLANG_OK;
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

    /// Returns everything written to the FIFO, once every writer has closed it.
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
    // its end.
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

        FileStream readStream;
        SLANG_CHECK(SLANG_FAILED(readStream.init(fifoPath, FileMode::Open)));
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
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            runSlangc(unitTestContext, "spirv", slangPath, kNullDevice, kNullDevice, result)));
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
        FifoReader spirvReader;
        FifoReader depfileReader;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(spirvReader.init(spirvFifoPath)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(depfileReader.init(depfileFifoPath)));

        ExecuteResult result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(runSlangc(
            unitTestContext,
            "spirv",
            slangPath,
            spirvFifoPath,
            depfileFifoPath,
            result)));
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
    }

    // Text targets take a different write path, which first tries to read the target back.
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
