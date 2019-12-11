#ifndef SLANG_DOWNSTREAM_COMPILER_H
#define SLANG_DOWNSTREAM_COMPILER_H

#include "slang-common.h"
#include "slang-string.h"

#include "slang-process-util.h"

#include "slang-platform.h"

#include "slang-io.h"

#include "../../slang-com-ptr.h"

namespace Slang
{

struct DownstreamDiagnostic
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
    static UnownedStringSlice getTypeText(Type type);

    Type type;                      ///< The type of error
    Stage stage;                    ///< The stage the error came from
    String text;                    ///< The text of the error
    String code;                    ///< The compiler specific error code
    String filePath;                ///< The path the error originated from
    Int fileLine;                   ///< The line number the error came from
};

struct DownstreamDiagnostics
{
    typedef DownstreamDiagnostic Diagnostic;

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

class DownstreamCompileResult : public RefObject
{
public:

    virtual SlangResult getHostCallableSharedLibrary(ComPtr<ISlangSharedLibrary>& outLibrary) = 0;
    virtual SlangResult getBinary(ComPtr<ISlangBlob>& outBlob) = 0;

    const DownstreamDiagnostics& getDiagnostics() const { return m_diagnostics; }
    
        /// Ctor
    DownstreamCompileResult(const DownstreamDiagnostics& diagnostics):
        m_diagnostics(diagnostics)
    {}

protected:
    DownstreamDiagnostics m_diagnostics;
};


class BlobDownstreamCompileResult : public DownstreamCompileResult
{
public:
    typedef DownstreamCompileResult Super;

    virtual SlangResult getHostCallableSharedLibrary(ComPtr<ISlangSharedLibrary>& outLibrary) SLANG_OVERRIDE { SLANG_UNUSED(outLibrary); return SLANG_FAIL; }
    virtual SlangResult getBinary(ComPtr<ISlangBlob>& outBlob) { outBlob = m_blob; return m_blob ? SLANG_OK : SLANG_FAIL; }

    BlobDownstreamCompileResult(const DownstreamDiagnostics& diags, ISlangBlob* blob):
        Super(diags),
        m_blob(blob)
    {

    }
protected:
    ComPtr<ISlangBlob> m_blob;
};

class DownstreamCompiler: public RefObject
{
public:
    typedef RefObject Super;

    typedef DownstreamCompileResult CompileResult;

    enum class CompilerType
    {
        Unknown,
        VisualStudio,
        GCC,
        Clang,
        SNC,
        GHS,
        NVRTC,
        CountOf,
    };
    enum class SourceType
    {
        C,              ///< C source
        CPP,            ///< C++ source
        CUDA,           ///< The CUDA language
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
                EnableSecurityChecks    = 0x04,
            };
        };

        OptimizationLevel optimizationLevel = OptimizationLevel::Default;
        DebugInfoType debugInfoType = DebugInfoType::Standard;
        TargetType targetType = TargetType::Executable;
        SourceType sourceType = SourceType::CPP;
        FloatingPointMode floatingPointMode = FloatingPointMode::Default;

        Flags flags = Flag::EnableExceptionHandling;

        PlatformKind platform = PlatformKind::Unknown;

            /// The path/name of the output module. Should not have the extension, as that will be added for each of the target types.
            /// If not set a module path will be internally generated internally on a command line based compiler
        String modulePath;                  

        List<Define> defines;

            /// The contents of the source to compile. This can be empty is sourceFiles is set.
            /// If the compiler is a commandLine file this source will be written to a temporary file.
        String sourceContents;
            /// 'Path' that the contents originated from. NOTE! This is for reporting only and doesn't have to exist on file system
        String sourceContentsPath;

            /// The names/paths of source to compile. This can be empty if sourceContents is set.
        List<String> sourceFiles;           

        List<String> includePaths;
        List<String> libraryPaths;
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

        /// Get the desc of this compiler
    const Desc& getDesc() const { return m_desc;  }
        /// Compile using the specified options. The result is in resOut
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) = 0;
 
        /// Return the compiler type as name
    static UnownedStringSlice getCompilerTypeAsText(CompilerType type);

protected:

    DownstreamCompiler(const Desc& desc) :
        m_desc(desc)
    {}
    DownstreamCompiler() {}

    Desc m_desc;
};

class CommandLineDownstreamCompileResult : public DownstreamCompileResult
{
public:
    typedef DownstreamCompileResult Super;

    virtual SlangResult getHostCallableSharedLibrary(ComPtr<ISlangSharedLibrary>& outLibrary) SLANG_OVERRIDE;
    virtual SlangResult getBinary(ComPtr<ISlangBlob>& outBlob) SLANG_OVERRIDE;

    CommandLineDownstreamCompileResult(const DownstreamDiagnostics& diagnostics, const String& moduleFilePath, TemporaryFileSet* temporaryFileSet) :
        Super(diagnostics),
        m_moduleFilePath(moduleFilePath),
        m_temporaryFiles(temporaryFileSet)
    {
    }
    
    RefPtr<TemporaryFileSet> m_temporaryFiles;

protected:

