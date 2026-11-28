// slang-unix-crash-report-file.cpp
#include "core/slang-crash-report-file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

namespace Slang
{

/* static */ CrashReportFileResult CrashReportFile::writeNew(
    const char* path,
    const char* text,
    size_t length)
{
    const int file = ::open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (file < 0)
        return errno == EEXIST ? CrashReportFileResult::AlreadyExists
                               : CrashReportFileResult::Failed;

    // `write` can accept fewer bytes than it was given, and a signal can interrupt it, so we
    // continue until every byte is written or a real error stops us.
    CrashReportFileResult result = CrashReportFileResult::Written;
    size_t writtenLength = 0;
    while (writtenLength < length)
    {
        const ssize_t count = ::write(file, text + writtenLength, length - writtenLength);
        if (count > 0)
        {
            writtenLength += size_t(count);
        }
        else if (count < 0 && errno == EINTR)
        {
            continue;
        }
        else
        {
            result = CrashReportFileResult::Failed;
            break;
        }
    }

    ::close(file);
    return result;
}

/* static */ bool CrashReportFile::getTemporaryDirectory(char* outPath, size_t pathSize)
{
    const char* directory = ::getenv("TMPDIR");
    if (!directory || !directory[0])
        directory = "/tmp";

    // `TMPDIR` often ends with a separator. We leave it out, so that the caller can join a
    // file name to the directory in the same way on every platform.
    size_t length = strlen(directory);
    while (length > 1 && directory[length - 1] == '/')
        length--;

    if (length >= pathSize)
        return false;
    memcpy(outPath, directory, length);
    outPath[length] = 0;
    return true;
}

} // namespace Slang
