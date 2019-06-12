#ifndef SLANG_WIN_VISUAL_STUDIO_UTIL_H
#define SLANG_WIN_VISUAL_STUDIO_UTIL_H

#include "../slang-list.h"
#include "../slang-string.h"

#include "../slang-process-util.h"

#include "../slang-cpp-compiler.h"

namespace Slang {

struct WinVisualStudioUtil
{
    enum class Version: uint32_t
    {
        Unknown = 0,                ///< This is an unknown (and not later) version
        Future = 0xff * 10,         ///< This is a version 'from the future' - that isn't specifically known. Will be treated as latest
    };
    
    struct VersionPath
    {
        Version version;            ///< The visual studio version
        String vcvarsPath;          ///< The path to vcvars bat files, that need to be executed before executing the compiler
    };
    
        ///  Find all the installations 
    static SlangResult find(List<VersionPath>& outVersionPaths);

        /// Given a version find it's path
    static SlangResult find(Version version, VersionPath& outPath);

        /// Run visual studio on specified path with the parameters specified on the command line. Output placed in outResult.
    static SlangResult executeCompiler(const VersionPath& versionPath, const CommandLine& commandLine, ExecuteResult& outResult);

        /// Get all the known version numbers
    static void getVersions(List<Version>& outVersions);

        /// Gets the msc compiler used to compile this version. Returning Version(0) means unknown
    static Version getCompiledVersion();

        /// Create a version from a high and low indices
    static Version makeVersion(int high, int low = 0) { SLANG_ASSERT(low >= 0 && low <= 9); return Version(high * 10 + low); }

        /// Convert a version number into a string
    static void append(Version version, StringBuilder& outBuilder);

        /// Calculate the command line args
    static void calcArgs(const CPPCompileOptions& options, CommandLine& cmdLine);

};

} // namespace Slang

#endif 
