// slang-process-util.h
#ifndef SLANG_PROCESS_UTIL_H
#define SLANG_PROCESS_UTIL_H

#include "slang-string.h"
#include "slang-list.h"

namespace Slang {

struct CommandLine
{
    enum class ExecutableType
    {
        Unknown,                    ///< The executable is not specified 
        Path,                       ///< The executable is set as a path
        Filename,                   ///< The executable is set as a filename
    };

    enum class ArgType
    {
        Escaped,
        Unescaped,
    };

    struct Arg
    {
        ArgType type;               ///< How to interpret the argument value
        String value;               ///< The argument value
    };

        /// Add args - assumed unescaped
    void addArg(const String& in) { m_args.add(Arg{ArgType::Unescaped, in}); }
    void addArgs(const String* args, Int argsCount) { for (Int i = 0; i < argsCount; ++i) addArg(args[i]); }

        /// Add args - all assumed unescaped
    void addArgs(const Arg* args, Int argCount) { m_args.addRange(args, argCount); }

        /// Add an escaped arg
    void addEscapedArg(const String& in) { m_args.add(Arg{ArgType::Escaped, in}); }
    void addEscapedArgs(const String* args, Int argsCount) { for (Int i = 0; i < argsCount; ++i) addEscapedArg(args[i]); }

        /// Find the index of an arg which is exact match for slice
    SLANG_INLINE Index findArgIndex(const UnownedStringSlice& slice) const;

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
    List<Arg> m_args;                   ///< The arguments  
};

struct ExecuteResult
{
    void init()
    {
        resultCode = 0;
        standardOutput = String();
        standardError = String();
    }

    typedef int ResultCode;
    ResultCode resultCode;
    Slang::String standardOutput;
    Slang::String standardError;
};

struct ProcessUtil
{
        /// Get the suffix used on this platform
    static UnownedStringSlice getExecutableSuffix();

        /// Output how the command line is executed on the target (with escaping and the such like)
    static String getCommandLineString(const CommandLine& commandLine);

        /// Execute the command line 
    static SlangResult execute(const CommandLine& commandLine, ExecuteResult& outExecuteResult);

        /// Append text escaped for using on a command line
    static void appendCommandLineEscaped(const UnownedStringSlice& slice, StringBuilder& out);

    static uint64_t getClockFrequency();

    static uint64_t getClockTick();
};

// -----------------------------------------------------------------------
SLANG_INLINE Index CommandLine::findArgIndex(const UnownedStringSlice& slice) const
{
    const Index count = m_args.getCount();

    for (Index i = 0; i < count; ++i)
    {
        const auto& arg = m_args[i];
        if (arg.value == slice)
        {
            return i;
        }
    }
    return -1;
}

// -----------------------------------------------------------------------
SLANG_INLINE void CommandLine::addPrefixPathArg(const char* prefix, const String& path, const char* pathPostfix)
{
    StringBuilder builder;
    builder << prefix;
    if (pathPostfix)
    {
        // Work out the path with the postfix
        StringBuilder fullPath;
        fullPath << path << pathPostfix;  
        ProcessUtil::appendCommandLineEscaped(fullPath.getUnownedSlice(), builder);
    }
    else
    {
        ProcessUtil::appendCommandLineEscaped(path.getUnownedSlice(), builder);
    }

    // This arg doesn't need subsequent escaping
    addEscapedArg(builder);
}

}

#endif // SLANG_PROCESS_UTIL_H
