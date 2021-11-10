// slang-command-line.h
#ifndef SLANG_COMMAND_LINE_H
#define SLANG_COMMAND_LINE_H

#include "slang-string.h"
#include "slang-list.h"

namespace Slang {

struct CommandLine
{
    enum class ExecutableType
    {
        Unknown,                    ///< The executable is not specified 
        Path,                       ///< The executable is set as a path (ie won't be searched for)
        Filename,                   ///< The executable is set as a filename
    };

        /// Add args - assumed unescaped
    void addArg(const String& in) { m_args.add(in); }
    void addArgs(const String* args, Int argsCount) { for (Int i = 0; i < argsCount; ++i) addArg(args[i]); }

        /// Find the index of an arg which is exact match for slice
    SLANG_INLINE Index findArgIndex(const UnownedStringSlice& slice) const { return m_args.indexOf(slice); }

        /// Set the executable path.
        /// NOTE! On some targets the executable path *must* include an extension to be able to start as a process
    void setExecutablePath(const String& path) { m_executableType = ExecutableType::Path; m_executable = path; }

        /// Set the executable path from a base directory and an executable name (no suffix such as '.exe' needed)
    void setExecutable(const String& dir, const String& name);

        /// Set a filename (such that the path will be looked up
    void setExecutableFilename(const String& filename) { m_executableType = ExecutableType::Filename; m_executable = filename; }

        /// For handling args where the switch is placed directly in front of the path 
    void addPrefixPathArg(const char* prefix, const String& path, const char* pathPostfix = nullptr);

        /// Get the total number of args
    SLANG_FORCE_INLINE Index getArgCount() const { return m_args.getCount(); }

        /// Reset to the initial state
    void reset() { *this = CommandLine();  }

        /// Append the command line to out
    void append(StringBuilder& out) const;
        /// convert into a string
    String toString() const;

        /// Ctor
    CommandLine():m_executableType(ExecutableType::Unknown) {}

    ExecutableType m_executableType;    ///< How the executable is specified
    String m_executable;                ///< Executable to run. Note that the executable is never escaped.
    List<String> m_args;                ///< The arguments (Stored *unescaped*)
};

}

#endif // SLANG_COMMAND_LINE_H
