// slang-win-crash-report-file.cpp
#include "core/slang-crash-report-file.h"

#include <windows.h>

namespace Slang
{

// The longest path, in UTF-16 units, that we convert. A report whose path is longer is not
// saved, which is the same outcome as any other path that cannot be created.
static const int kMaxPathLength = 1024;

/* static */ CrashReportFileResult CrashReportFile::writeNew(
    const char* path,
    const char* text,
    size_t length)
{
    wchar_t widePath[kMaxPathLength];
    if (::MultiByteToWideChar(CP_UTF8, 0, path, -1, widePath, kMaxPathLength) == 0)
        return CrashReportFileResult::Failed;

    // `CREATE_NEW` fails if the file exists, and the check and the creation are one step.
    const HANDLE file = ::CreateFileW(
        widePath,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return ::GetLastError() == ERROR_FILE_EXISTS ? CrashReportFileResult::AlreadyExists
                                                     : CrashReportFileResult::Failed;
    }

    CrashReportFileResult result = CrashReportFileResult::Written;
    size_t writtenLength = 0;
    while (writtenLength < length)
    {
        DWORD count = 0;
        const DWORD requestLength = DWORD(length - writtenLength);
        if (!::WriteFile(file, text + writtenLength, requestLength, &count, nullptr) || count == 0)
        {
            result = CrashReportFileResult::Failed;
            break;
        }
        writtenLength += count;
    }

    ::CloseHandle(file);
    return result;
}

/* static */ bool CrashReportFile::getTemporaryDirectory(char* outPath, size_t pathSize)
{
    wchar_t widePath[kMaxPathLength];
    const DWORD wideLength = ::GetTempPathW(kMaxPathLength, widePath);
    if (wideLength == 0 || wideLength >= DWORD(kMaxPathLength))
        return false;

    // The system returns the directory with a separator at its end. We remove it, so that the
    // caller can join a file name to the directory in the same way on every platform.
    DWORD length = wideLength;
    while (length > 0 && (widePath[length - 1] == L'\\' || widePath[length - 1] == L'/'))
        length--;
    widePath[length] = 0;

    return ::WideCharToMultiByte(
               CP_UTF8,
               0,
               widePath,
               -1,
               outPath,
               int(pathSize),
               nullptr,
               nullptr) != 0;
}

} // namespace Slang
