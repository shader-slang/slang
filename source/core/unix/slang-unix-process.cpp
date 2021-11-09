// slang-unix-process.cpp
#include "../slang-process.h"

#include "../slang-common.h"
#include "../slang-string-util.h"
#include "../slang-string-escape-util.h"
#include "../slang-memory-arena.h"

#include <stdio.h>
#include <stdlib.h>

//#include <dirent.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <time.h>

namespace Slang {

class UnixProcess : public Process
{
public:
    // Process 
    virtual bool isTerminated() SLANG_OVERRIDE;
    virtual void waitForTermination() SLANG_OVERRIDE;

    UnixProcess(pid_t pid, Stream*const* streams);
protected:

        /// Returns true if terminated
    bool _updateTerminationState(int options);

    bool m_isTerminated = false;
    pid_t m_pid;
};

class UnixPipeStream : public Stream
{
public:
    typedef UnixPipeStream ThisType;

    // Stream
    virtual Int64 getPosition() SLANG_OVERRIDE { return 0; }
    virtual SlangResult seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE { SLANG_UNUSED(origin); SLANG_UNUSED(offset); return SLANG_E_NOT_AVAILABLE; }
    virtual SlangResult read(void* buffer, size_t length, size_t& outReadBytes) SLANG_OVERRIDE;
    virtual SlangResult write(const void* buffer, size_t length) SLANG_OVERRIDE;
    virtual bool isEnd() SLANG_OVERRIDE { return m_isClosed; }
    virtual bool canRead() SLANG_OVERRIDE { return _has(FileAccess::Read) && !m_isClosed; }
    virtual bool canWrite() SLANG_OVERRIDE { return _has(FileAccess::Write) && !m_isClosed; }
    virtual void close() SLANG_OVERRIDE;
    virtual SlangResult flush() SLANG_OVERRIDE;

    UnixPipeStream(int fd, FileAccess access, bool isOwned) :
        m_fd(fd),
        m_access(access),
        m_isOwned(isOwned),
        m_isClosed(false)
    {
#if 0
        // Makes non blocking
        if (_has(FileAccess::Read))
        {
            // Make non blocking, for read
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
        }
#endif
    }

protected:
    bool _has(FileAccess access) const { return (Index(access) & Index(m_access)) != 0; }

