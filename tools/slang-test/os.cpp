// os.cpp
#include "os.h"

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
#include <fnmatch.h>

#endif

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

/* static */FindFilesResult FindFilesResult::findChildDirectories(const Slang::String& directoryPath)
{
    RefPtr<FindFilesState> state(FindFilesState::create());
    if (SLANG_FAILED(state->startFind(directoryPath, "*", FindTypeFlag::Directory)))
    {
        state.setNull();
    }
    return FindFilesResult(state);
}

/* static */FindFilesResult FindFilesResult::findFilesInDirectoryMatchingPattern(const Slang::String& directoryPath, const Slang::String& pattern)
{
    RefPtr<FindFilesState> state(FindFilesState::create());
    if (SLANG_FAILED(state->startFind(directoryPath, pattern, FindTypeFlag::Directory)))
    {
        state.setNull();
    }
    return FindFilesResult(state);
}

/* static */FindFilesResult FindFilesResult::findFilesInDirectory(const Slang::String& directoryPath)
{
    RefPtr<FindFilesState> state(FindFilesState::create());
    if (SLANG_FAILED(state->startFind(directoryPath, "*", FindTypeFlag::File)))
    {
        state.setNull();
    }
    return FindFilesResult(state);
}

#ifdef _WIN32

class WinFindFilesState : public FindFilesState
{
    public:
    virtual bool findNext() SLANG_OVERRIDE;
    virtual SlangResult startFind(const Slang::String& directoryPath, const Slang::String& pattern, FindTypeFlags flags) SLANG_OVERRIDE;
    virtual bool hasResult()  SLANG_OVERRIDE { return m_findHandle != nullptr; }
    
    WinFindFilesState():
        m_findHandle(nullptr)
    {
    }

    ~WinFindFilesState() { _close(); }

protected:
    void _close()
    {
        if (m_findHandle)
        {
            ::FindClose(m_findHandle);
            m_findHandle = nullptr;
            m_findType = FindType::Unknown;
        }
    }
    bool _advance()
    {
        m_findType = FindType::Unknown;
        if (m_findHandle == nullptr || FindNextFileW(m_findHandle, &m_fileData) == 0)
        {
            _close();
            return false;
        }
        return true;
    }

    HANDLE				m_findHandle;
    WIN32_FIND_DATAW	m_fileData;
};

bool WinFindFilesState::findNext()
{
    for (;;)
    {
        if (!_advance())
        {
            return false;
        }

        m_findType = (m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? FindType::Directory : FindType::File;

        const FindTypeFlags typeFlags = FindTypeFlags(1) << int(m_findType);

        if (((typeFlags & m_allowedTypes) == 0) ||
            (wcscmp(m_fileData.cFileName, L".") == 0) ||
            (wcscmp(m_fileData.cFileName, L"..") == 0))
        {
            continue;
        }

        m_filePath = m_directoryPath + String::fromWString(m_fileData.cFileName);

        if (m_findType == FindType::Directory)
            m_filePath.appendChar('/');

        return true;
    }
}

SlangResult WinFindFilesState::startFind(const Slang::String& directoryPath, const Slang::String& pattern, FindTypeFlags allowedTypes)
{
    _close();

    m_allowedTypes = allowedTypes;

    String searchPath = Path::combine(directoryPath, pattern);

    m_findHandle = FindFirstFileW(searchPath.toWString(), &m_fileData);
    if (!m_findHandle)
    {
        return SLANG_E_NOT_FOUND;
    }

    m_directoryPath = directoryPath;
    
    findNext();
    return SLANG_OK;
}

/* static */RefPtr<FindFilesState> FindFilesState::create()
{
    return new WinFindFilesState;
}

#else

class UnixFindFilesState : public FindFilesState
{
public:
    virtual bool findNext() SLANG_OVERRIDE;
    virtual SlangResult startFind(const String& directoryPath, const String& pattern, FindTypeFlags allowedTypes) SLANG_OVERRIDE;
    virtual bool hasResult()  SLANG_OVERRIDE { return m_directory != nullptr; }

    UnixFindFilesState():
        m_directory(nullptr),
        m_entry(nullptr)
    {}

    ~UnixFindFilesState() { _close(); }

protected:
    bool _advance();

    void _close()
    {
        if (m_directory)
        {
            closedir(m_directory);
            m_directory = nullptr;
            m_findType = FindType::Unknown;
        }
    }

    String m_pattern;       
    DIR* m_directory;
    dirent* m_entry;
};

bool UnixFindFilesState::_advance()
{
    m_findType = FindType::Unknown;
    if (m_directory)
    {
        m_entry = readdir(m_directory);
        if (m_entry == nullptr)
        {
            _close();
            return false;
        }    
        return true;
    }
    return false;
}

bool UnixFindFilesState::findNext()
{
    for (;;)
    {
        if (!_advance())
        {
            return false;
        }

        if (strcmp(m_entry->d_name, ".") == 0 ||
            strcmp(m_entry->d_name, "..") == 0)
        {
            continue;
        }

        // If there is a pattern, check if it matches, and if it doesn't ignore it
        if (m_pattern != "*" && fnmatch(m_pattern.begin(), m_entry->d_name, 0) != 0)
        {
            continue;
        }

        // Work out the path. Assumes there is a / in dir path
        m_filePath = m_directoryPath;
        m_filePath.append(m_entry->d_name);

        //    fprintf(stderr, "stat(%s)\n", path.getBuffer());
        struct stat fileInfo;
        if (stat(m_filePath.getBuffer(), &fileInfo) != 0)
        {
            continue;
        }

        if (S_ISDIR(fileInfo.st_mode))
        {
            m_findType = FindType::Directory;
        }
        else if (S_ISREG(fileInfo.st_mode))
        {
            m_findType = FindType::File;
        }
        else
        {
            // We don't know the type.. so ignore
            continue;
        }

        // Check the type is enabled, else ignore
        const FindTypeFlags typeFlags = FindTypeFlags(1) << int(m_findType);
        if ((typeFlags & m_allowedTypes) == 0)
        {
            // Has to be enabled
            continue;
        }

        if (m_findType == FindType::Directory)
            m_filePath.appendChar('/');

        return true;
    }
}

SlangResult UnixFindFilesState::startFind(const String& directoryPath, const String& pattern, FindTypeFlags allowedTypes)
{
    _close();

    //    fprintf(stderr, "osFindFilesInDirectory(%s)\n", directoryPath.getBuffer());
    m_allowedTypes = allowedTypes;
    m_directoryPath = directoryPath;
    m_pattern = pattern;

    m_directory = opendir(path.getBuffer());
    if (!m_directory)
    {
        return SLANG_E_NOT_FOUND;
    }
    findNext();

    return SLANG_OK;
}

/* static */RefPtr<FindFilesState> FindFilesState::create()
{
    return new UnixFindFilesState;
}

#endif
