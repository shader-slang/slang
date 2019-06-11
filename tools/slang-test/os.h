#ifndef SLANG_OS_H
#define SLANG_OS_H

// os.h

#include "../../source/core/slang-io.h"

// This file encapsulates the platform-specific operations needed by the test
// runner that are not already provided by the core Slang libs

#ifdef _WIN32

// Include Windows header in a way that minimized namespace pollution.
// TODO: We could try to avoid including this at all, but it would
// mean trying to hide certain struct layouts, which would add
// more dynamic allocation.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#else

#include <dirent.h>
#include <errno.h>
//#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#endif

// A simple set of error codes for possible runtime failures
enum OSError
{
    kOSError_None = 0,
    kOSError_InvalidArgument,
    kOSError_OperationFailed,
    kOSError_FileNotFound,
};

// A helper type used during enumeration of files in a directory.
struct OSFindFilesResult
{
    Slang::String				directoryPath_;
    Slang::String				filePath_;
#ifdef WIN32
    HANDLE				findHandle_;
    WIN32_FIND_DATAW	fileData_;
    DWORD				requiredMask_;
    DWORD				disallowedMask_;
    OSError				error_;
#else
    DIR*         directory_;
    dirent*      entry_;
#endif

    bool findNextFile();

    struct Iterator
    {
        OSFindFilesResult* context_;

        bool operator!=(Iterator other) const { return context_ != other.context_; }
        void operator++()
        {
            if (!context_->findNextFile())
            {
                context_ = NULL;
            }
        }
        Slang::String const& operator*() const
        {
            return context_->filePath_;
        }
    };

    Iterator begin()
    {
#ifdef WIN32
        Iterator result = { findHandle_ ? this : NULL };
#else
        Iterator result = { entry_ ? this : NULL };
#endif
        return result;
    }

    Iterator end()
    {
        Iterator result = { NULL };
        return result;
    }
};

// Enumerate subdirectories in the given `directoryPath` and return a logical
// collection of the results that can be iterated with a range-based
// `for` loop:
//
// for( auto subdir : osFindChildDirectories(dir))
// { ... }
//
// Each element in the range is a `Slang::String` representing the
// path to a subdirecotry of the directory.
OSFindFilesResult osFindChildDirectories(
    Slang::String directoryPath);

// Enumerate files in the given `directoryPath` that match the provided
// `pattern` as a simplified regex for files to return (e.g., "*.txt")
// and return a logical collection of the results
// that can be iterated with a range-based `for` loop:
//
// for( auto file : osFindFilesInDirectoryMatchingPattern(dir, "*.txt"))
// { ... }
//
// Each element in the range is a `Slang::String` representing the
// path to a file in the directory.
OSFindFilesResult osFindFilesInDirectoryMatchingPattern(
    Slang::String directoryPath,
    Slang::String pattern);

// Enumerate files in the given `directoryPath`  and return a logical
// collection of the results that can be iterated with a range-based
// `for` loop:
//
// for( auto file : osFindFilesInDirectory(dir))
// { ... }
//
// Each element in the range is a `Slang::String` representing the
// path to a file in the directory.
OSFindFilesResult osFindFilesInDirectory(
    Slang::String directoryPath);

#endif // SLANG_OS_H
