// slang-end-to-end-request.h
#pragma once

//
// This file provides the `EndToEndCompileRequest` type and
// related utilities.
//
// The primary purpose of `EndToEndCompileRequest` is to
// implement the overall flow of compilation for the
// `slangc` command-line tool. Command-line compiles need
// to deal with various details that aren't relevant to
// API-based compiles (e.g., writing output to files),
// and also need to implement a lot of complicated
// "do what I mean" policy that is expected by users of
// `slangc` but that can be dangerously implicit when
// that policy is enshrined in an API.
//
// In addition to serving the needs of `slangc`, the
// `EndToEndCompileRequest` type also implements the
// deprecated `slang::ICompileRequest` interface from
// the public API.
//

#include "../compiler-core/slang-source-embed-util.h"
#include "../core/slang-file-system.h"
#include "slang-compile-request.h"

namespace Slang
{

enum class ContainerFormat : SlangContainerFormatIntegral
{
    None = SLANG_CONTAINER_FORMAT_NONE,
    SlangModule = SLANG_CONTAINER_FORMAT_SLANG_MODULE,
};

// TODO: everything related to `StdWriters` should be removed.
enum class WriterChannel : SlangWriterChannelIntegral
{
    Diagnostic = SLANG_WRITER_CHANNEL_DIAGNOSTIC,
    StdOutput = SLANG_WRITER_CHANNEL_STD_OUTPUT,
    StdError = SLANG_WRITER_CHANNEL_STD_ERROR,
    CountOf = SLANG_WRITER_CHANNEL_COUNT_OF,
};

// TODO: everything related to `StdWriters` should be removed.
enum class WriterMode : SlangWriterModeIntegral
{
    Text = SLANG_WRITER_MODE_TEXT,
    Binary = SLANG_WRITER_MODE_BINARY,
};

/// A compile request that spans the front and back ends of the compiler
///
/// This is what the command-line `slangc` uses, as well as the legacy
/// C API. It ties together the functionality of `Linkage`,
/// `FrontEndCompileRequest`, and `BackEndCompileRequest`, plus a small
/// number of additional features that primarily make sense for
/// command-line usage.
///
class EndToEndCompileRequest : public RefObject, public slang::ICompileRequest
{
public:
    SLANG_CLASS_GUID(0xce6d2383, 0xee1b, 0x4fd7, {0xa0, 0xf, 0xb8, 0xb6, 0x33, 0x12, 0x95, 0xc8})

    // ISlangUnknown
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject)
        SLANG_OVERRIDE;
    SLANG_REF_OBJECT_IUNKNOWN_ADD_REF
    SLANG_REF_OBJECT_IUNKNOWN_RELEASE

