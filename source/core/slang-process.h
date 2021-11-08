// slang-process.h
#ifndef SLANG_PROCESS_H
#define SLANG_PROCESS_H

#include "slang-string.h"
#include "slang-list.h"
#include "slang-stream.h"

#include "slang-string-escape-util.h"

namespace Slang {

struct CommandLine
{
    enum class ExecutableType
    {
        Unknown,                    ///< The executable is not specified 
        Path,                       ///< The executable is set as a path
        Filename,                   ///< The executable is set as a filename
    };

        /// Add args - assumed unescaped
    void addArg(const String& in) { m_args.add(in); }
    void addArgs(const String* args, Int argsCount) { for (Int i = 0; i < argsCount; ++i) addArg(args[i]); }

        /// Find the index of an arg which is exact match for slice
    SLANG_INLINE Index findArgIndex(const UnownedStringSlice& slice) const { return m_args.indexOf(slice); }

        /// Set the executable path
    void setExecutablePath(const String& path) { m_executableType = ExecutableType::Path; m_executable = path; }
    void setExecutableFilename(const String& filename) { m_executableType = ExecutableType::Filename; m_executable = filename; }

        /// For handling args where the switch is placed directly in front of the path 
    SLANG_INLINE void addPrefixPathArg(const char* prefix, const String& path, const char* pathPostfix = nullptr);

        /// Get the total number of args
    SLANG_FORCE_INLINE Index getArgCount() const { return m_args.getCount(); }

        /// Reset to the initial state
    void reset() { *this = CommandLine();  }

        /// Ctor
    CommandLine():m_executableType(ExecutableType::Unknown) {}

    ExecutableType m_executableType;    ///< How the executable is specified
    String m_executable;                ///< Executable to run. Note that the executable is never escaped.
    List<String> m_args;                ///< The arguments (Stored *unescaped*)
};

// -----------------------------------------------------------------------
SLANG_INLINE void CommandLine::addPrefixPathArg(const char* prefix, const String& path, const char* pathPostfix)
{
    StringBuilder builder;
    builder << prefix << path;
    if (pathPostfix)
    {
        // Work out the path with the postfix
        builder << pathPostfix;
    }
    addArg(builder.ProduceString());
}

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
