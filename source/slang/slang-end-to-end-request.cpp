// slang-end-to-end-request.cpp
#include "slang-end-to-end-request.h"

#include "compiler-core/slang-pretty-writer.h"
#include "core/slang-memory-file-system.h"
#include "core/slang-performance-profiler.h"
#include "slang-check-impl.h"
#include "slang-compiler.h"
#include "slang-emit-dependency-file.h"
#include "slang-module-library.h"
#include "slang-options.h"
#include "slang-reflection-json.h"
#include "slang-repro.h"
#include "slang-serialize-container.h"

// TODO: The "artifact" system is a scourge.
#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-artifact-container-util.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-impl.h"
#include "compiler-core/slang-artifact-util.h"
#include "slang-artifact-output-util.h"

namespace Slang
{

EndToEndCompileRequest::EndToEndCompileRequest(Session* session)
    : m_session(session), m_sink(nullptr, Lexer::sourceLocationLexer)
{
    RefPtr<ASTBuilder> astBuilder(
        new ASTBuilder(session->getASTBuilder(), "EndToEnd::Linkage::astBuilder"));
    m_linkage = new Linkage(session, astBuilder, session->getBuiltinLinkage());
    init();
}

EndToEndCompileRequest::EndToEndCompileRequest(Linkage* linkage)
    : m_session(linkage->getSessionImpl())
    , m_linkage(linkage)
    , m_sink(nullptr, Lexer::sourceLocationLexer)
{
    init();
}

void EndToEndCompileRequest::init()
{
    m_sink.setSourceManager(m_linkage->getSourceManager());

    m_writers = new StdWriters;

    // Set all the default writers
    for (int i = 0; i < int(WriterChannel::CountOf); ++i)
    {
        setWriter(WriterChannel(i), nullptr);
    }

    m_frontEndReq = new FrontEndCompileRequest(getLinkage(), m_writers, getSink());
}

EndToEndCompileRequest::~EndToEndCompileRequest()
{
    // Flush any writers associated with the request
    m_writers->flushWriters();

    m_linkage.setNull();
    m_frontEndReq.setNull();
}

SLANG_NO_THROW SlangResult SLANG_MCALL
EndToEndCompileRequest::queryInterface(SlangUUID const& uuid, void** outObject)
{
    if (uuid == EndToEndCompileRequest::getTypeGuid())
    {
        // Special case to cast directly into internal type
        // NOTE! No addref(!)
        *outObject = this;
        return SLANG_OK;
    }

    if (uuid == ISlangUnknown::getTypeGuid() && uuid == ICompileRequest::getTypeGuid())
    {
        addReference();
        *outObject = static_cast<slang::ICompileRequest*>(this);
        return SLANG_OK;
    }

    return SLANG_E_NO_INTERFACE;
}

// Try to infer a single common source language for a request
static SourceLanguage inferSourceLanguage(FrontEndCompileRequest* request)
{
    SourceLanguage language = SourceLanguage::Unknown;
    for (auto& translationUnit : request->translationUnits)
    {
        // Allow any other language to overide Slang as a choice
        if (language == SourceLanguage::Unknown || language == SourceLanguage::Slang)
        {
            language = translationUnit->sourceLanguage;
        }
        else if (language == translationUnit->sourceLanguage)
        {
            // same language as we currently have, so keep going
        }
        else
        {
            // we found a mismatch, so inference fails
            return SourceLanguage::Unknown;
        }
    }
    return language;
}

SlangResult EndToEndCompileRequest::executeActionsInner()
{
    SLANG_PROFILE_SECTION(endToEndActions);
    // If no code-generation target was specified, then try to infer one from the source language,
    // just to make sure we can do something reasonable when invoked from the command line.
    //
    // TODO: This logic should be moved into `options.cpp` or somewhere else
    // specific to the command-line tool.
    //
    if (getLinkage()->targets.getCount() == 0)
    {
        auto language = inferSourceLanguage(getFrontEndReq());
        switch (language)
        {
        case SourceLanguage::HLSL:
            getLinkage()->addTarget(CodeGenTarget::DXBytecode);
            break;

        case SourceLanguage::GLSL:
            getLinkage()->addTarget(CodeGenTarget::SPIRV);
            break;

        default:
            break;
        }
    }

    // Update compiler settings in target requests.
    for (auto target : getLinkage()->targets)
        target->getOptionSet().inheritFrom(getOptionSet());
    m_frontEndReq->optionSet = getOptionSet();

    // We only do parsing and semantic checking if we *aren't* doing
    // a pass-through compilation.
    //
    if (m_passThrough == PassThroughMode::None)
    {
        SLANG_RETURN_ON_FAIL(getFrontEndReq()->executeActionsInner());
    }

    if (getOptionSet().getBoolOption(CompilerOptionName::PreprocessorOutput))
    {
        return SLANG_OK;
    }

    // If command line specifies to skip codegen, we exit here.
    // Note: this is a debugging option.
    //
    if (getOptionSet().getBoolOption(CompilerOptionName::SkipCodeGen))
    {
        // We will use the program (and matching layout information)
        // that was computed in the front-end for all subsequent
        // reflection queries, etc.
        //
        m_specializedGlobalComponentType = getUnspecializedGlobalComponentType();
        m_specializedGlobalAndEntryPointsComponentType =
            getUnspecializedGlobalAndEntryPointsComponentType();
        m_specializedEntryPoints = getFrontEndReq()->getUnspecializedEntryPoints();

        SLANG_RETURN_ON_FAIL(maybeCreateContainer());

        SLANG_RETURN_ON_FAIL(maybeWriteContainer(m_containerOutputPath));

        return SLANG_OK;
    }

    // If requested, attempt to compile the translation unit all the way down to the target
    // language(s) and stash the result blobs in IR.
    for (auto target : getLinkage()->targets)
    {
        SlangCompileTarget targetEnum = SlangCompileTarget(target->getTarget());
        if (target->getOptionSet().getBoolOption(CompilerOptionName::EmbedDownstreamIR))
        {
            auto frontEndReq = getFrontEndReq();

            for (auto translationUnit : frontEndReq->translationUnits)
            {
                SLANG_RETURN_ON_FAIL(
                    translationUnit->getModule()->precompileForTarget(targetEnum, nullptr));

                if (frontEndReq->optionSet.shouldDumpIR())
                {
                    DiagnosticSinkWriter writer(frontEndReq->getSink());

                    dumpIR(
                        translationUnit->getModule()->getIRModule(),
                        frontEndReq->m_irDumpOptions,
                        "PRECOMPILE_FOR_TARGET_COMPLETE_ALL",
                        frontEndReq->getSourceManager(),
                        &writer);

                    dumpIR(
                        translationUnit->getModule()->getIRModule()->getModuleInst(),
                        frontEndReq->m_irDumpOptions,
                        frontEndReq->getSourceManager(),
                        &writer);
                }
            }
        }
    }

    // If codegen is enabled, we need to move along to
    // apply any generic specialization that the user asked for.
    //
    if (m_passThrough == PassThroughMode::None)
    {
        m_specializedGlobalComponentType = createSpecializedGlobalComponentType(this);
        if (getSink()->getErrorCount() != 0)
            return SLANG_FAIL;

        m_specializedGlobalAndEntryPointsComponentType =
            createSpecializedGlobalAndEntryPointsComponentType(this, m_specializedEntryPoints);
        if (getSink()->getErrorCount() != 0)
            return SLANG_FAIL;

        // For each code generation target, we will generate specialized
        // parameter binding information (taking global generic
        // arguments into account at this time).
        //
        for (auto targetReq : getLinkage()->targets)
        {
            auto targetProgram =
                m_specializedGlobalAndEntryPointsComponentType->getTargetProgram(targetReq);
            targetProgram->getOrCreateLayout(getSink());
        }
        if (getSink()->getErrorCount() != 0)
            return SLANG_FAIL;
    }
    else
    {
        // We need to create dummy `EntryPoint` objects
        // to make sure that the logic in `generateOutput`
        // sees something worth processing.
        //
        List<RefPtr<ComponentType>> dummyEntryPoints;
        for (auto entryPointReq : getFrontEndReq()->getEntryPointReqs())
        {
            RefPtr<EntryPoint> dummyEntryPoint = EntryPoint::createDummyForPassThrough(
                getLinkage(),
                entryPointReq->getName(),
                entryPointReq->getProfile());

            dummyEntryPoints.add(dummyEntryPoint);
        }

        RefPtr<ComponentType> composedProgram =
            CompositeComponentType::create(getLinkage(), dummyEntryPoints);

        m_specializedGlobalComponentType = getUnspecializedGlobalComponentType();
        m_specializedGlobalAndEntryPointsComponentType = composedProgram;
        m_specializedEntryPoints = getFrontEndReq()->getUnspecializedEntryPoints();
    }

    // Generate output code, in whatever format was requested
    generateOutput();
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Act as expected of the API-based compiler
SlangResult EndToEndCompileRequest::executeActions()
{
    SlangResult res = executeActionsInner();

    m_diagnosticOutput = getSink()->outputBuffer.produceString();
    return res;
}


static ISlangWriter* _getDefaultWriter(WriterChannel chan)
{
    static FileWriter stdOut(stdout, WriterFlag::IsStatic | WriterFlag::IsUnowned);
    static FileWriter stdError(stderr, WriterFlag::IsStatic | WriterFlag::IsUnowned);
    static NullWriter nullWriter(WriterFlag::IsStatic | WriterFlag::IsConsole);

    switch (chan)
    {
    case WriterChannel::StdError:
        return &stdError;
    case WriterChannel::StdOutput:
        return &stdOut;
    case WriterChannel::Diagnostic:
        return &nullWriter;
    default:
        {
            SLANG_ASSERT(!"Unknown type");
            return &stdError;
        }
    }
}

void EndToEndCompileRequest::setWriter(WriterChannel chan, ISlangWriter* writer)
{
    // If the user passed in null, we will use the default writer on that channel
    m_writers->setWriter(SlangWriterChannel(chan), writer ? writer : _getDefaultWriter(chan));

    // For diagnostic output, if the user passes in nullptr, we set on m_sink.writer as that enables
    // buffering on DiagnosticSink
    if (chan == WriterChannel::Diagnostic)
    {
        m_sink.writer = writer;
    }
}

void EndToEndCompileRequest::writeArtifactToStandardOutput(
    IArtifact* artifact,
    DiagnosticSink* sink)
{
    // If it's host callable it's not available to write to output
    if (isDerivedFrom(artifact->getDesc().kind, ArtifactKind::HostCallable))
    {
        return;
    }

    auto session = getSession();
    ArtifactOutputUtil::maybeConvertAndWrite(
        session,
        artifact,
        sink,
        toSlice("stdout"),
        getWriter(WriterChannel::StdOutput));
}

String EndToEndCompileRequest::_getWholeProgramPath(TargetRequest* targetReq)
{
    RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
    if (m_targetInfos.tryGetValue(targetReq, targetInfo))
    {
        return targetInfo->wholeTargetOutputPath;
    }
    return String();
}

String EndToEndCompileRequest::_getEntryPointPath(TargetRequest* targetReq, Index entryPointIndex)
{
    // It is possible that we are dynamically discovering entry
    // points (using `[shader(...)]` attributes), so that there
    // might be entry points added to the program that did not
    // get paths specified via command-line options.
    //
    RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
    if (m_targetInfos.tryGetValue(targetReq, targetInfo))
    {
        String outputPath;
        if (targetInfo->entryPointOutputPaths.tryGetValue(entryPointIndex, outputPath))
        {
            return outputPath;
        }
    }

    return String();
}

SlangResult EndToEndCompileRequest::_writeArtifact(const String& path, IArtifact* artifact)
{
    if (path.getLength() > 0)
    {
        SLANG_RETURN_ON_FAIL(ArtifactOutputUtil::writeToFile(artifact, getSink(), path));
    }
    else if (m_containerFormat == ContainerFormat::None)
    {
        // If we aren't writing to a container and we didn't write to a file, we can output to
        // standard output
        writeArtifactToStandardOutput(artifact, getSink());
    }
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::_maybeWriteArtifact(const String& path, IArtifact* artifact)
{
    // We don't have to do anything if there is no artifact
    if (!artifact)
    {
        return SLANG_OK;
    }

    // If embedding is enabled...
    if (m_sourceEmbedStyle != SourceEmbedUtil::Style::None)
    {
        SourceEmbedUtil::Options options;

        options.style = m_sourceEmbedStyle;
        options.variableName = m_sourceEmbedName;
        options.language = (SlangSourceLanguage)m_sourceEmbedLanguage;

        ComPtr<IArtifact> embeddedArtifact;
        SLANG_RETURN_ON_FAIL(SourceEmbedUtil::createEmbedded(artifact, options, embeddedArtifact));

        if (!embeddedArtifact)
        {
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(
            _writeArtifact(SourceEmbedUtil::getPath(path, options), embeddedArtifact));
        return SLANG_OK;
    }
    else
    {
        SLANG_RETURN_ON_FAIL(_writeArtifact(path, artifact));
    }

    return SLANG_OK;
}

// These helper functions are used by the -separate-debug-info command line
// arg to extract the associated artifact containing the debug SPIRV data
// and save it to a file with a .dbg.spv extension.
static String _getDebugSpvPath(const String& basePath)
{
    // Find the last occurrence of ".spv" at the end of the string.
    static const char ext[] = ".spv";
    static const char dbgExt[] = ".dbg.spv";
    Index extLen = 4;
    if (basePath.getLength() >= extLen && basePath.endsWith(ext))
    {
        // Replace the ".spv" extension with ".dbg.spv"
        String prefix = String(basePath.subString(0, basePath.getLength() - extLen));
        return prefix + dbgExt;
    }
    // If it doesn't end with .spv, just append .dbg.spv
    return basePath + dbgExt;
}

SlangResult EndToEndCompileRequest::_maybeWriteDebugArtifact(
    TargetProgram* targetProgram,
    const String& path,
    IArtifact* artifact)
{
    if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::EmitSeparateDebug))
    {
        const auto dbgArtifact = getSeparateDbgArtifact(artifact);
        // Check if a debug artifact was actually created (only for SPIR-V targets)
        if (dbgArtifact)
        {
            // The artifact's name may have been set to the debug build id hash, use
            // it as the filename if it exists.
            String dbgPath = dbgArtifact->getName();
            if (dbgPath.getLength() == 0)
                dbgPath = _getDebugSpvPath(path);
            else
                dbgPath.append(".dbg.spv");
            return _maybeWriteArtifact(dbgPath, dbgArtifact);
        }
        // If no debug artifact exists (e.g., for non-SPIR-V targets), just silently succeed
        // The warning about unsupported targets is already issued during option parsing
    }

    return SLANG_OK;
}

void EndToEndCompileRequest::generateOutput(TargetProgram* targetProgram)
{
    auto program = targetProgram->getProgram();

    // Generate target code any entry points that
    // have been requested for compilation.
    auto entryPointCount = program->getEntryPointCount();
    if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::GenerateWholeProgram))
    {
        targetProgram->_createWholeProgramResult(getSink(), this);
    }
    else
    {
        for (Index ii = 0; ii < entryPointCount; ++ii)
        {
            targetProgram->_createEntryPointResult(ii, getSink(), this);
        }
    }
}