    // slang::ICompileRequest
    virtual SLANG_NO_THROW void SLANG_MCALL setFileSystem(ISlangFileSystem* fileSystem)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setCompileFlags(SlangCompileFlags flags) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangCompileFlags SLANG_MCALL getCompileFlags() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setDumpIntermediates(int enable) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setDumpIntermediatePrefix(const char* prefix)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setEnableEffectAnnotations(bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setLineDirectiveMode(SlangLineDirectiveMode mode)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setCodeGenTarget(SlangCompileTarget target)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW int SLANG_MCALL addCodeGenTarget(SlangCompileTarget target)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetProfile(int targetIndex, SlangProfileID profile) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setTargetFlags(int targetIndex, SlangTargetFlags flags)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetFloatingPointMode(int targetIndex, SlangFloatingPointMode mode) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetMatrixLayoutMode(int targetIndex, SlangMatrixLayoutMode mode) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetForceGLSLScalarBufferLayout(int targetIndex, bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setTargetForceDXLayout(int targetIndex, bool value)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetGenerateWholeProgram(int targetIndex, bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setTargetEmbedDownstreamIR(int targetIndex, bool value)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setTargetForceCLayout(int targetIndex, bool value)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setMatrixLayoutMode(SlangMatrixLayoutMode mode)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setDebugInfoLevel(SlangDebugInfoLevel level)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setOptimizationLevel(SlangOptimizationLevel level)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setOutputContainerFormat(SlangContainerFormat format)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setPassThrough(SlangPassThrough passThrough)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setDiagnosticCallback(SlangDiagnosticCallback callback, void const* userData) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setWriter(SlangWriterChannel channel, ISlangWriter* writer) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ISlangWriter* SLANG_MCALL getWriter(SlangWriterChannel channel)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addSearchPath(const char* searchDir) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    addPreprocessorDefine(const char* key, const char* value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    processCommandLineArguments(char const* const* args, int argCount) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW int SLANG_MCALL
    addTranslationUnit(SlangSourceLanguage language, char const* name) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setDefaultModuleName(const char* defaultModuleName)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitPreprocessorDefine(
        int translationUnitIndex,
        const char* key,
        const char* value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    addTranslationUnitSourceFile(int translationUnitIndex, char const* path) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceString(
        int translationUnitIndex,
        char const* path,
        char const* source) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL addLibraryReference(
        const char* basePath,
        const void* libData,
        size_t libDataSize) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceStringSpan(
        int translationUnitIndex,
        char const* path,
        char const* sourceBegin,
        char const* sourceEnd) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceBlob(
        int translationUnitIndex,
        char const* path,
        ISlangBlob* sourceBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW int SLANG_MCALL
    addEntryPoint(int translationUnitIndex, char const* name, SlangStage stage) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW int SLANG_MCALL addEntryPointEx(
        int translationUnitIndex,
        char const* name,
        SlangStage stage,
        int genericArgCount,
        char const** genericArgs) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setGlobalGenericArgs(int genericArgCount, char const** genericArgs) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setTypeNameForGlobalExistentialTypeParam(int slotIndex, char const* typeName) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL setTypeNameForEntryPointExistentialTypeParam(
        int entryPointIndex,
        int slotIndex,
        char const* typeName) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setAllowGLSLInput(bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW char const* SLANG_MCALL getDiagnosticOutput() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getDiagnosticOutputBlob(ISlangBlob** outBlob)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW int SLANG_MCALL getDependencyFileCount() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW char const* SLANG_MCALL getDependencyFilePath(int index) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW int SLANG_MCALL getTranslationUnitCount() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW char const* SLANG_MCALL getEntryPointSource(int entryPointIndex)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void const* SLANG_MCALL
    getEntryPointCode(int entryPointIndex, size_t* outSize) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCodeBlob(
        int entryPointIndex,
        int targetIndex,
        ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCodeBlob(int targetIndex, ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetHostCallable(int targetIndex, ISlangSharedLibrary** outSharedLibrary) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void const* SLANG_MCALL getCompileRequestCode(size_t* outSize)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL
    getCompileRequestResultAsFileSystem() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getContainerCode(ISlangBlob** outBlob)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadRepro(ISlangFileSystem* fileSystem, const void* data, size_t size) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveRepro(ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL enableReproCapture() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getProgram(slang::IComponentType** outProgram)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPoint(SlangInt entryPointIndex, slang::IComponentType** outEntryPoint) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getModule(SlangInt translationUnitIndex, slang::IModule** outModule) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getSession(slang::ISession** outSession)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangReflection* SLANG_MCALL getReflection() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setCommandLineCompilerMode() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    addTargetCapability(SlangInt targetIndex, SlangCapabilityID capability) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getProgramWithEntryPoints(slang::IComponentType** outProgram) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL isParameterLocationUsed(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        SlangParameterCategory category,
        SlangUInt spaceIndex,
        SlangUInt registerIndex,
        bool& outUsed) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetLineDirectiveMode(SlangInt targetIndex, SlangLineDirectiveMode mode) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    overrideDiagnosticSeverity(SlangInt messageID, SlangSeverity overrideSeverity) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangDiagnosticFlags SLANG_MCALL getDiagnosticFlags() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setDiagnosticFlags(SlangDiagnosticFlags flags)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setDebugInfoFormat(SlangDebugInfoFormat format)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setReportDownstreamTime(bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setReportPerfBenchmark(bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setSkipSPIRVValidation(bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setTargetUseMinimumSlangOptimization(int targetIndex, bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setIgnoreCapabilityCheck(bool value) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getCompileTimeProfile(ISlangProfiler** compileTimeProfile, bool isClear) SLANG_OVERRIDE;

    void setTrackLiveness(bool v);

    EndToEndCompileRequest(Session* session);

    EndToEndCompileRequest(Linkage* linkage);

    ~EndToEndCompileRequest();

    // If enabled will emit IR
    bool m_emitIr = false;

    // What container format are we being asked to generate?
    // If it's set to a format, the container blob will be calculated during compile
    ContainerFormat m_containerFormat = ContainerFormat::None;

    /// Where the container is stored. This is calculated as part of compile if m_containerFormat is
    /// set to a supported format.
    ComPtr<IArtifact> m_containerArtifact;
    /// Holds the container as a file system
    ComPtr<ISlangMutableFileSystem> m_containerFileSystem;

    /// File system used by repro system if a file couldn't be found within the repro (or associated
    /// directory)
    ComPtr<ISlangFileSystem> m_reproFallbackFileSystem =
        ComPtr<ISlangFileSystem>(OSFileSystem::getExtSingleton());

    // Path to output container to
    String m_containerOutputPath;

    // Should we just pass the input to another compiler?
    PassThroughMode m_passThrough = PassThroughMode::None;

    /// If output should be source embedded, define the style of the embedding
    SourceEmbedUtil::Style m_sourceEmbedStyle = SourceEmbedUtil::Style::None;
    /// The language to be used for source embedding
    SourceLanguage m_sourceEmbedLanguage = SourceLanguage::C;
    /// Source embed variable name. Note may be used as a basis for names if multiple items written
    String m_sourceEmbedName;

    /// Source code for the specialization arguments to use for the global specialization parameters
    /// of the program.
    List<String> m_globalSpecializationArgStrings;

    // Are we being driven by the command-line `slangc`, and should act accordingly?
    bool m_isCommandLineCompile = false;

    String m_diagnosticOutput;

    /// A blob holding the diagnostic output
    ComPtr<ISlangBlob> m_diagnosticOutputBlob;

    /// Per-entry-point information not tracked by other compile requests
    class EntryPointInfo : public RefObject
    {
    public:
        /// Source code for the specialization arguments to use for the specialization parameters of
        /// the entry point.
        List<String> specializationArgStrings;
    };
    List<EntryPointInfo> m_entryPoints;

    /// Per-target information only needed for command-line compiles
    class TargetInfo : public RefObject
    {
    public:
        // Requested output paths for each entry point.
        // An empty string indices no output desired for
        // the given entry point.
        Dictionary<Int, String> entryPointOutputPaths;
        String wholeTargetOutputPath;
        CompilerOptionSet targetOptions;
    };
    Dictionary<TargetRequest*, RefPtr<TargetInfo>> m_targetInfos;

    CompilerOptionSet m_optionSetForDefaultTarget;

    CompilerOptionSet& getTargetOptionSet(TargetRequest* req);

    CompilerOptionSet& getTargetOptionSet(Index targetIndex);

    String m_dependencyOutputPath;

    /// Writes the modules in a container to the stream
    SlangResult writeContainerToStream(Stream* stream);

    /// If a container format has been specified produce a container (stored in m_containerBlob)
    SlangResult maybeCreateContainer();
    /// If a container has been constructed and the filename/path has contents will try to write
    /// the container contents to the file
    SlangResult maybeWriteContainer(const String& fileName);

    Linkage* getLinkage() { return m_linkage; }

    int addEntryPoint(
        int translationUnitIndex,
        String const& name,
        Profile profile,
        List<String> const& genericTypeNames);

    void setWriter(WriterChannel chan, ISlangWriter* writer);
    ISlangWriter* getWriter(WriterChannel chan) const
    {
        return m_writers->getWriter(SlangWriterChannel(chan));
    }

    /// The end to end request can be passed as nullptr, if not driven by one
    SlangResult executeActionsInner();
    SlangResult executeActions();

    Session* getSession() { return m_session; }
    DiagnosticSink* getSink() { return &m_sink; }
    NamePool* getNamePool() { return getLinkage()->getNamePool(); }

    FrontEndCompileRequest* getFrontEndReq() { return m_frontEndReq; }

    ComponentType* getUnspecializedGlobalComponentType()
    {
        return getFrontEndReq()->getGlobalComponentType();
    }
    ComponentType* getUnspecializedGlobalAndEntryPointsComponentType()
    {
        return getFrontEndReq()->getGlobalAndEntryPointsComponentType();
    }

    ComponentType* getSpecializedGlobalComponentType() { return m_specializedGlobalComponentType; }
    ComponentType* getSpecializedGlobalAndEntryPointsComponentType()
    {
        return m_specializedGlobalAndEntryPointsComponentType;
    }

    ComponentType* getSpecializedEntryPointComponentType(Index index)
    {
        return m_specializedEntryPoints[index];
    }

    void writeArtifactToStandardOutput(IArtifact* artifact, DiagnosticSink* sink);

    void generateOutput();

    CompilerOptionSet& getOptionSet() { return m_linkage->m_optionSet; }

private:
    String _getWholeProgramPath(TargetRequest* targetReq);
    String _getEntryPointPath(TargetRequest* targetReq, Index entryPointIndex);

    /// Maybe write the artifact to the path (if set), or stdout (if there is no container or path)
    SlangResult _maybeWriteArtifact(const String& path, IArtifact* artifact);
    SlangResult _maybeWriteDebugArtifact(
        TargetProgram* targetProgram,
        const String& path,
        IArtifact* artifact);
    SlangResult _writeArtifact(const String& path, IArtifact* artifact);

    /// Adds any extra settings to complete a targetRequest
    void _completeTargetRequest(UInt targetIndex);

    ISlangUnknown* getInterface(const Guid& guid);

    void generateOutput(ComponentType* program);
    void generateOutput(TargetProgram* targetProgram);

    void init();

    Session* m_session = nullptr;
    RefPtr<Linkage> m_linkage;
    DiagnosticSink m_sink;
    RefPtr<FrontEndCompileRequest> m_frontEndReq;
    RefPtr<ComponentType> m_specializedGlobalComponentType;
    RefPtr<ComponentType> m_specializedGlobalAndEntryPointsComponentType;
    List<RefPtr<ComponentType>> m_specializedEntryPoints;

    // For output

    RefPtr<StdWriters> m_writers;
};

} // namespace Slang
