// slang-process.h
#ifndef SLANG_PROCESS_H
#define SLANG_PROCESS_H

#include "slang-string.h"
#include "slang-list.h"
#include "slang-stream.h"
#include "slang-io.h"

#include "slang-string-escape-util.h"

#include "slang-command-line.h"

namespace Slang {

class Process : public RefObject
{
public:
    enum class StreamType
    {
        ErrorOut,
        StdOut,
        StdIn,
        CountOf,
    };

    typedef uint32_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            AttachDebugger = 0x01,
        };
    };

        /// Get the stream for the type
    Stream* getStream(StreamType type) const { return m_streams[Index(type)]; }
    int32_t getReturnValue() const { return m_returnValue;  }

        /// True if the process has terminated
    virtual bool isTerminated() = 0;
        /// Blocks until the process has completed
    virtual void waitForTermination() = 0;

        /// Terminate the process
    virtual void terminate(int32_t returnCode) = 0;

        /// The quoting style used for the command line on this target. Currently just uses Space,
        /// but in future may take into account platform sec
    static StringEscapeHandler* getEscapeHandler();

        /// Get the suffix used on this platform
    static UnownedStringSlice getExecutableSuffix();

        /// Create a process using the executable/args defined from the commandLine
    static SlangResult create(const CommandLine& commandLine, Process::Flags flags, RefPtr<Process>& outProcess);

        /// Sleep the current thread for time specified in milliseconds. 0 indicates to OS ok to yield this thread.
    static void sleepCurrentThread(Index timeInMs);

        /// Get a standard stream 
    static SlangResult getStdStream(StreamType type, RefPtr<Stream>& out);

    static uint64_t getClockFrequency();

    static uint64_t getClockTick();

protected:
    int32_t m_returnValue = 0;                              ///< Value returned if process terminated
    RefPtr<Stream> m_streams[Index(StreamType::CountOf)];   ///< Streams to communicate with the process
};

}

#endif // SLANG_PROCESS_H