bool _shouldWriteSourceLocs(Linkage* linkage)
{
    // If debug information or source manager are not avaiable we can't/shouldn't write out locs
    if (linkage->m_optionSet.getEnumOption<DebugInfoLevel>(CompilerOptionName::DebugInformation) ==
            DebugInfoLevel::None ||
        linkage->getSourceManager() == nullptr)
    {
        return false;
    }

    // Otherwise we do want to write out the locs
    return true;
}

SlangResult EndToEndCompileRequest::writeContainerToStream(Stream* stream)
{
    auto linkage = getLinkage();

    // Set up options
    SerialContainerUtil::WriteOptions options;

    // If debug information is enabled, enable writing out source locs
    if (_shouldWriteSourceLocs(linkage))
    {
        options.sourceManagerToUseWhenSerializingSourceLocs = linkage->getSourceManager();
    }

    SLANG_RETURN_ON_FAIL(SerialContainerUtil::write(this, options, stream));

    return SLANG_OK;
}

static IBoxValue<SourceMap>* _getObfuscatedSourceMap(TranslationUnitRequest* translationUnit)
{
    if (auto module = translationUnit->getModule())
    {
        if (auto irModule = module->getIRModule())
        {
            return irModule->getObfuscatedSourceMap();
        }
    }
    return nullptr;
}

