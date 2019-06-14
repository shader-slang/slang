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

            /// Ctor
        Desc(Type inType = Type::Unknown, Int inMajorVersion = 0, Int inMinorVersion = 0):type(inType), majorVersion(inMajorVersion), minorVersion(inMinorVersion) {}

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
        m_func(func)
    {
        m_cmdLine.setExecutableFilename(exeName);
    }

    GenericCPPCompiler(const Desc& desc, const CommandLine& cmdLine, CalcArgsFunc func) :
        Super(desc),
        m_cmdLine(cmdLine),
        m_func(func)
    {}

    CalcArgsFunc m_func;
    CommandLine m_cmdLine;
};

class CPPCompilerSet : public RefObject
{
public:
    typedef RefObject Super;

    
        /// Find all the available compilers
    void getCompilerDescs(List<CPPCompiler::Desc>& outCompilerDescs) const;
        /// Returns list of all compilers
    void getCompilers(List<CPPCompiler*>& outCompilers) const;

        /// Get a compiler
    CPPCompiler* getCompiler(const CPPCompiler::Desc& compilerDesc) const;
  
        /// Will replace if there is one with same desc
    void addCompiler(CPPCompiler* compiler);

        /// Get a default compiler
    CPPCompiler* getDefaultCompiler() const { return m_defaultCompiler;  }
        /// Set the default compiler
    void setDefaultCompiler(CPPCompiler* compiler) { m_defaultCompiler = compiler;  }

protected:

    Index _findIndex(const CPPCompiler::Desc& desc) const;

    RefPtr<CPPCompiler> m_defaultCompiler;
    // This could be a dictionary/map - but doing a linear search is going to be fine and it makes
    // somethings easier.
    List<RefPtr<CPPCompiler>> m_compilers;
};

struct CPPCompilerUtil
{
    enum class MatchType
    {
        MinGreaterEqual,
        MinAbsolute,
        Newest,
    };

        /// Extracts version number into desc from text (assumes gcc/clang -v layout with a line with version)
    static SlangResult parseGCCFamilyVersion(const UnownedStringSlice& text, const UnownedStringSlice& prefix, CPPCompiler::Desc& outDesc);

        /// Runs the exeName, and extracts the version info into outDesc
    static SlangResult calcGCCFamilyVersion(const String& exeName, CPPCompiler::Desc& outDesc);

        /// Calculate gcc family compilers (including clang) cmdLine arguments from options
    static void calcGCCFamilyArgs(const CPPCompiler::CompileOptions& options, CommandLine& cmdLine);

        /// Find a compiler
    static CPPCompiler* findCompiler(const CPPCompilerSet* set, MatchType matchType, const CPPCompiler::Desc& desc);
    static CPPCompiler* findCompiler(const List<CPPCompiler*>& compilers, MatchType matchType, const CPPCompiler::Desc& desc);

        /// Find the compiler closest to the desc 
    static CPPCompiler* findClosestCompiler(const List<CPPCompiler*>& compilers, const CPPCompiler::Desc& desc);
    static CPPCompiler* findClosestCompiler(const CPPCompilerSet* set, const CPPCompiler::Desc& desc);

        /// Get the information on the compiler used to compile this source
    static const CPPCompiler::Desc& getCompiledWithDesc();

        /// Given a set, registers compilers found through standard means and determines a reasonable default compiler if possible
    static SlangResult initializeSet(CPPCompilerSet* set);
};


}

#endif
