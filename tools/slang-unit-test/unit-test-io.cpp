// unit-test-io.cpp

#include "core/slang-io.h"
#include "core/slang-stream.h"
#include "unit-test/slang-unit-test.h"

#if SLANG_WINDOWS_FAMILY
#include <windows.h>
#include <winioctl.h>
#else
#include <sys/stat.h>
#endif
#include <limits>

using namespace Slang;

static SlangResult _checkGenerateTemporary()
{
    /// Test temporary file functionality

    List<String> paths;

    for (Index i = 0; i < 10; ++i)
    {
        String path;
        SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slang-check"), path));

        // The path should exist
        SLANG_CHECK(File::exists(path));

        if (paths.contains(path))
        {
            return SLANG_FAIL;
        }

        paths.add(path);
    }

    // It should be possible to write to the temporary files
    for (auto& path : paths)
    {
        SLANG_RETURN_ON_FAIL(File::writeAllText(path, path));
    }
    // It should be possible to read from the temporary files

    for (auto& path : paths)
    {
        String contents;
        SLANG_RETURN_ON_FAIL(File::readAllText(path, contents))

        SLANG_CHECK(contents == path);
    }

    // Remove all the temporary files
    for (auto& path : paths)
    {
        SLANG_CHECK(File::exists(path));

        const auto removeResult = File::remove(path);
        SLANG_CHECK(SLANG_SUCCEEDED(removeResult));

        // Check remove worked
        SLANG_CHECK(!File::exists(path));
    }

    return SLANG_OK;
}

#if SLANG_WINDOWS_FAMILY
static SlangResult _setSparseFileSize(const String& path, Int64 size)
{
    HANDLE handle = ::CreateFileW(
        path.toWString(),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return SLANG_FAIL;
    }

    SlangResult result = SLANG_OK;
    DWORD bytesReturned = 0;
    if (!::DeviceIoControl(
            handle,
            FSCTL_SET_SPARSE,
            nullptr,
            0,
            nullptr,
            0,
            &bytesReturned,
            nullptr))
    {
        result = SLANG_FAIL;
        goto cleanup;
    }

    LARGE_INTEGER offset;
    offset.QuadPart = size;
    if (!::SetFilePointerEx(handle, offset, nullptr, FILE_BEGIN))
    {
        result = SLANG_FAIL;
        goto cleanup;
    }
    if (!::SetEndOfFile(handle))
    {
        result = SLANG_FAIL;
        goto cleanup;
    }

cleanup:
    ::CloseHandle(handle);
    return result;
}

static SlangResult _checkLargeFileExists()
{
    String path;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slang-large"), path));

    const Int64 kTwoGB = Int64(2) * 1024 * 1024 * 1024;
    const Int64 kOneKB = 1024;
    const Int64 kLargeFileSize = kTwoGB + kOneKB;
    const SlangPathType kInvalidPathType =
        static_cast<SlangPathType>(std::numeric_limits<SlangPathTypeIntegral>::max());
    SlangPathType pathType = kInvalidPathType;
    SlangResult pathTypeResult = SLANG_FAIL;
    SlangResult result = SLANG_OK;
    if (SLANG_FAILED(_setSparseFileSize(path, kLargeFileSize)))
    {
        result = SLANG_FAIL;
        goto cleanup;
    }

    SLANG_CHECK(File::exists(path));

    pathTypeResult = Path::getPathType(path, &pathType);
    if (SLANG_FAILED(pathTypeResult))
    {
        result = pathTypeResult;
        goto cleanup;
    }
    SLANG_CHECK(pathType == SLANG_PATH_TYPE_FILE);

cleanup:
    SlangResult removeResult = File::remove(path);
    if (SLANG_FAILED(removeResult))
    {
        return removeResult;
    }
    return result;
}
#endif

SLANG_UNIT_TEST(io)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_checkGenerateTemporary()));
}

// The host's null device is a character device, which `Path::getPathType` does not classify.
// Opening it for writing must still succeed, because callers pass it deliberately to discard
// output. Only this device is accepted, not character devices at large — see
// `fileStreamRefusesOtherCharacterDevices`.
SLANG_UNIT_TEST(fileStreamWritesToNullDevice)
{
    // The Windows spelling is matched case-insensitively, so check both cases there.
#if SLANG_WINDOWS_FAMILY
    const char* const nullDevicePaths[] = {"NUL", "nul"};
#else
    const char* const nullDevicePaths[] = {"/dev/null"};
#endif

    const char data[] = "discarded";
    for (const char* const nullDevicePath : nullDevicePaths)
    {
        FileStream stream;
        SLANG_CHECK(SLANG_SUCCEEDED(stream.init(
            nullDevicePath,
            FileMode::Create,
            FileAccess::Write,
            FileShare::ReadWrite)));
        SLANG_CHECK(SLANG_SUCCEEDED(stream.write(data, sizeof(data))));
        stream.close();

        SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllBytes(nullDevicePath, data, sizeof(data))));
    }
}