SlangResult EndToEndCompileRequest::maybeCreateContainer()
{
    m_containerArtifact.setNull();

    List<ComPtr<IArtifact>> artifacts;

    auto linkage = getLinkage();

    auto program = getSpecializedGlobalAndEntryPointsComponentType();

    for (auto targetReq : linkage->targets)
    {
        auto targetProgram = program->getTargetProgram(targetReq);

        if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::GenerateWholeProgram))
        {
            if (auto artifact = targetProgram->getExistingWholeProgramResult())
            {
                if (!targetProgram->getOptionSet().getBoolOption(
                        CompilerOptionName::EmbedDownstreamIR))
                {
                    artifacts.add(ComPtr<IArtifact>(artifact));
                }
            }
        }
        else
        {
            Index entryPointCount = program->getEntryPointCount();
            for (Index ee = 0; ee < entryPointCount; ++ee)
            {
                if (auto artifact = targetProgram->getExistingEntryPointResult(ee))
                {
                    artifacts.add(ComPtr<IArtifact>(artifact));
                }
            }
        }
    }

    // If IR emitting is enabled, add IR to the artifacts
    if (m_emitIr && (m_containerFormat == ContainerFormat::SlangModule))
    {
        OwnedMemoryStream stream(FileAccess::Write);
        SlangResult res = writeContainerToStream(&stream);
        if (SLANG_FAILED(res))
        {
            getSink()->diagnose(SourceLoc(), Diagnostics::unableToCreateModuleContainer);
            return res;
        }

        // Need to turn into a blob
        List<uint8_t> blobData;
        stream.swapContents(blobData);

        auto containerBlob = ListBlob::moveCreate(blobData);

        auto irArtifact = Artifact::create(ArtifactDesc::make(
            Artifact::Kind::CompileBinary,
            ArtifactPayload::SlangIR,
            ArtifactStyle::Unknown));
        irArtifact->addRepresentationUnknown(containerBlob);

        // Add the IR artifact
        artifacts.add(irArtifact);
    }

    // If there is only one artifact we can use that as the container
    if (artifacts.getCount() == 1)
    {
        m_containerArtifact = artifacts[0];
    }
    else
    {
        m_containerArtifact = ArtifactUtil::createArtifact(
            ArtifactDesc::make(ArtifactKind::Container, ArtifactPayload::CompileResults));

        for (IArtifact* childArtifact : artifacts)
        {
            m_containerArtifact->addChild(childArtifact);
        }
    }

    // Get all of the source obfuscated source maps and add those
    if (m_containerArtifact)
    {
        auto frontEndReq = getFrontEndReq();

        for (auto translationUnit : frontEndReq->translationUnits)
        {
            // Hmmm do I have to therefore add a map for all translation units(!)
            // I guess this is okay in so far as an association can always be looked up by name
            if (auto sourceMap = _getObfuscatedSourceMap(translationUnit))
            {
                auto artifactDesc = ArtifactDesc::make(
                    ArtifactKind::Json,
                    ArtifactPayload::SourceMap,
                    ArtifactStyle::Obfuscated);

                // Create the source map artifact
                auto sourceMapArtifact =
                    Artifact::create(artifactDesc, sourceMap->get().m_file.getUnownedSlice());

                // Add the repesentation
                sourceMapArtifact->addRepresentation(sourceMap);

                // Associate with the container
                m_containerArtifact->addAssociated(sourceMapArtifact);
            }
        }
    }

    return SLANG_OK;
}

CompilerOptionSet& EndToEndCompileRequest::getTargetOptionSet(TargetRequest* req)
{
    return req->getOptionSet();
}

CompilerOptionSet& EndToEndCompileRequest::getTargetOptionSet(Index targetIndex)
{
    return m_linkage->targets[targetIndex]->getOptionSet();
}

SlangResult EndToEndCompileRequest::maybeWriteContainer(const String& fileName)
{
    // If there is no container, or filename, don't write anything
    if (fileName.getLength() == 0 || !m_containerArtifact)
    {
        return SLANG_OK;
    }

    // Filter the containerArtifact into things that can be written
    ComPtr<IArtifact> writeArtifact;
    SLANG_RETURN_ON_FAIL(ArtifactContainerUtil::filter(m_containerArtifact, writeArtifact));

    // Only write if there is something to write
    if (writeArtifact)
    {
        SLANG_RETURN_ON_FAIL(ArtifactContainerUtil::writeContainer(writeArtifact, fileName));
    }

    return SLANG_OK;
}

void EndToEndCompileRequest::generateOutput(ComponentType* program)
{
    // When dynamic dispatch is disabled, the program must
    // be fully specialized by now. So we check if we still
    // have unspecialized generic/existential parameters,
    // and report them as an error.
    //
    auto specializationParamCount = program->getSpecializationParamCount();
    if (getOptionSet().getBoolOption(CompilerOptionName::DisableDynamicDispatch) &&
        specializationParamCount != 0)
    {
        auto sink = getSink();

        for (Index ii = 0; ii < specializationParamCount; ++ii)
        {
            auto specializationParam = program->getSpecializationParam(ii);
            if (auto decl = as<Decl>(specializationParam.object))
            {
                sink->diagnose(
                    specializationParam.loc,
                    Diagnostics::specializationParameterOfNameNotSpecialized,
                    decl);
            }
            else if (auto type = as<Type>(specializationParam.object))
            {
                sink->diagnose(
                    specializationParam.loc,
                    Diagnostics::specializationParameterOfNameNotSpecialized,
                    type);
            }
            else
            {
                sink->diagnose(
                    specializationParam.loc,
                    Diagnostics::specializationParameterNotSpecialized);
            }
        }

        return;
    }


    // Go through the code-generation targets that the user
    // has specified, and generate code for each of them.
    //
    auto linkage = getLinkage();
    for (auto targetReq : linkage->targets)
    {
        if (targetReq->getOptionSet().getBoolOption(CompilerOptionName::EmbedDownstreamIR))
            continue;

        auto targetProgram = program->getTargetProgram(targetReq);
        generateOutput(targetProgram);
    }
}

void EndToEndCompileRequest::generateOutput()
{
    SLANG_PROFILE;
    generateOutput(getSpecializedGlobalAndEntryPointsComponentType());

    // If we are in command-line mode, we might be expected to actually
    // write output to one or more files here.

    if (m_isCommandLineCompile && m_containerFormat == ContainerFormat::None)
    {
        auto linkage = getLinkage();
        auto program = getSpecializedGlobalAndEntryPointsComponentType();

        for (auto targetReq : linkage->targets)
        {
            auto targetProgram = program->getTargetProgram(targetReq);

            if (targetProgram->getOptionSet().getBoolOption(
                    CompilerOptionName::GenerateWholeProgram))
            {
                if (const auto artifact = targetProgram->getExistingWholeProgramResult())
                {
                    const auto path = _getWholeProgramPath(targetReq);

                    _maybeWriteArtifact(path, artifact);

                    // If we are compiling separate debug info, check for the additional
                    // SPIRV artifact and write that if needed.
                    _maybeWriteDebugArtifact(targetProgram, path, artifact);
                }
            }
            else
            {
                Index entryPointCount = program->getEntryPointCount();
                for (Index ee = 0; ee < entryPointCount; ++ee)
                {
                    if (const auto artifact = targetProgram->getExistingEntryPointResult(ee))
                    {
                        const auto path = _getEntryPointPath(targetReq, ee);

                        _maybeWriteArtifact(path, artifact);

                        // If we are compiling separate debug info, check for the additional
                        // SPIRV artifact and write that if needed.
                        _maybeWriteDebugArtifact(targetProgram, path, artifact);
                    }
                }
            }
        }
    }

    // Maybe create the container
    maybeCreateContainer();

    // If it's a command line compile we may need to write the container to a file
    if (m_isCommandLineCompile)
    {
        // TODO(JS):
        // We could write the container into a source embedded format potentially

        maybeWriteContainer(m_containerOutputPath);

        writeDependencyFile(this);
    }
}

void EndToEndCompileRequest::setFileSystem(ISlangFileSystem* fileSystem)
{
    getLinkage()->setFileSystem(fileSystem);
}

