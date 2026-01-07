// slang-code-gen.cpp
#include "slang-code-gen.h"

#include "../compiler-core/slang-slice-allocator.h"
#include "../core/slang-type-convert-util.h"
#include "../core/slang-type-text-util.h"
#include "slang-compiler.h"
#include "slang-emit-cuda.h"         // for `CUDAExtensionTracker`
#include "slang-extension-tracker.h" // for `ShaderExtensionTracker`

// TODO: The "artifact" system is a scourge.
#include "../compiler-core/slang-artifact-desc-util.h"
#include "../compiler-core/slang-artifact-impl.h"
#include "../compiler-core/slang-artifact-util.h"
#include "slang-artifact-output-util.h"

namespace Slang
{

EndToEndCompileRequest* CodeGenContext::isPassThroughEnabled()
{
    auto endToEndReq = isEndToEndCompile();

    // If there isn't an end-to-end compile going on,
    // there can be no pass-through.
    //
    if (!endToEndReq)
        return nullptr;

    // And if pass-through isn't set on that end-to-end compile,
    // then we clearly areb't doing a pass-through compile.
    //
    if (endToEndReq->m_passThrough == PassThroughMode::None)
        return nullptr;

    // If we have confirmed that pass-through compilation is going on,
    // we return the end-to-end request, because it has all the
    // relevant state that we need to implement pass-through mode.
    //
    return endToEndReq;
}

/// If there is a pass-through compile going on, find the translation unit for the given entry
/// point. Assumes isPassThroughEnabled has already been called
TranslationUnitRequest* getPassThroughTranslationUnit(
    EndToEndCompileRequest* endToEndReq,
    Int entryPointIndex)
{
    SLANG_ASSERT(endToEndReq);
    SLANG_ASSERT(endToEndReq->m_passThrough != PassThroughMode::None);
    auto frontEndReq = endToEndReq->getFrontEndReq();
    auto entryPointReq = frontEndReq->getEntryPointReq(entryPointIndex);
    auto translationUnit = entryPointReq->getTranslationUnit();
    return translationUnit;
}

TranslationUnitRequest* CodeGenContext::findPassThroughTranslationUnit(Int entryPointIndex)
{
    if (auto endToEndReq = isPassThroughEnabled())
        return getPassThroughTranslationUnit(endToEndReq, entryPointIndex);
    return nullptr;
}

static void _appendCodeWithPath(
    const UnownedStringSlice& filePath,
    const UnownedStringSlice& fileContent,
    StringBuilder& outCodeBuilder)
{
    outCodeBuilder << "#line 1 \"";
    auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp);
    handler->appendEscaped(filePath, outCodeBuilder);
    outCodeBuilder << "\"\n";
    outCodeBuilder << fileContent << "\n";
}

#if SLANG_VC
// TODO(JS): This is a workaround
// In debug VS builds there is a warning on line about it being unreachable.
// for (auto entryPointIndex : getEntryPointIndices())
// It's not clear how that could possibly be unreachable
//
// Note(tfoley): The diagnostic noted above arises because the `for`
// loop in question unconditionally exits on its first iteration.
// As a result the automatically-generated code for the "continue clause"
// (more or less the `operator++` on the iterator) is identified
// as unreachable code.
//
// The actual fix would be to make this code more explicit about its
// expectations. Either it expects there to be only a single entry
// point (in which case it should diagnose if that expectation is
// not met), or it just wants to query for the *first* entry point
// more explicitly, and include a comment explaining why that is valid.
//
#pragma warning(push)
#pragma warning(disable : 4702)
#endif
SlangResult CodeGenContext::emitEntryPointsSource(ComPtr<IArtifact>& outArtifact)
{
    outArtifact.setNull();

    SLANG_RETURN_ON_FAIL(requireTranslationUnitSourceFiles());

    auto endToEndReq = isPassThroughEnabled();
    if (endToEndReq)
    {
        for (auto entryPointIndex : getEntryPointIndices())
        {
            auto translationUnit = getPassThroughTranslationUnit(endToEndReq, entryPointIndex);
            SLANG_ASSERT(translationUnit);

            /// Make sure we have the source files
            SLANG_RETURN_ON_FAIL(translationUnit->requireSourceFiles());

            // Generate a string that includes the content of
            // the source file(s), along with a line directive
            // to ensure that we get reasonable messages
            // from the downstream compiler when in pass-through
            // mode.

            StringBuilder codeBuilder;
            if (getTargetFormat() == CodeGenTarget::GLSL)
            {
                // Special case GLSL
                int translationUnitCounter = 0;
                for (auto sourceFile : translationUnit->getSourceFiles())
                {
                    int translationUnitIndex = translationUnitCounter++;

                    // We want to output `#line` directives, but we need
                    // to skip this for the first file, since otherwise
                    // some GLSL implementations will get tripped up by
                    // not having the `#version` directive be the first
                    // thing in the file.
                    if (translationUnitIndex != 0)
                    {
                        codeBuilder << "#line 1 " << translationUnitIndex << "\n";
                    }
                    codeBuilder << sourceFile->getContent() << "\n";
                }
            }
            else
            {
                for (auto sourceFile : translationUnit->getSourceFiles())
                {
                    _appendCodeWithPath(
                        sourceFile->getPathInfo().foundPath.getUnownedSlice(),
                        sourceFile->getContent(),
                        codeBuilder);
                }
            }

            auto artifact =
                ArtifactUtil::createArtifactForCompileTarget(asExternal(getTargetFormat()));
            artifact->addRepresentationUnknown(StringBlob::moveCreate(codeBuilder));

            outArtifact.swap(artifact);
            return SLANG_OK;
        }
        return SLANG_OK;
    }
    else
    {
        return emitEntryPointsSourceFromIR(outArtifact);
    }
}
#if SLANG_VC
#pragma warning(pop)
#endif

SlangResult CodeGenContext::emitPrecompiledDownstreamIR(ComPtr<IArtifact>& outArtifact)
{
    return _emitEntryPoints(outArtifact);
}

static String _getDisplayPath(DiagnosticSink* sink, SourceFile* sourceFile)
{
    if (sink->isFlagSet(DiagnosticSink::Flag::VerbosePath))
    {
        return sourceFile->calcVerbosePath();
    }
    else
    {
        return sourceFile->getPathInfo().foundPath;
    }
}

