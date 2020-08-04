// os.cpp
#include "os.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Platform-specific code follows

#ifdef _WIN32

#include <Windows.h>

OSFindFilesResult::~OSFindFilesResult()
{
#ifdef WIN32
    if (m_findHandle)
    {
        ::FindClose(m_findHandle);
    }
#else
    if (m_directory)
    {
        closedir(m_directory);
    }
#endif
}

static bool advance(OSFindFilesResult& result)
{
    return FindNextFileW(result.m_findHandle, &result.m_fileData) != 0;
}

static bool adjustToValidResult(OSFindFilesResult& result)
{
    for (;;)
    {
        if ((result.m_fileData.dwFileAttributes & result.m_requiredMask) != result.m_requiredMask)
            goto skip;

        if ((result.m_fileData.dwFileAttributes & result.m_disallowedMask) != 0)
            goto skip;

        if (wcscmp(result.m_fileData.cFileName, L".") == 0)
            goto skip;

        if (wcscmp(result.m_fileData.cFileName, L"..") == 0)
            goto skip;

        result.m_filePath = result.m_directoryPath + String::fromWString(result.m_fileData.cFileName);
        if (result.m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            result.m_filePath = result.m_filePath + "/";

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
        &result.m_fileData);

    result.m_directoryPath = directoryPath;
    result.m_findHandle = findHandle;
    result.m_requiredMask = 0;
    result.m_disallowedMask = FILE_ATTRIBUTE_DIRECTORY;

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        result.m_findHandle = nullptr;
        result.m_error = kOSError_FileNotFound;
        return result;
    }

    result.m_error = kOSError_None;
    if (!adjustToValidResult(result))
    {
        result.m_findHandle = nullptr;
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
        &result.m_fileData);

    result.m_directoryPath = directoryPath;
    result.m_findHandle = findHandle;
    result.m_requiredMask = FILE_ATTRIBUTE_DIRECTORY;
    result.m_disallowedMask = 0;

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        result.m_findHandle = nullptr;
        result.m_error = kOSError_FileNotFound;
        return result;
    }

    result.m_error = kOSError_None;
    if (!adjustToValidResult(result))
    {
        result.m_findHandle = nullptr;
    }
    return result;
}

#else

static bool advance(OSFindFilesResult& result)
{
    result.m_entry = readdir(result.m_directory);
    return result.m_entry != nullptr;
}

static bool checkValidResult(OSFindFilesResult& result)
{
//    fprintf(stderr, "checkValidResullt(%s)\n", result.m_entry->d_name);

    if (strcmp(result.m_entry->d_name, ".") == 0)
        return false;

    if (strcmp(result.m_entry->d_name, "..") == 0)
        return false;

    String path = result.m_directoryPath
        + String(result.m_entry->d_name);

//    fprintf(stderr, "stat(%s)\n", path.getBuffer());
    struct stat fileInfo;
    if(stat(path.getBuffer(), &fileInfo) != 0)
        return false;

    if(S_ISDIR(fileInfo.st_mode))
        path = path + "/";


    result.m_filePath = path;
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

    result.m_directory = opendir(directoryPath.getBuffer());
    if(!result.m_directory)
    {
        result.m_entry = nullptr;
        return result;
    }

    result.m_directoryPath = directoryPath;
    result.findNextFile();
    return result;
}

OSFindFilesResult osFindChildDirectories(
    Slang::String directoryPath)
{
    OSFindFilesResult result;

    result.m_directory = opendir(directoryPath.getBuffer());
    if(!result.m_directory)
    {
        result.m_entry = nullptr;
        return result;
    }

    // TODO: Set attributes to ignore everything but directories

    result.m_directoryPath = directoryPath;
    result.findNextFile();
    return result;
}

#endif