void EndToEndCompileRequest::setCompileFlags(SlangCompileFlags flags)
{
    if (flags & SLANG_COMPILE_FLAG_NO_MANGLING)
        getOptionSet().set(CompilerOptionName::NoMangle, true);
    if (flags & SLANG_COMPILE_FLAG_NO_CODEGEN)
        getOptionSet().set(CompilerOptionName::SkipCodeGen, true);
    if (flags & SLANG_COMPILE_FLAG_OBFUSCATE)
        getOptionSet().set(CompilerOptionName::Obfuscate, true);
}

SlangCompileFlags EndToEndCompileRequest::getCompileFlags()
{
    SlangCompileFlags result = 0;
    if (getOptionSet().getBoolOption(CompilerOptionName::NoMangle))
        result |= SLANG_COMPILE_FLAG_NO_MANGLING;
    if (getOptionSet().getBoolOption(CompilerOptionName::SkipCodeGen))
        result |= SLANG_COMPILE_FLAG_NO_CODEGEN;
    if (getOptionSet().getBoolOption(CompilerOptionName::Obfuscate))
        result |= SLANG_COMPILE_FLAG_OBFUSCATE;
    return result;
}

void EndToEndCompileRequest::setDumpIntermediates(int enable)
{
    getOptionSet().set(CompilerOptionName::DumpIntermediates, enable);
}

void EndToEndCompileRequest::setTrackLiveness(bool v)
{
    getOptionSet().set(CompilerOptionName::TrackLiveness, v);
}

void EndToEndCompileRequest::setDumpIntermediatePrefix(const char* prefix)
{
    getOptionSet().set(CompilerOptionName::DumpIntermediatePrefix, String(prefix));
}

void EndToEndCompileRequest::setLineDirectiveMode(SlangLineDirectiveMode mode)
{
    getOptionSet().set(CompilerOptionName::LineDirectiveMode, mode);
}

void EndToEndCompileRequest::setCommandLineCompilerMode()
{
    m_isCommandLineCompile = true;

    // legacy slangc tool defaults to column major layout.
    if (!getOptionSet().hasOption(CompilerOptionName::MatrixLayoutRow))
        getOptionSet().setMatrixLayoutMode(kMatrixLayoutMode_ColumnMajor);
}

void EndToEndCompileRequest::_completeTargetRequest(UInt targetIndex)
{
    auto linkage = getLinkage();

    TargetRequest* targetRequest = linkage->targets[Index(targetIndex)];

    targetRequest->getOptionSet().inheritFrom(getLinkage()->m_optionSet);
    targetRequest->getOptionSet().inheritFrom(m_optionSetForDefaultTarget);
}

void EndToEndCompileRequest::setCodeGenTarget(SlangCompileTarget target)
{
    auto linkage = getLinkage();
    linkage->targets.clear();
    const auto targetIndex = linkage->addTarget(CodeGenTarget(target));
    SLANG_ASSERT(targetIndex == 0);
    _completeTargetRequest(0);
}

int EndToEndCompileRequest::addCodeGenTarget(SlangCompileTarget target)
{
    const auto targetIndex = getLinkage()->addTarget(CodeGenTarget(target));
    _completeTargetRequest(targetIndex);
    return int(targetIndex);
}

void EndToEndCompileRequest::setTargetProfile(int targetIndex, SlangProfileID profile)
{
    getTargetOptionSet(targetIndex).setProfile(Profile(profile));
}

void EndToEndCompileRequest::setTargetFlags(int targetIndex, SlangTargetFlags flags)
{
    getTargetOptionSet(targetIndex).setTargetFlags(flags);
}

void EndToEndCompileRequest::setTargetForceGLSLScalarBufferLayout(int targetIndex, bool value)
{
    getTargetOptionSet(targetIndex).set(CompilerOptionName::GLSLForceScalarLayout, value);
}

void EndToEndCompileRequest::setTargetForceDXLayout(int targetIndex, bool value)
{
    getTargetOptionSet(targetIndex).set(CompilerOptionName::ForceDXLayout, value);
}

void EndToEndCompileRequest::setTargetForceCLayout(int targetIndex, bool value)
{
    getTargetOptionSet(targetIndex).set(CompilerOptionName::ForceCLayout, value);
}

void EndToEndCompileRequest::setTargetFloatingPointMode(
    int targetIndex,
    SlangFloatingPointMode mode)
{
    getTargetOptionSet(targetIndex)
        .set(CompilerOptionName::FloatingPointMode, FloatingPointMode(mode));
}

void EndToEndCompileRequest::setMatrixLayoutMode(SlangMatrixLayoutMode mode)
{
    getOptionSet().setMatrixLayoutMode((MatrixLayoutMode)mode);
}

void EndToEndCompileRequest::setTargetMatrixLayoutMode(int targetIndex, SlangMatrixLayoutMode mode)
{
    getTargetOptionSet(targetIndex).setMatrixLayoutMode(MatrixLayoutMode(mode));
}

void EndToEndCompileRequest::setTargetGenerateWholeProgram(int targetIndex, bool value)
{
    getTargetOptionSet(targetIndex).set(CompilerOptionName::GenerateWholeProgram, value);
}

void EndToEndCompileRequest::setTargetEmbedDownstreamIR(int targetIndex, bool value)
{
    getTargetOptionSet(targetIndex).set(CompilerOptionName::EmbedDownstreamIR, value);
}

void EndToEndCompileRequest::setTargetLineDirectiveMode(
    SlangInt targetIndex,
    SlangLineDirectiveMode mode)
{
    getTargetOptionSet(targetIndex)
        .set(CompilerOptionName::LineDirectiveMode, LineDirectiveMode(mode));
}

void EndToEndCompileRequest::overrideDiagnosticSeverity(
    SlangInt messageID,
    SlangSeverity overrideSeverity)
{
    getSink()->overrideDiagnosticSeverity(int(messageID), Severity(overrideSeverity));
}

SlangDiagnosticFlags EndToEndCompileRequest::getDiagnosticFlags()
{
    DiagnosticSink::Flags sinkFlags = getSink()->getFlags();

    SlangDiagnosticFlags flags = 0;

    if (sinkFlags & DiagnosticSink::Flag::VerbosePath)
        flags |= SLANG_DIAGNOSTIC_FLAG_VERBOSE_PATHS;

    if (sinkFlags & DiagnosticSink::Flag::TreatWarningsAsErrors)
        flags |= SLANG_DIAGNOSTIC_FLAG_TREAT_WARNINGS_AS_ERRORS;

    return flags;
}

void EndToEndCompileRequest::setDiagnosticFlags(SlangDiagnosticFlags flags)
{
    DiagnosticSink::Flags sinkFlags = getSink()->getFlags();

    if (flags & SLANG_DIAGNOSTIC_FLAG_VERBOSE_PATHS)
        sinkFlags |= DiagnosticSink::Flag::VerbosePath;
    else
        sinkFlags &= ~DiagnosticSink::Flag::VerbosePath;

    if (flags & SLANG_DIAGNOSTIC_FLAG_TREAT_WARNINGS_AS_ERRORS)
        sinkFlags |= DiagnosticSink::Flag::TreatWarningsAsErrors;
    else
        sinkFlags &= ~DiagnosticSink::Flag::TreatWarningsAsErrors;

    getSink()->setFlags(sinkFlags);
}

SlangResult EndToEndCompileRequest::addTargetCapability(
    SlangInt targetIndex,
    SlangCapabilityID capability)
{
    auto& targets = getLinkage()->targets;
    if (targetIndex < 0 || targetIndex >= targets.getCount())
        return SLANG_E_INVALID_ARG;
    getTargetOptionSet(targetIndex).addCapabilityAtom(CapabilityName(capability));
    return SLANG_OK;
}

void EndToEndCompileRequest::setDebugInfoLevel(SlangDebugInfoLevel level)
{
    getOptionSet().set(CompilerOptionName::DebugInformation, DebugInfoLevel(level));
}

void EndToEndCompileRequest::setDebugInfoFormat(SlangDebugInfoFormat format)
{
    getOptionSet().set(CompilerOptionName::DebugInformationFormat, DebugInfoFormat(format));
}

void EndToEndCompileRequest::setOptimizationLevel(SlangOptimizationLevel level)
{
    getOptionSet().set(CompilerOptionName::Optimization, OptimizationLevel(level));
}