String CodeGenContext::calcSourcePathForEntryPoints()
{
    String failureMode = "slang-generated";
    if (getEntryPointCount() != 1)
        return failureMode;
    auto entryPointIndex = getSingleEntryPointIndex();
    auto translationUnitRequest = findPassThroughTranslationUnit(entryPointIndex);
    if (!translationUnitRequest)
        return failureMode;

    const auto& sourceFiles = translationUnitRequest->getSourceFiles();

    auto sink = getSink();

    const Index numSourceFiles = sourceFiles.getCount();

    switch (numSourceFiles)
    {
    case 0:
        return "unknown";
    case 1:
        return _getDisplayPath(sink, sourceFiles[0]);
    default:
        {
            StringBuilder builder;
            builder << _getDisplayPath(sink, sourceFiles[0]);
            for (int i = 1; i < numSourceFiles; ++i)
            {
                builder << ";" << _getDisplayPath(sink, sourceFiles[i]);
            }
            return builder;
        }
    }
}

static RefPtr<ExtensionTracker> _newExtensionTracker(CodeGenTarget target)
{
    switch (target)
    {
    case CodeGenTarget::PTX:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
        {
            return new CUDAExtensionTracker;
        }
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::GLSL:
    case CodeGenTarget::WGSL:
    case CodeGenTarget::WGSLSPIRV:
    case CodeGenTarget::WGSLSPIRVAssembly:
        {
            return new ShaderExtensionTracker;
        }
    default:
        return nullptr;
    }
}

static CodeGenTarget _getDefaultSourceForTarget(CodeGenTarget target)
{
    switch (target)
    {
    case CodeGenTarget::ShaderHostCallable:
    case CodeGenTarget::ShaderSharedLibrary:
        {
            return CodeGenTarget::CPPSource;
        }
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::HostExecutable:
    case CodeGenTarget::HostSharedLibrary:
        {
            return CodeGenTarget::HostCPPSource;
        }
    case CodeGenTarget::PTX:
        return CodeGenTarget::CUDASource;
    case CodeGenTarget::DXBytecode:
        return CodeGenTarget::HLSL;
    case CodeGenTarget::DXIL:
        return CodeGenTarget::HLSL;
    case CodeGenTarget::SPIRV:
        return CodeGenTarget::GLSL;
    case CodeGenTarget::MetalLib:
        return CodeGenTarget::Metal;
    case CodeGenTarget::WGSLSPIRV:
        return CodeGenTarget::WGSL;
    default:
        break;
    }
    return CodeGenTarget::Unknown;
}

void trackGLSLTargetCaps(ShaderExtensionTracker* extensionTracker, CapabilitySet const& caps)
{
    for (auto& conjunctions : caps.getAtomSets())
    {
        for (auto atom : conjunctions)
        {
            switch (asAtom(atom))
            {
            default:
                break;

            case CapabilityAtom::glsl_spirv_1_0:
                extensionTracker->requireSPIRVVersion(SemanticVersion(1, 0));
                break;
            case CapabilityAtom::glsl_spirv_1_1:
                extensionTracker->requireSPIRVVersion(SemanticVersion(1, 1));
                break;
            case CapabilityAtom::glsl_spirv_1_2:
                extensionTracker->requireSPIRVVersion(SemanticVersion(1, 2));
                break;
            case CapabilityAtom::glsl_spirv_1_3:
                extensionTracker->requireSPIRVVersion(SemanticVersion(1, 3));
                break;
            case CapabilityAtom::glsl_spirv_1_4:
                extensionTracker->requireSPIRVVersion(SemanticVersion(1, 4));
                break;
            case CapabilityAtom::glsl_spirv_1_5:
                extensionTracker->requireSPIRVVersion(SemanticVersion(1, 5));
                break;
            case CapabilityAtom::glsl_spirv_1_6:
                extensionTracker->requireSPIRVVersion(SemanticVersion(1, 6));
                break;
            }
        }
    }
}

// True if it's best to use 'emitted' source for complication. For a downstream compiler
// that is not file based, this is always ok.
///
/// If the downstream compiler is file system based, we may want to just use the file that was
/// passed to be compiled. That the downstream compiler can determine if it will then save the file
/// or not based on if it's a match - and generally there will not be a match with emitted source.
///
/// This test is only used for pass through mode.
static bool _useEmittedSource(
    IDownstreamCompiler* compiler,
    TranslationUnitRequest* translationUnit)
{
    // We only bother if it's a file based compiler.
    if (compiler->isFileBased())
    {
        // It can only have *one* source file as otherwise we have to combine to make a new source
        // file anyway
        return translationUnit->getSourceArtifacts().getCount() != 1;
    }
    return true;
}

static bool _shouldSetEntryPointName(TargetProgram* targetProgram)
{
    if (!isKhronosTarget(targetProgram->getTargetReq()))
        return true;
    if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::VulkanUseEntryPointName))
        return true;
    return false;
}

static bool _isCPUHostTarget(CodeGenTarget target)
{
    auto desc = ArtifactDescUtil::makeDescForCompileTarget(asExternal(target));
    return desc.style == ArtifactStyle::Host;
}

