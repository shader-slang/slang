#ifndef SLANG_DOWNSTREAM_COMPILER_H
#define SLANG_DOWNSTREAM_COMPILER_H

#include "../core/slang-common.h"
#include "../core/slang-string.h"

#include "../core/slang-process-util.h"

#include "../core/slang-platform.h"
#include "../core/slang-semantic-version.h"

#include "../core/slang-io.h"

#include "../../slang-com-ptr.h"

namespace Slang
{

struct DownstreamDiagnostic
{
    typedef DownstreamDiagnostic ThisType;

    enum class Severity
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
        severity = Severity::Unknown;
        stage = Stage::Compile;
        fileLine = 0;
    }

    bool operator==(const ThisType& rhs) const
    {
        return severity == rhs.severity &&
            stage == rhs.stage &&
            text == rhs.text &&
            code == rhs.code &&
            filePath == rhs.filePath &&
            fileLine == rhs.fileLine;
    }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    static UnownedStringSlice getSeverityText(Severity severity);

    Severity severity;              ///< The severity of error
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

        /// Get the number of diagnostics by severity
    Index getCountBySeverity(Diagnostic::Severity severity) const;
        /// True if there are any diagnostics  of severity
    bool has(Diagnostic::Severity severity) const { return getCountBySeverity(severity) > 0; }

        /// Stores in outCounts, the amount of diagnostics for the stage of each severity
    Int countByStage(Diagnostic::Stage stage, Index outCounts[Int(Diagnostic::Severity::CountOf)]) const;

        /// Append a summary to out
    void appendSummary(StringBuilder& out) const;
        /// Appends a summary that just identifies if there is an error of a type (not a count)
    void appendSimplifiedSummary(StringBuilder& out) const;

        /// Remove all diagnostics of the type
    void removeBySeverity(Diagnostic::Severity severity);

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
    virtual SlangResult getBinary(ComPtr<ISlangBlob>& outBlob) SLANG_OVERRIDE { outBlob = m_blob; return m_blob ? SLANG_OK : SLANG_FAIL; }

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

    typedef uint32_t SourceLanguageFlags;
    struct SourceLanguageFlag
    {
        enum Enum : SourceLanguageFlags
        {
            Unknown = SourceLanguageFlags(1) << SLANG_SOURCE_LANGUAGE_UNKNOWN,
            Slang   = SourceLanguageFlags(1) << SLANG_SOURCE_LANGUAGE_SLANG,
            HLSL    = SourceLanguageFlags(1) << SLANG_SOURCE_LANGUAGE_HLSL,
            GLSL    = SourceLanguageFlags(1) << SLANG_SOURCE_LANGUAGE_GLSL,
            C       = SourceLanguageFlags(1) << SLANG_SOURCE_LANGUAGE_C,
            CPP     = SourceLanguageFlags(1) << SLANG_SOURCE_LANGUAGE_CPP,
            CUDA    = SourceLanguageFlags(1) << SLANG_SOURCE_LANGUAGE_CUDA,
        };
    };

    struct Info
    {
        Info():sourceLanguageFlags(0) {}

        Info(SourceLanguageFlags inSourceLanguageFlags):
            sourceLanguageFlags(inSourceLanguageFlags)
        {}
        SourceLanguageFlags sourceLanguageFlags;
    };
    struct Infos
    {
        Info infos[int(SLANG_PASS_THROUGH_COUNT_OF)];
    };

    struct Desc
    {
        typedef Desc ThisType;

        HashCode getHashCode() const { return combineHash(HashCode(type), combineHash(HashCode(majorVersion), HashCode(minorVersion))); }
        bool operator==(const ThisType& rhs) const { return type == rhs.type && majorVersion == rhs.majorVersion && minorVersion == rhs.minorVersion;  }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

            /// Get the version as a value
        Int getVersionValue() const { return majorVersion * 100 + minorVersion;  }

        void appendAsText(StringBuilder& out) const;
            /// true if has a version set
        bool hasVersion() const { return majorVersion || minorVersion; }

            /// Ctor
        explicit Desc(SlangPassThrough inType = SLANG_PASS_THROUGH_NONE, Int inMajorVersion = 0, Int inMinorVersion = 0):type(inType), majorVersion(inMajorVersion), minorVersion(inMinorVersion) {}

        SlangPassThrough type;          ///< The type of the compiler
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

    enum PipelineType
    {
        Unknown,
        Compute,
        Rasterization,
        RayTracing,
    };

    struct Define
    {
        String nameWithSig;             ///< If macro takes parameters include in brackets
        String value;
    };

    struct CapabilityVersion
    {
        enum class Kind
        {
            CUDASM,                     ///< What the version is for
        };
        Kind kind;
        SemanticVersion version;
    };

