// os.cpp
#include "os.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Platform-specific code follows

#ifdef _WIN32

#include <Windows.h>

static bool advance(OSFindFilesResult& result)
{
    return FindNextFileW(result.findHandle_, &result.fileData_) != 0;
}

static bool adjustToValidResult(OSFindFilesResult& result)
{
    for (;;)
    {
        if ((result.fileData_.dwFileAttributes & result.requiredMask_) != result.requiredMask_)
            goto skip;

        if ((result.fileData_.dwFileAttributes & result.disallowedMask_) != 0)
            goto skip;

        if (wcscmp(result.fileData_.cFileName, L".") == 0)
            goto skip;

        if (wcscmp(result.fileData_.cFileName, L"..") == 0)
            goto skip;

        result.filePath_ = result.directoryPath_ + String::fromWString(result.fileData_.cFileName);
        if (result.fileData_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            result.filePath_ = result.filePath_ + "/";

        return true;

    skip:
        if (!advance(result))
            return false;
    }
}


bool OSFindFilesResult::findNextFile()
{
    if (!advance(*this)) return false;
    return adjustToValidResult(*this);
}

OSFindFilesResult osFindFilesInDirectoryMatchingPattern(
    Slang::String directoryPath,
    Slang::String pattern)
{
    // TODO: add separator to end of directory path if needed

    String searchPath = directoryPath + pattern;

    OSFindFilesResult result;
    HANDLE findHandle = FindFirstFileW(
        searchPath.toWString(),
        &result.fileData_);

    result.directoryPath_ = directoryPath;
    result.findHandle_ = findHandle;
    result.requiredMask_ = 0;
    result.disallowedMask_ = FILE_ATTRIBUTE_DIRECTORY;

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        result.findHandle_ = NULL;
        result.error_ = kOSError_FileNotFound;
        return result;
    }

    result.error_ = kOSError_None;
    if (!adjustToValidResult(result))
    {
        result.findHandle_ = NULL;
    }
    return result;
}

OSFindFilesResult osFindFilesInDirectory(
    Slang::String directoryPath)
{
    return osFindFilesInDirectoryMatchingPattern(directoryPath, "*");
}

OSFindFilesResult osFindChildDirectories(
    Slang::String directoryPath)
{
    // TODO: add separator to end of directory path if needed

    String searchPath = directoryPath + "*";

    OSFindFilesResult result;
    HANDLE findHandle = FindFirstFileW(
        searchPath.toWString(),
        &result.fileData_);

    result.directoryPath_ = directoryPath;
    result.findHandle_ = findHandle;
    result.requiredMask_ = FILE_ATTRIBUTE_DIRECTORY;
    result.disallowedMask_ = 0;

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        result.findHandle_ = NULL;
        result.error_ = kOSError_FileNotFound;
        return result;
    }

    result.error_ = kOSError_None;
    if (!adjustToValidResult(result))
    {
        result.findHandle_ = NULL;
    }
    return result;
}

#else

static bool advance(OSFindFilesResult& result)
{
    result.entry_ = readdir(result.directory_);
    return result.entry_ != NULL;
}

static bool checkValidResult(OSFindFilesResult& result)
{
//    fprintf(stderr, "checkValidResullt(%s)\n", result.entry_->d_name);

    if (strcmp(result.entry_->d_name, ".") == 0)
        return false;

    if (strcmp(result.entry_->d_name, "..") == 0)
        return false;

    String path = result.directoryPath_
        + String(result.entry_->d_name);

//    fprintf(stderr, "stat(%s)\n", path.getBuffer());
    struct stat fileInfo;
    if(stat(path.getBuffer(), &fileInfo) != 0)
        return false;

    if(S_ISDIR(fileInfo.st_mode))
        path = path + "/";


    result.filePath_ = path;
    return true;    
}

static bool adjustToValidResult(OSFindFilesResult& result)
{
    for (;;)
    {
        if(checkValidResult(result))
            return true;

        if (!advance(result))
            return false;
    }
}


bool OSFindFilesResult::findNextFile()
{
//    fprintf(stderr, "OSFindFilesResult::findNextFile()\n");
    if (!advance(*this)) return false;
    return adjustToValidResult(*this);
}

OSFindFilesResult osFindFilesInDirectory(
    Slang::String directoryPath)
{
    OSFindFilesResult result;

//    fprintf(stderr, "osFindFilesInDirectory(%s)\n", directoryPath.getBuffer());

    result.directory_ = opendir(directoryPath.getBuffer());
    if(!result.directory_)
    {
        result.entry_ = NULL;
        return result;
    }

    result.directoryPath_ = directoryPath;
    result.findNextFile();
    return result;
}

OSFindFilesResult osFindChildDirectories(
    Slang::String directoryPath)
{
    OSFindFilesResult result;

    result.directory_ = opendir(directoryPath.getBuffer());
    if(!result.directory_)
    {
        result.entry_ = NULL;
        return result;
    }

    // TODO: Set attributes to ignore everything but directories

    result.directoryPath_ = directoryPath;
    result.findNextFile();
    return result;
}

#endif
