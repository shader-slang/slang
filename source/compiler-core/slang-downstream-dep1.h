#ifndef SLANG_DOWNSTREAM_DEP1_H
#define SLANG_DOWNSTREAM_DEP1_H


#include "slang-downstream-compiler.h"

namespace Slang
{

// (DEPRECIATED)


struct SemanticVersion_Dep1
{
    uint32_t m_major;
    uint16_t m_minor;
    uint16_t m_patch; 
};

struct DownstreamCompileOptions_Dep1
{
    typedef uint32_t Flags;
    enum class SomeEnum { First, B, C };

    struct Define
    {
        String nameWithSig;             ///< If macro takes parameters include in brackets
        String value;
    };

    struct CapabilityVersion
    {
        SomeEnum kind;
        SemanticVersion_Dep1 version;
    };

    SomeEnum optimizationLevel = SomeEnum::First;
    SomeEnum debugInfoType = SomeEnum::First;
    SlangCompileTarget targetType = SLANG_HOST_EXECUTABLE;
    SlangSourceLanguage sourceLanguage = SLANG_SOURCE_LANGUAGE_CPP;
    SomeEnum floatingPointMode = SomeEnum::First;
    SomeEnum pipelineType = SomeEnum::First;
    SlangMatrixLayoutMode matrixLayout = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;

    Flags flags = 0;

    SomeEnum platform = SomeEnum::First;

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

// Compiler description
struct DownstreamCompilerDesc_Dep1
{
    SlangPassThrough type;      ///< The type of the compiler

    /// TODO(JS): Would probably be better if changed to SemanticVersion, but not trivial to change
    // because this type is part of the DownstreamCompiler interface, which is used with `slang-llvm`.
    Int majorVersion;           ///< Major version (interpretation is type specific)
    Int minorVersion;           ///< Minor version (interpretation is type specific)
};

struct DownstreamDiagnostic_Dep1
{
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

    Severity severity = Severity::Unknown;          ///< The severity of error
    Stage stage = Stage::Compile;                   ///< The stage the error came from
    String text;                                    ///< The text of the error
    String code;                                    ///< The compiler specific error code
    String filePath;                                ///< The path the error originated from
    Int fileLine = 0;                               ///< The line number the error came from
};

struct DownstreamDiagnostics_Dep1
{
    typedef DownstreamDiagnostic_Dep1 Diagnostic;

    String rawDiagnostics;

    SlangResult result = SLANG_OK;
    List<Diagnostic> diagnostics;
};

class DownstreamCompileResult_Dep1 : public RefObject
{
public:
    SLANG_CLASS_GUID(0xdfc5d318, 0x8675, 0x40ef, { 0xbd, 0x7b, 0x4, 0xa4, 0xff, 0x66, 0x11, 0x30 })

    virtual SlangResult getHostCallableSharedLibrary(ComPtr<ISlangSharedLibrary>& outLibrary) { SLANG_UNUSED(outLibrary); return SLANG_FAIL; }
    virtual SlangResult getBinary(ComPtr<ISlangBlob>& outBlob) { SLANG_UNUSED(outBlob); return SLANG_FAIL; }

    const DownstreamDiagnostics_Dep1& getDiagnostics() const { return m_diagnostics; }

    /// Ctor
    DownstreamCompileResult_Dep1(const DownstreamDiagnostics_Dep1& diagnostics) :
        m_diagnostics(diagnostics)
    {}

protected:
    DownstreamDiagnostics_Dep1 m_diagnostics;
};

class DownstreamCompiler_Dep1: public RefObject
{
public:
    typedef RefObject Super;

        /// Get the desc of this compiler
    const DownstreamCompilerDesc_Dep1& getDesc() const { return m_desc;  }
        /// Compile using the specified options. The result is in resOut
    virtual SlangResult compile(const DownstreamCompileOptions_Dep1& options, RefPtr<DownstreamCompileResult_Dep1>& outResult) = 0;
        /// Some compilers have support converting a binary blob into disassembly. Output disassembly is held in the output blob
    virtual SlangResult disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out);

        /// True if underlying compiler uses file system to communicate source
    virtual bool isFileBased() = 0;

protected:

    DownstreamCompilerDesc_Dep1 m_desc;
};

class DownstreamCompilerAdapter_Dep1 : public DownstreamCompilerBase
{
public:
    // IDownstreamCompiler
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() SLANG_OVERRIDE { return m_desc; }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, IArtifact** outArtifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL canConvert(const ArtifactDesc& from, const ArtifactDesc& to) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return m_dep->isFileBased(); }

    DownstreamCompilerAdapter_Dep1(DownstreamCompiler_Dep1* dep, ArtifactPayload disassemblyPayload);

protected:

    DownstreamCompilerDesc m_desc;

    ArtifactPayload m_disassemblyPayload;
    RefPtr<DownstreamCompiler_Dep1> m_dep;
};

struct DownstreamUtil_Dep1
{
    static SlangResult getDownstreamSharedLibrary(DownstreamCompileResult_Dep1* downstreamResult, ComPtr<ISlangSharedLibrary>& outSharedLibrary);
};

}

#endif
