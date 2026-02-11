// slang-unix-process.cpp
#include "../slang-common.h"
#include "../slang-memory-arena.h"
#include "../slang-process.h"
#include "../slang-string-escape-util.h"
#include "../slang-string-util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if SLANG_OSX
#include <signal.h>
#endif

#include <time.h>

extern char** environ;

namespace Slang
{

class UnixProcess : public Process
{
public:
    // Process
    virtual bool isTerminated() SLANG_OVERRIDE;
    virtual bool waitForTermination(Int timeInMs) SLANG_OVERRIDE;
    virtual void terminate(int32_t returnValue) SLANG_OVERRIDE;
    virtual void kill(int32_t returnValue) SLANG_OVERRIDE;

    UnixProcess(pid_t pid, Stream* const* streams);

protected:
    /// Returns true if terminated
    bool _updateTerminationState(int options);

    bool m_isTerminated = false; ///< True if ths process is terminated
    pid_t m_pid;                 ///< The process id
};

class UnixPipeStream : public Stream
{
public:
    typedef UnixPipeStream ThisType;

    // Stream
    virtual Int64 getPosition() SLANG_OVERRIDE { return 0; }
    virtual SlangResult seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE
    {
        SLANG_UNUSED(origin);
        SLANG_UNUSED(offset);
        return SLANG_E_NOT_AVAILABLE;
    }
    virtual SlangResult read(void* buffer, size_t length, size_t& outReadBytes) SLANG_OVERRIDE;
    virtual SlangResult write(const void* buffer, size_t length) SLANG_OVERRIDE;
    virtual bool isEnd() SLANG_OVERRIDE { return m_isClosed; }
    virtual bool canRead() SLANG_OVERRIDE { return _has(FileAccess::Read) && !m_isClosed; }
    virtual bool canWrite() SLANG_OVERRIDE { return _has(FileAccess::Write) && !m_isClosed; }
    virtual void close() SLANG_OVERRIDE;
    virtual SlangResult flush() SLANG_OVERRIDE;

    UnixPipeStream(int fd, FileAccess access, bool isOwned)
        : m_fd(fd), m_access(access), m_isOwned(isOwned), m_isClosed(false)
    {
    }

    ~UnixPipeStream() SLANG_OVERRIDE { close(); }

protected:
    /// This read file descriptor non blocking. Doing so will change the behavior of
    /// read - it can fail and return an error indicating there is no data, instead of blocking.
    /// Currently this mechanism isn't used, as checking via poll seemed to work.
    void _setReadNonBlocking()
    {
        // Makes non blocking
        if (_has(FileAccess::Read))
        {
            // Make non blocking, for read
            fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL) | O_NONBLOCK);
        }
    }
    bool _has(FileAccess access) const { return (Index(access) & Index(m_access)) != 0; }

    bool m_isClosed;     ///< If true this stream has been closed (ie cannot read/write to anymore)
    bool m_isOwned;      ///< True if m_fd is owned by this object.
    FileAccess m_access; ///< Access allowed to this stream - either Read or Write
    int m_fd;            /// The 'file descriptor' for the pipe
};

