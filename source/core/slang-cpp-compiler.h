#ifndef SLANG_CPP_COMPILER_H
#define SLANG_CPP_COMPILER_H

#include "slang-common.h"
#include "slang-string.h"

#include "slang-process-util.h"

namespace Slang
{

class CPPCompiler: public RefObject
{
public:
    typedef RefObject Super;
    enum class Type
    {
        Unknown,
        VisualStudio,
        GCC,
        Clang,
        SNC,
        GHS,
        CountOf,
    };

    struct Desc
    {
        typedef Desc ThisType;

        UInt GetHashCode() const { return combineHash(int(type), combineHash(int(majorVersion), int(minorVersion))); }
        bool operator==(const ThisType& rhs) const { return type == rhs.type && majorVersion == rhs.majorVersion && minorVersion == rhs.minorVersion;  }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

            /// Get the version as a value
        Int getVersionValue() const { return majorVersion * 100 + minorVersion;  }

        Type type;                      ///< The type of the compiler
        Int majorVersion;               ///< Major version (interpretation is type specific)
        Int minorVersion;               ///< Minor version
    };

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

    struct CompileOptions
    {
        OptimizationLevel optimizationLevel = OptimizationLevel::Debug;
        DebugInfoType debugInfoType = DebugInfoType::Normal;
        TargetType targetType = TargetType::Executable;

        String modulePath;      ///< The path/name of the output module. Should not have the extension, as that will be added for each of the target types

        List<Define> defines;

        List<String> sourceFiles;

        List<String> includePaths;
        List<String> libraryPaths;
    };

        /// Get the desc of this compiler
    const Desc& getDesc() const { return m_desc;  }
        /// Compile using the specified options. The result is in resOut
    virtual SlangResult compile(const CompileOptions& options, ExecuteResult& outResult) = 0;

protected:

    CPPCompiler(const Desc& desc) :
        m_desc(desc)
    {}

    Desc m_desc;
};

class GenericCPPCompiler : public CPPCompiler
{
public:
    typedef CPPCompiler Super;

    typedef void(*CalcArgsFunc)(const CPPCompiler::CompileOptions& options, CommandLine& cmdLine);

    virtual SlangResult compile(const CompileOptions& options, ExecuteResult& outResult) SLANG_OVERRIDE;

    GenericCPPCompiler(const Desc& desc, const String& exeName, CalcArgsFunc func) :
        Super(desc),
        m_exeName(exeName),
        m_func(func)
    {}

    CalcArgsFunc m_func;
    String m_exeName;
};

class CPPCompilerSystem: public RefObject 
{
public:
    typedef RefObject Super;

    enum class MatchType
    {
        MinGreaterEqual,
        MinAbsolute,
        Newest,
    };

        /// Get the information on the compiler used to compile this source
    const CPPCompiler::Desc& getCompiledWithDesc() const { return m_compiledWith;  }

        /// Find all the available compilers
    void getCompilerDescs(List<CPPCompiler::Desc>& outCompilerDescs) const;

        /// Get a compiler
    CPPCompiler* getCompiler(const CPPCompiler::Desc& compilerDesc);
        /// Get a compiler by index
    CPPCompiler* getCompilerByIndex(Index index) const { return m_compilers[index]; }

        /// Find a compiler
    CPPCompiler* findCompiler(MatchType matchType, CPPCompiler::Desc& desc) const;

        /// Will replace if there is one with same desc
    void addCompiler(CPPCompiler* compiler);

        /// Get the closest to runtime
    CPPCompiler* getClosestRuntimeCompiler() const { return m_closestRuntimeCompiler;  }

        /// Find the compiler 'closest' to the compiler that built this binary
    CPPCompiler* findClosestRuntimeCompiler();

        /// Create a factory
    static RefPtr<CPPCompilerSystem> create();

        /// Extracts version number into desc from text (assumes gcc/slang type layout with a line with version starting with versionPrefix)
    static SlangResult parseGccFamilyVersion(const UnownedStringSlice& text, const UnownedStringSlice& versionPrefix, CPPCompiler::Desc& outDesc);

        /// Runs the exeName, and extracts the version info into outDesc
    static SlangResult calcGccFamilyVersion(const String& exeName, const UnownedStringSlice& versionPrefix, CPPCompiler::Desc& outDesc);

protected:
    CPPCompilerSystem();
    SlangResult _init();

    RefPtr<CPPCompiler> m_closestRuntimeCompiler;

    List<RefPtr<CPPCompiler>> m_compilers;
    CPPCompiler::Desc m_compiledWith;
};

}

#endif