SlangResult CodeGenContext::emitWithDownstreamForEntryPoints(ComPtr<IArtifact>& outArtifact)
{
    outArtifact.setNull();

    auto sink = getSink();
    auto session = getSession();

    CodeGenTarget sourceTarget = CodeGenTarget::None;
    SourceLanguage sourceLanguage = SourceLanguage::Unknown;

    auto target = getTargetFormat();
    RefPtr<ExtensionTracker> extensionTracker = _newExtensionTracker(target);
    PassThroughMode compilerType;

    SliceAllocator allocator;

    if (auto endToEndReq = isPassThroughEnabled())
    {
        compilerType = endToEndReq->m_passThrough;
    }
    else
    {
        // If we are not in pass through, lookup the default compiler for the emitted source type

        // Get the default source codegen type for a given target
        sourceTarget = _getDefaultSourceForTarget(target);
        compilerType = (PassThroughMode)session->getDownstreamCompilerForTransition(
            (SlangCompileTarget)sourceTarget,
            (SlangCompileTarget)target);
        // We should have a downstream compiler set at this point
        if (compilerType == PassThroughMode::None)
        {
            auto sourceName = TypeTextUtil::getCompileTargetName(SlangCompileTarget(sourceTarget));
            auto targetName = TypeTextUtil::getCompileTargetName(SlangCompileTarget(target));

            sink->diagnose(
                SourceLoc(),
                Diagnostics::compilerNotDefinedForTransition,
                sourceName,
                targetName);
            return SLANG_FAIL;
        }
    }

    SLANG_ASSERT(compilerType != PassThroughMode::None);

    // Get the required downstream compiler
    IDownstreamCompiler* compiler = session->getOrLoadDownstreamCompiler(compilerType, sink);
    if (!compiler)
    {
        auto compilerName = TypeTextUtil::getPassThroughAsHumanText((SlangPassThrough)compilerType);
        sink->diagnose(SourceLoc(), Diagnostics::passThroughCompilerNotFound, compilerName);
        return SLANG_FAIL;
    }

    Dictionary<String, String> preprocessorDefinitions;
    List<String> includePaths;

    typedef DownstreamCompileOptions CompileOptions;
    CompileOptions options;

    List<DownstreamCompileOptions::CapabilityVersion> requiredCapabilityVersions;
    List<String> compilerSpecificArguments;
    List<ComPtr<IArtifact>> libraries;
    List<String> libraryPaths;

    // Set compiler specific args
    {
        auto name = TypeTextUtil::getPassThroughName((SlangPassThrough)compilerType);
        List<String> downstreamArgs = getTargetProgram()->getOptionSet().getDownstreamArgs(name);
        for (const auto& arg : downstreamArgs)
        {
            // We special case some kinds of args, that can be handled directly
            if (arg.startsWith("-I"))
            {
                // We handle the -I option, by just adding to the include paths
                includePaths.add(arg.getUnownedSlice().tail(2));
            }
            else
            {
                compilerSpecificArguments.add(arg);
            }
        }
    }

    ComPtr<IArtifact> sourceArtifact;

    /* This is more convoluted than the other scenarios, because when we invoke C/C++ compiler we
    would ideally like to use the original file. We want to do this because we want includes
    relative to the source file to work, and for that to work most easily we want to use the
    original file, if there is one */
    if (auto endToEndReq = isPassThroughEnabled())
    {
        // If we are pass through, we may need to set extension tracker state.
        if (ShaderExtensionTracker* glslTracker = as<ShaderExtensionTracker>(extensionTracker))
        {
            trackGLSLTargetCaps(glslTracker, getTargetCaps());
        }

        auto translationUnit =
            getPassThroughTranslationUnit(endToEndReq, getSingleEntryPointIndex());

        // We are just passing thru, so it's whatever it originally was
        sourceLanguage = translationUnit->sourceLanguage;

        // TODO(JS): This seems like a bit of a hack
        // That if a pass-through is being performed and the source language is Slang
        // no downstream compiler knows how to deal with that, so probably means 'HLSL'
        sourceLanguage =
            (sourceLanguage == SourceLanguage::Slang) ? SourceLanguage::HLSL : sourceLanguage;
        sourceTarget = CodeGenTarget(TypeConvertUtil::getCompileTargetFromSourceLanguage(
            (SlangSourceLanguage)sourceLanguage));

        // If it's pass through we accumulate the preprocessor definitions.
        for (const auto& define :
             endToEndReq->getOptionSet().getArray(CompilerOptionName::MacroDefine))
            preprocessorDefinitions.add(define.stringValue, define.stringValue2);
        for (const auto& define : translationUnit->preprocessorDefinitions)
            preprocessorDefinitions.add(define);

        {
            /* TODO(JS): Not totally clear what options should be set here. If we are using the pass
            through - then using say the defines/includes all makes total sense. If we are
            generating C++ code from slang, then should we really be using these values -> aren't
            they what is being set for the *slang* source, not for the C++ generated code. That
            being the case it implies that there needs to be a mechanism (if there isn't already) to
            specify such information on a particular pass/pass through etc.

            On invoking DXC for example include paths do not appear to be set at all (even with
            pass-through).
            */

            auto linkage = getLinkage();

            // Add all the search paths

            const auto searchDirectories = linkage->getSearchDirectories();
            const SearchDirectoryList* searchList = &searchDirectories;
            while (searchList)
            {
                for (const auto& searchDirectory : searchList->searchDirectories)
                {
                    includePaths.add(searchDirectory.path);
                }
                searchList = searchList->parent;
            }
        }

        // If emitted source is required, emit and set the path
        if (_useEmittedSource(compiler, translationUnit))
        {
            CodeGenContext sourceCodeGenContext(this, sourceTarget, extensionTracker);

            SLANG_RETURN_ON_FAIL(sourceCodeGenContext.emitEntryPointsSource(sourceArtifact));

            // If it's not file based we can set an appropriate path name, and it doesn't matter if
            // it doesn't exist on the file system. We set the name to the path as this will be used
            // for downstream reporting.
            auto sourcePath = calcSourcePathForEntryPoints();
            sourceArtifact->setName(sourcePath.getBuffer());

            sourceCodeGenContext.maybeDumpIntermediate(sourceArtifact);
        }
        else
        {
            // Special case if we have a single file, so that we pass the path, and the contents as
            // is.
            const auto& sourceArtifacts = translationUnit->getSourceArtifacts();
            SLANG_ASSERT(sourceArtifacts.getCount() == 1);

            sourceArtifact = sourceArtifacts[0];
            SLANG_ASSERT(sourceArtifact);
        }
    }
    else
    {
        CodeGenContext sourceCodeGenContext(this, sourceTarget, extensionTracker);

        sourceCodeGenContext.removeAvailableInDownstreamIR = true;

        SLANG_RETURN_ON_FAIL(sourceCodeGenContext.emitEntryPointsSource(sourceArtifact));
        sourceCodeGenContext.maybeDumpIntermediate(sourceArtifact);

        sourceLanguage = (SourceLanguage)TypeConvertUtil::getSourceLanguageFromTarget(
            (SlangCompileTarget)sourceTarget);
    }

    if (sourceArtifact)
    {
        // Set the source artifacts
        options.sourceArtifacts = makeSlice(sourceArtifact.readRef(), 1);
    }

    // Add any preprocessor definitions associated with the linkage
    {
        // TODO(JS): This is somewhat arguable - should defines passed to Slang really be
        // passed to downstream compilers? It does appear consistent with the behavior if
        // there is an endToEndReq.
        //
        // That said it's very convenient and provides way to control aspects
        // of downstream compilation.

        for (const auto& define :
             getTargetProgram()->getOptionSet().getArray(CompilerOptionName::MacroDefine))
        {
            preprocessorDefinitions.addIfNotExists(define.stringValue, define.stringValue2);
        }
    }


    // If we have an extension tracker, we may need to set options such as SPIR-V version
    // and CUDA Shader Model.
    if (extensionTracker)
    {
        // Look for the version
        if (auto cudaTracker = as<CUDAExtensionTracker>(extensionTracker))
        {
            cudaTracker->finalize();

            if (cudaTracker->m_smVersion.isSet())
            {
                DownstreamCompileOptions::CapabilityVersion version;
                version.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
                version.version = cudaTracker->m_smVersion;

                requiredCapabilityVersions.add(version);
            }

            if (cudaTracker->isBaseTypeRequired(BaseType::Half))
            {
                options.flags |= CompileOptions::Flag::EnableFloat16;
            }
        }
        else if (ShaderExtensionTracker* glslTracker = as<ShaderExtensionTracker>(extensionTracker))
        {
            DownstreamCompileOptions::CapabilityVersion version;
            version.kind = DownstreamCompileOptions::CapabilityVersion::Kind::SPIRV;
            version.version = glslTracker->getSPIRVVersion();

            requiredCapabilityVersions.add(version);
        }
    }

    CapabilitySet targetCaps = getTargetCaps();
    for (auto atomSets : targetCaps.getAtomSets())
    {
        for (auto atomVal : atomSets)
        {
            auto atom = CapabilityAtom(atomVal);
            switch (atom)
            {
            default:
                break;

#define CASE(KIND, NAME, VERSION)                                                   \
    case CapabilityAtom::NAME:                                                      \
        requiredCapabilityVersions.add(DownstreamCompileOptions::CapabilityVersion{ \
            DownstreamCompileOptions::CapabilityVersion::Kind::KIND,                \
            VERSION});                                                              \
        break

                CASE(CUDASM, _cuda_sm_1_0, SemanticVersion(1, 0));
                CASE(CUDASM, _cuda_sm_2_0, SemanticVersion(2, 0));
                CASE(CUDASM, _cuda_sm_3_0, SemanticVersion(3, 0));
                CASE(CUDASM, _cuda_sm_4_0, SemanticVersion(4, 0));
                CASE(CUDASM, _cuda_sm_5_0, SemanticVersion(5, 0));
                CASE(CUDASM, _cuda_sm_6_0, SemanticVersion(6, 0));
                CASE(CUDASM, _cuda_sm_7_0, SemanticVersion(7, 0));
                CASE(CUDASM, _cuda_sm_8_0, SemanticVersion(8, 0));
                CASE(CUDASM, _cuda_sm_9_0, SemanticVersion(9, 0));

#undef CASE
            }
        }
    }

    // Set the file sytem and source manager, as *may* be used by downstream compiler
    options.fileSystemExt = getFileSystemExt();
    options.sourceManager = getSourceManager();

    // Set the source type
    options.sourceLanguage = SlangSourceLanguage(sourceLanguage);

    switch (target)
    {
    case CodeGenTarget::ShaderHostCallable:
    case CodeGenTarget::ShaderSharedLibrary:
        // Disable exceptions and security checks
        options.flags &=
            ~(CompileOptions::Flag::EnableExceptionHandling |
              CompileOptions::Flag::EnableSecurityChecks);
        break;
    }

    Profile profile;

    if (compilerType == PassThroughMode::Fxc || compilerType == PassThroughMode::Dxc ||
        compilerType == PassThroughMode::Glslang)
    {
        const auto entryPointIndices = getEntryPointIndices();
        auto targetReq = getTargetReq();

        const auto entryPointIndicesCount = entryPointIndices.getCount();

        // Whole program means
        // * can have 0-N entry points
        // * 'doesn't build into an executable/kernel'
        //
        // So in some sense it is a library
        if (getTargetProgram()->getOptionSet().getBoolOption(
                CompilerOptionName::GenerateWholeProgram))
        {
            if (compilerType == PassThroughMode::Dxc)
            {
                // Can support no entry points on DXC because we can build libraries
                profile =
                    Profile(getTargetProgram()->getOptionSet().getEnumOption<Profile::RawEnum>(
                        CompilerOptionName::Profile));
            }
            else
            {
                auto downstreamCompilerName =
                    TypeTextUtil::getPassThroughName((SlangPassThrough)compilerType);

                sink->diagnose(
                    SourceLoc(),
                    Diagnostics::downstreamCompilerDoesntSupportWholeProgramCompilation,
                    downstreamCompilerName);
                return SLANG_FAIL;
            }
        }
        else if (entryPointIndicesCount == 1)
        {
            // All support a single entry point
            const Index entryPointIndex = entryPointIndices[0];

            auto entryPoint = getEntryPoint(entryPointIndex);
            profile = getEffectiveProfile(entryPoint, targetReq);

            if (_shouldSetEntryPointName(getTargetProgram()))
            {
                options.entryPointName = allocator.allocate(getText(entryPoint->getName()));
                auto entryPointNameOverride =
                    getProgram()->getEntryPointNameOverride(entryPointIndex);
                if (entryPointNameOverride.getLength() != 0)
                {
                    options.entryPointName = allocator.allocate(entryPointNameOverride);
                }
            }
        }
        else
        {
            // We only support a single entry point on this target
            SLANG_ASSERT(!"Can only compile with a single entry point on this target");
            return SLANG_FAIL;
        }

        options.stage = SlangStage(profile.getStage());

        if (compilerType == PassThroughMode::Dxc)
        {
            // We will enable the flag to generate proper code for 16 - bit types
            // by default, as long as the user is requesting a sufficiently
            // high shader model.
            //
            // TODO: Need to check that this is safe to enable in all cases,
            // or if it will make a shader demand hardware features that
            // aren't always present.
            //
            // TODO: Ideally the dxc back-end should be passed some information
            // on the "capabilities" that were used and/or requested in the code.
            //
            if (profile.getVersion() >= ProfileVersion::DX_6_2)
            {
                options.flags |= CompileOptions::Flag::EnableFloat16;
            }

            // Set the matrix layout
            options.matrixLayout =
                (SlangMatrixLayoutMode)getTargetProgram()->getOptionSet().getMatrixLayoutMode();
        }

        // Set the profile
        options.profileName = allocator.allocate(getHLSLProfileName(profile));
    }

    // If we aren't using LLVM 'host callable', we want downstream compile to produce a shared
    // library
    if (compilerType != PassThroughMode::LLVM &&
        ArtifactDescUtil::makeDescForCompileTarget(asExternal(target)).kind ==
            ArtifactKind::HostCallable)
    {
        target = CodeGenTarget::ShaderSharedLibrary;
    }

    if (!isPassThroughEnabled())
    {
        if (_isCPUHostTarget(target))
        {
            libraryPaths.add(Path::getParentDirectory(Path::getExecutablePath()));
            libraryPaths.add(
                Path::combine(Path::getParentDirectory(Path::getExecutablePath()), "../lib"));

            // Set up the library artifact
            auto artifact = Artifact::create(
                ArtifactDesc::make(ArtifactKind::Library, Artifact::Payload::HostCPU),
                toSlice("slang-rt"));

            ComPtr<IOSFileArtifactRepresentation> fileRep(new OSFileArtifactRepresentation(
                IOSFileArtifactRepresentation::Kind::NameOnly,
                toSlice("slang-rt"),
                nullptr));
            artifact->addRepresentation(fileRep);

            libraries.add(artifact);
        }
    }

    options.targetType = (SlangCompileTarget)target;

    // Need to configure for the compilation

    {
        auto linkage = getLinkage();

        switch (getTargetProgram()->getOptionSet().getEnumOption<OptimizationLevel>(
            CompilerOptionName::Optimization))
        {
        case OptimizationLevel::None:
            options.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::None;
            break;
        case OptimizationLevel::Default:
            options.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::Default;
            break;
        case OptimizationLevel::High:
            options.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::High;
            break;
        case OptimizationLevel::Maximal:
            options.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::Maximal;
            break;
        default:
            SLANG_ASSERT(!"Unhandled optimization level");
            break;
        }

        switch (getTargetProgram()->getOptionSet().getEnumOption<DebugInfoLevel>(
            CompilerOptionName::DebugInformation))
        {
        case DebugInfoLevel::None:
            options.debugInfoType = DownstreamCompileOptions::DebugInfoType::None;
            break;
        case DebugInfoLevel::Minimal:
            options.debugInfoType = DownstreamCompileOptions::DebugInfoType::Minimal;
            break;

        case DebugInfoLevel::Standard:
            options.debugInfoType = DownstreamCompileOptions::DebugInfoType::Standard;
            break;
        case DebugInfoLevel::Maximal:
            options.debugInfoType = DownstreamCompileOptions::DebugInfoType::Maximal;
            break;
        default:
            SLANG_ASSERT(!"Unhandled debug level");
            break;
        }

        switch (getTargetProgram()->getOptionSet().getEnumOption<FloatingPointMode>(
            CompilerOptionName::FloatingPointMode))
        {
        case FloatingPointMode::Default:
            options.floatingPointMode = DownstreamCompileOptions::FloatingPointMode::Default;
            break;
        case FloatingPointMode::Precise:
            options.floatingPointMode = DownstreamCompileOptions::FloatingPointMode::Precise;
            break;
        case FloatingPointMode::Fast:
            options.floatingPointMode = DownstreamCompileOptions::FloatingPointMode::Fast;
            break;
        default:
            SLANG_ASSERT(!"Unhandled floating point mode");
        }

        if (getTargetProgram()->getOptionSet().hasOption(CompilerOptionName::DenormalModeFp16))
        {
            switch (getTargetProgram()->getOptionSet().getEnumOption<FloatingPointDenormalMode>(
                CompilerOptionName::DenormalModeFp16))
            {
            case FloatingPointDenormalMode::Any:
                options.denormalModeFp16 = DownstreamCompileOptions::FloatingPointDenormalMode::Any;
                break;
            case FloatingPointDenormalMode::Preserve:
                options.denormalModeFp16 =
                    DownstreamCompileOptions::FloatingPointDenormalMode::Preserve;
                break;
            case FloatingPointDenormalMode::FlushToZero:
                options.denormalModeFp16 =
                    DownstreamCompileOptions::FloatingPointDenormalMode::FlushToZero;
                break;
            default:
                SLANG_ASSERT(!"Unhandled fp16 denormal handling mode");
            }
        }

        if (getTargetProgram()->getOptionSet().hasOption(CompilerOptionName::DenormalModeFp32))
        {
            switch (getTargetProgram()->getOptionSet().getEnumOption<FloatingPointDenormalMode>(
                CompilerOptionName::DenormalModeFp32))
            {
            case FloatingPointDenormalMode::Any:
                options.denormalModeFp32 = DownstreamCompileOptions::FloatingPointDenormalMode::Any;
                break;
            case FloatingPointDenormalMode::Preserve:
                options.denormalModeFp32 =
                    DownstreamCompileOptions::FloatingPointDenormalMode::Preserve;
                break;
            case FloatingPointDenormalMode::FlushToZero:
                options.denormalModeFp32 =
                    DownstreamCompileOptions::FloatingPointDenormalMode::FlushToZero;
                break;
            default:
                SLANG_ASSERT(!"Unhandled fp32 denormal handling mode");
            }
        }

        if (getTargetProgram()->getOptionSet().hasOption(CompilerOptionName::DenormalModeFp64))
        {
            switch (getTargetProgram()->getOptionSet().getEnumOption<FloatingPointDenormalMode>(
                CompilerOptionName::DenormalModeFp64))
            {
            case FloatingPointDenormalMode::Any:
                options.denormalModeFp64 = DownstreamCompileOptions::FloatingPointDenormalMode::Any;
                break;
            case FloatingPointDenormalMode::Preserve:
                options.denormalModeFp64 =
                    DownstreamCompileOptions::FloatingPointDenormalMode::Preserve;
                break;
            case FloatingPointDenormalMode::FlushToZero:
                options.denormalModeFp64 =
                    DownstreamCompileOptions::FloatingPointDenormalMode::FlushToZero;
                break;
            default:
                SLANG_ASSERT(!"Unhandled fp64 denormal handling mode");
            }
        }

        {
            // We need to look at the stage of the entry point(s) we are
            // being asked to compile, since this will determine the
            // "pipeline" that the result should be compiled for (e.g.,
            // compute vs. ray tracing).
            //
            // TODO: This logic is kind of messy in that it assumes
            // a program to be compiled will only contain kernels for
            // a single pipeline type, but that invariant isn't expressed
            // at all in the front-end today. It also has no error
            // checking for the case where there are conflicts.
            //
            // HACK: Right now none of the above concerns matter
            // because we always perform code generation on a single
            // entry point at a time.
            //
            Index entryPointCount = getEntryPointCount();
            for (Index ee = 0; ee < entryPointCount; ++ee)
            {
                auto stage = getEntryPoint(ee)->getStage();
                switch (stage)
                {
                default:
                    break;

                case Stage::Compute:
                    options.pipelineType = DownstreamCompileOptions::PipelineType::Compute;
                    break;

                case Stage::Vertex:
                case Stage::Hull:
                case Stage::Domain:
                case Stage::Geometry:
                case Stage::Fragment:
                    options.pipelineType = DownstreamCompileOptions::PipelineType::Rasterization;
                    break;

                case Stage::RayGeneration:
                case Stage::Intersection:
                case Stage::AnyHit:
                case Stage::ClosestHit:
                case Stage::Miss:
                case Stage::Callable:
                    options.pipelineType = DownstreamCompileOptions::PipelineType::RayTracing;
                    break;
                }
            }
        }

        // Add all the search paths (as calculated earlier - they will only be set if this is a pass
        // through else will be empty)
        options.includePaths = allocator.allocate(includePaths);

        // Add the specified defines (as calculated earlier - they will only be set if this is a
        // pass through else will be empty)
        {
            const auto count = preprocessorDefinitions.getCount();
            auto dst = allocator.getArena().allocateArray<DownstreamCompileOptions::Define>(count);

            Index i = 0;

            for (const auto& [defKey, defValue] : preprocessorDefinitions)
            {
                auto& define = dst[i];

                define.nameWithSig = allocator.allocate(defKey);
                define.value = allocator.allocate(defValue);

                ++i;
            }
            options.defines = makeSlice(dst, count);
        }

        // Add all of the module libraries
        libraries.addRange(linkage->m_libModules.getBuffer(), linkage->m_libModules.getCount());
    }

    auto program = getProgram();

    // Load embedded precompiled libraries from IR into library artifacts
    program->enumerateIRModules(
        [&](IRModule* irModule)
        {
            for (auto globalInst : irModule->getModuleInst()->getChildren())
            {
                if (target == CodeGenTarget::DXILAssembly || target == CodeGenTarget::DXIL)
                {
                    if (auto inst = as<IREmbeddedDownstreamIR>(globalInst))
                    {
                        if (inst->getTarget() == CodeGenTarget::DXIL)
                        {
                            auto slice = inst->getBlob()->getStringSlice();
                            ArtifactDesc desc =
                                ArtifactDescUtil::makeDescForCompileTarget(SLANG_DXIL);
                            desc.kind = ArtifactKind::Library;

                            auto library = ArtifactUtil::createArtifact(desc);

                            library->addRepresentationUnknown(StringBlob::create(slice));
                            libraries.add(library);
                        }
                    }
                }
            }
        });

    options.compilerSpecificArguments = allocator.allocate(compilerSpecificArguments);
    options.requiredCapabilityVersions = SliceUtil::asSlice(requiredCapabilityVersions);
    options.libraries = SliceUtil::asSlice(libraries);
    options.libraryPaths = allocator.allocate(libraryPaths);

    if (m_targetProfile.getFamily() == ProfileFamily::DX)
    {
        options.enablePAQ = m_targetProfile.getVersion() >= ProfileVersion::DX_6_7;
    }

    // Compile
    ComPtr<IArtifact> artifact;
    auto downstreamStartTime = std::chrono::high_resolution_clock::now();
    SlangResult compileResult = compiler->compile(options, artifact.writeRef());
    auto downstreamElapsedTime =
        (std::chrono::high_resolution_clock::now() - downstreamStartTime).count() * 0.000000001;
    getSession()->addDownstreamCompileTime(downstreamElapsedTime);

    // Extract diagnostics regardless of compile result
    SLANG_RETURN_ON_FAIL(passthroughDownstreamDiagnostics(getSink(), compiler, artifact));

    // Now check if compile failed
    SLANG_RETURN_ON_FAIL(compileResult);

    // Copy over all of the information associated with the source into the output
    if (sourceArtifact)
    {
        for (auto associatedArtifact : sourceArtifact->getAssociated())
        {
            artifact->addAssociated(associatedArtifact);
        }
    }

    // Set the artifact
    outArtifact.swap(artifact);
    return SLANG_OK;
}

