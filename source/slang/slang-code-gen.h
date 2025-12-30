// slang-code-gen.h
#pragma once

//
// This file defines the `CodeGenContext` type and related
// utilities. The `CodeGenContext` is used to bundle together
// the information needed by the back-end of the Slang
// compiler, and to help ensure that the back-end is not able
// to access (and thus rely on) information that should only
// be available to the front-end. Maintaining that split
// ensures that results are consistent between end-to-end
// and seaprate-compilation scenarios.
//

#include "slang-entry-point.h"
#include "slang-session.h"
#include "slang-target-program.h"

namespace Slang
{

/// A back-end-specific object to track optional feaures/capabilities/extensions
/// that are discovered to be used by a program/kernel as part of code generation.
class ExtensionTracker : public RefObject
{
    // TODO: The existence of this type is evidence of a design/architecture problem.
    //
    // A better formulation of things requires a few key changes:
    //
    // 1. All optional capabilities need to be enumerated as part of the `CapabilitySet`
    //  system, so that they can be reasoned about uniformly across different targets
    //  and different layers of the compiler.
    //
    // 2. The front-end should be responsible for either or both of:
    //
    //   * Checking that `public` or otherwise externally-visible items (declarations/definitions)
    //     explicitly declare the capabilities they require, and that they only ever
    //     make use of items that are comatible with those required capabilities.
    //
    //   * Inferring the capabilities required by items that are not externally visible,
    //     and attaching those capabilities explicit as a modifier or other synthesized AST node.
    //
    // 3. The capabilities required by a given `ComponentType` and its entry points should be
    // explicitly know-able, and they should be something we can compare to the capabilities
    // of a code generation target *before* back-end code generation is started. We should be
    // able to issue error messages around lacking capabilities in a way the user can understand,
    // in terms of the high-level-language entities.

public:
};

struct RequiredLoweringPassSet
{
    bool debugInfo;
    bool resultType;
    bool optionalType;
    bool enumType;
    bool combinedTextureSamplers;
    bool reinterpret;
    bool generics;
    bool bindExistential;
    bool autodiff;
    bool derivativePyBindWrapper;
    bool bitcast;
    bool existentialTypeLayout;
    bool bindingQuery;
    bool meshOutput;
    bool higherOrderFunc;
    bool globalVaryingVar;
    bool glslSSBO;
    bool byteAddressBuffer;
    bool dynamicResource;
    bool dynamicResourceHeap;
    bool resolveVaryingInputRef;
    bool specializeStageSwitch;
    bool missingReturn;
    bool nonVectorCompositeSelect;
};

/// A context for code generation in the compiler back-end
struct CodeGenContext
{
public:
    typedef List<Index> EntryPointIndices;

    struct Shared
    {
    public:
        Shared(
            TargetProgram* targetProgram,
            EntryPointIndices const& entryPointIndices,
            DiagnosticSink* sink,
            EndToEndCompileRequest* endToEndReq)
            : targetProgram(targetProgram)
            , entryPointIndices(entryPointIndices)
            , sink(sink)
            , endToEndReq(endToEndReq)
        {
        }

        //            Shared(
        //                TargetProgram*              targetProgram,
        //                EndToEndCompileRequest*     endToEndReq);

        TargetProgram* targetProgram = nullptr;
        EntryPointIndices entryPointIndices;
        DiagnosticSink* sink = nullptr;
        EndToEndCompileRequest* endToEndReq = nullptr;
    };

    CodeGenContext(Shared* shared)
        : m_shared(shared)
        , m_targetFormat(shared->targetProgram->getTargetReq()->getTarget())
        , m_targetProfile(shared->targetProgram->getOptionSet().getProfile())
    {
    }

    CodeGenContext(
        CodeGenContext* base,
        CodeGenTarget targetFormat,
        ExtensionTracker* extensionTracker = nullptr)
        : m_shared(base->m_shared)
        , m_targetFormat(targetFormat)
        , m_extensionTracker(extensionTracker)
    {
    }

    /// Get the diagnostic sink
    DiagnosticSink* getSink() { return m_shared->sink; }

    TargetProgram* getTargetProgram() { return m_shared->targetProgram; }

    EntryPointIndices const& getEntryPointIndices() { return m_shared->entryPointIndices; }

    CodeGenTarget getTargetFormat() { return m_targetFormat; }

    ExtensionTracker* getExtensionTracker() { return m_extensionTracker; }

    TargetRequest* getTargetReq() { return getTargetProgram()->getTargetReq(); }

    CapabilitySet getTargetCaps() { return getTargetReq()->getTargetCaps(); }

    CodeGenTarget getFinalTargetFormat() { return getTargetReq()->getTarget(); }

