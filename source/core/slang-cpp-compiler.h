#ifndef SLANG_CPP_COMPILER_H
#define SLANG_CPP_COMPILER_H

#include "slang-common.h"
#include "slang-string.h"

#include "slang-process-util.h"

#include "slang-platform.h"

namespace Slang
{

class CPPCompiler: public RefObject
{
public:
    typedef RefObject Super;
    enum class CompilerType
    {
        Unknown,
        VisualStudio,
        GCC,
        Clang,
        SNC,
        GHS,
        CountOf,
    };
    enum class SourceType
    {
        C,              ///< C source
        CPP,            ///< C++ source
    };

    struct Desc
    {
        typedef Desc ThisType;

        UInt GetHashCode() const { return combineHash(int(type), combineHash(int(majorVersion), int(minorVersion))); }
        bool operator==(const ThisType& rhs) const { return type == rhs.type && majorVersion == rhs.majorVersion && minorVersion == rhs.minorVersion;  }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

            /// Get the version as a value
        Int getVersionValue() const { return majorVersion * 100 + minorVersion;  }

        void appendAsText(StringBuilder& out) const;

            /// Ctor
        Desc(CompilerType inType = CompilerType::Unknown, Int inMajorVersion = 0, Int inMinorVersion = 0):type(inType), majorVersion(inMajorVersion), minorVersion(inMinorVersion) {}

        CompilerType type;                      ///< The type of the compiler
        Int majorVersion;               ///< Major version (interpretation is type specific)
        Int minorVersion;               ///< Minor version
    };

    enum class OptimizationLevel
    {
        None,           ///< Don't optimize at all. 
        Default,        ///< Default optimization level: balance code quality and compilation time. 
        High,           ///< Optimize aggressively. 
        Maximal,        ///< Include optimizations that may take a very long time, or may involve severe space-vs-speed tradeoffs 
    };

    enum class DebugInfoType
    {
        None,       ///< Don't emit debug information at all. 
        Minimal,    ///< Emit as little debug information as possible, while still supporting stack traces. 
        Standard,   ///< Emit whatever is the standard level of debug information for each target. 
        Maximal,    ///< Emit as much debug information as possible for each target. 
    };
    enum class FloatingPointMode
    {
        Default, 
        Fast,
        Precise,
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
        typedef uint32_t Flags;
        struct Flag 
        {
            enum Enum : Flags
            {
                EnableExceptionHandling = 0x01,
                Verbose                 = 0x02, 
            };
        };

        OptimizationLevel optimizationLevel = OptimizationLevel::Default;
        DebugInfoType debugInfoType = DebugInfoType::Standard;
        TargetType targetType = TargetType::Executable;
        SourceType sourceType = SourceType::CPP;
        FloatingPointMode floatingPointMode = FloatingPointMode::Default;

        Flags flags = Flag::EnableExceptionHandling;

        PlatformKind platform = PlatformKind::Unknown;

        String modulePath;      ///< The path/name of the output module. Should not have the extension, as that will be added for each of the target types

        List<Define> defines;

        List<String> sourceFiles;

        List<String> includePaths;
        List<String> libraryPaths;
    };

    struct Diagnostic
    {
        enum class Type
        {
            Unknown,
            Info,
            Warning,
            Error,
            CountOf,
        };
        enum class Stage
        {
            Compile,
            Link,
        };

        void reset()
        {
            type = Type::Unknown;
            stage = Stage::Compile;
            fileLine = 0;
        }
        static UnownedStringSlice getTypeText(Diagnostic::Type type);


        Type type;                      ///< The type of error
        Stage stage;                    ///< The stage the error came from
        String text;                    ///< The text of the error
        String code;                    ///< The compiler specific error code
        String filePath;                ///< The path the error originated from
        Int fileLine;                   ///< The line number the error came from
    };

    typedef uint32_t ProductFlags;
    struct ProductFlag
    {
        enum Enum : ProductFlags
        {
            Debug               = 0x1,                ///< Used by debugger during execution
            Execution           = 0x2,                ///< Required for execution
            Compile             = 0x4,                ///< A product *required* for compilation
            Miscellaneous       = 0x8,                ///< Anything else    
        };
        enum Mask : ProductFlags
        {
            All                 = 0xf,                ///< All the flags
        };
    };

    enum class Product
    {
        DebugRun,
        Run,
        CompileTemporary,
        All,
    };

    struct Output
    {
            /// Reset to an initial empty state
        void reset() { diagnostics.clear(); rawDiagnostics = String(); result = SLANG_OK; }
        
            /// Get the number of diagnostics by type
        Index getCountByType(Diagnostic::Type type) const;
            /// True if there are any diagnostics  of the type
        bool has(Diagnostic::Type type) const { return getCountByType(type) > 0; }