SlangResult emitSPIRVForEntryPointsDirectly(
    CodeGenContext* codeGenContext,
    ComPtr<IArtifact>& outArtifact);

SlangResult emitHostVMCode(CodeGenContext* codeGenContext, ComPtr<IArtifact>& outArtifact);

SlangResult emitLLVMForEntryPoints(CodeGenContext* codeGenContext, ComPtr<IArtifact>& outArtifact);

static CodeGenTarget _getIntermediateTarget(CodeGenTarget target)
{
    switch (target)
    {
    case CodeGenTarget::DXBytecodeAssembly:
        return CodeGenTarget::DXBytecode;
    case CodeGenTarget::DXILAssembly:
        return CodeGenTarget::DXIL;
    case CodeGenTarget::SPIRVAssembly:
        return CodeGenTarget::SPIRV;
    case CodeGenTarget::WGSLSPIRVAssembly:
        return CodeGenTarget::WGSLSPIRV;
    default:
        return CodeGenTarget::None;
    }
}

IArtifact* getSeparateDbgArtifact(IArtifact* artifact)
{
    if (!artifact)
        return nullptr;

    // The first associated artifact of kind ObjectCode and SPIRV payload should be the debug
    // artifact.
    for (auto* associated : artifact->getAssociated())
    {
        auto desc = associated->getDesc();
        if (desc.kind == ArtifactKind::ObjectCode && desc.payload == ArtifactPayload::SPIRV)
            return associated;
    }

    return nullptr;
}

