#ifndef SLANG_CPP_COMPILER_H
#define SLANG_CPP_COMPILER_H

#include "slang-common.h"
#include "slang-string.h"

namespace Slang
{

class CPPCompiler
{
public:
    enum class Type
    {
        VisualStudio,
        GCC,
        Clang,
    };

    struct Version
    {
        Type type;      ///< The compiler type
        Int major;      ///< The major version number
        Int minor;      ///< The minor version number
    };
};

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

}

#endif