void EndToEndCompileRequest::setOutputContainerFormat(SlangContainerFormat format)
{
    m_containerFormat = ContainerFormat(format);
}

void EndToEndCompileRequest::setPassThrough(SlangPassThrough inPassThrough)
{
    m_passThrough = PassThroughMode(inPassThrough);
}

void EndToEndCompileRequest::setReportDownstreamTime(bool value)
{
    getOptionSet().set(CompilerOptionName::ReportDownstreamTime, value);
}

void EndToEndCompileRequest::setReportPerfBenchmark(bool value)
{
    getOptionSet().set(CompilerOptionName::ReportPerfBenchmark, value);
}

void EndToEndCompileRequest::setSkipSPIRVValidation(bool value)
{
    getOptionSet().set(CompilerOptionName::SkipSPIRVValidation, value);
}

void EndToEndCompileRequest::setTargetUseMinimumSlangOptimization(int targetIndex, bool value)
{
    getTargetOptionSet(targetIndex).set(CompilerOptionName::MinimumSlangOptimization, value);
}

void EndToEndCompileRequest::setIgnoreCapabilityCheck(bool value)
{
    getOptionSet().set(CompilerOptionName::IgnoreCapabilities, value);
}

void EndToEndCompileRequest::setDiagnosticCallback(
    SlangDiagnosticCallback callback,
    void const* userData)
{
    ComPtr<ISlangWriter> writer(new CallbackWriter(callback, userData, WriterFlag::IsConsole));
    setWriter(WriterChannel::Diagnostic, writer);
}

void EndToEndCompileRequest::setWriter(SlangWriterChannel chan, ISlangWriter* writer)
{
    setWriter(WriterChannel(chan), writer);
}

ISlangWriter* EndToEndCompileRequest::getWriter(SlangWriterChannel chan)
{
    return getWriter(WriterChannel(chan));
}

void EndToEndCompileRequest::addSearchPath(const char* path)
{
    getOptionSet().addSearchPath(path);
}

void EndToEndCompileRequest::addPreprocessorDefine(const char* key, const char* value)
{
    getOptionSet().addPreprocessorDefine(key, value);
}

void EndToEndCompileRequest::setEnableEffectAnnotations(bool value)
{
    getOptionSet().set(CompilerOptionName::EnableEffectAnnotations, value);
}

char const* EndToEndCompileRequest::getDiagnosticOutput()
{
    return m_diagnosticOutput.begin();
}

SlangResult EndToEndCompileRequest::getDiagnosticOutputBlob(ISlangBlob** outBlob)
{
    if (!outBlob)
        return SLANG_E_INVALID_ARG;

    if (!m_diagnosticOutputBlob)
    {
        m_diagnosticOutputBlob = StringUtil::createStringBlob(m_diagnosticOutput);
    }

    ComPtr<ISlangBlob> resultBlob = m_diagnosticOutputBlob;
    *outBlob = resultBlob.detach();
    return SLANG_OK;
}

int EndToEndCompileRequest::addTranslationUnit(SlangSourceLanguage language, char const* inName)
{
    auto frontEndReq = getFrontEndReq();
    NamePool* namePool = frontEndReq->getNamePool();

    // Work out a module name. Can be nullptr if so will generate a name
    Name* moduleName = inName ? namePool->getName(inName) : frontEndReq->m_defaultModuleName;

    // If moduleName is nullptr a name will be generated
    return frontEndReq->addTranslationUnit(Slang::SourceLanguage(language), moduleName);
}

void EndToEndCompileRequest::setDefaultModuleName(const char* defaultModuleName)
{
    auto frontEndReq = getFrontEndReq();
    NamePool* namePool = frontEndReq->getNamePool();
    frontEndReq->m_defaultModuleName = namePool->getName(defaultModuleName);
}

SlangResult _addLibraryReference(
    EndToEndCompileRequest* req,
    ModuleLibrary* moduleLibrary,
    bool includeEntryPoint)
{
    FrontEndCompileRequest* frontEndRequest = req->getFrontEndReq();

    if (includeEntryPoint)
    {
        frontEndRequest->m_extraEntryPoints.addRange(
            moduleLibrary->m_entryPoints.getBuffer(),
            moduleLibrary->m_entryPoints.getCount());
    }

    for (auto m : moduleLibrary->m_modules)
    {
        RefPtr<TranslationUnitRequest> tu = new TranslationUnitRequest(frontEndRequest, m);
        frontEndRequest->translationUnits.add(tu);
        // For modules loaded for EndToEndCompileRequest,
        // we don't need the automatically discovered entrypoints.
        if (!includeEntryPoint)
            m->getEntryPoints().clear();
    }
    return SLANG_OK;
}