/// Function to simplify the logic around emitting, and dissassembling
SlangResult CodeGenContext::_emitEntryPoints(ComPtr<IArtifact>& outArtifact)
{
    auto target = getTargetFormat();
    switch (target)
    {
    case CodeGenTarget::SPIRVAssembly:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::DXILAssembly:
    case CodeGenTarget::MetalLibAssembly:
    case CodeGenTarget::WGSLSPIRVAssembly:
        {
            // First compile to an intermediate target for the corresponding binary format.
            const CodeGenTarget intermediateTarget = _getIntermediateTarget(target);
            CodeGenContext intermediateContext(this, intermediateTarget);

            ComPtr<IArtifact> intermediateArtifact;

            SLANG_RETURN_ON_FAIL(intermediateContext._emitEntryPoints(intermediateArtifact));
            intermediateContext.maybeDumpIntermediate(intermediateArtifact);

            // Then disassemble the intermediate binary result to get the desired output
            // Output the disassemble
            ComPtr<IArtifact> disassemblyArtifact;
            SLANG_RETURN_ON_FAIL(ArtifactOutputUtil::dissassembleWithDownstream(
                getSession(),
                intermediateArtifact,
                getSink(),
                disassemblyArtifact.writeRef()));

            // Also disassemble the debug artifact if one exists.
            auto debugArtifact = getSeparateDbgArtifact(intermediateArtifact);
            ComPtr<IArtifact> disassemblyDebugArtifact;
            if (debugArtifact)
            {
                SLANG_RETURN_ON_FAIL(ArtifactOutputUtil::dissassembleWithDownstream(
                    getSession(),
                    debugArtifact,
                    getSink(),
                    disassemblyDebugArtifact.writeRef()));
                disassemblyDebugArtifact->setName(debugArtifact->getName());

                // The disassembly needs both the metadata for the debug build identifier
                // and the debug spirv to be associated with is.
                for (auto associated : intermediateArtifact->getAssociated())
                {
                    if (associated->getDesc().payload == ArtifactPayload::Metadata ||
                        associated->getDesc().payload == ArtifactPayload::PostEmitMetadata)
                    {
                        disassemblyArtifact->addAssociated(associated);
                        break;
                    }
                }
                disassemblyArtifact->addAssociated(disassemblyDebugArtifact);
            }

            outArtifact.swap(disassemblyArtifact);
            return SLANG_OK;
        }
    case CodeGenTarget::SPIRV:
        if (getTargetProgram()->getOptionSet().shouldEmitSPIRVDirectly())
        {
            SLANG_RETURN_ON_FAIL(emitSPIRVForEntryPointsDirectly(this, outArtifact));
            return SLANG_OK;
        }
        [[fallthrough]];
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::MetalLib:
    case CodeGenTarget::PTX:
    case CodeGenTarget::WGSLSPIRV:
        SLANG_RETURN_ON_FAIL(emitWithDownstreamForEntryPoints(outArtifact));
        return SLANG_OK;
    case CodeGenTarget::ShaderSharedLibrary:
    case CodeGenTarget::HostExecutable:
    case CodeGenTarget::HostSharedLibrary:
    case CodeGenTarget::ShaderHostCallable:
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::HostLLVMIR:
    case CodeGenTarget::ShaderLLVMIR:
    case CodeGenTarget::HostObjectCode:
    case CodeGenTarget::ShaderObjectCode:
        if (isCPUTargetViaLLVM(getTargetReq()))
        {
            SLANG_RETURN_ON_FAIL(emitLLVMForEntryPoints(this, outArtifact));
        }
        else
        {
            SLANG_RETURN_ON_FAIL(emitWithDownstreamForEntryPoints(outArtifact));
        }
        return SLANG_OK;
    case CodeGenTarget::HostVM:
        SLANG_RETURN_ON_FAIL(emitHostVMCode(this, outArtifact));
        return SLANG_OK;
    default:
        break;
    }

    return SLANG_FAIL;
}

