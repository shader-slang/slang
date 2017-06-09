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
    CoreLib::Basic::String				directoryPath_;
    CoreLib::Basic::String				filePath_;
#ifdef WIN32
    HANDLE				findHandle_;
    WIN32_FIND_DATAW	fileData_;
    DWORD				requiredMask_;
    DWORD				disallowedMask_;
    OSError				error_;
#else
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
        CoreLib::Basic::String const& operator*() const
        {
            return context_->filePath_;
        }
    };

    Iterator begin()
    {
#ifdef WIN32
        Iterator result = { findHandle_ ? this : NULL };
#else
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
// Each element in the range is a `CoreLib::Basic::String` representing the
// path to a subdirecotry of the directory.
OSFindFilesResult osFindChildDirectories(
    CoreLib::Basic::String directoryPath);

// Enumerate files in the given `directoryPath` that match the provided
// `pattern` as a simplified regex for files to return (e.g., "*.txt")
// and return a logical collection of the results
// that can be iterated with a range-based `for` loop:
//
// for( auto file : osFindFilesInDirectoryMatchingPattern(dir, "*.txt"))
// { ... }
//
// Each element in the range is a `CoreLib::Basic::String` representing the
// path to a file in the directory.
OSFindFilesResult osFindFilesInDirectoryMatchingPattern(
    CoreLib::Basic::String directoryPath,
    CoreLib::Basic::String pattern);

// Enumerate files in the given `directoryPath`  and return a logical
// collection of the results that can be iterated with a range-based
// `for` loop:
//
// for( auto file : osFindFilesInDirectory(dir))
// { ... }
//
// Each element in the range is a `CoreLib::Basic::String` representing the
// path to a file in the directory.
OSFindFilesResult osFindFilesInDirectory(
    CoreLib::Basic::String directoryPath);


// An `OSProcessSpawner` can be used to launch a process, and handles
// putting together the arguments in the form required by the target
// platform, as well as capturing any output from the process (both
// standard output and standard error) as strings.
struct OSProcessSpawner
{
    // Set the executable name for the process to be spawned.
    // Note: this call must be made before any arguments are pushed.
    void pushExecutableName(
        CoreLib::Basic::String executableName);

    // Append an argument for the process to be spawned.
    void pushArgument(
        CoreLib::Basic::String argument);

    // Attempt to spawn the process, and wait for it to complete.
    // Returns an error if the attempt to spawn and/or wait fails,
    // but returns `kOSError_None` if the process is run to completion,
    // whether or not the process returns "successfully" (with a zero
    // result code);
    OSError spawnAndWaitForCompletion();

    // If the process is successfully spawned and completes, then
    // the user can query the result code that the process produce
    // on exit, along with the output it wrote to stdout and stderr.
    typedef int ResultCode;
    ResultCode getResultCode() { return resultCode_; }
    CoreLib::Basic::String const& getStandardOutput() { return standardOutput_; }
    CoreLib::Basic::String const& getStandardError() { return standardError_; }

    // "private" data follows
    CoreLib::Basic::String standardOutput_;
    CoreLib::Basic::String standardError_;
    ResultCode resultCode_;
#ifdef WIN32
    CoreLib::Basic::String executableName_;
    CoreLib::Basic::StringBuilder commandLine_;
#else
#endif
};