/* !!!!!!!!!!!!!!!!!!!!!! UnixProcess !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

UnixProcess::UnixProcess(pid_t pid, Stream* const* streams)
    : m_pid(pid)
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

bool UnixProcess::waitForTermination(Int timeInMs)
{
    // If < 0 we will wait blocking until terminated
    if (timeInMs < 0)
    {
        while (!_updateTerminationState(0))
            ;
        return true;
    }

    // Note that the amount of time waiting is very approximate (we are relying on sleeps time and
    // don't take into account time outside of sleeping)

    // How often to test
    const Int checkRateMs = 100; /// Check every 0.1 seconds

    while (timeInMs > 0)
    {
        if (_updateTerminationState(WNOHANG))
        {
            return true;
        }

        // Work out how long to sleep for
        const Int sleepMs = (timeInMs >= checkRateMs) ? checkRateMs : timeInMs;

        // Sleep
        sleepCurrentThread(sleepMs);

        timeInMs -= sleepMs;
    }

    return _updateTerminationState(WNOHANG);
}

void UnixProcess::terminate(int32_t returnValue)
{
    // Using this mechanism, we can't set a returnValue so just ignore
    SLANG_UNUSED(returnValue);

    if (!isTerminated())
    {
        // Request the process terminates
        ::kill(m_pid, SIGTERM);
    }
}

void UnixProcess::kill(int32_t returnValue)
{
    if (!isTerminated())
    {
        // We waited, lets just terminate with kill
        ::kill(m_pid, SIGKILL);

        // Set the return value
        m_returnValue = returnValue;
        // Mark as terminated
        m_isTerminated = true;
    }
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
#if 0
    // https://stackoverflow.com/questions/43184035/flushing-pipe-without-closing-in-c
    // Makes the case that flushing is not applicable with pipes.
    if (canWrite())
    {
        // We might want to use
        ::fsync(m_fd);
    }
#endif
    return SLANG_OK;
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

    // If there are no poll events, we are done
    if (pollResult == 0)
    {
        return SLANG_OK;
    }

    // If there is data read that first
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
            // Okay - guess we have an error then
            return SLANG_FAIL;
        }

        outReadBytes = size_t(count);

        // If no bytes were wanted, then there could still be bytes in the pipe
        // before a HUP. So don't fall through to check for HUP.
        //
        // If some bytes *were* wanted and none were read, we can allow fall through to
        // handle HUP.
        if (length == 0 || count > 0)
        {
            return SLANG_OK;
        }

        // End of file.
        if (count == 0)
        {
            close();
        }
    }

    if (pollInfo.revents & POLLHUP)
    {
        close();
    }

    if (pollInfo.revents & POLLERR || pollInfo.revents & POLLNVAL)
    {
        return SLANG_FAIL;
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

    if (writeResult < 0 || size_t(writeResult) != length)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!! Process !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */ UnownedStringSlice Process::getExecutableSuffix()
{
#if __CYGWIN__
    return UnownedStringSlice::fromLiteral(".exe");
#else
    return UnownedStringSlice::fromLiteral("");
#endif
}

/* static */ StringEscapeHandler* Process::getEscapeHandler()
{
    return StringEscapeUtil::getHandler(StringEscapeUtil::Style::Space);
}