// Helper class for recording compile time.
struct CompileTimerRAII
{
    std::chrono::high_resolution_clock::time_point startTime;
    Session* session;
    CompileTimerRAII(Session* inSession)
    {
        startTime = std::chrono::high_resolution_clock::now();
        session = inSession;
    }
    ~CompileTimerRAII()
    {
        double elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(
                                 std::chrono::high_resolution_clock::now() - startTime)
                                 .count() /
                             1e6;
        session->addTotalCompileTime(elapsedTime);
    }
};

// Do emit logic for a zero or more entry points
SlangResult CodeGenContext::emitEntryPoints(ComPtr<IArtifact>& outArtifact)
{
    CompileTimerRAII recordCompileTime(getSession());

    auto target = getTargetFormat();

    switch (target)
    {
    case CodeGenTarget::SPIRVAssembly:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::DXILAssembly:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::MetalLib:
    case CodeGenTarget::MetalLibAssembly:
    case CodeGenTarget::PTX:
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::ShaderHostCallable:
    case CodeGenTarget::ShaderSharedLibrary:
    case CodeGenTarget::HostExecutable:
    case CodeGenTarget::HostSharedLibrary:
    case CodeGenTarget::WGSLSPIRVAssembly:
    case CodeGenTarget::HostVM:
    case CodeGenTarget::HostObjectCode:
    case CodeGenTarget::ShaderObjectCode:
    case CodeGenTarget::HostLLVMIR:
    case CodeGenTarget::ShaderLLVMIR:
        {
            SLANG_RETURN_ON_FAIL(_emitEntryPoints(outArtifact));

            maybeDumpIntermediate(outArtifact);
            return SLANG_OK;
        }
        break;
    case CodeGenTarget::GLSL:
    case CodeGenTarget::HLSL:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::HostCPPSource:
    case CodeGenTarget::PyTorchCppBinding:
    case CodeGenTarget::CSource:
    case CodeGenTarget::Metal:
    case CodeGenTarget::WGSL:
        {
            RefPtr<ExtensionTracker> extensionTracker = _newExtensionTracker(target);

            CodeGenContext subContext(this, target, extensionTracker);

            ComPtr<IArtifact> sourceArtifact;

            SLANG_RETURN_ON_FAIL(subContext.emitEntryPointsSource(sourceArtifact));

            subContext.maybeDumpIntermediate(sourceArtifact);
            outArtifact = sourceArtifact;
            return SLANG_OK;
        }
        break;

    case CodeGenTarget::None:
        // The user requested no output
        return SLANG_OK;

        // Note(tfoley): We currently hit this case when compiling the core module
    case CodeGenTarget::Unknown:
        return SLANG_OK;

    default:
        SLANG_UNEXPECTED("unhandled code generation target");
        break;
    }
    return SLANG_FAIL;
}