    bool m_isClosed;
    bool m_isOwned;
    FileAccess m_access;
    int m_fd;
};

/* !!!!!!!!!!!!!!!!!!!!!! UnixProcess !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

UnixProcess::UnixProcess(pid_t pid, Stream* const* streams):
    m_pid(pid)
{
    // Set to an 'odd value'
    m_returnValue = -1;

    for (Index i = 0; i < SLANG_COUNT_OF(m_streams); ++i)
    {
        m_streams[i] = streams[i];
    }
}

bool UnixProcess::_updateTerminationState(int options)
{
    if (!m_isTerminated)
    {
        int childStatus;
        const pid_t terminatedPid = waitpid(m_pid, &childStatus, options);
        if (terminatedPid == -1)
        {
            // Guess we should just mark as terminated
            m_isTerminated = true;
            fprintf(stderr, "error: `waitpid` failed\n");
        }
        else if (terminatedPid == m_pid)
        {
            if (WIFEXITED(childStatus))
            {
                m_returnValue = (int)(int8_t)WEXITSTATUS(childStatus);
            }
            m_isTerminated = true;
        }
    }
    return m_isTerminated;
}

bool UnixProcess::isTerminated()
{
    if (m_isTerminated)
    {
        return true;
    }
    return _updateTerminationState(WNOHANG);
}

void UnixProcess::waitForTermination()
{
    while (!_updateTerminationState(0));
}

/* !!!!!!!!!!!!!!!!!!!!!! UnixPipeStream !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void UnixPipeStream::close()
{
    if (!m_isClosed)
    {
        if (m_isOwned)
        {
            ::close(m_fd);
        }

        m_isClosed = true;
        // Make something hopefully invalid
        m_fd = -1;
    }
}

SlangResult UnixPipeStream::flush()
{
    if (canWrite())
    {
        // We might want to use
        ::fsync(m_fd);
    }
}

SlangResult UnixPipeStream::read(void* buffer, size_t length, size_t& outReadBytes)
{
    outReadBytes = 0;

    if (!_has(FileAccess::Read))
    {
        return SLANG_E_NOT_AVAILABLE;
    }
    if (m_isClosed)
    {
        return SLANG_OK;
    }

    // Check if it's hung up.
    pollfd pollInfo;

    pollInfo.fd = m_fd;
    pollInfo.events = POLLIN | POLLHUP;
    pollInfo.revents = 0;

    // https://linux.die.net/man/2/poll

    // Return immediately
    const int pollTimeout = 0;

    const int pollResult = ::poll(&pollInfo, 1, pollTimeout);
    if (pollResult < 0)
    {
        return SLANG_FAIL;
    }

    if (pollInfo.revents & POLLIN)
    {
        auto count = ::read(m_fd, buffer, length);

        // If it's -1 it seems like an error
        if (count == -1)
        {
            const int err = errno;

            // On non blocking pipe these indicate there could be more to come
            if (err == EAGAIN || err == EWOULDBLOCK)
            {
                return SLANG_OK;
            }
        }

        outReadBytes = size_t(count);
        return SLANG_OK;
    }

    if (pollInfo.revents & POLLHUP)
    {
        close();
    }

    return SLANG_OK;
}

SlangResult UnixPipeStream::write(const void* buffer, size_t length)
{
    if (!_has(FileAccess::Write))
    {
        return SLANG_E_NOT_AVAILABLE;
    }
    if (m_isClosed)
    {
        // The pipe is closed
        return SLANG_FAIL;
    }

    pollfd pollInfo;

    pollInfo.fd = m_fd;
    pollInfo.events = POLLHUP;
    pollInfo.revents = 0;

    // https://linux.die.net/man/2/poll

    // Return immediately
    const int pollTimeout = 0;

    int pollResult = ::poll(&pollInfo, 1, pollTimeout);
    if (pollResult < 0)
    {
        return SLANG_FAIL;
    }

    if (pollInfo.revents & POLLHUP)
    {
        close();
        return SLANG_FAIL;
    }

    const ssize_t writeResult = ::write(m_fd, buffer, length);

    if (writeResult < 0 || writeResult != length)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!! Process !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */UnownedStringSlice Process::getExecutableSuffix()
{
#if __CYGWIN__
    return UnownedStringSlice::fromLiteral(".exe");
#else
    return UnownedStringSlice::fromLiteral("");
#endif
}

/* static */StringEscapeHandler* Process::getEscapeHandler()
{
    return StringEscapeUtil::getHandler(StringEscapeUtil::Style::Space);
}

/* static */SlangResult Process::create(const CommandLine& commandLine, Process::Flags flags, RefPtr<Process>& outProcess)
{
    List<char const*> argPtrs;

    // Add the command
    argPtrs.add(commandLine.m_executable.getBuffer());

    // Add all the args - they don't need any explicit escaping 
    for (auto arg : commandLine.m_args)
    {
        // All args for this target must be unescaped (as they are in CommandLine)
        argPtrs.add(arg.getBuffer());
    }

    // Terminate with a null
    argPtrs.add(nullptr);

    int stdoutPipe[2];
    int stderrPipe[2];
    int stdinPipe[2];

    if (pipe(stdoutPipe) == -1 || pipe(stderrPipe) == -1 || pipe(stdinPipe) == -1)
    {
        fprintf(stderr, "error: `pipe` failed\n");
        return SLANG_FAIL;
    }

    pid_t childProcessID = fork();
    if (childProcessID == -1)
    {
        fprintf(stderr, "error: `fork` failed\n");
        return SLANG_FAIL;
    }

    if (childProcessID == 0)
    {
        // We are the child process.

        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);
        dup2(stdinPipe[0], STDIN_FILENO);

        ::close(stdoutPipe[0]);
        ::close(stdoutPipe[1]);

        ::close(stderrPipe[0]);
        ::close(stderrPipe[1]);

        ::close(stdinPipe[0]);
        ::close(stdinPipe[1]);

        ::execvp(argPtrs[0], (char* const*)&argPtrs[0]);

        // If we get here, then `exec` failed
        fprintf(stderr, "error: `exec` failed\n");
        return SLANG_FAIL;
    }
    else
    {
        // We are the parent process
        ::close(stdoutPipe[1]);
        ::close(stderrPipe[1]);
        ::close(stdinPipe[0]);

        RefPtr<Stream> streams[Index(Process::StreamType::CountOf)];

        // Previously code didn't need to close, so we'll make stream not own the handles
        streams[Index(Process::StreamType::StdOut)] = new UnixPipeStream(stdoutPipe[0], FileAccess::Read, true);
        streams[Index(Process::StreamType::ErrorOut)] = new UnixPipeStream(stderrPipe[0], FileAccess::Read, true);
        streams[Index(Process::StreamType::StdIn)] = new UnixPipeStream(stdinPipe[1], FileAccess::Write, true);

        outProcess = new UnixProcess(childProcessID, streams[0].readRef());
        return SLANG_OK;
    }
}

/* static */uint64_t Process::getClockFrequency()
{
    return 1000000000;
}

/* static */uint64_t Process::getClockTick()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return uint64_t(now.tv_sec) * 1000000000 + now.tv_nsec;
}

/* static */void Process::sleepCurrentThread(Index timeInMs)
{
    struct timespec timeSpec;

    if (timeInMs > 0)
    {
        timeSpec.tv_sec = timeInMs / 1000;
        timeSpec.tv_nsec = (timeInMs % 1000) * 1000 * 1000;
    }
    else
    {
        timeSpec.tv_sec = 0;
        timeSpec.tv_nsec = 0;
    }
    nanosleep(&timeSpec, nullptr);
}

/* static */SlangResult Process::getStdStream(StreamType type, RefPtr<Stream>& out)
{
    switch (type)
    {
        case StreamType::StdIn:
        {
            out = new UnixPipeStream(STDIN_FILENO, FileAccess::Read, false);
            break;
        }
        case StreamType::StdOut:
        {
            out = new UnixPipeStream(STDOUT_FILENO, FileAccess::Write, false);
            break; 
        }
        case StreamType::ErrorOut:
        {
            out = new UnixPipeStream(STDERR_FILENO, FileAccess::Write, false);
            break;
        }
        default: return SLANG_FAIL;
    }
    return SLANG_OK;
}

} // namespace Slang