            /// Stores in outCounts, the amount of diagnostics for the stage of each type
        Int countByStage(Diagnostic::Stage stage, Index outCounts[Int(Diagnostic::Type::CountOf)]) const;

            /// Append a summary to out
        void appendSummary(StringBuilder& out) const;
            /// Appends a summary that just identifies if there is an error of a type (not a count)
        void appendSimplifiedSummary(StringBuilder& out) const;
        
            /// Remove all diagnostics of the type
        void removeByType(Diagnostic::Type type);

        String rawDiagnostics;

        SlangResult result;
        List<Diagnostic> diagnostics;
    };

        /// Get the desc of this compiler
    const Desc& getDesc() const { return m_desc;  }
        /// Compile using the specified options. The result is in resOut
    virtual SlangResult compile(const CompileOptions& options, Output& outOutput) = 0;
        /// Given the compilation options and the module name, determines the actual file name used for output
    virtual SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath) = 0;
        /// Given options determines the paths to products produced (including the 'moduleFilePath').
        /// Note that does *not* guarentee all products were or should be produced. Just aims to include all that could
        /// be produced, such that can be removed on completion.
    virtual SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags flags, List<String>& outPaths) = 0;

        /// Return the compiler type as name
    static UnownedStringSlice getCompilerTypeAsText(CompilerType type);

protected:

    CPPCompiler(const Desc& desc) :
        m_desc(desc)
    {}

    Desc m_desc;
};

class CommandLineCPPCompiler : public CPPCompiler
{
public:
    typedef CPPCompiler Super;

    // CPPCompiler
    virtual SlangResult compile(const CompileOptions& options, Output& outOutput) SLANG_OVERRIDE;
    
    // Functions to be implemented for a specific CommandLine    
    virtual SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine) = 0;
    virtual SlangResult parseOutput(const ExecuteResult& exeResult, Output& output) = 0;

    CommandLineCPPCompiler(const Desc& desc, const String& exeName) :
        Super(desc)
    {
        m_cmdLine.setExecutableFilename(exeName);
    }

    CommandLineCPPCompiler(const Desc& desc, const CommandLine& cmdLine) :
        Super(desc),
        m_cmdLine(cmdLine)
    {}

    CommandLineCPPCompiler(const Desc& desc):Super(desc) {}

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

        /// True if has a compiler of the specified type
    bool hasCompiler(CPPCompiler::CompilerType compilerType) const;

protected:

    Index _findIndex(const CPPCompiler::Desc& desc) const;

    RefPtr<CPPCompiler> m_defaultCompiler;
    // This could be a dictionary/map - but doing a linear search is going to be fine and it makes
    // somethings easier.
    List<RefPtr<CPPCompiler>> m_compilers;
};

/* Only purpose of having base-class here is to make all the CPPCompiler types available directly in derived Utils */
struct CPPCompilerBaseUtil
{
    typedef CPPCompiler::CompileOptions CompileOptions;
    typedef CPPCompiler::OptimizationLevel OptimizationLevel;
    typedef CPPCompiler::TargetType TargetType;
    typedef CPPCompiler::DebugInfoType DebugInfoType;
    typedef CPPCompiler::SourceType SourceType;
    typedef CPPCompiler::CompilerType CompilerType;

    typedef CPPCompiler::Diagnostic Diagnostic;
    typedef CPPCompiler::FloatingPointMode FloatingPointMode;
    typedef CPPCompiler::ProductFlag ProductFlag;
    typedef CPPCompiler::ProductFlags ProductFlags;
};

struct CPPCompilerUtil: public CPPCompilerBaseUtil
{
    enum class MatchType
    {
        MinGreaterEqual,
        MinAbsolute,
        Newest,
    };

    struct InitializeSetDesc
    {
        const String& getPath(CompilerType type) const { return paths[int(type)]; }
        void setPath(CompilerType type, const String& path) { paths[int(type)] = path; }

        String paths[int(CPPCompiler::CompilerType::CountOf)];
    };

        /// Find a compiler
    static CPPCompiler* findCompiler(const CPPCompilerSet* set, MatchType matchType, const CPPCompiler::Desc& desc);
    static CPPCompiler* findCompiler(const List<CPPCompiler*>& compilers, MatchType matchType, const CPPCompiler::Desc& desc);

        /// Find the compiler closest to the desc 
    static CPPCompiler* findClosestCompiler(const List<CPPCompiler*>& compilers, const CPPCompiler::Desc& desc);
    static CPPCompiler* findClosestCompiler(const CPPCompilerSet* set, const CPPCompiler::Desc& desc);

        /// Get the information on the compiler used to compile this source
    static const CPPCompiler::Desc& getCompiledWithDesc();

        /// Given a set, registers compilers found through standard means and determines a reasonable default compiler if possible
    static SlangResult initializeSet(const InitializeSetDesc& desc, CPPCompilerSet* set);    
};

}

#endif
