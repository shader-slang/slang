#ifndef SLANG_DOWNSTREAM_COMPILER_H
#define SLANG_DOWNSTREAM_COMPILER_H

#include "../core/slang-common.h"
#include "../core/slang-string.h"

#include "../core/slang-process-util.h"

#include "../core/slang-platform.h"
#include "../core/slang-semantic-version.h"

#include "../core/slang-io.h"

#include "../../slang-com-ptr.h"

#include "slang-artifact.h"

namespace Slang
{

struct SourceManager;

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

        /// Given a path, that holds line number and potentially column number in () after path, writes result into outDiagnostic
    static SlangResult splitPathLocation(const UnownedStringSlice& pathLocation, DownstreamDiagnostic& outDiagnostic);

        /// Split the line (separated by :), where a path is at pathIndex 
    static SlangResult splitColonDelimitedLine(const UnownedStringSlice& line, Int pathIndex, List<UnownedStringSlice>& outSlices);

    typedef SlangResult (*LineParser)(const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic);

        /// Given diagnostics in inText that are colon delimited, use lineParser to do per line parsing.
    static SlangResult parseColonDelimitedDiagnostics(const UnownedStringSlice& inText, Int pathIndex, LineParser lineParser, List<DownstreamDiagnostic>& outDiagnostics);

    Severity severity = Severity::Unknown;          ///< The severity of error
    Stage stage = Stage::Compile;                   ///< The stage the error came from
    String text;                                    ///< The text of the error
    String code;                                    ///< The compiler specific error code
    String filePath;                                ///< The path the error originated from
    Int fileLine = 0;                               ///< The line number the error came from
};

struct DownstreamDiagnostics
{
    typedef DownstreamDiagnostic Diagnostic;

        /// Reset to an initial empty state
    void reset() { diagnostics.clear(); rawDiagnostics = String(); result = SLANG_OK; }

        /// Count the number of diagnostics which have 'severity' or greater 
    Index getCountAtLeastSeverity(Diagnostic::Severity severity) const;

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

        /// Add a note
    void addNote(const UnownedStringSlice& in);

        /// If there are no error diagnostics, adds a generic error diagnostic
    void requireErrorDiagnostic();

    static void addNote(const UnownedStringSlice& in, List<DownstreamDiagnostic>& ioDiagnostics);

    String rawDiagnostics;

    SlangResult result = SLANG_OK;
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

// Combination of a downstream compiler type (pass through) and 
// a match version.
struct DownstreamCompilerMatchVersion
{
    DownstreamCompilerMatchVersion(SlangPassThrough inType, MatchSemanticVersion inMatchVersion):
        type(inType),
        matchVersion(inMatchVersion)
    {}

    DownstreamCompilerMatchVersion():type(SLANG_PASS_THROUGH_NONE) {}

    SlangPassThrough type;                  ///< The type of the compiler
    MatchSemanticVersion matchVersion;      ///< The match version
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

    
    // Compiler description
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

        explicit Desc(SlangPassThrough inType, const SemanticVersion& version):type(inType), majorVersion(version.m_major), minorVersion(version.m_minor) {}

        SlangPassThrough type;      ///< The type of the compiler

        /// TODO(JS): Would probably be better if changed to SemanticVersion, but not trivial to change
        // because this type is part of the DownstreamCompiler interface, which is used with `slang-llvm`.
        Int majorVersion;           ///< Major version (interpretation is type specific)
        Int minorVersion;           ///< Minor version (interpretation is type specific)
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
            SPIRV,
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
        SlangCompileTarget targetType = SLANG_HOST_EXECUTABLE;
        SlangSourceLanguage sourceLanguage = SLANG_SOURCE_LANGUAGE_CPP;
        FloatingPointMode floatingPointMode = FloatingPointMode::Default;
        PipelineType pipelineType = PipelineType::Unknown;
        SlangMatrixLayoutMode matrixLayout = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;

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

            /// Libraries to link against.
        List<ComPtr<IArtifact>> libraries;

        List<CapabilityVersion> requiredCapabilityVersions;

            /// For compilers/compiles that require an entry point name, else can be empty
        String entryPointName;
            /// Profile name to use, only required for compiles that need to compile against a a specific profiles.
            /// Profile names are tied to compilers and targets.
        String profileName;

            /// The stage being compiled for 
        SlangStage stage = SLANG_STAGE_NONE;

            /// Arguments that are specific to a particular compiler implementation.
        List<String> compilerSpecificArguments;

            /// NOTE! Not all downstream compilers can use the fileSystemExt/sourceManager. This option will be ignored in those scenarios.
        ISlangFileSystemExt* fileSystemExt = nullptr;
        SourceManager* sourceManager = nullptr;
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
        /// Some compilers have support converting a binary blob into disassembly. Output disassembly is held in the output blob
    virtual SlangResult disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out);

        /// True if underlying compiler uses file system to communicate source
    virtual bool isFileBased() = 0;

