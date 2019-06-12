#ifndef SLANG_WIN_VISUAL_STUDIO_UTIL_H
#define SLANG_WIN_VISUAL_STUDIO_UTIL_H

#include "../slang-list.h"
#include "../slang-string.h"

#include "../slang-process-util.h"

namespace Slang {

struct CPPCompileOptions
{
    enum class OptimizationLevel
    {
        Normal,             ///< Normal optimization
        Debug,              ///< General has no optimizations
    };

    enum DebugInfoType
    {
        None,               ///< Binary has no debug information
        Maximum,            ///< Has maximum debug information
        Normal,             ///< Has normal debug information
    };
    enum TargetType
    {
        Executable,         ///< Produce an executable
        SharedLibrary,      ///< Produce a shared library object/dll 
        Object,             ///< Produce an object file
    };

    struct Define
    {
        String nameWithSig;             ///< If macro takes parameters include in brackets
        String value;
    };

    OptimizationLevel optimizationLevel = OptimizationLevel::Debug;
    DebugInfoType debugInfoType = DebugInfoType::Normal;
    TargetType targetType = TargetType::Executable;

    String modulePath;      ///< The path/name of the output module. Should not have the extension, as that will be added for each of the target types

    List<Define> defines;

    List<String> sourceFiles;

    List<String> includePaths;
    List<String> libraryPaths;
};

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