    struct CompileOptions
    {
        typedef uint32_t Flags;
        struct Flag 
        {
            enum Enum : Flags
            {
                EnableExceptionHandling = 0x01,             ///< Enables exception handling support (say as optionally supported by C++)
                Verbose                 = 0x02,             ///< Give more verbose diagnostics
                EnableSecurityChecks    = 0x04,             ///< Enable runtime security checks (such as for buffer overruns) - enabling typically decreases performance
                EnableFloat16           = 0x08,             ///< If set compiles with support for float16/half
            };
        };

        OptimizationLevel optimizationLevel = OptimizationLevel::Default;
        DebugInfoType debugInfoType = DebugInfoType::Standard;
        TargetType targetType = TargetType::Executable;
        SlangSourceLanguage sourceLanguage = SLANG_SOURCE_LANGUAGE_CPP;
        FloatingPointMode floatingPointMode = FloatingPointMode::Default;
        PipelineType pipelineType = PipelineType::Unknown;

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

        List<CapabilityVersion> requiredCapabilityVersions;
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
        /// Some downstream compilers are backed by a shared library. This allows access to the shared library to access internal functions. 
    virtual ISlangSharedLibrary* getSharedLibrary() { return nullptr; }

        /// Get info for a compiler type
    static const Info& getInfo(SlangPassThrough compiler) { return s_infos.infos[int(compiler)]; }
        /// True if this compiler can compile the specified language
    static bool canCompile(SlangPassThrough compiler, SlangSourceLanguage sourceLanguage);

        /// Given a source language return as the equivalent compile target
    static SlangCompileTarget getCompileTarget(SlangSourceLanguage sourceLanguage);

protected:
    static Infos s_infos;

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

class SharedLibraryDownstreamCompiler: public DownstreamCompiler
{
public:
    typedef DownstreamCompiler Super;

    // DownstreamCompiler
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE { SLANG_UNUSED(options); SLANG_UNUSED(outResult); return SLANG_E_NOT_IMPLEMENTED; }
    virtual ISlangSharedLibrary* getSharedLibrary() SLANG_OVERRIDE { return m_library; }

    SharedLibraryDownstreamCompiler(const Desc& desc, ISlangSharedLibrary* library):
        Super(desc),
        m_library(library)
    {
    }
protected:
    ComPtr<ISlangSharedLibrary> m_library;
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
    DownstreamCompiler* getDefaultCompiler(SlangSourceLanguage sourceLanguage) const { return m_defaultCompilers[int(sourceLanguage)];  }
        /// Set the default compiler
    void setDefaultCompiler(SlangSourceLanguage sourceLanguage, DownstreamCompiler* compiler) { m_defaultCompilers[int(sourceLanguage)] = compiler;  }

        /// True if has a compiler of the specified type
    bool hasCompiler(SlangPassThrough compilerType) const;

    void remove(SlangPassThrough compilerType);

    void clear() { m_compilers.clear(); }

protected:

    Index _findIndex(const DownstreamCompiler::Desc& desc) const;

    RefPtr<DownstreamCompiler> m_defaultCompilers[int(SLANG_SOURCE_LANGUAGE_COUNT_OF)];
    // This could be a dictionary/map - but doing a linear search is going to be fine and it makes
    // somethings easier.
    List<RefPtr<DownstreamCompiler>> m_compilers;
};

typedef SlangResult (*DownstreamCompilerLocatorFunc)(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);

/* Only purpose of having base-class here is to make all the DownstreamCompiler types available directly in derived Utils */
struct DownstreamCompilerBaseUtil
{
    typedef DownstreamCompiler::CompileOptions CompileOptions;
    typedef DownstreamCompiler::OptimizationLevel OptimizationLevel;
    typedef DownstreamCompiler::TargetType TargetType;
    typedef DownstreamCompiler::DebugInfoType DebugInfoType;
    
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

        /// Find a compiler
    static DownstreamCompiler* findCompiler(const DownstreamCompilerSet* set, MatchType matchType, const DownstreamCompiler::Desc& desc);
    static DownstreamCompiler* findCompiler(const List<DownstreamCompiler*>& compilers, MatchType matchType, const DownstreamCompiler::Desc& desc);

        /// Find the compiler closest to the desc 
    static DownstreamCompiler* findClosestCompiler(const List<DownstreamCompiler*>& compilers, const DownstreamCompiler::Desc& desc);
    static DownstreamCompiler* findClosestCompiler(const DownstreamCompilerSet* set, const DownstreamCompiler::Desc& desc);

        /// Get the information on the compiler used to compile this source
    static const DownstreamCompiler::Desc& getCompiledWithDesc();

    static void updateDefault(DownstreamCompilerSet* set, SlangSourceLanguage sourceLanguage);
    static void updateDefaults(DownstreamCompilerSet* set);

    static void setDefaultLocators(DownstreamCompilerLocatorFunc outFuncs[int(SLANG_PASS_THROUGH_COUNT_OF)]);
};

}

#endif