/* static */ SlangResult Process::create(
    const CommandLine& commandLine,
    Process::Flags,
    RefPtr<Process>& outProcess)
{
    const char* whatFailed = nullptr;
    int spawnResult = 0;
    pid_t childPid;

    //
    // Set up command line
    //
    List<char const*> argPtrs;

    const auto& exe = commandLine.m_executableLocation;

    // The command
    argPtrs.add(exe.m_pathOrName.getBuffer());

    // The args - they don't need any explicit escaping
    for (auto arg : commandLine.m_args)
    {
        argPtrs.add(arg.getBuffer());
    }
    argPtrs.add(nullptr);

    //
    // Set up pipes
    //
    int stdinPipe[2] = {-1, -1};
    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};

    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1 || pipe(stderrPipe) == -1)
    {
        whatFailed = "pipe";
    }
    else
    {
        //
        // The meat
        //
        posix_spawn_file_actions_t file_actions;
        posix_spawnattr_t attr;

        if (posix_spawn_file_actions_init(&file_actions) != 0 || posix_spawnattr_init(&attr) != 0)
        {
            whatFailed = "posix_spawn init";
        }
        else
        {
            //
            // Stdio redirections
            //
            posix_spawn_file_actions_adddup2(&file_actions, stdinPipe[0], STDIN_FILENO);
            posix_spawn_file_actions_addclose(&file_actions, stdinPipe[0]);
            posix_spawn_file_actions_addclose(&file_actions, stdinPipe[1]);

            posix_spawn_file_actions_adddup2(&file_actions, stdoutPipe[1], STDOUT_FILENO);
            posix_spawn_file_actions_addclose(&file_actions, stdoutPipe[0]);
            posix_spawn_file_actions_addclose(&file_actions, stdoutPipe[1]);

            posix_spawn_file_actions_adddup2(&file_actions, stderrPipe[1], STDERR_FILENO);
            posix_spawn_file_actions_addclose(&file_actions, stderrPipe[0]);
            posix_spawn_file_actions_addclose(&file_actions, stderrPipe[1]);

            //
            // Set up environment - inherit parent but override LC_ALL=C
            //
            List<char const*> envPtrs;

            for (char** env = environ; *env != nullptr; env++)
            {
                if (strncmp(*env, "LC_ALL=", 7) != 0)
                {
                    envPtrs.add(*env);
                }
            }
            envPtrs.add("LC_ALL=C");
            envPtrs.add(nullptr);

            // WASM doesn't have posix_spawnp, but it also doesn't have things
            // like PATH so it's fine to fall back to posix_spawn (what are we even spawning on WASM
            // anyway)
#if !SLANG_WASM
            if (exe.m_type == ExecutableLocation::Type::Name)
            {
                spawnResult = posix_spawnp(
                    &childPid,
                    argPtrs[0],
                    &file_actions,
                    &attr,
                    (char* const*)&argPtrs[0],
                    (char* const*)&envPtrs[0]);
            }
            else
#endif
            {
                spawnResult = posix_spawn(
                    &childPid,
                    argPtrs[0],
                    &file_actions,
                    &attr,
                    (char* const*)&argPtrs[0],
                    (char* const*)&envPtrs[0]);
            }

            //
            // cleanup
            //
            posix_spawn_file_actions_destroy(&file_actions);
            posix_spawnattr_destroy(&attr);

            if (spawnResult != 0)
            {
                // Only report unexpected errors - ENOENT/EACCES are expected for
                // missing/inaccessible tools
                if (spawnResult != ENOENT && spawnResult != EACCES)
                {
                    whatFailed = "posix_spawn";
                }

                // Don't print messages by default for failed spawns (some are speculative)
                const bool verbose = false;
                if (verbose)
                {
                    fprintf(
                        stderr,
                        "error: posix_spawn for \"%s\" failed: %s\n",
                        argPtrs[0],
                        strerror(spawnResult));
                }
            }
            else
            {
                // Close child-side pipes in parent
                ::close(stdinPipe[0]);
                ::close(stdoutPipe[1]);
                ::close(stderrPipe[1]);
                stdinPipe[0] = stdoutPipe[1] = stderrPipe[1] = -1;

                // Create stream objects for parent-side pipes
                RefPtr<Stream> streams[Index(StdStreamType::CountOf)];
                streams[Index(StdStreamType::Out)] =
                    new UnixPipeStream(stdoutPipe[0], FileAccess::Read, true);
                streams[Index(StdStreamType::ErrorOut)] =
                    new UnixPipeStream(stderrPipe[0], FileAccess::Read, true);
                streams[Index(StdStreamType::In)] =
                    new UnixPipeStream(stdinPipe[1], FileAccess::Write, true);

                // Mark as owned by streams so cleanup doesn't close them
                stdoutPipe[0] = stderrPipe[0] = stdinPipe[1] = -1;

                outProcess = new UnixProcess(childPid, streams[0].readRef());
            }
        }
    }

    // Report any error
    if (whatFailed)
    {
        fprintf(
            stderr,
            "error: `%s` failed (%s)\n",
            whatFailed,
            strerror(spawnResult ? spawnResult : errno));
    }

    // Clean up any remaining open pipes
    ::close(stdinPipe[0]);
    ::close(stdinPipe[1]);
    ::close(stdoutPipe[0]);
    ::close(stdoutPipe[1]);
    ::close(stderrPipe[0]);
    ::close(stderrPipe[1]);

    return whatFailed || spawnResult ? SLANG_FAIL : SLANG_OK;
}

/* static */ uint64_t Process::getClockFrequency()
{
    return 1000000000;
}

/* static */ uint64_t Process::getClockTick()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return uint64_t(now.tv_sec) * 1000000000 + now.tv_nsec;
}

/* static */ void Process::sleepCurrentThread(Int timeInMs)
{
    struct timespec timeSpec;

    if (timeInMs >= 1000)
    {
        timeSpec.tv_sec = timeInMs / 1000;
        timeSpec.tv_nsec = (timeInMs % 1000) * 1000 * 1000;
    }
    else if (timeInMs > 0)
    {
        timeSpec.tv_sec = 0;
        timeSpec.tv_nsec = timeInMs * 1000 * 1000;
    }
    else
    {
        timeSpec.tv_sec = 0;
        timeSpec.tv_nsec = 0;
    }
    nanosleep(&timeSpec, nullptr);
}

/* static */ SlangResult Process::getStdStream(StdStreamType type, RefPtr<Stream>& out)
{
    switch (type)
    {
    case StdStreamType::In:
        {
            out = new UnixPipeStream(STDIN_FILENO, FileAccess::Read, false);
            break;
        }
    case StdStreamType::Out:
        {
            out = new UnixPipeStream(STDOUT_FILENO, FileAccess::Write, false);
            break;
        }
    case StdStreamType::ErrorOut:
        {
            out = new UnixPipeStream(STDERR_FILENO, FileAccess::Write, false);
            break;
        }
    default:
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

uint32_t Process::getId()
{
    return getpid();
}

} // namespace Slang
