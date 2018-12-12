// os.cpp
#include "os.h"

#include <assert.h>
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
    Slang::String directoryPath,
    Slang::String pattern)
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

            *writeCursor++ = (char)c;
        }
        bytesRead = (DWORD)(writeCursor - buffer);

        // Note: Current "core" implementation gives no way to know
        // the length of the buffer, so we ultimately have
        // to just assume null termination...
        outputBuilder.Append(buffer, bytesRead);
    }

    info->output = outputBuilder.ProduceString();

    return 0;
}

void OSProcessSpawner::pushExecutableName(
    Slang::String executableName)
{
    executableName_ = executableName;
    commandLine_.Append(executableName);
    isExecutablePath_ = false;
}

void OSProcessSpawner::pushExecutablePath(
    Slang::String executablePath)
{
    executableName_ = executablePath;
    commandLine_.Append(executablePath);
    isExecutablePath_ = true;
}

void OSProcessSpawner::pushArgument(
    Slang::String argument)
{
    // TODO(tfoley): handle cases where arguments need some escaping
    commandLine_.Append(" ");
    commandLine_.Append(argument);

    argumentList_.Add(argument);
}

Slang::String OSProcessSpawner::getCommandLine()
{
    return commandLine_;
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
        isExecutablePath_ ? executableName_.ToWString().begin() : nullptr,
        (LPWSTR)commandLine_.ToString().ToWString().begin(),
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

char const* osGetExecutableSuffix()
{
    return ".exe";
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

//    fprintf(stderr, "stat(%s)\n", path.Buffer());
    struct stat fileInfo;
    if(stat(path.Buffer(), &fileInfo) != 0)
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

//    fprintf(stderr, "osFindFilesInDirectory(%s)\n", directoryPath.Buffer());

    result.directory_ = opendir(directoryPath.Buffer());
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

    result.directory_ = opendir(directoryPath.Buffer());
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

// OSProcessSpawner

void OSProcessSpawner::pushExecutableName(
    Slang::String executableName)
{
    executableName_ = executableName;
    arguments_.Add(executableName);
    isExecutablePath_ = false;
}

void OSProcessSpawner::pushExecutablePath(
    Slang::String executablePath)
{
    executableName_ = executablePath;
    arguments_.Add(executablePath);
    isExecutablePath_ = true;
}

void OSProcessSpawner::pushArgument(
    Slang::String argument)
{
    arguments_.Add(argument);
    argumentList_.Add(argument);
}

Slang::String OSProcessSpawner::getCommandLine()
{
    Slang::UInt argCount = arguments_.Count();

    Slang::StringBuilder sb;
    for(Slang::UInt ii = 0; ii < argCount;  ++ii)
    {
        if(ii != 0) sb << " ";
        sb << arguments_[ii];
    }
    return sb.ProduceString();
}

OSError OSProcessSpawner::spawnAndWaitForCompletion()
{
    List<char const*> argPtrs;
    for(auto arg : arguments_)
    {
        argPtrs.Add(arg.Buffer());
    }
    argPtrs.Add(NULL);

    int stdoutPipe[2];
    int stderrPipe[2];

    if(pipe(stdoutPipe) == -1)
        return kOSError_OperationFailed;

    if(pipe(stderrPipe) == -1)
        return kOSError_OperationFailed;

    pid_t childProcessID = fork();
    if (childProcessID == -1)
        return kOSError_OperationFailed;

    if(childProcessID == 0)
    {
        // We are the child process.

        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);

        close(stdoutPipe[0]);
        close(stdoutPipe[1]);

        close(stderrPipe[0]);
        close(stderrPipe[1]);

        execvp(
            argPtrs[0],
            (char* const*) &argPtrs[0]);

        // If we get here, then `exec` failed
        fprintf(stderr, "error: `exec` failed\n");
        exit(1);
    }
    else
    {
        // We are the parent process

        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        int stdoutFD = stdoutPipe[0];
        int stderrFD = stderrPipe[0];

        pollfd pollInfos[2];
        nfds_t pollInfoCount = 2;

        pollInfos[0].fd = stdoutFD;
        pollInfos[0].events = POLLIN;
        pollInfos[0].revents = 0;
        pollInfos[1].fd = stderrFD;
        pollInfos[1].events = POLLIN;
        pollInfos[1].revents = 0;

        int remainingCount =  2;
        int iterations = 0;
        while(remainingCount)
        {
            // Safeguard against infinite loop:
            iterations++;
            if (iterations > 10000)
            {
                fprintf(stderr, "poll(): %d iterations\n", iterations);
                return kOSError_OperationFailed;
            }

            // Set a timeout of ten seconds;
            // we really shouldn't wait too long...
            int pollTimeout = 10000;
            int pollResult = poll(pollInfos, pollInfoCount, pollTimeout);
            if (pollResult <= 0)
            {
                // If there was a signal that got in
                // the way, then retry...
                if(pollResult == -1 && errno == EINTR)
                    continue;

                // timeout or error...
                return kOSError_OperationFailed;
            }

            enum { kBufferSize = 1024 };
            char buffer[kBufferSize];

            if(pollInfos[0].revents)
            {
                auto count = read(stdoutFD, buffer, kBufferSize);
                if (count <= 0)
                {
                    // end-of-file
                    close(stdoutFD);
                    pollInfos[0].fd = -1;
                    remainingCount--;
                }

                standardOutput_.append(
                    buffer, buffer + count);
            }

            if(pollInfos[1].revents)
            {
                auto count = read(stderrFD, buffer, kBufferSize);
                if (count <= 0)
                {
                    // end-of-file
                    close(stderrFD);
                    pollInfos[1].fd = -1;
                    remainingCount--;
                }

                standardError_.append(
                    buffer, buffer + count);
            }
        }

        int childStatus = 0;
        iterations = 0;
        for(;;)
        {
            // Safeguard against infinite loop:
            iterations++;
            if (iterations > 10000)
            {
                fprintf(stderr, "waitpid(): %d iterations\n", iterations);
                return kOSError_OperationFailed;
            }


            pid_t terminatedProcessID = waitpid(
                childProcessID,
                &childStatus,
                0);
            if (terminatedProcessID == -1)
            {
                return kOSError_OperationFailed;
            }

            if(terminatedProcessID == childProcessID)
            {
                if(WIFEXITED(childStatus))
                {
                    resultCode_ = (int)(int8_t)WEXITSTATUS(childStatus);
                }
                else
                {
                    resultCode_ = 1;
                }

                return kOSError_None;
            }
        }

    }

    return kOSError_OperationFailed;
}

char const* osGetExecutableSuffix()
{
    return "";
}

#endif