    ComponentType* getProgram() { return getTargetProgram()->getProgram(); }

    Linkage* getLinkage() { return getProgram()->getLinkage(); }

    Session* getSession() { return getLinkage()->getSessionImpl(); }

    /// Get the source manager
    SourceManager* getSourceManager() { return getLinkage()->getSourceManager(); }

    ISlangFileSystemExt* getFileSystemExt() { return getLinkage()->getFileSystemExt(); }

    EndToEndCompileRequest* isEndToEndCompile() { return m_shared->endToEndReq; }

    EndToEndCompileRequest* isPassThroughEnabled();

    Count getEntryPointCount() { return getEntryPointIndices().getCount(); }

    EntryPoint* getEntryPoint(Index index) { return getProgram()->getEntryPoint(index); }

    Index getSingleEntryPointIndex()
    {
        SLANG_ASSERT(getEntryPointCount() == 1);
        return getEntryPointIndices()[0];
    }

    //

    IRDumpOptions getIRDumpOptions();

    bool shouldValidateIR();
    bool shouldDumpIR();
    bool shouldReportCheckpointIntermediates();
    bool shouldReportDynamicDispatchSites();

    bool shouldTrackLiveness();

    bool shouldDumpIntermediates();
    String getIntermediateDumpPrefix();

    bool getUseUnknownImageFormatAsDefault();

    bool isSpecializationDisabled();

    bool shouldSkipSPIRVValidation();

    SlangResult requireTranslationUnitSourceFiles();

    //

    SlangResult emitEntryPoints(ComPtr<IArtifact>& outArtifact);

    SlangResult emitPrecompiledDownstreamIR(ComPtr<IArtifact>& outArtifact);

    void maybeDumpIntermediate(IArtifact* artifact);

    // Used to cause instructions available in precompiled blobs to be
    // removed between IR linking and target source generation.
    bool removeAvailableInDownstreamIR = false;

    // Determines if program level compilation like getTargetCode() or getEntryPointCode()
    // should return a fully linked downstream program or just the glue SPIR-V/DXIL that
    // imports and uses the precompiled SPIR-V/DXIL from constituent modules.
    // This is a no-op if modules are not precompiled.
    bool shouldSkipDownstreamLinking();

    RequiredLoweringPassSet& getRequiredLoweringPassSet() { return m_requiredLoweringPassSet; }

protected:
    CodeGenTarget m_targetFormat = CodeGenTarget::Unknown;
    Profile m_targetProfile;
    ExtensionTracker* m_extensionTracker = nullptr;

    // To improve the performance of our backend, we will try to avoid running
    // passes related to features not used in the user code.
    // To do so, we will scan the IR module once, and determine which passes are needed
    // based on the instructions used in the IR module.
    // This will allow us to skip running passes that are not needed, without having to
    // run all the passes only to find out that no work is needed.
    // This is especially important for the performance of the backend, as some passes
    // have an initialization cost (such as building reference graphs or DOM trees) that
    // can be expensive.
    RequiredLoweringPassSet m_requiredLoweringPassSet;

    /// Will output assembly as well as the artifact if appropriate for the artifact type for
    /// assembly output and conversion is possible
    void _dumpIntermediateMaybeWithAssembly(IArtifact* artifact);

    void _dumpIntermediate(IArtifact* artifact);
    void _dumpIntermediate(const ArtifactDesc& desc, void const* data, size_t size);

    /* Emits entry point source taking into account if a pass-through or not. Uses 'targetFormat' to
    determine the target (not targetReq) */
    SlangResult emitEntryPointsSource(ComPtr<IArtifact>& outArtifact);

    SlangResult emitEntryPointsSourceFromIR(ComPtr<IArtifact>& outArtifact);

    SlangResult emitWithDownstreamForEntryPoints(ComPtr<IArtifact>& outArtifact);

    /* Determines a suitable filename to identify the input for a given entry point being compiled.
    If the end-to-end compile is a pass-through case, will attempt to find the (unique) source file
    pathname for the translation unit containing the entry point at `entryPointIndex.
    If the compilation is not in a pass-through case, then always returns `"slang-generated"`.
    @param endToEndReq The end-to-end compile request which might be using pass-through compilation
    @param entryPointIndex The index of the entry point to compute a filename for.
    @return the appropriate source filename */
    String calcSourcePathForEntryPoints();

    TranslationUnitRequest* findPassThroughTranslationUnit(Int entryPointIndex);


    SlangResult _emitEntryPoints(ComPtr<IArtifact>& outArtifact);

private:
    Shared* m_shared = nullptr;
};

// TODO: The "artifact" system is a scourge.
IArtifact* getSeparateDbgArtifact(IArtifact* artifact);

} // namespace Slang