SlangResult _addLibraryReference(
    EndToEndCompileRequest* req,
    String path,
    IArtifact* artifact,
    bool includeEntryPoint)
{
    auto desc = artifact->getDesc();

    // TODO(JS):
    // This isn't perhaps the best way to handle this scenario, as IArtifact can
    // support lazy evaluation, with suitable hander.
    // For now we just read in and strip out the bits we want.
    if (isDerivedFrom(desc.kind, ArtifactKind::Container) &&
        isDerivedFrom(desc.payload, ArtifactPayload::CompileResults))
    {
        // We want to read as a file system
        ComPtr<IArtifact> container;

        SLANG_RETURN_ON_FAIL(ArtifactContainerUtil::readContainer(artifact, container));

        // Find the payload... It should be linkable
        if (!ArtifactDescUtil::isLinkable(container->getDesc()))
        {
            return SLANG_FAIL;
        }

        ComPtr<IModuleLibrary> libraryIntf;
        SLANG_RETURN_ON_FAIL(
            loadModuleLibrary(ArtifactKeep::Yes, container, path, req, libraryIntf));

        auto library = as<ModuleLibrary>(libraryIntf);

        // Look for source maps
        for (auto associated : container->getAssociated())
        {
            auto assocDesc = associated->getDesc();

            // If we find an obfuscated source map load it and associate
            if (isDerivedFrom(assocDesc.kind, ArtifactKind::Json) &&
                isDerivedFrom(assocDesc.payload, ArtifactPayload::SourceMap) &&
                isDerivedFrom(assocDesc.style, ArtifactStyle::Obfuscated))
            {
                ComPtr<ICastable> castable;
                SLANG_RETURN_ON_FAIL(associated->getOrCreateRepresentation(
                    SourceMap::getTypeGuid(),
                    ArtifactKeep::Yes,
                    castable.writeRef()));
                auto sourceMapBox = asBoxValue<SourceMap>(castable);
                SLANG_ASSERT(sourceMapBox);

                // TODO(JS):
                // There is perhaps (?) a risk here that we might copy the obfuscated map
                // into some output container. Currently that only happens for source maps
                // that are from translation units.
                //
                // On the other hand using "import" is a way that such source maps *would* be
                // copied into the output, and that is something that could be a vector
                // for leaking.
                //
                // That isn't a risk from -r though because, it doesn't create a translation
                // unit(s).
                for (auto module : library->m_modules)
                {
                    module->getIRModule()->setObfuscatedSourceMap(sourceMapBox);
                }

                // Look up the source file
                auto sourceManager = req->getSink()->getSourceManager();

                auto name = Path::getFileNameWithoutExt(associated->getName());

                if (name.getLength())
                {
                    // Note(tfoley): There is a subtle requirement here, that any
                    // source file `name` that might be searched for here *must*
                    // have been added to the `sourceManager` already, as a
                    // byproduct of debug source location information getting
                    // deserialized as part of the call to `loadModuleLibrary()` above.
                    //
                    // The implicit dependency is frustrating, and could potentially
                    // break if somehow the debug info chunk was stripped from a binary,
                    // while the source map was left in (which should be valid, even if
                    // it is unlikely to be what a user wants).
                    //
                    // Ideally the source map would either be made an integral part of
                    // the debug source location chunk, so they are loaded together,
                    // or the `SourceManager` would be adapted so that it can store
                    // registered source maps independent of whether or not the
                    // corresponding source file(s) have been loaded.

                    auto sourceFile = sourceManager->findSourceFileByPathRecursively(name);
                    SLANG_ASSERT(sourceFile);
                    sourceFile->setSourceMap(sourceMapBox, SourceMapKind::Obfuscated);
                }
            }
        }

        SLANG_RETURN_ON_FAIL(_addLibraryReference(req, library, includeEntryPoint));
        return SLANG_OK;
    }

    if (desc.kind == ArtifactKind::Library && desc.payload == ArtifactPayload::SlangIR)
    {
        ComPtr<IModuleLibrary> libraryIntf;

        SLANG_RETURN_ON_FAIL(
            loadModuleLibrary(ArtifactKeep::Yes, artifact, path, req, libraryIntf));

        auto library = as<ModuleLibrary>(libraryIntf);
        if (!library)
        {
            return SLANG_FAIL;
        }

        SLANG_RETURN_ON_FAIL(_addLibraryReference(req, library, includeEntryPoint));
        return SLANG_OK;
    }

    // TODO(JS):
    // Do we want to check the path exists?

    // Add to the m_libModules
    auto linkage = req->getLinkage();
    linkage->m_libModules.add(ComPtr<IArtifact>(artifact));

    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::addLibraryReference(
    const char* basePath,
    const void* libData,
    size_t libDataSize)
{
    // We need to deserialize and add the modules
    ComPtr<IModuleLibrary> library;

    auto libBlob = RawBlob::create((const Byte*)libData, libDataSize);

    SLANG_RETURN_ON_FAIL(
        loadModuleLibrary(libBlob, (const Byte*)libData, libDataSize, basePath, this, library));

    // Create an artifact without any name (as one is not provided)
    auto artifact =
        Artifact::create(ArtifactDesc::make(ArtifactKind::Library, ArtifactPayload::SlangIR));
    artifact->addRepresentation(library);

    return _addLibraryReference(this, basePath, artifact, true);
}

void EndToEndCompileRequest::addTranslationUnitPreprocessorDefine(
    int translationUnitIndex,
    const char* key,
    const char* value)
{
    getFrontEndReq()->translationUnits[translationUnitIndex]->preprocessorDefinitions[key] = value;
}

void EndToEndCompileRequest::addTranslationUnitSourceFile(
    int translationUnitIndex,
    char const* path)
{
    auto frontEndReq = getFrontEndReq();
    if (!path)
        return;
    if (translationUnitIndex < 0)
        return;
    if (Index(translationUnitIndex) >= frontEndReq->translationUnits.getCount())
        return;

    frontEndReq->addTranslationUnitSourceFile(translationUnitIndex, path);
}

void EndToEndCompileRequest::addTranslationUnitSourceString(
    int translationUnitIndex,
    char const* path,
    char const* source)
{
    if (!source)
        return;
    addTranslationUnitSourceStringSpan(translationUnitIndex, path, source, source + strlen(source));
}

void EndToEndCompileRequest::addTranslationUnitSourceStringSpan(
    int translationUnitIndex,
    char const* path,
    char const* sourceBegin,
    char const* sourceEnd)
{
    auto frontEndReq = getFrontEndReq();
    if (!sourceBegin)
        return;
    if (translationUnitIndex < 0)
        return;
    if (Index(translationUnitIndex) >= frontEndReq->translationUnits.getCount())
        return;

    if (!path)
        path = "";

    const auto slice = UnownedStringSlice(sourceBegin, sourceEnd);

    auto blob = RawBlob::create(slice.begin(), slice.getLength());

    frontEndReq->addTranslationUnitSourceBlob(translationUnitIndex, path, blob);
}

void EndToEndCompileRequest::addTranslationUnitSourceBlob(
    int translationUnitIndex,
    char const* path,
    ISlangBlob* sourceBlob)
{
    auto frontEndReq = getFrontEndReq();
    if (!sourceBlob)
        return;
    if (translationUnitIndex < 0)
        return;
    if (Slang::Index(translationUnitIndex) >= frontEndReq->translationUnits.getCount())
        return;

    if (!path)
        path = "";

    frontEndReq->addTranslationUnitSourceBlob(translationUnitIndex, path, sourceBlob);
}


int EndToEndCompileRequest::addEntryPoint(
    int translationUnitIndex,
    char const* name,
    SlangStage stage)
{
    return addEntryPointEx(translationUnitIndex, name, stage, 0, nullptr);
}

int EndToEndCompileRequest::addEntryPointEx(
    int translationUnitIndex,
    char const* name,
    SlangStage stage,
    int genericParamTypeNameCount,
    char const** genericParamTypeNames)
{
    auto frontEndReq = getFrontEndReq();
    if (!name)
        return -1;
    if (translationUnitIndex < 0)
        return -1;
    if (Index(translationUnitIndex) >= frontEndReq->translationUnits.getCount())
        return -1;

    List<String> typeNames;
    for (int i = 0; i < genericParamTypeNameCount; i++)
        typeNames.add(genericParamTypeNames[i]);

    return addEntryPoint(translationUnitIndex, name, Profile(Stage(stage)), typeNames);
}

SlangResult EndToEndCompileRequest::setGlobalGenericArgs(
    int genericArgCount,
    char const** genericArgs)
{
    auto& argStrings = m_globalSpecializationArgStrings;
    argStrings.clear();
    for (int i = 0; i < genericArgCount; i++)
        argStrings.add(genericArgs[i]);

    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::setTypeNameForGlobalExistentialTypeParam(
    int slotIndex,
    char const* typeName)
{
    if (slotIndex < 0)
        return SLANG_FAIL;
    if (!typeName)
        return SLANG_FAIL;

    auto& typeArgStrings = m_globalSpecializationArgStrings;
    if (Index(slotIndex) >= typeArgStrings.getCount())
        typeArgStrings.setCount(slotIndex + 1);
    typeArgStrings[slotIndex] = String(typeName);
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::setTypeNameForEntryPointExistentialTypeParam(
    int entryPointIndex,
    int slotIndex,
    char const* typeName)
{
    if (entryPointIndex < 0)
        return SLANG_FAIL;
    if (slotIndex < 0)
        return SLANG_FAIL;
    if (!typeName)
        return SLANG_FAIL;

    if (Index(entryPointIndex) >= m_entryPoints.getCount())
        return SLANG_FAIL;

    auto& entryPointInfo = m_entryPoints[entryPointIndex];
    auto& typeArgStrings = entryPointInfo.specializationArgStrings;
    if (Index(slotIndex) >= typeArgStrings.getCount())
        typeArgStrings.setCount(slotIndex + 1);
    typeArgStrings[slotIndex] = String(typeName);
    return SLANG_OK;
}

void EndToEndCompileRequest::setAllowGLSLInput(bool value)
{
    getOptionSet().set(CompilerOptionName::AllowGLSL, value);
}

SlangResult EndToEndCompileRequest::compile()
{
    SlangResult res = SLANG_FAIL;
    double downstreamStartTime = 0.0;
    double totalStartTime = 0.0;

    if (getOptionSet().getBoolOption(CompilerOptionName::ReportDownstreamTime))
    {
        getSession()->getCompilerElapsedTime(&totalStartTime, &downstreamStartTime);
        PerformanceProfiler::getProfiler()->clear();
    }
#if !defined(SLANG_DEBUG_INTERNAL_ERROR)
    // By default we'd like to catch as many internal errors as possible,
    // and report them to the user nicely (rather than just crash their
    // application). Internally Slang currently uses exceptions for this.
    //
    // TODO: Consider using `setjmp()`-style escape so that we can work
    // with applications that disable exceptions.
    //
    // TODO: Consider supporting Windows "Structured Exception Handling"
    // so that we can also recover from a wider class of crashes.

    try
    {
        SLANG_PROFILE_SECTION(compileInner);
        res = executeActions();
    }
    catch (const AbortCompilationException& e)
    {
        // This situation indicates a fatal (but not necessarily internal) error
        // that forced compilation to terminate. There should already have been
        // a diagnostic produced, so we don't need to add one here.
        if (getSink()->getErrorCount() == 0)
        {
            // If for some reason we didn't output any diagnostic, something is
            // going wrong, but we want to make sure we at least output something.
            getSink()->diagnose(
                SourceLoc(),
                Diagnostics::compilationAbortedDueToException,
                typeid(e).name(),
                e.Message);
        }
    }
    catch (const Exception& e)
    {
        // The compiler failed due to an internal error that was detected.
        // We will print out information on the exception to help out the user
        // in either filing a bug, or locating what in their code created
        // a problem.
        getSink()->diagnose(
            SourceLoc(),
            Diagnostics::compilationAbortedDueToException,
            typeid(e).name(),
            e.Message);
    }
    catch (...)
    {
        // The compiler failed due to some exception that wasn't a sublass of
        // `Exception`, so something really fishy is going on. We want to
        // let the user know that we messed up, so they know to blame Slang
        // and not some other component in their system.
        getSink()->diagnose(SourceLoc(), Diagnostics::compilationAborted);
    }
    m_diagnosticOutput = getSink()->outputBuffer.produceString();

#else
    // When debugging, we probably don't want to filter out any errors, since
    // we are probably trying to root-cause and *fix* those errors.
    {
        res = req->executeActions();
    }
#endif

    if (getOptionSet().getBoolOption(CompilerOptionName::ReportDownstreamTime))
    {
        double downstreamEndTime = 0;
        double totalEndTime = 0;
        getSession()->getCompilerElapsedTime(&totalEndTime, &downstreamEndTime);
        double downstreamTime = downstreamEndTime - downstreamStartTime;
        String downstreamTimeStr = String(downstreamTime, "%.2f");
        getSink()->diagnose(SourceLoc(), Diagnostics::downstreamCompileTime, downstreamTimeStr);
    }
    if (getOptionSet().getBoolOption(CompilerOptionName::ReportPerfBenchmark))
    {
        StringBuilder perfResult;
        PerformanceProfiler::getProfiler()->getResult(perfResult);
        perfResult << "\nType Dictionary Size: " << getSession()->m_typeDictionarySize << "\n";
        getSink()->diagnose(
            SourceLoc(),
            Diagnostics::performanceBenchmarkResult,
            perfResult.produceString());
    }

    // Repro dump handling
    {
        auto dumpRepro = getOptionSet().getStringOption(CompilerOptionName::DumpRepro);
        auto dumpReproOnError = getOptionSet().getBoolOption(CompilerOptionName::DumpReproOnError);

        if (dumpRepro.getLength())
        {
            SlangResult saveRes = ReproUtil::saveState(this, dumpRepro);
            if (SLANG_FAILED(saveRes))
            {
                getSink()->diagnose(SourceLoc(), Diagnostics::unableToWriteReproFile, dumpRepro);
                return saveRes;
            }
        }
        else if (dumpReproOnError && SLANG_FAILED(res))
        {
            String reproFileName;
            SlangResult saveRes = SLANG_FAIL;

            RefPtr<Stream> stream;
            if (SLANG_SUCCEEDED(ReproUtil::findUniqueReproDumpStream(this, reproFileName, stream)))
            {
                saveRes = ReproUtil::saveState(this, stream);
            }

            if (SLANG_FAILED(saveRes))
            {
                getSink()->diagnose(
                    SourceLoc(),
                    Diagnostics::unableToWriteReproFile,
                    reproFileName);
            }
        }
    }

    auto reflectionPath = getOptionSet().getStringOption(CompilerOptionName::EmitReflectionJSON);
    if (reflectionPath.getLength() != 0)
    {
        auto reflection = this->getReflection();
        if (!reflection)
        {
            getSink()->diagnose(SourceLoc(), Diagnostics::cannotEmitReflectionWithoutTarget);
            return SLANG_FAIL;
        }
        auto bufferWriter = PrettyWriter();
        emitReflectionJSON(this, reflection, bufferWriter);
        if (reflectionPath == "-")
        {
            auto builder = bufferWriter.getBuilder();
            StdWriters::getOut().write(builder.getBuffer(), builder.getLength());
        }
        else if (SLANG_FAILED(File::writeAllText(reflectionPath, bufferWriter.getBuilder())))
        {
            getSink()->diagnose(SourceLoc(), Diagnostics::unableToWriteFile, reflectionPath);
        }
    }

    return res;
}

int EndToEndCompileRequest::getDependencyFileCount()
{
    auto frontEndReq = getFrontEndReq();
    auto program = frontEndReq->getGlobalAndEntryPointsComponentType();
    return (int)program->getFileDependencies().getCount();
}

char const* EndToEndCompileRequest::getDependencyFilePath(int index)
{
    auto frontEndReq = getFrontEndReq();
    auto program = frontEndReq->getGlobalAndEntryPointsComponentType();
    SourceFile* sourceFile = program->getFileDependencies()[index];
    return sourceFile->getPathInfo().hasFoundPath()
               ? sourceFile->getPathInfo().getMostUniqueIdentity().getBuffer()
               : "unknown";
}

int EndToEndCompileRequest::getTranslationUnitCount()
{
    return (int)getFrontEndReq()->translationUnits.getCount();
}

void const* EndToEndCompileRequest::getEntryPointCode(int entryPointIndex, size_t* outSize)
{
    // Zero the size initially, in case need to return nullptr for error.
    if (outSize)
    {
        *outSize = 0;
    }

    auto linkage = getLinkage();
    auto program = getSpecializedGlobalAndEntryPointsComponentType();

    // TODO: We should really accept a target index in this API
    Index targetIndex = 0;
    auto targetCount = linkage->targets.getCount();
    if (targetIndex >= targetCount)
        return nullptr;
    auto targetReq = linkage->targets[targetIndex];


    if (entryPointIndex < 0)
        return nullptr;
    if (Index(entryPointIndex) >= program->getEntryPointCount())
        return nullptr;
    auto entryPoint = program->getEntryPoint(entryPointIndex);

    auto targetProgram = program->getTargetProgram(targetReq);
    if (!targetProgram)
        return nullptr;
    IArtifact* artifact = targetProgram->getExistingEntryPointResult(entryPointIndex);
    if (!artifact)
    {
        return nullptr;
    }

    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_NULL_ON_FAIL(artifact->loadBlob(ArtifactKeep::Yes, blob.writeRef()));

    if (outSize)
    {
        *outSize = blob->getBufferSize();
    }

    return (void*)blob->getBufferPointer();
}

SlangResult EndToEndCompileRequest::getCompileTimeProfile(
    ISlangProfiler** compileTimeProfile,
    bool shouldClear)
{
    if (compileTimeProfile == nullptr)
    {
        return SLANG_E_INVALID_ARG;
    }

    SlangProfiler* profiler = new SlangProfiler(PerformanceProfiler::getProfiler());

    if (shouldClear)
    {
        PerformanceProfiler::getProfiler()->clear();
    }

    ComPtr<ISlangProfiler> result(profiler);
    *compileTimeProfile = result.detach();
    return SLANG_OK;
}

static SlangResult _getEntryPointResult(
    EndToEndCompileRequest* req,
    int entryPointIndex,
    int targetIndex,
    ComPtr<IArtifact>& outArtifact)
{
    auto linkage = req->getLinkage();
    auto program = req->getSpecializedGlobalAndEntryPointsComponentType();

    Index targetCount = linkage->targets.getCount();
    if ((targetIndex < 0) || (targetIndex >= targetCount))
    {
        return SLANG_E_INVALID_ARG;
    }
    auto targetReq = linkage->targets[targetIndex];

    // Get the entry point count on the program, rather than (say) req->m_entryPoints.getCount()
    // because
    // 1) The entry point is fetched from the program anyway so must be consistent
    // 2) The req may not have all entry points (for example when an entry point is in a module)
    const Index entryPointCount = program->getEntryPointCount();

    if ((entryPointIndex < 0) || (entryPointIndex >= entryPointCount))
    {
        return SLANG_E_INVALID_ARG;
    }
    auto entryPointReq = program->getEntryPoint(entryPointIndex);

    auto targetProgram = program->getTargetProgram(targetReq);
    if (!targetProgram)
        return SLANG_FAIL;

    outArtifact = targetProgram->getExistingEntryPointResult(entryPointIndex);
    return SLANG_OK;
}

static SlangResult _getWholeProgramResult(
    EndToEndCompileRequest* req,
    int targetIndex,
    ComPtr<IArtifact>& outArtifact)
{
    auto linkage = req->getLinkage();
    auto program = req->getSpecializedGlobalAndEntryPointsComponentType();

    if (!program)
    {
        return SLANG_FAIL;
    }

    Index targetCount = linkage->targets.getCount();
    if ((targetIndex < 0) || (targetIndex >= targetCount))
    {
        return SLANG_E_INVALID_ARG;
    }
    auto targetReq = linkage->targets[targetIndex];

    auto targetProgram = program->getTargetProgram(targetReq);
    if (!targetProgram)
        return SLANG_FAIL;
    outArtifact = targetProgram->getExistingWholeProgramResult();
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::getEntryPointCodeBlob(
    int entryPointIndex,
    int targetIndex,
    ISlangBlob** outBlob)
{
    if (!outBlob)
        return SLANG_E_INVALID_ARG;
    ComPtr<IArtifact> artifact;
    SLANG_RETURN_ON_FAIL(_getEntryPointResult(this, entryPointIndex, targetIndex, artifact));
    SLANG_RETURN_ON_FAIL(artifact->loadBlob(ArtifactKeep::Yes, outBlob));

    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::getEntryPointHostCallable(
    int entryPointIndex,
    int targetIndex,
    ISlangSharedLibrary** outSharedLibrary)
{
    if (!outSharedLibrary)
        return SLANG_E_INVALID_ARG;
    ComPtr<IArtifact> artifact;
    SLANG_RETURN_ON_FAIL(_getEntryPointResult(this, entryPointIndex, targetIndex, artifact));
    SLANG_RETURN_ON_FAIL(artifact->loadSharedLibrary(ArtifactKeep::Yes, outSharedLibrary));
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::getTargetCodeBlob(int targetIndex, ISlangBlob** outBlob)
{
    if (!outBlob)
        return SLANG_E_INVALID_ARG;

    ComPtr<IArtifact> artifact;
    SLANG_RETURN_ON_FAIL(_getWholeProgramResult(this, targetIndex, artifact));
    SLANG_RETURN_ON_FAIL(artifact->loadBlob(ArtifactKeep::Yes, outBlob));
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::getTargetHostCallable(
    int targetIndex,
    ISlangSharedLibrary** outSharedLibrary)
{
    if (!outSharedLibrary)
        return SLANG_E_INVALID_ARG;

    ComPtr<IArtifact> artifact;
    SLANG_RETURN_ON_FAIL(_getWholeProgramResult(this, targetIndex, artifact));
    SLANG_RETURN_ON_FAIL(artifact->loadSharedLibrary(ArtifactKeep::Yes, outSharedLibrary));
    return SLANG_OK;
}

char const* EndToEndCompileRequest::getEntryPointSource(int entryPointIndex)
{
    return (char const*)getEntryPointCode(entryPointIndex, nullptr);
}

ISlangMutableFileSystem* EndToEndCompileRequest::getCompileRequestResultAsFileSystem()
{
    if (!m_containerFileSystem)
    {
        if (m_containerArtifact)
        {
            ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem);

            // Filter the containerArtifact into things that can be written
            ComPtr<IArtifact> writeArtifact;
            if (SLANG_SUCCEEDED(
                    ArtifactContainerUtil::filter(m_containerArtifact, writeArtifact)) &&
                writeArtifact)
            {
                if (SLANG_SUCCEEDED(
                        ArtifactContainerUtil::writeContainer(writeArtifact, "", fileSystem)))
                {
                    m_containerFileSystem.swap(fileSystem);
                }
            }
        }
    }

    return m_containerFileSystem;
}

void const* EndToEndCompileRequest::getCompileRequestCode(size_t* outSize)
{
    if (m_containerArtifact)
    {
        ComPtr<ISlangBlob> containerBlob;
        if (SLANG_SUCCEEDED(
                m_containerArtifact->loadBlob(ArtifactKeep::Yes, containerBlob.writeRef())))
        {
            *outSize = containerBlob->getBufferSize();
            return containerBlob->getBufferPointer();
        }
    }

    // Container blob does not have any contents
    *outSize = 0;
    return nullptr;
}

SlangResult EndToEndCompileRequest::getContainerCode(ISlangBlob** outBlob)
{
    if (m_containerArtifact)
    {
        ComPtr<ISlangBlob> containerBlob;
        if (SLANG_SUCCEEDED(
                m_containerArtifact->loadBlob(ArtifactKeep::Yes, containerBlob.writeRef())))
        {
            *outBlob = containerBlob.detach();
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

SlangResult EndToEndCompileRequest::loadRepro(
    ISlangFileSystem* fileSystem,
    const void* data,
    size_t size)
{
    List<uint8_t> buffer;
    SLANG_RETURN_ON_FAIL(ReproUtil::loadState((const uint8_t*)data, size, getSink(), buffer));

    MemoryOffsetBase base;
    base.set(buffer.getBuffer(), buffer.getCount());

    ReproUtil::RequestState* requestState = ReproUtil::getRequest(buffer);

    SLANG_RETURN_ON_FAIL(ReproUtil::load(base, requestState, fileSystem, this));
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::saveRepro(ISlangBlob** outBlob)
{
    OwnedMemoryStream stream(FileAccess::Write);

    SLANG_RETURN_ON_FAIL(ReproUtil::saveState(this, &stream));

    // Put the content of the stream in the blob

    List<uint8_t> data;
    stream.swapContents(data);

    *outBlob = ListBlob::moveCreate(data).detach();
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::enableReproCapture()
{
    getLinkage()->setRequireCacheFileSystem(true);
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::processCommandLineArguments(
    char const* const* args,
    int argCount)
{
    return parseOptions(this, argCount, args);
}

SlangReflection* EndToEndCompileRequest::getReflection()
{
    auto linkage = getLinkage();
    auto program = getSpecializedGlobalAndEntryPointsComponentType();

    // Note(tfoley): The API signature doesn't let the client
    // specify which target they want to access reflection
    // information for, so for now we default to the first one.
    //
    // TODO: Add a new `spGetReflectionForTarget(req, targetIndex)`
    // so that we can do this better, and make it clear that
    // `spGetReflection()` is shorthand for `targetIndex == 0`.
    //
    Slang::Index targetIndex = 0;
    auto targetCount = linkage->targets.getCount();
    if (targetIndex >= targetCount)
        return nullptr;

    auto targetReq = linkage->targets[targetIndex];
    auto targetProgram = program->getTargetProgram(targetReq);


    DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
    auto programLayout = targetProgram->getOrCreateLayout(&sink);

    return (SlangReflection*)programLayout;
}

SlangResult EndToEndCompileRequest::getProgram(slang::IComponentType** outProgram)
{
    auto program = getSpecializedGlobalComponentType();
    *outProgram = Slang::ComPtr<slang::IComponentType>(program).detach();
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::getProgramWithEntryPoints(slang::IComponentType** outProgram)
{
    auto program = getSpecializedGlobalAndEntryPointsComponentType();
    *outProgram = Slang::ComPtr<slang::IComponentType>(program).detach();
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::getModule(
    SlangInt translationUnitIndex,
    slang::IModule** outModule)
{
    auto module = getFrontEndReq()->getTranslationUnit(translationUnitIndex)->getModule();

    *outModule = Slang::ComPtr<slang::IModule>(module).detach();
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::getSession(slang::ISession** outSession)
{
    auto session = getLinkage();
    *outSession = Slang::ComPtr<slang::ISession>(session).detach();
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::getEntryPoint(
    SlangInt entryPointIndex,
    slang::IComponentType** outEntryPoint)
{
    auto entryPoint = getSpecializedEntryPointComponentType(entryPointIndex);
    *outEntryPoint = Slang::ComPtr<slang::IComponentType>(entryPoint).detach();
    return SLANG_OK;
}

SlangResult EndToEndCompileRequest::isParameterLocationUsed(
    Int entryPointIndex,
    Int targetIndex,
    SlangParameterCategory category,
    UInt spaceIndex,
    UInt registerIndex,
    bool& outUsed)
{
    if (!ShaderBindingRange::isUsageTracked((slang::ParameterCategory)category))
        return SLANG_E_NOT_AVAILABLE;

    ComPtr<IArtifact> artifact;
    if (SLANG_FAILED(_getEntryPointResult(
            this,
            static_cast<int>(entryPointIndex),
            static_cast<int>(targetIndex),
            artifact)))
        return SLANG_E_INVALID_ARG;

    if (!artifact)
        return SLANG_E_NOT_AVAILABLE;

    // Find a rep
    auto metadata = findAssociatedRepresentation<IArtifactPostEmitMetadata>(artifact);
    if (!metadata)
        return SLANG_E_NOT_AVAILABLE;

    return metadata->isParameterLocationUsed(category, spaceIndex, registerIndex, outUsed);
}

} // namespace Slang
