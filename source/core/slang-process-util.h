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

    void addArg(const String& in) { m_args.add(in); }
    void addArgs(const String* args, Int argsCount) { m_args.addRange(args, argsCount); }
    void setExecutablePath(const String& path) { m_executableType = ExecutableType::Path; m_executable = path; }
    void setExecutableFilename(const String& filename) { m_executableType = ExecutableType::Filename; m_executable = filename; }

        /// Ctor
    CommandLine():m_executableType(ExecutableType::Unknown) {}

    ExecutableType m_executableType;    ///< How the executable is specified
    String m_executable;                ///< Executable to run 
    List<String> m_args;          ///< The parameters to pass 
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
};

}

#endif // SLANG_PROCESS_UTIL_H
