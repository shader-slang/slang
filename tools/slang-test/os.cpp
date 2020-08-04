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

#endif

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

/* static */FindFilesResult FindFilesResult::findChildDirectories(const Slang::String& directoryPath)
{
    RefPtr<FindFilesState> state(FindFilesState::create());
    if (SLANG_FAILED(state->startFindChildDirectories(directoryPath)))
    {
        state.setNull();
    }
    return FindFilesResult(state);
}

/* static */FindFilesResult FindFilesResult::findFilesInDirectoryMatchingPattern(const Slang::String& directoryPath, const Slang::String& pattern)
{
    RefPtr<FindFilesState> state(FindFilesState::create());
    if (SLANG_FAILED(state->startFindFilesInDirectoryMatchingPattern(directoryPath, pattern)))
    {
        state.setNull();
    }
    return FindFilesResult(state);
}

/* static */FindFilesResult FindFilesResult::findFilesInDirectory(const Slang::String& directoryPath)
{
    RefPtr<FindFilesState> state(FindFilesState::create());
    if (SLANG_FAILED(state->startFindFilesInDirectory(directoryPath)))
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
    virtual SlangResult startFindChildDirectories(const String& directoryPath) SLANG_OVERRIDE;
    virtual SlangResult startFindFilesInDirectory(const String& path) SLANG_OVERRIDE;
    virtual SlangResult startFindFilesInDirectoryMatchingPattern(const String& directoryPath, const String& pattern) SLANG_OVERRIDE;
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
        }
    }
    bool _advance()
    {
        if (m_findHandle == nullptr || FindNextFileW(m_findHandle, &m_fileData) == 0)
        {
            _close();
            return false;
        }
        return true;
    }

    HANDLE				m_findHandle;
    WIN32_FIND_DATAW	m_fileData;
    DWORD				m_requiredMask;
    DWORD				m_disallowedMask;
};

bool WinFindFilesState::findNext()
{
    for (;;)
    {
        if (!_advance())
        {
            return false;
        }

        if (((m_fileData.dwFileAttributes & m_requiredMask) != m_requiredMask) ||
            (m_fileData.dwFileAttributes & m_disallowedMask) != 0 ||
            (wcscmp(m_fileData.cFileName, L".") == 0) ||
            (wcscmp(m_fileData.cFileName, L"..") == 0))
        {
            continue;
        }

        m_filePath = m_directoryPath + String::fromWString(m_fileData.cFileName);

        if (m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            m_filePath.appendChar('/');

        return true;
    }
}

SlangResult WinFindFilesState::startFindFilesInDirectoryMatchingPattern(const String& directoryPath, const String& pattern)
{
    _close();

    // TODO: add separator to end of directory path if needed

    String searchPath = directoryPath + pattern;

    m_findHandle = FindFirstFileW(searchPath.toWString(), &m_fileData);
    if (!m_findHandle)
    {
        return SLANG_E_NOT_FOUND;
    }

    m_directoryPath = directoryPath;
    m_requiredMask = 0;
    m_disallowedMask = FILE_ATTRIBUTE_DIRECTORY;

    findNext();
    return SLANG_OK;
}

SlangResult WinFindFilesState::startFindChildDirectories(const String& directoryPath)
{
    _close();

    // TODO: add separator to end of directory path if needed

    String searchPath = directoryPath + "*";

    m_findHandle = FindFirstFileW( searchPath.toWString(), &m_fileData);
    if (!m_findHandle)
    {
        return SLANG_E_NOT_FOUND;
    }

    m_directoryPath = directoryPath;
    
    m_requiredMask = FILE_ATTRIBUTE_DIRECTORY;
    m_disallowedMask = 0;

    findNext();
    return SLANG_OK;
}

SlangResult WinFindFilesState::startFindFilesInDirectory(const Slang::String& path)
{
    return startFindFilesInDirectoryMatchingPattern(path, "*");
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
    virtual SlangResult startFindChildDirectories(const String& directoryPath) SLANG_OVERRIDE;
    virtual SlangResult startFindFilesInDirectory(const String& path) SLANG_OVERRIDE;
    virtual SlangResult startFindFilesInDirectoryMatchingPattern(const String& directoryPath, const String& pattern) SLANG_OVERRIDE;
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
        }
    }

    DIR*         m_directory;
    dirent*      m_entry;
};

bool UnixFindFilesState::_advance()
{
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
            m_filePath.appendChar('/');

        return true;
    }
}

SlangResult UnixFindFilesState::startFindChildDirectories(const String& directoryPath)
{
    _close();

    m_directory = opendir(directoryPath.getBuffer());
    if (!m_directory)
    {
        return SLANG_E_NOT_FOUND;
    }

    // TODO: Set attributes to ignore everything but directories

    m_directoryPath = directoryPath;
    findNext();
    return SLANG_OK;
}

SlangResult UnixFindFilesState::startFindFilesInDirectory(const Slang::String& path)
{
    _close();

//    fprintf(stderr, "osFindFilesInDirectory(%s)\n", directoryPath.getBuffer());

    m_directoryPath = path;

    m_directory = opendir(path.getBuffer());
    if(!m_directory)
    {
        return SLANG_E_NOT_FOUND;
    }
    findNext();
    return SLANG_OK;
}

SlangResult UnixFindFilesState::startFindFilesInDirectoryMatchingPattern(const String& directoryPath, const String& pattern)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

/* static */RefPtr<FindFilesState> FindFilesState::create()
{
    return new UnixFindFilesState;
}

#endif
