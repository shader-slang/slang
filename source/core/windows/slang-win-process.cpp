// slang-win-process.cpp
#include "../slang-process.h"

#include "../slang-string.h"

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
#endif

#include <stdio.h>
#include <stdlib.h>

namespace Slang {

namespace { // anonymous

struct ThreadInfo
{
    HANDLE	file;
    String	output;
};

// Has behavior very similar to unique_ptr. That assignment is a move.
class Handle
{
public:
        /// Detach the encapsulated handle. Returns the handle (which now must be externally handled) 
    HANDLE detach() { HANDLE handle = m_handle; m_handle = nullptr; return handle; }

        /// Return as a handle
    operator HANDLE() const { return m_handle; }

        /// Assign
    void operator=(HANDLE handle) { setNull(); m_handle = handle; }
    void operator=(Handle&& rhs) { HANDLE handle = m_handle; m_handle = rhs.m_handle; rhs.m_handle = handle; }

        /// Get ready for writing 
    SLANG_FORCE_INLINE HANDLE* writeRef() { setNull(); return &m_handle; }
        /// Get for read access
    SLANG_FORCE_INLINE const HANDLE* readRef() const { return &m_handle; }

    void setNull()
    {
        if (m_handle)
        {
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
    }

        /// Ctor
    Handle(HANDLE handle = nullptr):m_handle(handle) {}
    Handle(Handle&& rhs):m_handle(rhs.m_handle) { rhs.m_handle = nullptr; }

        /// Dtor
    ~Handle() { setNull(); }

private:
    
    Handle(const Handle&) = delete;
    void operator=(const Handle& rhs) = delete;

    HANDLE m_handle;
};

} // anonymous

static DWORD WINAPI _readerThreadProc(LPVOID threadParam)
{
    ThreadInfo* info = (ThreadInfo*)threadParam;
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


/* static */UnownedStringSlice ProcessUtil::getExecutableSuffix()
{
    return UnownedStringSlice::fromLiteral(".exe");
}

static void _appendEscaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    // TODO(JS): This escaping is not complete... !
    if (slice.indexOf(' ') >= 0 || slice.indexOf('"') >= 0)
    {
        out << "\"";

        const char* cur = slice.begin();
        const char* end = slice.end();

        while (cur < end)
        {
            char c= *cur++;
            switch (c)
            {
                case '\"':
                {
                    // Escape quotes.
                    out << "\\\"";
                    break;
                }
                default:
                    out.append(c);
            }
        }

        out << "\"";

        return;
    }

    out << slice;
}

/* static */String ProcessUtil::getCommandLineString(const CommandLine& commandLine)
{
    StringBuilder cmd;
    _appendEscaped(commandLine.m_executable.getUnownedSlice(), cmd);
    for (const auto& arg : commandLine.m_args)
    {
        cmd << " ";
        _appendEscaped(arg.getUnownedSlice(), cmd);
    }
    return cmd.ToString();
}

#define SLANG_RETURN_FAIL_ON_FALSE(x) if (!(x)) return SLANG_FAIL;

/* static */SlangResult ProcessUtil::execute(const CommandLine& commandLine, ExecuteResult& outExecuteResult)
{
    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.lpSecurityDescriptor = nullptr;
    securityAttributes.bInheritHandle = true;

    // create stdout pipe for child process
    Handle childStdOutReadTmp;
    Handle childStdOutWrite;
    SLANG_RETURN_FAIL_ON_FALSE(CreatePipe(childStdOutReadTmp.writeRef(), childStdOutWrite.writeRef(), &securityAttributes, 0));

    // create stderr pipe for child process
    Handle childStdErrReadTmp;
    Handle childStdErrWrite;
    SLANG_RETURN_FAIL_ON_FALSE(CreatePipe(childStdErrReadTmp.writeRef(), childStdErrWrite.writeRef(), &securityAttributes, 0));

    // create stdin pipe for child process
    Handle childStdInRead;
    Handle childStdInWriteTmp;
    SLANG_RETURN_FAIL_ON_FALSE(CreatePipe(childStdInRead.writeRef(), childStdInWriteTmp.writeRef(), &securityAttributes, 0));

    HANDLE currentProcess = GetCurrentProcess();

    // create a non-inheritable duplicate of the stdout reader
    Handle childStdOutRead;
    SLANG_RETURN_FAIL_ON_FALSE(DuplicateHandle(currentProcess, childStdOutReadTmp, currentProcess, childStdOutRead.writeRef(), 0, FALSE, DUPLICATE_SAME_ACCESS));
    childStdOutReadTmp.setNull();

    // create a non-inheritable duplicate of the stderr reader
    Handle childStdErrRead;
    SLANG_RETURN_FAIL_ON_FALSE(DuplicateHandle(currentProcess, childStdErrReadTmp, currentProcess, childStdErrRead.writeRef(), 0, FALSE, DUPLICATE_SAME_ACCESS));
    childStdErrReadTmp.setNull();

    // create a non-inheritable duplicate of the stdin writer
    Handle childStdInWrite;
    SLANG_RETURN_FAIL_ON_FALSE(DuplicateHandle(currentProcess, childStdInWriteTmp, currentProcess, childStdInWrite.writeRef(), 0, FALSE, DUPLICATE_SAME_ACCESS));
    childStdInWriteTmp.setNull();

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

    LPCWSTR path = (commandLine.m_executableType == CommandLine::ExecutableType::Path) ? commandLine.m_executable.toWString().begin() : nullptr;

    // Produce the command line string
    String cmdString = getCommandLineString(commandLine);

    // https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-createprocessa
    // `CreateProcess` requires write access to this, for some reason...
    BOOL success = CreateProcessW(
        path,
        (LPWSTR)cmdString.toWString().begin(),
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
        return SLANG_FAIL;
    }

    // close handles we are now done with
    CloseHandle(processInfo.hThread);

    // Create a thread to read from the child's stdout.
    ThreadInfo stdOutThreadInfo;
    stdOutThreadInfo.file = childStdOutRead;
    Handle stdOutThread = CreateThread(nullptr, 0, &_readerThreadProc, (LPVOID)&stdOutThreadInfo, 0, nullptr);

    // Create a thread to read from the child's stderr.
    ThreadInfo stdErrThreadInfo;
    stdErrThreadInfo.file = childStdErrRead;
    Handle stdErrThread = CreateThread(nullptr, 0, &_readerThreadProc, (LPVOID)&stdErrThreadInfo, 0, nullptr);

    // wait for the process to exit
    // TODO: set a timeout as a safety measure...
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    // get exit code for process
    // https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-getexitcodeprocess

    DWORD childExitCode = 0;
    if (!GetExitCodeProcess(processInfo.hProcess, &childExitCode))
    {
        // TODO(JS): Do we want to close here? It seems plausible because just because reading the exit code failed, doesn't mean the handle is closed
        CloseHandle(processInfo.hProcess);

        return SLANG_FAIL;
    }

    // wait for the reader threads
    WaitForSingleObject(stdOutThread, INFINITE);
    WaitForSingleObject(stdErrThread, INFINITE);

    CloseHandle(processInfo.hProcess);

    outExecuteResult.standardOutput = stdOutThreadInfo.output;
    outExecuteResult.standardError = stdErrThreadInfo.output;
    outExecuteResult.resultCode = childExitCode;

    return SLANG_OK;
}

} 
