// slang-crash-report-file.h
#ifndef SLANG_CORE_CRASH_REPORT_FILE_H
#define SLANG_CORE_CRASH_REPORT_FILE_H

#include "slang-common.h"

#include <stddef.h>

namespace Slang
{

enum class CrashReportFileResult
{
    /// The file did not exist, and it now holds the text.
    Written,
    /// A file of that name was already there. It was left as it was.
    AlreadyExists,
    /// The file could not be created or written.
    Failed,
};

/// The file operations that saving a crash report needs.
///
/// We call the operating system directly, with fixed buffers and without the heap, because the
/// same code runs after a crash. Each platform has its own implementation.
struct CrashReportFile
{
    /// Create the file at `path`, which is UTF-8, and write `length` bytes of `text` to it,
    /// but only if no file exists at `path`.
    ///
    /// The file is created exclusively, which the operating system makes atomic. When several
    /// threads or processes report the same failure at once, exactly one of them writes the
    /// file and the others get `AlreadyExists`.
    static CrashReportFileResult writeNew(const char* path, const char* text, size_t length);

    /// Write the directory for temporary files to `outPath`, which holds `pathSize` bytes.
    /// The path has no separator at its end. Return false if there is no such directory or
    /// its path does not fit.
    static bool getTemporaryDirectory(char* outPath, size_t pathSize);
};

} // namespace Slang

#endif