    String m_moduleFilePath;
    DownstreamCompiler::CompileOptions m_options;
    ComPtr<ISlangBlob> m_binaryBlob;
    /// Cache of the shared library if appropriate
    ComPtr<ISlangSharedLibrary> m_hostCallableSharedLibrary;
};

class CommandLineDownstreamCompiler : public DownstreamCompiler
{
public:
    typedef DownstreamCompiler Super;

    // DownstreamCompiler
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE;
    
    // Functions to be implemented for a specific CommandLine

        /// Given the compilation options and the module name, determines the actual file name used for output
    virtual SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath) = 0;
        /// Given options determines the paths to products produced (including the 'moduleFilePath').
        /// Note that does *not* guarentee all products were or should be produced. Just aims to include all that could
        /// be produced, such that can be removed on completion.
    virtual SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags flags, List<String>& outPaths) = 0;

    virtual SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine) = 0;
    virtual SlangResult parseOutput(const ExecuteResult& exeResult, DownstreamDiagnostics& output) = 0;

    CommandLineDownstreamCompiler(const Desc& desc, const String& exeName) :
        Super(desc)
    {
        m_cmdLine.setExecutableFilename(exeName);
    }

    CommandLineDownstreamCompiler(const Desc& desc, const CommandLine& cmdLine) :
        Super(desc),
        m_cmdLine(cmdLine)
    {}

    CommandLineDownstreamCompiler(const Desc& desc):Super(desc) {}

    CommandLine m_cmdLine;
};

class DownstreamCompilerSet : public RefObject
{
public:
    typedef RefObject Super;

        /// Find all the available compilers
    void getCompilerDescs(List<DownstreamCompiler::Desc>& outCompilerDescs) const;
        /// Returns list of all compilers
    void getCompilers(List<DownstreamCompiler*>& outCompilers) const;

        /// Get a compiler
    DownstreamCompiler* getCompiler(const DownstreamCompiler::Desc& compilerDesc) const;
  
        /// Will replace if there is one with same desc
    void addCompiler(DownstreamCompiler* compiler);

        /// Get a default compiler
    DownstreamCompiler* getDefaultCompiler(DownstreamCompiler::SourceType sourceType) const { return m_defaultCompilers[int(sourceType)];  }
        /// Set the default compiler
    void setDefaultCompiler(DownstreamCompiler::SourceType sourceType, DownstreamCompiler* compiler) { m_defaultCompilers[int(sourceType)] = compiler;  }

        /// True if has a compiler of the specified type
    bool hasCompiler(DownstreamCompiler::CompilerType compilerType) const;

protected:

    Index _findIndex(const DownstreamCompiler::Desc& desc) const;

    RefPtr<DownstreamCompiler> m_defaultCompilers[int(DownstreamCompiler::SourceType::CountOf)];
    // This could be a dictionary/map - but doing a linear search is going to be fine and it makes
    // somethings easier.
    List<RefPtr<DownstreamCompiler>> m_compilers;
};

typedef SlangResult (*DownstreamCompilerLocatorFunc)(const char* path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);

/* Only purpose of having base-class here is to make all the DownstreamCompiler types available directly in derived Utils */
struct DownstreamCompilerBaseUtil
{
    typedef DownstreamCompiler::CompileOptions CompileOptions;
    typedef DownstreamCompiler::OptimizationLevel OptimizationLevel;
    typedef DownstreamCompiler::TargetType TargetType;
    typedef DownstreamCompiler::DebugInfoType DebugInfoType;
    typedef DownstreamCompiler::SourceType SourceType;
    typedef DownstreamCompiler::CompilerType CompilerType;

    typedef DownstreamDiagnostics::Diagnostic Diagnostic;

    typedef DownstreamCompiler::FloatingPointMode FloatingPointMode;
    typedef DownstreamCompiler::ProductFlag ProductFlag;
    typedef DownstreamCompiler::ProductFlags ProductFlags;
};

struct DownstreamCompilerUtil: public DownstreamCompilerBaseUtil
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

        String paths[int(DownstreamCompiler::CompilerType::CountOf)];
    };

        /// Find a compiler
    static DownstreamCompiler* findCompiler(const DownstreamCompilerSet* set, MatchType matchType, const DownstreamCompiler::Desc& desc);
    static DownstreamCompiler* findCompiler(const List<DownstreamCompiler*>& compilers, MatchType matchType, const DownstreamCompiler::Desc& desc);

        /// Find the compiler closest to the desc 
    static DownstreamCompiler* findClosestCompiler(const List<DownstreamCompiler*>& compilers, const DownstreamCompiler::Desc& desc);
    static DownstreamCompiler* findClosestCompiler(const DownstreamCompilerSet* set, const DownstreamCompiler::Desc& desc);

        /// Get the information on the compiler used to compile this source
    static const DownstreamCompiler::Desc& getCompiledWithDesc();

        /// Given a set, registers compilers found through standard means and determines a reasonable default compiler if possible
    static SlangResult initializeSet(const InitializeSetDesc& desc, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
        
    static void updateDefault(DownstreamCompilerSet* set, DownstreamCompiler::SourceType type);
    static void updateDefaults(DownstreamCompilerSet* set);
};

}

#endif