        /// Get info for a compiler type
    static const Info& getInfo(SlangPassThrough compiler) { return s_infos.infos[int(compiler)]; }
        /// True if this compiler can compile the specified language
    static bool canCompile(SlangPassThrough compiler, SlangSourceLanguage sourceLanguage);

    
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
    virtual bool isFileBased() SLANG_OVERRIDE { return true; }

    // Functions to be implemented for a specific CommandLine

        /// Given the compilation options and the module name, determines the actual file name used for output
    virtual SlangResult calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath) = 0;
        /// Given options determines the paths to products produced (including the 'moduleFilePath').
        /// Note that does *not* guarentee all products were or should be produced. Just aims to include all that could
        /// be produced, such that can be removed on completion.
    virtual SlangResult calcCompileProducts(const CompileOptions& options, ProductFlags flags, List<String>& outPaths) = 0;

    virtual SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine) = 0;
    virtual SlangResult parseOutput(const ExecuteResult& exeResult, DownstreamDiagnostics& output) = 0;

    CommandLineDownstreamCompiler(const Desc& desc, const ExecutableLocation& exe) :
        Super(desc)
    {
        m_cmdLine.setExecutableLocation(exe);
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
    DownstreamCompiler* getDefaultCompiler(SlangSourceLanguage sourceLanguage) const { return m_defaultCompilers[int(sourceLanguage)];  }
        /// Set the default compiler
    void setDefaultCompiler(SlangSourceLanguage sourceLanguage, DownstreamCompiler* compiler) { m_defaultCompilers[int(sourceLanguage)] = compiler;  }

        /// True if has a compiler of the specified type
    bool hasCompiler(SlangPassThrough compilerType) const;

    void remove(SlangPassThrough compilerType);

    void clear() { m_compilers.clear(); }

    bool hasSharedLibrary(ISlangSharedLibrary* lib);
    void addSharedLibrary(ISlangSharedLibrary* lib);

    ~DownstreamCompilerSet()
    {
        // A compiler may be implemented in a shared library, so release all first.
        m_compilers.clearAndDeallocate();
        for (auto& defaultCompiler : m_defaultCompilers)
        {
            defaultCompiler.setNull();
        }

        // Release any shared libraries
        m_sharedLibraries.clearAndDeallocate();
    }

protected:

    Index _findIndex(const DownstreamCompiler::Desc& desc) const;


    RefPtr<DownstreamCompiler> m_defaultCompilers[int(SLANG_SOURCE_LANGUAGE_COUNT_OF)];
    // This could be a dictionary/map - but doing a linear search is going to be fine and it makes
    // somethings easier.
    List<RefPtr<DownstreamCompiler>> m_compilers;

    List<ComPtr<ISlangSharedLibrary>> m_sharedLibraries;
};

typedef SlangResult (*DownstreamCompilerLocatorFunc)(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);

/* Only purpose of having base-class here is to make all the DownstreamCompiler types available directly in derived Utils */
struct DownstreamCompilerBaseUtil
{
    typedef DownstreamCompiler::CompileOptions CompileOptions;
    typedef DownstreamCompiler::OptimizationLevel OptimizationLevel;
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

    static DownstreamCompiler* findCompiler(const List<DownstreamCompiler*>& compilers, SlangPassThrough type, const SemanticVersion& version);
    static DownstreamCompiler* findCompiler(const List<DownstreamCompiler*>& compilers, const DownstreamCompiler::Desc& desc);

        /// Find all the compilers with the version
    static void findVersions(const List<DownstreamCompiler*>& compilers, SlangPassThrough compiler, List<SemanticVersion>& versions);

    
        /// Find the compiler closest to the desc 
    static DownstreamCompiler* findClosestCompiler(const List<DownstreamCompiler*>& compilers, const DownstreamCompilerMatchVersion& version);
    static DownstreamCompiler* findClosestCompiler(const DownstreamCompilerSet* set, const DownstreamCompilerMatchVersion& version);

        /// Get the information on the compiler used to compile this source
    static DownstreamCompilerMatchVersion getCompiledVersion();

    static void updateDefault(DownstreamCompilerSet* set, SlangSourceLanguage sourceLanguage);
    static void updateDefaults(DownstreamCompilerSet* set);

    static void setDefaultLocators(DownstreamCompilerLocatorFunc outFuncs[int(SLANG_PASS_THROUGH_COUNT_OF)]);

        /// Attempts to determine what 'path' is and load appropriately. Is it a path to a shared library? Is it a directory holding the libraries?
        /// Some downstream shared libraries need other shared libraries to be loaded before the main shared library, such that they are in the same directory
        /// otherwise the shared library could come from some unwanted location.
        /// dependentNames names shared libraries which should be attempted to be loaded in the path of the main shared library.
        /// The list is optional (nullptr can be passed in), and the list is terminated by nullptr.
    static SlangResult loadSharedLibrary(const String& path, ISlangSharedLibraryLoader* loader, const char*const* dependantNames, const char* libraryName, ComPtr<ISlangSharedLibrary>& outSharedLib);

};

}

#endif
