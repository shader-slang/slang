// os.cpp
#include "os.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace CoreLib::Basic;

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

        result.filePath_ = result.directoryPath_ + String::FromWString(result.fileData_.cFileName);
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
    CoreLib::Basic::String directoryPath,
    CoreLib::Basic::String pattern)
{
    // TODO: add separator to end of directory path if needed

    String searchPath = directoryPath + pattern;

    OSFindFilesResult result;
    HANDLE findHandle = FindFirstFileW(
        searchPath.ToWString(),
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
    CoreLib::Basic::String directoryPath)
{
    return osFindFilesInDirectoryMatchingPattern(directoryPath, "*");
}

OSFindFilesResult osFindChildDirectories(
    CoreLib::Basic::String directoryPath)
{
    // TODO: add separator to end of directory path if needed

    String searchPath = directoryPath + "*";

    OSFindFilesResult result;
    HANDLE findHandle = FindFirstFileW(
        searchPath.ToWString(),
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

// OSProcessSpawner

struct OSProcessSpawner_ReaderThreadInfo
{
    HANDLE	file;
    String	output;
};

static DWORD WINAPI osReaderThreadProc(LPVOID threadParam)
{
    OSProcessSpawner_ReaderThreadInfo* info = (OSProcessSpawner_ReaderThreadInfo*)threadParam;
    HANDLE file = info->file;

    static const int kChunkSize = 1024;
    char buffer[kChunkSize];

    StringBuilder outputBuilder;

    // We need to re-write the output to deal with line
    // endings, so we check for paired '\r' and '\n'
    // characters, which may span chunks.
    int prevChar = -1;

    for (;;)
    {
        DWORD bytesRead = 0;
        BOOL readResult = ReadFile(file, buffer, kChunkSize, &bytesRead, nullptr);

        if (!readResult || GetLastError() == ERROR_BROKEN_PIPE)
        {
            break;
        }

        // walk the buffer and rewrite to eliminate '\r' '\n' pairs
        char* readCursor = buffer;
        char const* end = buffer + bytesRead;
        char* writeCursor = buffer;

        while (readCursor != end)
        {
            int p = prevChar;
            int c = *readCursor++;
            prevChar = c;
            switch (c)
            {
            case '\r': case '\n':
                // swallow input if '\r' and '\n' appear in sequence
                if ((p ^ c) == ('\r' ^ '\n'))
                {
                    // but don't swallow the next byte
                    prevChar = -1;
                    continue;
                }
                // always replace '\r' with '\n'
                c = '\n';
                break;

            default:
                break;
            }

            *writeCursor++ = c;
        }
        bytesRead = (DWORD)(writeCursor - buffer);

        // Note: Current Slang CoreLib gives no way to know
        // the length of the buffer, so we ultimately have
        // to just assume null termination...
        outputBuilder.Append(buffer, bytesRead);
    }

    info->output = outputBuilder.ProduceString();

    return 0;
}

void OSProcessSpawner::pushExecutableName(
    CoreLib::Basic::String executableName)
{
    executableName_ = executableName;
    commandLine_.Append(executableName);
}

void OSProcessSpawner::pushArgument(
    CoreLib::Basic::String argument)
{
    // TODO(tfoley): handle cases where arguments need some escaping
    commandLine_.Append(" ");
    commandLine_.Append(argument);
}

OSError OSProcessSpawner::spawnAndWaitForCompletion()
{
    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.lpSecurityDescriptor = nullptr;
    securityAttributes.bInheritHandle = true;

    // create stdout pipe for child process
    HANDLE childStdOutReadTmp = nullptr;
    HANDLE childStdOutWrite = nullptr;
    if (!CreatePipe(&childStdOutReadTmp, &childStdOutWrite, &securityAttributes, 0))
    {
        return kOSError_OperationFailed;
    }

    // create stderr pipe for child process
    HANDLE childStdErrReadTmp = nullptr;
    HANDLE childStdErrWrite = nullptr;
    if (!CreatePipe(&childStdErrReadTmp, &childStdErrWrite, &securityAttributes, 0))
    {
        return kOSError_OperationFailed;
    }

    // create stdin pipe for child process
    HANDLE childStdInRead = nullptr;
    HANDLE childStdInWriteTmp = nullptr;
    if (!CreatePipe(&childStdInRead, &childStdInWriteTmp, &securityAttributes, 0))
    {
        return kOSError_OperationFailed;
    }

    HANDLE currentProcess = GetCurrentProcess();

    // create a non-inheritable duplicate of the stdout reader
    HANDLE childStdOutRead = nullptr;
    if (!DuplicateHandle(
        currentProcess, childStdOutReadTmp,
        currentProcess, &childStdOutRead,
        0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        return kOSError_OperationFailed;
    }
    if (!CloseHandle(childStdOutReadTmp))
    {
        return kOSError_OperationFailed;
    }

    // create a non-inheritable duplicate of the stderr reader
    HANDLE childStdErrRead = nullptr;
    if (!DuplicateHandle(
        currentProcess, childStdErrReadTmp,
        currentProcess, &childStdErrRead,
        0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        return kOSError_OperationFailed;
    }
    if (!CloseHandle(childStdErrReadTmp))
    {
        return kOSError_OperationFailed;
    }

    // create a non-inheritable duplicate of the stdin writer
    HANDLE childStdInWrite = nullptr;
    if (!DuplicateHandle(
        currentProcess, childStdInWriteTmp,
        currentProcess, &childStdInWrite,
        0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        return kOSError_OperationFailed;
    }
    if (!CloseHandle(childStdInWriteTmp))
    {
        return kOSError_OperationFailed;
    }

    // Now we can actually get around to starting a process
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));

    // TODO: switch to proper wide-character versions of these...
    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.hStdError = childStdErrWrite;
    startupInfo.hStdOutput = childStdOutWrite;
    startupInfo.hStdInput = childStdInRead;
    startupInfo.dwFlags = STARTF_USESTDHANDLES;

    // `CreateProcess` requires write access to this, for some reason...
    BOOL success = CreateProcessW(
        executableName_.ToWString(),
        (LPWSTR)commandLine_.ToString().ToWString(),
        nullptr,
        nullptr,
        true,
        CREATE_NO_WINDOW,
        nullptr, // TODO: allow specifying environment variables?
        nullptr,
        &startupInfo,
        &processInfo);
    if (!success)
    {
        return kOSError_OperationFailed;
    }

    // close handles we are now done with
    CloseHandle(processInfo.hThread);
    CloseHandle(childStdOutWrite);
    CloseHandle(childStdErrWrite);
    CloseHandle(childStdInRead);

    // Create a thread to read from the child's stdout.
    OSProcessSpawner_ReaderThreadInfo stdOutThreadInfo;
    stdOutThreadInfo.file = childStdOutRead;
    HANDLE stdOutThread = CreateThread(nullptr, 0, &osReaderThreadProc, (LPVOID)&stdOutThreadInfo, 0, nullptr);

    // Create a thread to read from the child's stderr.
    OSProcessSpawner_ReaderThreadInfo stdErrThreadInfo;
    stdErrThreadInfo.file = childStdErrRead;
    HANDLE stdErrThread = CreateThread(nullptr, 0, &osReaderThreadProc, (LPVOID)&stdErrThreadInfo, 0, nullptr);

    // wait for the process to exit
    // TODO: set a timeout as a safety measure...
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    // get exit code for process
    DWORD childExitCode = 0;
    if (!GetExitCodeProcess(processInfo.hProcess, &childExitCode))
    {
        return kOSError_OperationFailed;
    }

    // wait for the reader threads
    WaitForSingleObject(stdOutThread, INFINITE);
    WaitForSingleObject(stdErrThread, INFINITE);

    CloseHandle(processInfo.hProcess);
    CloseHandle(childStdOutRead);
    CloseHandle(childStdErrRead);
    CloseHandle(childStdInWrite);

    standardOutput_ = stdOutThreadInfo.output;
    standardError_ = stdErrThreadInfo.output;
    resultCode_ = childExitCode;

    return kOSError_None;
}

#else

// TODO(tfoley): write a default POSIX implementation

#endif
