// slang-process.h
#ifndef SLANG_PROCESS_H
#define SLANG_PROCESS_H

#include "slang-command-line.h"
#include "slang-io.h"
#include "slang-list.h"
#include "slang-stream.h"
#include "slang-string-escape-util.h"
#include "slang-string.h"

namespace Slang
{

class Process : public RefObject
{
public:
    typedef uint32_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            // AttachDebugger and DisableStdErrRedirection are ignored on non-Windows platforms.
            AttachDebugger = 0x01,
            DisableStdErrRedirection = 0x02,
            /// Give the child a stdin it cannot read from, instead of a pipe. Honored on all
            /// platforms. Contract: a read of the child's stdin reports an *error* rather than
            /// reaching end-of-file (an EOF read would instead succeed with empty input, which is a
            /// different outcome this flag does not provide), and `getStream(In)` is null. A null
            /// input stream is an established shape, not a new hazard: `DisableStdErrRedirection`
            /// already leaves `getStream(ErrorOut)` null and `ProcessUtil::readUntilTermination`
            /// already tolerates it.
            ///
            /// Implemented by opening the child's fd 0 write-only on the null device. It is left
            /// open on an unreadable target rather than closed, because a closed fd 0 is taken by
            /// the next file the child opens, and the child's `stdin` would then silently read that
            /// file instead of failing.
            ///
            /// The error-not-EOF guarantee is directly established only on POSIX (write-only
            /// `/dev/null`). On Windows it is inferred from the equivalent in-process precedent
            /// (write-only `NUL` on fd 0) rather than measured for the inherited-handle path, and
            /// the child may reach the read-failure via `_setmode` failing rather than a read
            /// error. It is exercised end-to-end by the `SlangcReadFromStdin` unit test, which runs
            /// on Windows CI and asserts the read-failure diagnostic.
            UnreadableStdin = 0x04
        };
    };

    /// Get the stream for the type
    Stream* getStream(StdStreamType type) const { return m_streams[Index(type)]; }

    /// Get the value returned from the process when it exited/returned.
    int32_t getReturnValue() const { return m_returnValue; }

    /// True if the process has terminated
    virtual bool isTerminated() = 0;

    /// Blocks until the process has terminated or the timeout completes
    /// Can optionally supply a timeout time. -1 means 'infinite' and is the default.
    /// Note that the timeOut is only used approximately.
    /// Returns true if has terminated.
    virtual bool waitForTermination(Int timeOutInMs = -1) = 0;

    /// Terminate the process gracefully.
    /// After calling it may take time before the process actually terminates
    /// Ie calling isTerminated directly after `terminate` may return false.
    /// The return code depending on implementation/termination style, may not be set.
    virtual void terminate(int32_t returnCode) = 0;

    /// Kill the process - attempt to terminate immediately.
    virtual void kill(int32_t returnCode) = 0;

    /// The quoting style used for the command line on this target. Currently just uses Space,
    /// but in future may take into account platform sec
    static StringEscapeHandler* getEscapeHandler();

    /// Get the suffix used on this platform
    static UnownedStringSlice getExecutableSuffix();

    /// Create a process using the executable/args defined from the commandLine
    static SlangResult create(
        const CommandLine& commandLine,
        Process::Flags flags,
        RefPtr<Process>& outProcess);

    /// Sleep the current thread for time specified in milliseconds. 0 indicates to OS ok to yield
    /// this thread.
    static void sleepCurrentThread(Int timeInMs);

    /// Get a standard stream
    static SlangResult getStdStream(StdStreamType type, RefPtr<Stream>& out);

    /// Get the clock frequency
    static uint64_t getClockFrequency();

    /// Get the clock tick.
    static uint64_t getClockTick();

    static uint32_t getId();

protected:
    int32_t m_returnValue = 0; ///< Value returned if process terminated
    RefPtr<Stream>
        m_streams[Index(StdStreamType::CountOf)]; ///< Streams to communicate with the process
};

} // namespace Slang

#endif // SLANG_PROCESS_H