void CodeGenContext::_dumpIntermediateMaybeWithAssembly(IArtifact* artifact)
{
    _dumpIntermediate(artifact);

    ComPtr<IArtifact> assembly;
    ArtifactOutputUtil::maybeDisassemble(getSession(), artifact, nullptr, assembly);

    if (assembly)
    {
        _dumpIntermediate(assembly);
    }
}

void CodeGenContext::_dumpIntermediate(IArtifact* artifact)
{
    ComPtr<ISlangBlob> blob;
    if (SLANG_FAILED(artifact->loadBlob(ArtifactKeep::No, blob.writeRef())))
    {
        return;
    }
    _dumpIntermediate(artifact->getDesc(), blob->getBufferPointer(), blob->getBufferSize());
}

void CodeGenContext::_dumpIntermediate(const ArtifactDesc& desc, void const* data, size_t size)
{
    // Try to generate a unique ID for the file to dump,
    // even in cases where there might be multiple threads
    // doing compilation.
    //
    // This is primarily a debugging aid, so we don't
    // really need/want to do anything too elaborate

    static std::atomic<uint32_t> counter(0);

    const uint32_t id = ++counter;

    // Just use the counter for the 'base name'
    StringBuilder basename;

    // Add the prefix
    basename << getIntermediateDumpPrefix();

    // Add the id
    basename << int(id);

    // Work out the filename based on the desc and the basename
    StringBuilder filename;
    ArtifactDescUtil::calcNameForDesc(desc, basename.getUnownedSlice(), filename);

    // If didn't produce a filename, use basename with .unknown extension
    if (filename.getLength() == 0)
    {
        filename = basename;
        filename << ".unknown";
    }

    // Write to a file
    ArtifactOutputUtil::writeToFile(desc, data, size, filename);
}

