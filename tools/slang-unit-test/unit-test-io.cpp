// unit-test-io.cpp

#include "../../source/core/slang-io.h"
#include "unit-test/slang-unit-test.h"

#if SLANG_WINDOWS_FAMILY
#include <windows.h>
#include <winioctl.h>
#endif

using namespace Slang;

static SlangResult _checkGenerateTemporary()
{
    /// Test temporary file functionality

    List<String> paths;

    for (Index i = 0; i < 10; ++i)
    {
        String path;
        SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slang-check"), path));

        // The path should exist exist
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
    if (SLANG_FAILED(_setSparseFileSize(path, kLargeFileSize)))
    {
        File::remove(path);
        return SLANG_FAIL;
    }

    SLANG_CHECK(File::exists(path));

    const SlangPathType kInvalidPathType = static_cast<SlangPathType>(-1);
    SlangPathType pathType = kInvalidPathType;
    SlangResult pathTypeResult = Path::getPathType(path, &pathType);
    if (SLANG_FAILED(pathTypeResult))
    {
        File::remove(path);
        return pathTypeResult;
    }
    SLANG_CHECK(pathType == SLANG_PATH_TYPE_FILE);

    SLANG_RETURN_ON_FAIL(File::remove(path));
    return SLANG_OK;
}
#endif

SLANG_UNIT_TEST(io)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_checkGenerateTemporary()));
}

SLANG_UNIT_TEST(ioLargeFileExists)
{
#if SLANG_WINDOWS_FAMILY
    SLANG_CHECK(SLANG_SUCCEEDED(_checkLargeFileExists()));
#else
    SLANG_IGNORE_TEST
#endif
}