// A directory can never be opened as a stream, so it must still be refused. This is the case the
// existing path-type guard was written for, and relaxing the guard must not relax this.
SLANG_UNIT_TEST(fileStreamRefusesDirectory)
{
    String directoryPath;
    SLANG_CHECK(SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-io-dir"), directoryPath)));
    // `generateTemporary` leaves a file at the path; replace it with a directory of the same name.
    SLANG_CHECK(SLANG_SUCCEEDED(File::remove(directoryPath)));
    SLANG_CHECK(Path::createDirectory(directoryPath));

    SlangPathType pathType;
    SLANG_CHECK(SLANG_SUCCEEDED(Path::getPathType(directoryPath, &pathType)));
    SLANG_CHECK(pathType == SLANG_PATH_TYPE_DIRECTORY);

    FileStream stream;
    SLANG_CHECK(SLANG_FAILED(
        stream.init(directoryPath, FileMode::Create, FileAccess::Write, FileShare::ReadWrite)));
    SLANG_CHECK(SLANG_FAILED(File::writeAllBytes(directoryPath, "x", 1)));

    SLANG_CHECK(SLANG_SUCCEEDED(Path::remove(directoryPath)));
}

// Character devices other than the null device must stay refused, because they do not share its
// guarantee that writes are discarded and succeed. `/dev/full` fails every write with `ENOSPC`, and
// stdio buffering hides that until flush time, so accepting it would report success for output that
// was never written.
SLANG_UNIT_TEST(fileStreamRefusesOtherCharacterDevices)
{
#if SLANG_WINDOWS_FAMILY
    SLANG_IGNORE_TEST
#else
    if (!File::exists("/dev/full"))
    {
        SLANG_IGNORE_TEST
    }
    else
    {
        FileStream stream;
        SLANG_CHECK(SLANG_FAILED(
            stream.init("/dev/full", FileMode::Create, FileAccess::Write, FileShare::ReadWrite)));
    }
#endif
}

// A FIFO must be refused rather than opened. Opening one for writing blocks until a reader
// appears, so accepting it would hang the caller instead of failing — worse than the error it
// would replace.
SLANG_UNIT_TEST(fileStreamRefusesFifo)
{
#if SLANG_WINDOWS_FAMILY
    SLANG_IGNORE_TEST
#else
    String fifoPath;
    SLANG_CHECK(SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-io-fifo"), fifoPath)));
    SLANG_CHECK(SLANG_SUCCEEDED(File::remove(fifoPath)));
    SLANG_CHECK(::mkfifo(fifoPath.getBuffer(), 0600) == 0);

    FileStream stream;
    SLANG_CHECK(SLANG_FAILED(
        stream.init(fifoPath, FileMode::Create, FileAccess::Write, FileShare::ReadWrite)));

    SLANG_CHECK(SLANG_SUCCEEDED(File::remove(fifoPath)));
#endif
}

// A path that genuinely cannot be written must still report failure, so that callers keep
// diagnosing real write errors instead of silently succeeding.
SLANG_UNIT_TEST(fileStreamReportsUnwritablePath)
{
    const String unwritablePath =
        Path::combine("slang-nonexistent-directory-for-io-test", "output.bin");
    SLANG_CHECK(!File::exists(unwritablePath));

    FileStream stream;
    SLANG_CHECK(SLANG_FAILED(
        stream.init(unwritablePath, FileMode::Create, FileAccess::Write, FileShare::ReadWrite)));
    SLANG_CHECK(SLANG_FAILED(File::writeAllBytes(unwritablePath, "x", 1)));
}

// An ordinary file must still round-trip, so the relaxed guard has not disturbed the common path.
SLANG_UNIT_TEST(fileStreamRoundTripsRegularFile)
{
    String path;
    SLANG_CHECK(SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-io-file"), path)));

    const char data[] = "round trip";
    SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllBytes(path, data, sizeof(data))));

    List<unsigned char> readBack;
    SLANG_CHECK(SLANG_SUCCEEDED(File::readAllBytes(path, readBack)));
    SLANG_CHECK(readBack.getCount() == Index(sizeof(data)));
    SLANG_CHECK(::memcmp(readBack.getBuffer(), data, sizeof(data)) == 0);

    SLANG_CHECK(SLANG_SUCCEEDED(File::remove(path)));
}

SLANG_UNIT_TEST(ioLargeFileExists)
{
#if SLANG_WINDOWS_FAMILY
    SLANG_CHECK(SLANG_SUCCEEDED(_checkLargeFileExists()));
#else
    SLANG_IGNORE_TEST
#endif
}

SLANG_UNIT_TEST(uriGetPathPercentDecode)
{
    SLANG_CHECK(URI::fromString(toSlice("file://path%20name")).getPath() == "path name");
    SLANG_CHECK(URI::fromString(toSlice("file://%20")).getPath() == " ");
    SLANG_CHECK(URI::fromString(toSlice("file://a%20?x=1")).getPath() == "a ");
    SLANG_CHECK(URI::fromString(toSlice("file://path%")).getPath() == "path%");
    SLANG_CHECK(URI::fromString(toSlice("file://path%2")).getPath() == "path%2");
    SLANG_CHECK(URI::fromString(toSlice("file://path%2g")).getPath() == "path%2g");
    SLANG_CHECK(URI::fromString(toSlice("file://path%g0")).getPath() == "path%g0");
    SLANG_CHECK(URI::fromString(toSlice("file://path%gg")).getPath() == "path%gg");
    SLANG_CHECK(URI::fromString(toSlice("file://path%2?x=1")).getPath() == "path%2");
    SLANG_CHECK(URI::fromLocalFilePath(toSlice("path\tname")).getPath() == "path\tname");

    const char highBitPath[] = {'h', 'i', (char)0x80, (char)0xff, 0};
    String highBitRoundTrip = URI::fromLocalFilePath(UnownedStringSlice(highBitPath)).getPath();
    SLANG_CHECK(highBitRoundTrip.getUnownedSlice() == UnownedStringSlice(highBitPath));
}