void CodeGenContext::maybeDumpIntermediate(IArtifact* artifact)
{
    if (!shouldDumpIntermediates())
        return;


    _dumpIntermediateMaybeWithAssembly(artifact);
}

IRDumpOptions CodeGenContext::getIRDumpOptions()
{
    if (auto endToEndReq = isEndToEndCompile())
    {
        return endToEndReq->getFrontEndReq()->m_irDumpOptions;
    }
    return IRDumpOptions();
}

bool CodeGenContext::shouldValidateIR()
{
    return getTargetProgram()->getOptionSet().getBoolOption(CompilerOptionName::ValidateIr);
}

bool CodeGenContext::shouldSkipSPIRVValidation()
{
    return getTargetProgram()->getOptionSet().getBoolOption(
        CompilerOptionName::SkipSPIRVValidation);
}

bool CodeGenContext::shouldDumpIR()
{
    return getTargetProgram()->getOptionSet().getBoolOption(CompilerOptionName::DumpIr);
}

bool CodeGenContext::shouldSkipDownstreamLinking()
{
    return getTargetProgram()->getOptionSet().getBoolOption(
        CompilerOptionName::SkipDownstreamLinking);
}

bool CodeGenContext::shouldReportCheckpointIntermediates()
{
    return getTargetProgram()->getOptionSet().getBoolOption(
        CompilerOptionName::ReportCheckpointIntermediates);
}

bool CodeGenContext::shouldReportDynamicDispatchSites()
{
    return getTargetProgram()->getOptionSet().getBoolOption(
        CompilerOptionName::ReportDynamicDispatchSites);
}

bool CodeGenContext::shouldDumpIntermediates()
{
    return getTargetProgram()->getOptionSet().getBoolOption(CompilerOptionName::DumpIntermediates);
}

bool CodeGenContext::shouldTrackLiveness()
{
    return getTargetProgram()->getOptionSet().getBoolOption(CompilerOptionName::TrackLiveness);
}

String CodeGenContext::getIntermediateDumpPrefix()
{
    return getTargetProgram()->getOptionSet().getStringOption(
        CompilerOptionName::DumpIntermediatePrefix);
}

bool CodeGenContext::getUseUnknownImageFormatAsDefault()
{
    return getTargetProgram()->getOptionSet().getBoolOption(
        CompilerOptionName::DefaultImageFormatUnknown);
}

bool CodeGenContext::isSpecializationDisabled()
{
    return getTargetProgram()->getOptionSet().getBoolOption(
        CompilerOptionName::DisableSpecialization);
}

SlangResult CodeGenContext::requireTranslationUnitSourceFiles()
{
    if (auto endToEndReq = isPassThroughEnabled())
    {
        for (auto entryPointIndex : getEntryPointIndices())
        {
            auto translationUnit = getPassThroughTranslationUnit(endToEndReq, entryPointIndex);
            SLANG_ASSERT(translationUnit);
            /// Make sure we have the source files
            SLANG_RETURN_ON_FAIL(translationUnit->requireSourceFiles());
        }
    }

    return SLANG_OK;
}

} // namespace Slang
