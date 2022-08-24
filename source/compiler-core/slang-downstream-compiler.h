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
#include "slang-artifact-associated.h"

namespace Slang
{

struct SourceManager;

// Compiler description
struct DownstreamCompilerDesc
{
    typedef DownstreamCompilerDesc ThisType;

    HashCode getHashCode() const { return combineHash(HashCode(type), version.getHashCode()); }
    bool operator==(const ThisType& rhs) const { return type == rhs.type && version == rhs.version; }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        /// Get the version as a value
    Int getVersionValue() const { return version.m_major * 100 + version.m_minor; }

        /// true if has a version set
    bool hasVersion() const { return version.isSet(); }

    /// Ctor
    explicit DownstreamCompilerDesc(SlangPassThrough inType = SLANG_PASS_THROUGH_NONE, Int inMajorVersion = 0, Int inMinorVersion = 0) :type(inType), version(int(inMajorVersion), int(inMinorVersion)) {}
    explicit DownstreamCompilerDesc(SlangPassThrough inType, const SemanticVersion& inVersion) : type(inType), version(inVersion) {}

    SlangPassThrough type;      ///< The type of the compiler
    SemanticVersion version;    ///< The version of the compiler
};

struct DownstreamCompileOptions
{
    typedef uint32_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            EnableExceptionHandling     = 0x01,             ///< Enables exception handling support (say as optionally supported by C++)
            Verbose                     = 0x02,             ///< Give more verbose diagnostics
            EnableSecurityChecks        = 0x04,             ///< Enable runtime security checks (such as for buffer overruns) - enabling typically decreases performance
            EnableFloat16               = 0x08,             ///< If set compiles with support for float16/half
        };
    };

    enum class OptimizationLevel : uint8_t
    {
        None,           ///< Don't optimize at all. 
        Default,        ///< Default optimization level: balance code quality and compilation time. 
        High,           ///< Optimize aggressively. 
        Maximal,        ///< Include optimizations that may take a very long time, or may involve severe space-vs-speed tradeoffs 
    };

    enum class DebugInfoType : uint8_t
    {
        None,       ///< Don't emit debug information at all. 
        Minimal,    ///< Emit as little debug information as possible, while still supporting stack traces. 
        Standard,   ///< Emit whatever is the standard level of debug information for each target. 
        Maximal,    ///< Emit as much debug information as possible for each target. 
    };
    enum class FloatingPointMode : uint8_t
    {
        Default,
        Fast,
        Precise,
    };

    enum PipelineType : uint8_t
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
        enum class Kind : uint8_t
        {
            CUDASM,                     ///< What the version is for
            SPIRV,
        };
        Kind kind;
        SemanticVersion version;
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

/* Used to indicate what kind of products are expected to be produced for a compilation. */
typedef uint32_t DownstreamProductFlags;
struct DownstreamProductFlag
{
    enum Enum : DownstreamProductFlags
    {
        Debug = 0x1,                    ///< Used by debugger during execution
        Execution = 0x2,                ///< Required for execution
        Compile = 0x4,                  ///< A product *required* for compilation
        Miscellaneous = 0x8,            ///< Anything else    
    };
    enum Mask : DownstreamProductFlags
    {
        All = 0xf,                ///< All the flags
    };
};

class IDownstreamCompiler : public ICastable
{
public:
    SLANG_COM_INTERFACE(0x167b8ba7, 0xbd41, 0x469a, { 0x92, 0x28, 0xb8, 0x53, 0xc8, 0xea, 0x56, 0x6d })

    typedef DownstreamCompilerDesc Desc;
    typedef DownstreamCompileOptions CompileOptions;
    
    typedef CompileOptions::OptimizationLevel OptimizationLevel;
    typedef CompileOptions::DebugInfoType DebugInfoType;
    typedef CompileOptions::FloatingPointMode FloatingPointMode;
    typedef CompileOptions::PipelineType PipelineType;
    typedef CompileOptions::Define Define;
    typedef CompileOptions::CapabilityVersion CapabilityVersion;

        /// Get the desc of this compiler
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() = 0;
        /// Compile using the specified options. The result is in resOut
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, IArtifact** outArtifact) = 0;
        /// Returns true if compiler can do a transformation of `from` to `to` Artifact types
    virtual SLANG_NO_THROW bool SLANG_MCALL canConvert(const ArtifactDesc& from, const ArtifactDesc& to) = 0;
        /// Converts an artifact `from` to a desc of `to` and puts the result in outArtifact
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) = 0;

        /// True if underlying compiler uses file system to communicate source
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() = 0;
};

class DownstreamCompilerBase : public ComBaseObject, public IDownstreamCompiler
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL 

    // ICastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // IDownstreamCompiler
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() SLANG_OVERRIDE { return m_desc; }
    virtual SLANG_NO_THROW bool SLANG_MCALL canConvert(const ArtifactDesc& from, const ArtifactDesc& to) SLANG_OVERRIDE { SLANG_UNUSED(from); SLANG_UNUSED(to); return false; } 
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) SLANG_OVERRIDE;

    DownstreamCompilerBase(const Desc& desc):
        m_desc(desc)
    {
    }
    DownstreamCompilerBase() {}

    void* getInterface(const Guid& guid);
    void* getObject(const Guid& guid);

    Desc m_desc;
};

class CommandLineDownstreamCompiler : public DownstreamCompilerBase
{
public:
    typedef DownstreamCompilerBase Super;

    // IDownstreamCompiler
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, IArtifact** outArtifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return true; }

    // Functions to be implemented for a specific CommandLine

        /// Given options determines the paths to products produced (including the 'moduleFilePath').
        /// Note that does *not* guarentee all products were or should be produced. Just aims to include all that could
        /// be produced, such that can be removed on completion.
    virtual SlangResult calcCompileProducts(const CompileOptions& options, DownstreamProductFlags flags, IFileArtifactRepresentation* lockFile, List<ComPtr<IArtifact>>& outArtifacts) = 0;

    virtual SlangResult calcArgs(const CompileOptions& options, CommandLine& cmdLine) = 0;
    virtual SlangResult parseOutput(const ExecuteResult& exeResult, IArtifactDiagnostics* diagnostics) = 0;

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

/* Only purpose of having base-class here is to make all the DownstreamCompiler types available directly in derived Utils */
struct DownstreamCompilerUtilBase
{
    typedef DownstreamCompileOptions CompileOptions;

    typedef CompileOptions::OptimizationLevel OptimizationLevel;
    typedef CompileOptions::DebugInfoType DebugInfoType;

    typedef CompileOptions::FloatingPointMode FloatingPointMode;

    typedef DownstreamProductFlag ProductFlag;
    typedef DownstreamProductFlags ProductFlags;
};

}

#endif
