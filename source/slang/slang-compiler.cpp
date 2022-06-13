// Compiler.cpp : Defines the entry point for the console application.
//
#include "../core/slang-basic.h"
#include "../core/slang-platform.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"
#include "../core/slang-hex-dump-util.h"
#include "../core/slang-riff.h"
#include "../core/slang-type-text-util.h"
#include "../core/slang-type-convert-util.h"

#include "slang-check.h"
#include "slang-compiler.h"

#include "../compiler-core/slang-lexer.h"
#include "../compiler-core/slang-artifact.h"

#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-parameter-binding.h"
#include "slang-parser.h"
#include "slang-preprocessor.h"
#include "slang-type-layout.h"

#include "slang-glsl-extension-tracker.h"
#include "slang-emit-cuda.h"

#include "slang-serialize-container.h"
//


// Includes to allow us to control console
// output when writing assembly dumps.
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

namespace Slang
{

    // !!!!!!!!!!!!!!!!!!!!!! free functions for DiagnosicSink !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool isHeterogeneousTarget(CodeGenTarget target)
{
    return ArtifactDesc::makeFromCompileTarget(asExternal(target)).style == ArtifactStyle::Host;
}

void printDiagnosticArg(StringBuilder& sb, CodeGenTarget val)
    {
        switch (val)
        {
            default:
                sb << "<unknown>";
                break;

    #define CASE(TAG, STR) case CodeGenTarget::TAG: sb << STR; break
                CASE(GLSL, "glsl");
                CASE(HLSL, "hlsl");
                CASE(SPIRV, "spirv");
                CASE(SPIRVAssembly, "spriv-assembly");
                CASE(DXBytecode, "dxbc");
                CASE(DXBytecodeAssembly, "dxbc-assembly");
                CASE(DXIL, "dxil");
                CASE(DXILAssembly, "dxil-assembly");
    #undef CASE
        }
    }

    void printDiagnosticArg(StringBuilder& sb, PassThroughMode val)
    {
        sb << TypeTextUtil::getPassThroughName(SlangPassThrough(val));
    }


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! CompileResult !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    SlangResult CompileResult::getSharedLibrary(ComPtr<ISlangSharedLibrary>& outSharedLibrary)
    {
        if (downstreamResult)
        {
            return downstreamResult->getHostCallableSharedLibrary(outSharedLibrary);
        }
        return SLANG_FAIL;
    }

    SlangResult CompileResult::getBlob(ComPtr<ISlangBlob>& outBlob) const
    {
        if(!blob)
        {
            switch(format)
            {
                default:
                case ResultFormat::None:
                {
                    // If no blob is returned, it's an error
                    return SLANG_FAIL;
                }
                case ResultFormat::Text:
                {
                    blob = StringUtil::createStringBlob(outputString);
                    break;
                }
                case ResultFormat::Binary:
                {
                    if (downstreamResult)
                    {
                        // TODO(JS): 
                        // This seems a little questionable. As it stands downstreamResult, if it doesn't have a blob
                        // can try and read a file. How this currently works is that every getBlob will potentially try to read that file.
                        // Setting result to None would stop this, but is that reasonable as the state.
                        // Perhaps downstreamResult should hold some state that the read failed.
                        // For now we don't worry though.

                        SLANG_RETURN_ON_FAIL(downstreamResult->getBinary(blob));
                    }
                    break;
                }
            }
        }

        outBlob = blob;
        return SLANG_OK;
    }

    SlangResult CompileResult::isParameterLocationUsed(SlangParameterCategory category, UInt spaceIndex, UInt registerIndex, bool& outUsed)
    {
        if (!postEmitMetadata)
            return SLANG_E_NOT_AVAILABLE;

        if (!ShaderBindingRange::isUsageTracked((slang::ParameterCategory)category))
            return SLANG_E_NOT_AVAILABLE;

        // TODO: optimize this with a binary search through a sorted list
        for (const auto& range : postEmitMetadata->usedBindings)
        {
            if (range.containsBinding((slang::ParameterCategory)category, spaceIndex, registerIndex))
            {
                outUsed = true;
                return SLANG_OK;
            }
        }
        
        outUsed = false;
        return SLANG_OK;
    }

    //
    // FrontEndEntryPointRequest
    //

    FrontEndEntryPointRequest::FrontEndEntryPointRequest(
        FrontEndCompileRequest* compileRequest,
        int                     translationUnitIndex,
        Name*                   name,
        Profile                 profile)
        : m_compileRequest(compileRequest)
        , m_translationUnitIndex(translationUnitIndex)
        , m_name(name)
        , m_profile(profile)
    {}


    TranslationUnitRequest* FrontEndEntryPointRequest::getTranslationUnit()
    {
        return getCompileRequest()->translationUnits[m_translationUnitIndex];
    }

    //
    // EntryPoint
    //

    ISlangUnknown* EntryPoint::getInterface(const Guid& guid)
    {
        if(guid == slang::IEntryPoint::getTypeGuid())
            return static_cast<slang::IEntryPoint*>(this);

        return Super::getInterface(guid);
    }

    RefPtr<EntryPoint> EntryPoint::create(
        Linkage*            linkage,
        DeclRef<FuncDecl>   funcDeclRef,
        Profile             profile)
    {
        RefPtr<EntryPoint> entryPoint = new EntryPoint(
            linkage,
            funcDeclRef.getName(),
            profile,
            funcDeclRef);
        entryPoint->m_mangledName = getMangledName(linkage->getASTBuilder(), funcDeclRef);
        return entryPoint;
    }

    RefPtr<EntryPoint> EntryPoint::createDummyForPassThrough(
        Linkage*    linkage,
        Name*       name,
        Profile     profile)
    {
        RefPtr<EntryPoint> entryPoint = new EntryPoint(
            linkage,
            name,
            profile,
            DeclRef<FuncDecl>());
        return entryPoint;
    }

    RefPtr<EntryPoint> EntryPoint::createDummyForDeserialize(
        Linkage*    linkage,
        Name*       name,
        Profile     profile,
        String      mangledName)
    {
        RefPtr<EntryPoint> entryPoint = new EntryPoint(
            linkage,
            name,
            profile,
            DeclRef<FuncDecl>());
        entryPoint->m_mangledName = mangledName;
        return entryPoint;
    }

    EntryPoint::EntryPoint(
        Linkage*            linkage,
        Name*               name,
        Profile             profile,
        DeclRef<FuncDecl>   funcDeclRef)
        : ComponentType(linkage)
        , m_name(name)
        , m_profile(profile)
        , m_funcDeclRef(funcDeclRef)
    {
        // Collect any specialization parameters used by the entry point
        //
        _collectShaderParams();
    }

    Module* EntryPoint::getModule()
    {
        return Slang::getModule(getFuncDecl());
    }

    Index EntryPoint::getSpecializationParamCount()
    {
        return m_genericSpecializationParams.getCount() + m_existentialSpecializationParams.getCount();
    }

    SpecializationParam const& EntryPoint::getSpecializationParam(Index index)
    {
        auto genericParamCount = m_genericSpecializationParams.getCount();
        if(index < genericParamCount)
        {
            return m_genericSpecializationParams[index];
        }
        else
        {
            return m_existentialSpecializationParams[index - genericParamCount];
        }
    }

    Index EntryPoint::getRequirementCount()
    {
        // The only requirement of an entry point is the module that contains it.
        //
        // TODO: We will eventually want to support the case of an entry
        // point nested in a `struct` type, in which case there should be
        // a single requirement representing that outer type (so that multiple
        // entry points nested under the same type can share the storage
        // for parameters at that scope).

        // Note: the defensive coding is here because the
        // "dummy" entry points we create for pass-through
        // compilation will not have an associated module.
        //
        if( auto module = getModule() )
        {
            return 1;
        }
        return 0;
    }

    RefPtr<ComponentType> EntryPoint::getRequirement(Index index)
    {
        SLANG_UNUSED(index);
        SLANG_ASSERT(index == 0);
        SLANG_ASSERT(getModule());
        return getModule();
    }

    String EntryPoint::getEntryPointMangledName(Index index)
    {
        SLANG_UNUSED(index);
        SLANG_ASSERT(index == 0);

        return m_mangledName;
    }

    String EntryPoint::getEntryPointNameOverride(Index index)
    {
        SLANG_UNUSED(index);
        SLANG_ASSERT(index == 0);

        return m_name ? m_name->text : "";
    }

    void EntryPoint::acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
    {
        visitor->visitEntryPoint(this, as<EntryPointSpecializationInfo>(specializationInfo));
    }

    List<Module*> const& EntryPoint::getModuleDependencies()
    {
        if(auto module = getModule())
            return module->getModuleDependencies();

        static List<Module*> empty;
        return empty;
    }

    List<String> const& EntryPoint::getFilePathDependencies()
    {
        if(auto module = getModule())
            return getModule()->getFilePathDependencies();
        
        static List<String> empty;
        return empty;
    }

    TypeConformance::TypeConformance(
        Linkage* linkage,
        SubtypeWitness* witness,
        Int confomrmanceIdOverride,
        DiagnosticSink* sink)
        : ComponentType(linkage)
        , m_subtypeWitness(witness)
        , m_conformanceIdOverride(confomrmanceIdOverride)
    {
        addDepedencyFromWitness(witness);
        m_irModule = generateIRForTypeConformance(this, m_conformanceIdOverride, sink);
    }

    void TypeConformance::addDepedencyFromWitness(SubtypeWitness* witness)
    {
        if (auto declaredWitness = as<DeclaredSubtypeWitness>(witness))
        {
            auto declModule = getModule(declaredWitness->declRef.getDecl());
            m_moduleDependency.addDependency(declModule);
            m_pathDependency.addDependency(declModule);
            if (m_requirementSet.Add(declModule))
            {
                m_requirements.add(declModule);
            }
            // TODO: handle the specialization arguments in declaredWitness->declRef.substitutions.
        }
        else if (auto transitiveWitness = as<TransitiveSubtypeWitness>(witness))
        {
            addDepedencyFromWitness(transitiveWitness->midToSup);
            addDepedencyFromWitness(transitiveWitness->subToMid);
        }
        else if (auto conjunctionWitness = as<ConjunctionSubtypeWitness>(witness))
        {
            auto left = as<SubtypeWitness>(conjunctionWitness->leftWitness);
            if (left)
                addDepedencyFromWitness(left);
            auto right = as<SubtypeWitness>(conjunctionWitness->rightWitness);
            if (right)
                addDepedencyFromWitness(right);
        }
    }

    ISlangUnknown* TypeConformance::getInterface(const Guid& guid)
    {
        if (guid == slang::ITypeConformance::getTypeGuid())
            return static_cast<slang::ITypeConformance*>(this);

        return Super::getInterface(guid);
    }

    List<Module*> const& TypeConformance::getModuleDependencies()
    {
        return m_moduleDependency.getModuleList();
    }

    List<String> const& TypeConformance::getFilePathDependencies()
    {
        return m_pathDependency.getFilePathList();
    }

    Index TypeConformance::getRequirementCount() { return m_requirements.getCount(); }

    RefPtr<ComponentType> TypeConformance::getRequirement(Index index)
    {
        return m_requirements[index];
    }

    void TypeConformance::acceptVisitor(
        ComponentTypeVisitor* visitor,
        ComponentType::SpecializationInfo* specializationInfo)
    {
        SLANG_UNUSED(specializationInfo);
        visitor->visitTypeConformance(this);
    }

    RefPtr<ComponentType::SpecializationInfo> TypeConformance::_validateSpecializationArgsImpl(
        SpecializationArg const* args,
        Index argCount,
        DiagnosticSink* sink)
    {
        SLANG_UNUSED(args);
        SLANG_UNUSED(argCount);
        SLANG_UNUSED(sink);
        return nullptr;
    }

    //

    Profile Profile::lookUp(UnownedStringSlice const& name)
    {
        #define PROFILE(TAG, NAME, STAGE, VERSION)	if(name == UnownedTerminatedStringSlice(#NAME)) return Profile::TAG;
        #define PROFILE_ALIAS(TAG, DEF, NAME)		if(name == UnownedTerminatedStringSlice(#NAME)) return Profile::TAG;
        #include "slang-profile-defs.h"

        return Profile::Unknown;
    }

    Profile Profile::lookUp(char const* name)
    {
        return lookUp(UnownedTerminatedStringSlice(name));
    }

    char const* Profile::getName()
    {
        switch( raw )
        {
        default:
            return "unknown";

        #define PROFILE(TAG, NAME, STAGE, VERSION) case Profile::TAG: return #NAME;
        #define PROFILE_ALIAS(TAG, DEF, NAME) /* empty */
        #include "slang-profile-defs.h"
        }
    }

    static const struct
    {
        char const* name;
        Stage       stage;
    } kStages[] =
    {
    #define PROFILE_STAGE(ID, NAME, ENUM) \
            { #NAME,    Stage::ID },

        #define PROFILE_STAGE_ALIAS(ID, NAME, VAL) \
            { #NAME,    Stage::ID },

        #include "slang-profile-defs.h"
    };

    Stage findStageByName(String const& name)
    {
        for(auto entry : kStages)
        {
            if(name == entry.name)
            {
                return entry.stage;
            }
        }

        return Stage::Unknown;
    }

    UnownedStringSlice getStageText(Stage stage)
    {
        for (auto entry : kStages)
        {
            if (stage == entry.stage)
            {
                return UnownedStringSlice(entry.name);
            }
        }
        return UnownedStringSlice();
    }

    SlangResult checkExternalCompilerSupport(Session* session, PassThroughMode passThrough)
    {
        // Check if the type is supported on this compile
        if (passThrough == PassThroughMode::None)
        {
            // If no pass through -> that will always work!
            return SLANG_OK;
        }

        return session->getOrLoadDownstreamCompiler(passThrough, nullptr) ? SLANG_OK: SLANG_E_NOT_FOUND;
    }

    SourceLanguage getDefaultSourceLanguageForDownstreamCompiler(PassThroughMode compiler)
    {
        switch (compiler)
        {
            case PassThroughMode::None:
            {
                return SourceLanguage::Unknown;
            }
            case PassThroughMode::Fxc:   
            case PassThroughMode::Dxc:
            {
                return SourceLanguage::HLSL;
            }
            case PassThroughMode::Glslang:
            {
                return SourceLanguage::GLSL;
            }
            case PassThroughMode::LLVM:
            case PassThroughMode::Clang:
            case PassThroughMode::VisualStudio:
            case PassThroughMode::Gcc:
            case PassThroughMode::GenericCCpp:
            {
                // These could ingest C, but we only have this function to work out a
                // 'default' language to ingest.
                return SourceLanguage::CPP;
            }
            case PassThroughMode::NVRTC:
            {
                return SourceLanguage::CUDA;
            }
            
            default: break;
        }
        SLANG_ASSERT(!"Unknown compiler");
        return SourceLanguage::Unknown;
    }

    PassThroughMode getDownstreamCompilerRequiredForTarget(CodeGenTarget target)
    {
        switch (target)
        {
            // Don't *require* a downstream compiler for source output
            case CodeGenTarget::GLSL:
            case CodeGenTarget::HLSL:
            case CodeGenTarget::CUDASource:
            case CodeGenTarget::CPPSource:
            case CodeGenTarget::HostCPPSource:
            case CodeGenTarget::CSource:
            {
                return PassThroughMode::None;
            }
            case CodeGenTarget::None:
            {
                return PassThroughMode::None;
            }
            case CodeGenTarget::SPIRVAssembly:
            case CodeGenTarget::SPIRV:
            {
                return PassThroughMode::Glslang;
            }
            case CodeGenTarget::DXBytecode:
            case CodeGenTarget::DXBytecodeAssembly:
            {
                return PassThroughMode::Fxc;
            }
            case CodeGenTarget::DXIL:
            case CodeGenTarget::DXILAssembly:
            {
                return PassThroughMode::Dxc;
            }
            case CodeGenTarget::GLSL_Vulkan:
            case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                return PassThroughMode::Glslang;
            }
            case CodeGenTarget::ShaderHostCallable:
            case CodeGenTarget::ShaderSharedLibrary:
            case CodeGenTarget::HostExecutable:
            case CodeGenTarget::HostHostCallable:
            {
                // We need some C/C++ compiler
                return PassThroughMode::GenericCCpp;
            }
            case CodeGenTarget::PTX:
            {
                return PassThroughMode::NVRTC;
            }

            default: break;
        }

        SLANG_ASSERT(!"Unhandled target");
        return PassThroughMode::None;
    }

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
        if(endToEndReq->m_passThrough == PassThroughMode::None)
            return nullptr;

        // If we have confirmed that pass-through compilation is going on,
        // we return the end-to-end request, because it has all the
        // relevant state that we need to implement pass-through mode.
        //
        return endToEndReq;
    }

    /// If there is a pass-through compile going on, find the translation unit for the given entry point.
    /// Assumes isPassThroughEnabled has already been called
    TranslationUnitRequest* getPassThroughTranslationUnit(
        EndToEndCompileRequest* endToEndReq,
        Int                     entryPointIndex)
    {
        SLANG_ASSERT(endToEndReq);
        SLANG_ASSERT(endToEndReq->m_passThrough != PassThroughMode::None);
        auto frontEndReq = endToEndReq->getFrontEndReq();
        auto entryPointReq = frontEndReq->getEntryPointReq(entryPointIndex);
        auto translationUnit = entryPointReq->getTranslationUnit();
        return translationUnit;
    }

    TranslationUnitRequest* CodeGenContext::findPassThroughTranslationUnit(
        Int                     entryPointIndex)
    {
        if (auto endToEndReq = isPassThroughEnabled())
            return getPassThroughTranslationUnit(endToEndReq, entryPointIndex);
        return nullptr;
    }

    static void _appendCodeWithPath(const UnownedStringSlice& filePath, const UnownedStringSlice& fileContent, StringBuilder& outCodeBuilder)
    {
        outCodeBuilder << "#line 1 \"";
        auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp);
        handler->appendEscaped(filePath, outCodeBuilder);
        outCodeBuilder << "\"\n";
        outCodeBuilder << fileContent << "\n";
    }

    void trackGLSLTargetCaps(
        GLSLExtensionTracker*   extensionTracker,
        CapabilitySet const&    caps)
    {
        for( auto atom : caps.getExpandedAtoms() )
        {
            switch( atom )
            {
            default:
                break;

            case CapabilityAtom::SPIRV_1_0: extensionTracker->requireSPIRVVersion(SemanticVersion(1, 0)); break;
            case CapabilityAtom::SPIRV_1_1: extensionTracker->requireSPIRVVersion(SemanticVersion(1, 1)); break;
            case CapabilityAtom::SPIRV_1_2: extensionTracker->requireSPIRVVersion(SemanticVersion(1, 2)); break;
            case CapabilityAtom::SPIRV_1_3: extensionTracker->requireSPIRVVersion(SemanticVersion(1, 3)); break;
            case CapabilityAtom::SPIRV_1_4: extensionTracker->requireSPIRVVersion(SemanticVersion(1, 4)); break;
            case CapabilityAtom::SPIRV_1_5: extensionTracker->requireSPIRVVersion(SemanticVersion(1, 5)); break;
            }
        }
    }

    SlangResult CodeGenContext::emitEntryPointsSource(
        String&                 outSource,
        RefPtr<PostEmitMetadata>& outMetadata)
    {
        outSource = String();

        if(auto endToEndReq = isPassThroughEnabled())
        {
            for (auto entryPointIndex : getEntryPointIndices())
            {
                auto translationUnit = getPassThroughTranslationUnit(endToEndReq, entryPointIndex);
                SLANG_ASSERT(translationUnit);
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
                        _appendCodeWithPath(sourceFile->getPathInfo().foundPath.getUnownedSlice(), sourceFile->getContent(), codeBuilder);
                    }
                }

                outSource = codeBuilder.ProduceString();
            }
            return SLANG_OK;
        }
        else
        {
            return emitEntryPointsSourceFromIR(
                outSource, outMetadata);
        }
    }

    String GetHLSLProfileName(Profile profile)
    {
        switch( profile.getFamily() )
        {
        case ProfileFamily::DX:
            // Profile version is a DX one, so stick with it.
            break;

        default:
            // Profile is a non-DX profile family, so we need to try
            // to clobber it with something to get a default.
            //
            // TODO: This is a huge hack...
            profile.setVersion(ProfileVersion::DX_5_0);
            break;
        }

        char const* stagePrefix = nullptr;
        switch( profile.getStage() )
        {
            // Note: All of the raytracing-related stages require
            // compiling for a `lib_*` profile, even when only a
            // single entry point is present.
            //
            // We also go ahead and use this target in any case
            // where we don't know the actual stage to compiel for,
            // as a fallback option.
            //
            // TODO: We also want to use this option when compiling
            // multiple entry points to a DXIL library.
            //
        default:
            stagePrefix = "lib";
            break;

            // The traditional rasterization pipeline and compute
            // shaders all have custom profile names that identify
            // both the stage and shader model, which need to be
            // used when compiling a single entry point.
            //
    #define CASE(NAME, PREFIX) case Stage::NAME: stagePrefix = #PREFIX; break
        CASE(Vertex,    vs);
        CASE(Hull,      hs);
        CASE(Domain,    ds);
        CASE(Geometry,  gs);
        CASE(Fragment,  ps);
        CASE(Compute,   cs);
        CASE(Amplification, as);
        CASE(Mesh,          ms);
    #undef CASE
        }

        char const* versionSuffix = nullptr;
        switch(profile.getVersion())
        {
    #define CASE(TAG, SUFFIX) case ProfileVersion::TAG: versionSuffix = #SUFFIX; break
        CASE(DX_4_0,             _4_0);
        CASE(DX_4_0_Level_9_0,   _4_0_level_9_0);
        CASE(DX_4_0_Level_9_1,   _4_0_level_9_1);
        CASE(DX_4_0_Level_9_3,   _4_0_level_9_3);
        CASE(DX_4_1,             _4_1);
        CASE(DX_5_0,             _5_0);
        CASE(DX_5_1,             _5_1);
        CASE(DX_6_0,             _6_0);
        CASE(DX_6_1,             _6_1);
        CASE(DX_6_2,             _6_2);
        CASE(DX_6_3,             _6_3);
        CASE(DX_6_4,             _6_4);
        CASE(DX_6_5,             _6_5);
        CASE(DX_6_6,             _6_6);
    #undef CASE

        default:
            return "unknown";
        }

        String result;
        result.append(stagePrefix);
        result.append(versionSuffix);
        return result;
    }

    void reportExternalCompileError(const char* compilerName, Severity severity, SlangResult res, const UnownedStringSlice& diagnostic, DiagnosticSink* sink)
    {
        StringBuilder builder;
        if (compilerName)
        {
            builder << compilerName << ": ";
        }

        if (SLANG_FAILED(res) && res != SLANG_FAIL)
        {
            {
                char tmp[17];
                sprintf_s(tmp, SLANG_COUNT_OF(tmp), "0x%08x", uint32_t(res));
                builder << "Result(" << tmp << ") ";
            }

            PlatformUtil::appendResult(res, builder);
        }

        if (diagnostic.getLength() > 0)
        {
            builder.Append(diagnostic);
            if (!diagnostic.endsWith("\n"))
            {
                builder.Append("\n");
            }
        }

        sink->diagnoseRaw(severity, builder.getUnownedSlice());
    }

    void reportExternalCompileError(const char* compilerName, SlangResult res, const UnownedStringSlice& diagnostic, DiagnosticSink* sink)
    {
        // TODO(tfoley): need a better policy for how we translate diagnostics
        // back into the Slang world (although we should always try to generate
        // HLSL that doesn't produce any diagnostics...)
        reportExternalCompileError(compilerName, SLANG_FAILED(res) ? Severity::Error : Severity::Warning, res, diagnostic, sink);
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
            case 0:     return "unknown";
            case 1:     return _getDisplayPath(sink, sourceFiles[0]);
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

    // Helper function for cases where we can assume a single entry point
    Int assertSingleEntryPoint(List<Int> const& entryPointIndices) {
        SLANG_ASSERT(entryPointIndices.getCount() == 1);
        return *entryPointIndices.begin();
    }

        // True if it's best to use 'emitted' source for complication. For a downstream compiler
        // that is not file based, this is always ok.
        /// 
        /// If the downstream compiler is file system based, we may want to just use the file that was passed to be compiled.
        /// That the downstream compiler can determine if it will then save the file or not based on if it's a match -
        /// and generally there will not be a match with emitted source.
        ///
        /// This test is only used for pass through mode. 
    static bool _useEmittedSource(DownstreamCompiler* compiler, TranslationUnitRequest* translationUnit)
    {
        // We only bother if it's a file based compiler.
        if (compiler->isFileBased())
        {
            // It can only have *one* source file as otherwise we have to combine to make a new source file anyway
            const auto& sourceFiles = translationUnit->getSourceFiles();

            // The *assumption* here is that if it's file based that assuming it can find the file with the same contents
            // it can compile directly without having to save off as a file
            if (sourceFiles.getCount() == 1)
            {
                const SourceFile* sourceFile = sourceFiles[0];
                // We need the path to be found and set
                // 
                // NOTE! That the downstream compiler can determine if the path and contents match such that it can be used
                // without writing file
                const PathInfo& pathInfo = sourceFile->getPathInfo();
                if ((pathInfo.type == PathInfo::Type::FoundPath || pathInfo.type == PathInfo::Type::Normal) && pathInfo.foundPath.getLength())
                {
                    return false;
                }
            }
        }
        return true;
    }

    static Severity _getDiagnosticSeverity(DownstreamDiagnostic::Severity severity)
    {
        typedef DownstreamDiagnostic::Severity DownstreamSeverity;
        switch (severity)
        {
            case DownstreamSeverity::Warning:   return Severity::Warning;
            case DownstreamSeverity::Info:      return Severity::Note;
            default: return Severity::Error;
        }
    }

    static RefPtr<ExtensionTracker> _newExtensionTracker(CodeGenTarget target)
    {
        switch (target)
        {
            case CodeGenTarget::PTX:
            case CodeGenTarget::CUDASource:
            {
                return new CUDAExtensionTracker;
            }
            case CodeGenTarget::SPIRV:
            case CodeGenTarget::GLSL:
            {
                return new GLSLExtensionTracker;
            }
            default:                            return nullptr;
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
            {
                return CodeGenTarget::HostCPPSource;
            }
            case CodeGenTarget::PTX:                return CodeGenTarget::CUDASource;
            case CodeGenTarget::DXBytecode:         return CodeGenTarget::HLSL;
            case CodeGenTarget::DXIL:               return CodeGenTarget::HLSL;
            case CodeGenTarget::SPIRV:              return CodeGenTarget::GLSL;
            default: break;
        }
        return CodeGenTarget::Unknown;
    }

    static bool _isCPUHostTarget(CodeGenTarget target)
    {
        auto desc = ArtifactDesc::makeFromCompileTarget(asExternal(target));
        return desc.style == ArtifactStyle::Host;
    }

    SlangResult CodeGenContext::emitWithDownstreamForEntryPoints(
        RefPtr<DownstreamCompileResult>& outResult,
        RefPtr<PostEmitMetadata>& outMetadata)
    {
        outResult.setNull();

        auto sink = getSink();
        auto session = getSession();

        CodeGenTarget sourceTarget = CodeGenTarget::None;
        SourceLanguage sourceLanguage = SourceLanguage::Unknown;

        auto target = getTargetFormat();
        RefPtr<ExtensionTracker> extensionTracker = _newExtensionTracker(target);
        PassThroughMode compilerType;

        if (auto endToEndReq = isPassThroughEnabled())
        {
            compilerType = endToEndReq->m_passThrough;
        }
        else
        {
            // If we are not in pass through, lookup the default compiler for the emitted source type

            // Get the default source codegen type for a given target
            sourceTarget = _getDefaultSourceForTarget(target);
            compilerType = (PassThroughMode)session->getDownstreamCompilerForTransition((SlangCompileTarget)sourceTarget, (SlangCompileTarget)target);
            // We should have a downstream compiler set at this point
            if (compilerType == PassThroughMode::None)
            {
                auto sourceName = TypeTextUtil::getCompileTargetName(SlangCompileTarget(sourceTarget));
                auto targetName = TypeTextUtil::getCompileTargetName(SlangCompileTarget(target));

                sink->diagnose(SourceLoc(), Diagnostics::compilerNotDefinedForTransition, sourceName, targetName);
                return SLANG_FAIL;
            }
        }

        SLANG_ASSERT(compilerType != PassThroughMode::None);

        // Get the required downstream compiler
        DownstreamCompiler* compiler = session->getOrLoadDownstreamCompiler(compilerType, sink);
        if (!compiler)
        {
            auto compilerName = TypeTextUtil::getPassThroughAsHumanText((SlangPassThrough)compilerType);
            sink->diagnose(SourceLoc(), Diagnostics::passThroughCompilerNotFound, compilerName);
            return SLANG_FAIL;
        }

        Dictionary<String, String> preprocessorDefinitions;
        List<String> includePaths;

        typedef DownstreamCompiler::CompileOptions CompileOptions;
        CompileOptions options;

        // Set compiler specific args
        {
            auto linkage = getLinkage();
        
            auto name = TypeTextUtil::getPassThroughName((SlangPassThrough)compilerType);
            const Index nameIndex = linkage->m_downstreamArgs.findName(name);
            if (nameIndex >= 0)
            {
                auto& args = linkage->m_downstreamArgs.getArgsAt(nameIndex);
                for (const auto& arg : args.m_args)
                {
                    options.compilerSpecificArguments.add(arg.value);
                }
            }
        }

        /* This is more convoluted than the other scenarios, because when we invoke C/C++ compiler we would ideally like
        to use the original file. We want to do this because we want includes relative to the source file to work, and
        for that to work most easily we want to use the original file, if there is one */
        if (auto endToEndReq = isPassThroughEnabled())
        {
            // If we are pass through, we may need to set extension tracker state. 
            if (GLSLExtensionTracker* glslTracker = as<GLSLExtensionTracker>(extensionTracker))
            {
                trackGLSLTargetCaps(glslTracker, getTargetCaps());
            }

            auto translationUnit = getPassThroughTranslationUnit(endToEndReq, getSingleEntryPointIndex());

            // We are just passing thru, so it's whatever it originally was
            sourceLanguage = translationUnit->sourceLanguage;

            // TODO(JS): This seems like a bit of a hack
            // That if a pass-through is being performed and the source language is Slang
            // no downstream compiler knows how to deal with that, so probably means 'HLSL'
            sourceLanguage = (sourceLanguage == SourceLanguage::Slang) ? SourceLanguage::HLSL : sourceLanguage;
            sourceTarget = CodeGenTarget(TypeConvertUtil::getCompileTargetFromSourceLanguage((SlangSourceLanguage)sourceLanguage));

            // If it's pass through we accumulate the preprocessor definitions. 
            for (auto& define : translationUnit->compileRequest->preprocessorDefinitions)
            {
                preprocessorDefinitions.Add(define.Key, define.Value);
            }
            for (auto& define : translationUnit->preprocessorDefinitions)
            {
                preprocessorDefinitions.Add(define.Key, define.Value);
            }
            
            {
                /* TODO(JS): Not totally clear what options should be set here. If we are using the pass through - then using say the defines/includes
                all makes total sense. If we are generating C++ code from slang, then should we really be using these values -> aren't they what is
                being set for the *slang* source, not for the C++ generated code. That being the case it implies that there needs to be a mechanism
                (if there isn't already) to specify such information on a particular pass/pass through etc.

                On invoking DXC for example include paths do not appear to be set at all (even with pass-through).
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
                // If it's not file based we can set an appropriate path name, and it doesn't matter if it doesn't
                // exist on the file system
                options.sourceContentsPath = calcSourcePathForEntryPoints();

                CodeGenContext sourceCodeGenContext(this, sourceTarget, extensionTracker);
                SLANG_RETURN_ON_FAIL(sourceCodeGenContext.emitEntryPointsSource(options.sourceContents, outMetadata));
            }
            else
            {
                // Special case if we have a single file, so that we pass the path, and the contents as is.
                const auto& sourceFiles = translationUnit->getSourceFiles();
                SLANG_ASSERT(sourceFiles.getCount() == 1);

                const SourceFile* sourceFile = sourceFiles[0];
                
                options.sourceContentsPath = sourceFile->getPathInfo().foundPath;
                options.sourceContents = sourceFile->getContent();
            }
        }
        else
        {
            CodeGenContext sourceCodeGenContext(this, sourceTarget, extensionTracker);
            SLANG_RETURN_ON_FAIL(sourceCodeGenContext.emitEntryPointsSource(options.sourceContents, outMetadata));
            sourceCodeGenContext.maybeDumpIntermediate(options.sourceContents.getBuffer());

            sourceLanguage = (SourceLanguage)TypeConvertUtil::getSourceLanguageFromTarget((SlangCompileTarget)sourceTarget);
        }

        // Add any preprocessor definitions associated with the linkage
        {
            // TODO(JS): This is somewhat arguable - should defines passed to Slang really be
            // passed to downstream compilers? It does appear consistent with the behavior if 
            // there is an endToEndReq.
            // 
            // That said it's very convenient and provides way to control aspects 
            // of downstream compilation. 
            
            auto linkage = getLinkage();
            for (auto& define : linkage->preprocessorDefinitions)
            {
                preprocessorDefinitions.Add(define.Key, define.Value);
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
                    DownstreamCompiler::CapabilityVersion version;
                    version.kind = DownstreamCompiler::CapabilityVersion::Kind::CUDASM;
                    version.version = cudaTracker->m_smVersion;

                    options.requiredCapabilityVersions.add(version);
                }

                if (cudaTracker->isBaseTypeRequired(BaseType::Half))
                {
                    options.flags |= CompileOptions::Flag::EnableFloat16;
                }
            }
            else if (GLSLExtensionTracker* glslTracker = as<GLSLExtensionTracker>(extensionTracker))
            {
                DownstreamCompiler::CapabilityVersion version;
                version.kind = DownstreamCompiler::CapabilityVersion::Kind::SPIRV;
                version.version = glslTracker->getSPIRVVersion();

                options.requiredCapabilityVersions.add(version);
            }
        }

        // Set the file sytem and source manager, as *may* be used by downstream compiler
        options.fileSystemExt = getFileSystemExt();
        options.sourceManager = getSourceManager();

        // Set the source type
        options.sourceLanguage = SlangSourceLanguage(sourceLanguage);
        
        // Disable exceptions and security checks
        options.flags &= ~(CompileOptions::Flag::EnableExceptionHandling | CompileOptions::Flag::EnableSecurityChecks);

        Profile profile;

        if (compilerType == PassThroughMode::Fxc ||
            compilerType == PassThroughMode::Dxc ||
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
            if (targetReq->isWholeProgramRequest())
            {
                if (compilerType == PassThroughMode::Dxc)
                {
                    // Can support no entry points on DXC because we can build libraries
                    profile = targetReq->getTargetProfile();
                }
                else
                {
                    auto downstreamCompilerName = TypeTextUtil::getPassThroughName((SlangPassThrough)compilerType);

                    sink->diagnose(SourceLoc(), Diagnostics::downstreamCompilerDoesntSupportWholeProgramCompilation, downstreamCompilerName);
                    return SLANG_FAIL;
                }
            }
            else if (entryPointIndicesCount == 1)
            {
                // All support a single entry point
                const Index entryPointIndex = entryPointIndices[0];

                auto entryPoint = getEntryPoint(entryPointIndex);
                profile = getEffectiveProfile(entryPoint, targetReq);

                options.entryPointName = getText(entryPoint->getName());
                auto entryPointNameOverride = getProgram()->getEntryPointNameOverride(entryPointIndex);
                if (entryPointNameOverride.getLength() != 0)
                {
                    options.entryPointName = entryPointNameOverride;
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
                options.matrixLayout = getTargetReq()->getDefaultMatrixLayoutMode();
            }

            // Set the profile
            options.profileName = GetHLSLProfileName(profile);
        }

        // If we aren't using LLVM 'host callable', we want downstream compile to produce a shared library
        if (compilerType != PassThroughMode::LLVM && 
            ArtifactDesc::makeFromCompileTarget(asExternal(target)).kind == ArtifactKind::Callable)
        {
            target = CodeGenTarget::ShaderSharedLibrary;
        }

        if (!isPassThroughEnabled())
        {
            if (_isCPUHostTarget(target))
            {
                options.libraryPaths.add(Path::getParentDirectory(Path::getExecutablePath()));
                // Set up the library artifact
                ComPtr<IArtifact> artifact(new Artifact(ArtifactDesc::make(ArtifactKind::Library, Artifact::Payload::HostCPU), "slang-rt"));
                options.libraries.add(artifact);
            }
        }

        options.targetType = (SlangCompileTarget)target;

        // Need to configure for the compilation

        {
            auto linkage = getLinkage();

            switch (linkage->optimizationLevel)
            {
                case OptimizationLevel::None:       options.optimizationLevel = DownstreamCompiler::OptimizationLevel::None; break;
                case OptimizationLevel::Default:    options.optimizationLevel = DownstreamCompiler::OptimizationLevel::Default;  break;
                case OptimizationLevel::High:       options.optimizationLevel = DownstreamCompiler::OptimizationLevel::High;  break;
                case OptimizationLevel::Maximal:    options.optimizationLevel = DownstreamCompiler::OptimizationLevel::Maximal;  break;
                default: SLANG_ASSERT(!"Unhandled optimization level"); break;
            }

            switch (linkage->debugInfoLevel)
            {
                case DebugInfoLevel::None:          options.debugInfoType = DownstreamCompiler::DebugInfoType::None; break; 
                case DebugInfoLevel::Minimal:       options.debugInfoType = DownstreamCompiler::DebugInfoType::Minimal; break; 
                
                case DebugInfoLevel::Standard:      options.debugInfoType = DownstreamCompiler::DebugInfoType::Standard; break; 
                case DebugInfoLevel::Maximal:       options.debugInfoType = DownstreamCompiler::DebugInfoType::Maximal; break; 
                default: SLANG_ASSERT(!"Unhandled debug level"); break;
            }

            switch( getTargetReq()->getFloatingPointMode())
            {
                case FloatingPointMode::Default:    options.floatingPointMode = DownstreamCompiler::FloatingPointMode::Default; break;
                case FloatingPointMode::Precise:    options.floatingPointMode = DownstreamCompiler::FloatingPointMode::Precise; break;
                case FloatingPointMode::Fast:       options.floatingPointMode = DownstreamCompiler::FloatingPointMode::Fast; break;
                default: SLANG_ASSERT(!"Unhandled floating point mode");
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
                for(Index ee = 0; ee < entryPointCount; ++ee)
                {
                    auto stage = getEntryPoint(ee)->getStage();
                    switch(stage)
                    {
                    default:
                        break;

                    case Stage::Compute:
                        options.pipelineType = DownstreamCompiler::PipelineType::Compute;
                        break;

                    case Stage::Vertex:
                    case Stage::Hull:
                    case Stage::Domain:
                    case Stage::Geometry:
                    case Stage::Fragment:
                        options.pipelineType = DownstreamCompiler::PipelineType::Rasterization;
                        break;

                    case Stage::RayGeneration:
                    case Stage::Intersection:
                    case Stage::AnyHit:
                    case Stage::ClosestHit:
                    case Stage::Miss:
                    case Stage::Callable:
                        options.pipelineType = DownstreamCompiler::PipelineType::RayTracing;
                        break;
                    }
                }   
            }

            // Add all the search paths (as calculated earlier - they will only be set if this is a pass through else will be empty)
            options.includePaths = includePaths;

            // Add the specified defines (as calculated earlier - they will only be set if this is a pass through else will be empty)
            {
                for(auto& def : preprocessorDefinitions)
                {
                    DownstreamCompiler::Define define;
                    define.nameWithSig = def.Key;
                    define.value = def.Value;

                    options.defines.add(define);
                }
            }

            // Add all of the module libraries
            options.libraries.addRange(linkage->m_libModules.getBuffer(), linkage->m_libModules.getCount());
        }

        // Compile
        RefPtr<DownstreamCompileResult> downstreamCompileResult;
        auto downstreamStartTime = std::chrono::high_resolution_clock::now();
        SLANG_RETURN_ON_FAIL(compiler->compile(options, downstreamCompileResult));
        auto downstreamElapsedTime =
            (std::chrono::high_resolution_clock::now() - downstreamStartTime).count() * 0.000000001;
        getSession()->addDownstreamCompileTime(downstreamElapsedTime);

        const auto& diagnostics = downstreamCompileResult->getDiagnostics();

        if (diagnostics.diagnostics.getCount())
        {
            StringBuilder compilerText;
            compiler->getDesc().appendAsText(compilerText);

            StringBuilder builder;

            for (const auto& diagnostic : diagnostics.diagnostics)
            {
                builder.Clear();

                const Severity severity = _getDiagnosticSeverity(diagnostic.severity);
                
                if (diagnostic.filePath.getLength() == 0 && diagnostic.fileLine == 0 && severity == Severity::Note)
                {
                    // If theres no filePath line number and it's info, output severity and text alone
                    builder << getSeverityName(severity) << " : ";
                }
                else
                {
                    if (diagnostic.filePath.getLength())
                    {
                        builder << diagnostic.filePath;
                    }

                    if (diagnostic.fileLine)
                    {
                        builder << "(" << diagnostic.fileLine <<")";
                    }

                    builder << ": ";

                    if (diagnostic.stage == DownstreamDiagnostic::Stage::Link)
                    {
                        builder << "link ";
                    }

                    builder << getSeverityName(severity);
                    builder << " " << diagnostic.code << ": ";
                }

                builder << diagnostic.text;
                reportExternalCompileError(compilerText.getBuffer(), severity, SLANG_OK, builder.getUnownedSlice(), sink);
            }
        }

        // If any errors are emitted, then we are done
        if (diagnostics.has(DownstreamDiagnostic::Severity::Error))
        {
            return SLANG_FAIL;
        }

        outResult = downstreamCompileResult;
        return SLANG_OK;
    }

    SlangResult CodeGenContext::dissassembleWithDownstream(
        const void*                 data,
        size_t                      dataSizeInBytes, 
        ISlangBlob**                outBlob)
    {
        auto session = getSession();
        auto sink = getSink();
        auto target = getTargetFormat();

        // Get the downstream compiler that can be used for this target

        // TODO(JS):
        // This could perhaps be performed in some other manner if there was more than one way to produce
        // disassembly from a binary.
        auto downstreamCompiler = getDownstreamCompilerRequiredForTarget(target);

        // Get the required downstream compiler
        DownstreamCompiler* compiler = session->getOrLoadDownstreamCompiler(downstreamCompiler, sink);

        if (!compiler)
        {
            auto compilerName = TypeTextUtil::getPassThroughAsHumanText((SlangPassThrough)downstreamCompiler);
            sink->diagnose(SourceLoc(), Diagnostics::passThroughCompilerNotFound, compilerName);
            return SLANG_FAIL;
        }

        ComPtr<ISlangBlob> dissassemblyBlob;
        SLANG_RETURN_ON_FAIL(compiler->disassemble(SlangCompileTarget(target), data, dataSizeInBytes, dissassemblyBlob.writeRef()));

        *outBlob = dissassemblyBlob.detach();
        return SLANG_OK;
    }

    SlangResult CodeGenContext::dissassembleWithDownstream(
        DownstreamCompileResult*    downstreamResult, 
        ISlangBlob**                outBlob)
    {
        
        ComPtr<ISlangBlob> codeBlob;
        SLANG_RETURN_ON_FAIL(downstreamResult->getBinary(codeBlob));
        return dissassembleWithDownstream(codeBlob->getBufferPointer(), codeBlob->getBufferSize(), outBlob);
    }

    SlangResult emitSPIRVForEntryPointsDirectly(
        CodeGenContext* codeGenContext,
        List<uint8_t>&  spirvOut,
        RefPtr<PostEmitMetadata>& outMetadata);

    static CodeGenTarget _getIntermediateTarget(CodeGenTarget target)
    {
        switch (target)
        {
            case CodeGenTarget::DXBytecodeAssembly: return CodeGenTarget::DXBytecode;
            case CodeGenTarget::DXILAssembly:       return CodeGenTarget::DXIL;
            case CodeGenTarget::SPIRVAssembly:      return CodeGenTarget::SPIRV;
            default:    return CodeGenTarget::None;
        }
    }

        /// Function to simplify the logic around emitting, and dissassembling
    SlangResult CodeGenContext::_emitEntryPoints(
        RefPtr<DownstreamCompileResult>& outDownstreamResult,
        RefPtr<PostEmitMetadata>& outMetadata)
    {
        auto target = getTargetFormat();
        switch (target)
        {
            case CodeGenTarget::SPIRVAssembly:
            case CodeGenTarget::DXBytecodeAssembly:
            case CodeGenTarget::DXILAssembly:
            {
                // First compile to an intermediate target for the corresponding binary format.
                const CodeGenTarget intermediateTarget = _getIntermediateTarget(target);
                CodeGenContext intermediateContext(this, intermediateTarget);

                RefPtr<DownstreamCompileResult> code;
                SLANG_RETURN_ON_FAIL(intermediateContext._emitEntryPoints(code, outMetadata));
                intermediateContext.maybeDumpIntermediate(code);

                // Then disassemble the intermediate binary result to get the desired output
                // Output the disassembly
                ComPtr<ISlangBlob> disassemblyBlob;
                SLANG_RETURN_ON_FAIL(intermediateContext.dissassembleWithDownstream(code, disassemblyBlob.writeRef()));

                outDownstreamResult = new BlobDownstreamCompileResult(DownstreamDiagnostics(), disassemblyBlob);
                return SLANG_OK;
            }
            case CodeGenTarget::SPIRV:
                if (getTargetReq()->shouldEmitSPIRVDirectly())
                {
                    List<uint8_t> spirv;
                    SLANG_RETURN_ON_FAIL(emitSPIRVForEntryPointsDirectly(this, spirv, outMetadata));
                    auto spirvBlob = ListBlob::moveCreate(spirv);
                    outDownstreamResult = new BlobDownstreamCompileResult(DownstreamDiagnostics(), spirvBlob);
                    return SLANG_OK;
                }
                /* fall through to: */
            case CodeGenTarget::DXIL:
            case CodeGenTarget::DXBytecode:
            case CodeGenTarget::PTX:
            case CodeGenTarget::ShaderHostCallable:
            case CodeGenTarget::ShaderSharedLibrary:
            case CodeGenTarget::HostExecutable:
            case CodeGenTarget::HostHostCallable:
                SLANG_RETURN_ON_FAIL(emitWithDownstreamForEntryPoints(outDownstreamResult, outMetadata));
                return SLANG_OK;

            default: break;
        }

        return SLANG_FAIL;
    }

    // Do emit logic for a zero or more entry points
    CompileResult CodeGenContext::emitEntryPoints()
    {
        CompileResult result;

        auto target = getTargetFormat();

        switch (target)
        {
        case CodeGenTarget::SPIRVAssembly:
        case CodeGenTarget::DXBytecodeAssembly:
        case CodeGenTarget::DXILAssembly:
        case CodeGenTarget::SPIRV:
        case CodeGenTarget::DXIL:
        case CodeGenTarget::DXBytecode:
        case CodeGenTarget::PTX:
        case CodeGenTarget::HostHostCallable:
        case CodeGenTarget::ShaderHostCallable:
        case CodeGenTarget::ShaderSharedLibrary:
        case CodeGenTarget::HostExecutable:
            {
                RefPtr<DownstreamCompileResult> downstreamResult;
                RefPtr<PostEmitMetadata> metadata;

                if (SLANG_SUCCEEDED(_emitEntryPoints(downstreamResult, metadata)))
                {
                    maybeDumpIntermediate(downstreamResult);
                    result = CompileResult(downstreamResult, metadata);
                }
            }
            break;
        case CodeGenTarget::GLSL:
        case CodeGenTarget::HLSL:
        case CodeGenTarget::CUDASource:
        case CodeGenTarget::CPPSource:
        case CodeGenTarget::HostCPPSource:
        case CodeGenTarget::CSource:
            {
                RefPtr<ExtensionTracker> extensionTracker = _newExtensionTracker(target);
                RefPtr<PostEmitMetadata> metadata;

                CodeGenContext subContext(this, target, extensionTracker);

                String code;
                if (SLANG_FAILED(subContext.emitEntryPointsSource(code, metadata)))
                {
                    return result;
                }

                subContext.maybeDumpIntermediate(code.getBuffer());
                result = CompileResult(code, metadata);
            }
            break;

        case CodeGenTarget::None:
            // The user requested no output
            break;

            // Note(tfoley): We currently hit this case when compiling the stdlib
        case CodeGenTarget::Unknown:
            break;

        default:
            SLANG_UNEXPECTED("unhandled code generation target");
            break;
        }

        return result;
    }

    enum class OutputFileKind
    {
        Text,
        Binary,
    };

    static void writeOutputFile(
        CodeGenContext*         context,
        FILE*                   file,
        String const&           path,
        void const*             data,
        size_t                  size)
    {
        size_t count = fwrite(data, size, 1, file);
        if (count != 1)
        {
            context->getSink()->diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
        }
    }

    static void writeOutputFile(
        CodeGenContext*         context,
        ISlangWriter*           writer,
        String const&           path,
        void const*             data,
        size_t                  size)
    {

        if (SLANG_FAILED(writer->write((const char*)data, size)))
        {
            context->getSink()->diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
        }
    }

    static void writeOutputFile(
        CodeGenContext*         context,
        String const&           path,
        void const*             data,
        size_t                  size,
        OutputFileKind          kind)
    {
        FILE* file = fopen(
            path.getBuffer(),
            kind == OutputFileKind::Binary ? "wb" : "w");
        if (!file)
        {
            context->getSink()->diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
            return;
        }

        writeOutputFile(context, file, path, data, size);
        fclose(file);
    }

    static void writeCompileResultToFile(
        CodeGenContext* context,
        String const& outputPath,
        CompileResult const& result)
    {
        switch (result.format)
        {
        case ResultFormat::Text:
            {
                auto text = result.outputString;
                writeOutputFile(context,
                    outputPath,
                    text.begin(),
                    text.end() - text.begin(),
                    OutputFileKind::Text);
            }
            break;

        case ResultFormat::Binary:
            {
                ComPtr<ISlangBlob> blob;
                if (SLANG_FAILED(result.getBlob(blob)))
                {
                    SLANG_UNEXPECTED("No blob to emit");
                    return;
                }
                writeOutputFile(context,
                    outputPath,
                    blob->getBufferPointer(),
                    blob->getBufferSize(),
                    OutputFileKind::Binary);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled output format");
            break;
        }

    }

    static void writeOutputToConsole(
        ISlangWriter* writer,
        String const&   text)
    {
        writer->write(text.getBuffer(), text.getLength());
    }

    static void writeCompileResultToStandardOutput(
        CodeGenContext*         codeGenContext,
        EndToEndCompileRequest* endToEndReq,
        CompileResult const&    result)
    {
        auto targetReq = codeGenContext->getTargetReq();
        ISlangWriter* writer = endToEndReq->getWriter(WriterChannel::StdOutput);

        switch (result.format)
        {
        case ResultFormat::Text:
            writeOutputToConsole(writer, result.outputString);
            break;

        case ResultFormat::Binary:
            {
                ComPtr<ISlangBlob> blob;
                if (SLANG_FAILED(result.getBlob(blob)))
                {
                    if (ArtifactDesc::makeFromCompileTarget(asExternal(targetReq->getTarget())).kind == ArtifactKind::Callable)
                    {
                        // Some HostCallable are not directly representable as a 'binary'.
                        // So here, we just ignore if that appears the case, and don't output an unexpected error.
                        return;
                    }

                    SLANG_UNEXPECTED("No blob to emit");
                    return;
                }

                const void* blobData = blob->getBufferPointer();
                size_t blobSize = blob->getBufferSize();

                if (writer->isConsole())
                {
                    // Writing to console, so we need to generate text output.

                    switch (targetReq->getTarget())
                    {
                    case CodeGenTarget::SPIRVAssembly:
                    case CodeGenTarget::DXBytecodeAssembly:
                    case CodeGenTarget::DXILAssembly:
                        {
                            const UnownedStringSlice disassembly = StringUtil::getSlice(blob);
                            writeOutputToConsole(writer, disassembly);
                        }
                        break;
                    case CodeGenTarget::SPIRV:
                    case CodeGenTarget::DXIL:
                    case CodeGenTarget::DXBytecode:
                        {
                            ComPtr<ISlangBlob> disassemblyBlob;

                            if (SLANG_SUCCEEDED(codeGenContext->dissassembleWithDownstream(blobData, blobSize,  disassemblyBlob.writeRef())))
                            {
                                const UnownedStringSlice disassembly = StringUtil::getSlice(disassemblyBlob);
                                writeOutputToConsole(writer, disassembly);
                            }
                        }
                        break;
                    case CodeGenTarget::PTX:
                        // For now we just dump PTX out as hex

                    case CodeGenTarget::HostHostCallable:
                    case CodeGenTarget::ShaderHostCallable:
                    case CodeGenTarget::ShaderSharedLibrary:
                    case CodeGenTarget::HostExecutable:
                        HexDumpUtil::dumpWithMarkers((const uint8_t*)blobData, blobSize, 24, writer);
                        break;


                    default:
                        SLANG_UNEXPECTED("unhandled output format");
                        return;
                        }
                }
            else
            {
                // Redirecting stdout to a file, so do the usual thing
                writer->setMode(SLANG_WRITER_MODE_BINARY);

                writeOutputFile(
                    codeGenContext,
                    writer,
                    "stdout",
                    blobData,
                    blobSize);
            }
        }
        break;

        default:
            SLANG_UNEXPECTED("unhandled output format");
            break;
        }

    }

    void EndToEndCompileRequest::writeWholeProgramResult(
        TargetRequest* targetReq)
    {
        auto program = getSpecializedGlobalAndEntryPointsComponentType();
        auto targetProgram = program->getTargetProgram(targetReq);

        auto& result = targetProgram->getExistingWholeProgramResult();

        // Skip the case with no output
        if (result.format == ResultFormat::None)
            return;

        CodeGenContext::EntryPointIndices entryPointIndices;
        for (Index i = 0; i < program->getEntryPointCount(); ++i)
            entryPointIndices.add(i);
        CodeGenContext::Shared sharedCodeGenContext(targetProgram, entryPointIndices, getSink(), this);
        CodeGenContext codeGenContext(&sharedCodeGenContext);

        // It is possible that we are dynamically discovering entry
        // points (using `[shader(...)]` attributes), so that there
        // might be entry points added to the program that did not
        // get paths specified via command-line options.
        //
        RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
        if (m_targetInfos.TryGetValue(targetReq, targetInfo))
        {
            String outputPath = targetInfo->wholeTargetOutputPath;
            if (outputPath != "")
            {
                writeCompileResultToFile(&codeGenContext, outputPath, result);
                return;
            }
        }

        writeCompileResultToStandardOutput(&codeGenContext, this, result);
    }

    void EndToEndCompileRequest::writeEntryPointResult(
        TargetRequest*  targetReq,
        Int             entryPointIndex)
    {
        auto program = getSpecializedGlobalAndEntryPointsComponentType();
        auto targetProgram = program->getTargetProgram(targetReq);

        auto& result = targetProgram->getExistingEntryPointResult(entryPointIndex);

        // Skip the case with no output
        if (result.format == ResultFormat::None)
            return;

        CodeGenContext::EntryPointIndices entryPointIndices;
        entryPointIndices.add(entryPointIndex);

        CodeGenContext::Shared sharedCodeGenContext(targetProgram, entryPointIndices, getSink(), this);
        CodeGenContext codeGenContext(&sharedCodeGenContext);

        // It is possible that we are dynamically discovering entry
        // points (using `[shader(...)]` attributes), so that there
        // might be entry points added to the program that did not
        // get paths specified via command-line options.
        //
        RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
        auto entryPoint = program->getEntryPoint(entryPointIndex);
        if(m_targetInfos.TryGetValue(targetReq, targetInfo))
        {
            String outputPath;
            if(targetInfo->entryPointOutputPaths.TryGetValue(entryPointIndex, outputPath))
            {
                writeCompileResultToFile(&codeGenContext, outputPath, result);
                return;
            }
        }

        writeCompileResultToStandardOutput(&codeGenContext, this, result);
    }

    CompileResult& TargetProgram::_createWholeProgramResult(
        DiagnosticSink* sink,
        EndToEndCompileRequest* endToEndReq)
    {
        // We want to call `emitEntryPoints` function to generate code that contains
        // all the entrypoints defined in `m_program`.
        // The current logic of `emitEntryPoints` takes a list of entry-point indices to
        // emit code for, so we construct such a list first.
        List<Int> entryPointIndices;

        m_entryPointResults.setCount(m_program->getEntryPointCount());
        entryPointIndices.setCount(m_program->getEntryPointCount());
        for (Index i = 0; i < entryPointIndices.getCount(); i++)
            entryPointIndices[i] = i;
    
        auto& result = m_wholeProgramResult;

        CodeGenContext::Shared sharedCodeGenContext(this, entryPointIndices, sink, endToEndReq);
        CodeGenContext codeGenContext(&sharedCodeGenContext);

        result = codeGenContext.emitEntryPoints();

        return result;
    }

    CompileResult& TargetProgram::_createEntryPointResult(
        Int                     entryPointIndex,
        DiagnosticSink*         sink,
        EndToEndCompileRequest* endToEndReq)
    {
        // It is possible that entry points got added to the `Program`
        // *after* we created this `TargetProgram`, so there might be
        // a request for an entry point that we didn't allocate space for.
        //
        // TODO: Change the construction logic so that a `Program` is
        // constructed all at once rather than incrementally, to avoid
        // this problem.
        //
        if(entryPointIndex >= m_entryPointResults.getCount())
            m_entryPointResults.setCount(entryPointIndex+1);

        auto& result = m_entryPointResults[entryPointIndex];

        CodeGenContext::EntryPointIndices entryPointIndices;
        entryPointIndices.add(entryPointIndex);

        CodeGenContext::Shared sharedCodeGenContext(this, entryPointIndices, sink, endToEndReq);
        CodeGenContext codeGenContext(&sharedCodeGenContext);
        result = codeGenContext.emitEntryPoints();

        return result;
    }

    CompileResult& TargetProgram::getOrCreateWholeProgramResult(
        DiagnosticSink* sink)
    {
        auto& result = m_wholeProgramResult;
        if (result.format != ResultFormat::None)
            return result;

        // If we haven't yet computed a layout for this target
        // program, we need to make sure that is done before
        // code generation.
        //
        if (!getOrCreateIRModuleForLayout(sink))
        {
            return result;
        }

        return _createWholeProgramResult(sink);
    }

    CompileResult& TargetProgram::getOrCreateEntryPointResult(
        Int entryPointIndex,
        DiagnosticSink* sink)
    {
        if(entryPointIndex >= m_entryPointResults.getCount())
            m_entryPointResults.setCount(entryPointIndex+1);

        auto& result = m_entryPointResults[entryPointIndex];
        if( result.format != ResultFormat::None )
            return result;

        // If we haven't yet computed a layout for this target
        // program, we need to make sure that is done before
        // code generation.
        //
        if( !getOrCreateIRModuleForLayout(sink) )
        {
            return result;
        }

        return _createEntryPointResult(
            entryPointIndex,
            sink);
    }

    void EndToEndCompileRequest::generateOutput(
        TargetProgram* targetProgram)
    {
        auto program = targetProgram->getProgram();
        auto targetReq = targetProgram->getTargetReq();

        // Generate target code any entry points that
        // have been requested for compilation.
        auto entryPointCount = program->getEntryPointCount();
        if (targetReq->isWholeProgramRequest())
        {
            targetProgram->_createWholeProgramResult(getSink(), this);
        }
        else
        {
            for (Index ii = 0; ii < entryPointCount; ++ii)
            {
                targetProgram->_createEntryPointResult(
                    ii,
                    getSink(),
                    this);
            }
        }
    }

    
    SlangResult EndToEndCompileRequest::writeContainerToStream(Stream* stream)
    {
        auto linkage = getLinkage();

        // Set up options
        SerialContainerUtil::WriteOptions options;

        options.compressionType = linkage->serialCompressionType;
        if (linkage->m_obfuscateCode)
        {
            // If code is obfuscated, we *disable* AST output as it is not obfuscated and will reveal
            // too much about IR.
            // Also currently only IR is needed.
            options.optionFlags &= ~SerialOptionFlag::ASTModule;
        }
        else if (linkage->debugInfoLevel != DebugInfoLevel::None && linkage->getSourceManager())
        {
            options.optionFlags |= SerialOptionFlag::SourceLocation;
            options.sourceManager = linkage->getSourceManager();
        }

        {
            RiffContainer container;
            {
                SerialContainerData data;
                SLANG_RETURN_ON_FAIL(SerialContainerUtil::addEndToEndRequestToData(this, options, data));
                SLANG_RETURN_ON_FAIL(SerialContainerUtil::write(data, options, &container));
            }
            // We now write the RiffContainer to the stream
            SLANG_RETURN_ON_FAIL(RiffUtil::write(container.getRoot(), true, stream));
        }

        return SLANG_OK;
    }

    SlangResult EndToEndCompileRequest::maybeCreateContainer()
    {
        switch (m_containerFormat)
        {
            case ContainerFormat::SlangModule:
            {
                m_containerBlob.setNull();

                OwnedMemoryStream stream(FileAccess::Write);
                SlangResult res = writeContainerToStream(&stream);
                if (SLANG_FAILED(res))
                {
                    getSink()->diagnose(SourceLoc(), Diagnostics::unableToCreateModuleContainer);
                    return res;
                }

                // Need to turn into a blob
                RefPtr<ListBlob> blob(new ListBlob);
                // Swap the streams contents into the blob
                stream.swapContents(blob->m_data);
                m_containerBlob = blob;

                return res;
            }
            default: break;
        }
        return SLANG_OK;
    }

    SlangResult EndToEndCompileRequest::maybeWriteContainer(const String& fileName)
    {
        // If there is no container, or filename, don't write anything 
        if (fileName.getLength() == 0 || !m_containerBlob)
        {
            return SLANG_OK;
        }

        FileStream stream;
        SLANG_RETURN_ON_FAIL(stream.init(fileName, FileMode::Create, FileAccess::Write, FileShare::ReadWrite));
        SLANG_RETURN_ON_FAIL(stream.write(m_containerBlob->getBufferPointer(), m_containerBlob->getBufferSize()));
        return SLANG_OK;
    }

    static void _writeString(Stream& stream, const char* string)
    {
        stream.write(string, strlen(string));
    }

    static void _escapeDependencyString(const char* string, StringBuilder& outBuilder)
    {
        // make has unusual escaping rules, but we only care about characters that are acceptable in a path
        for (const char* p = string; *p; ++p)
        {
            char c = *p;
            switch(c)
            {
            case ' ':
            case ':':
            case '#':
            case '[':
            case ']':
            case '\\':
                outBuilder.appendChar('\\');
                break;

            case '$':
                outBuilder.appendChar('$');
                break;
            }

            outBuilder.appendChar(c);
        }
    }

    // Writes a line to the file stream, formatted like this:
    //   <output-file>: <dependency-file> <dependency-file...>
    static void _writeDependencyStatement(Stream& stream, EndToEndCompileRequest* compileRequest, const String& outputPath)
    {
        if (outputPath.getLength() == 0)
            return;

        StringBuilder builder;
        _escapeDependencyString(outputPath.begin(), builder);
        _writeString(stream, builder.begin());
        _writeString(stream, ": ");

        int dependencyCount = compileRequest->getDependencyFileCount();
        for (int dependencyIndex = 0; dependencyIndex < dependencyCount; ++dependencyIndex)
        {
            builder.Clear();
            _escapeDependencyString(compileRequest->getDependencyFilePath(dependencyIndex), builder);
            _writeString(stream, builder.begin());
            _writeString(stream, (dependencyIndex + 1 < dependencyCount) ? " " : "\n");
        }
    }

    // Writes a file with dependency info, with one line in the output file per compile product.
    static SlangResult _writeDependencyFile(EndToEndCompileRequest* compileRequest)
    {
        if (compileRequest->m_dependencyOutputPath.getLength() == 0)
            return SLANG_OK;

        FileStream stream;
        SLANG_RETURN_ON_FAIL(stream.init(compileRequest->m_dependencyOutputPath, FileMode::Create, FileAccess::Write, FileShare::ReadWrite));

        auto linkage = compileRequest->getLinkage();
        auto program = compileRequest->getSpecializedGlobalAndEntryPointsComponentType();

        // Iterate over all the targets and their outputs
        for (const auto& targetReq : linkage->targets)
        {
            if (targetReq->isWholeProgramRequest())
            {
                RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
                if (compileRequest->m_targetInfos.TryGetValue(targetReq, targetInfo))
                {
                    _writeDependencyStatement(stream, compileRequest, targetInfo->wholeTargetOutputPath);
                }
            }
            else
            {
                Index entryPointCount = program->getEntryPointCount();
                for (Index entryPointIndex = 0; entryPointIndex < entryPointCount; ++entryPointIndex)
                {
                    RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
                    if (compileRequest->m_targetInfos.TryGetValue(targetReq, targetInfo))
                    {
                        String outputPath;
                        if (targetInfo->entryPointOutputPaths.TryGetValue(entryPointIndex, outputPath))
                        {
                            _writeDependencyStatement(stream, compileRequest, outputPath);
                        }
                    }
                }
            }
        }

        return SLANG_OK;
    }


    void EndToEndCompileRequest::generateOutput(
        ComponentType* program)
    {
        // When dynamic dispatch is disabled, the program must
        // be fully specialized by now. So we check if we still
        // have unspecialized generic/existential parameters,
        // and report them as an error.
        //
        auto specializationParamCount = program->getSpecializationParamCount();
        if (disableDynamicDispatch && specializationParamCount != 0)
        {
            auto sink = getSink();
            
            for( Index ii = 0; ii < specializationParamCount; ++ii )
            {
                auto specializationParam = program->getSpecializationParam(ii);
                if( auto decl = as<Decl>(specializationParam.object) )
                {
                    sink->diagnose(specializationParam.loc, Diagnostics::specializationParameterOfNameNotSpecialized, decl);
                }
                else if( auto type = as<Type>(specializationParam.object) )
                {
                    sink->diagnose(specializationParam.loc, Diagnostics::specializationParameterOfNameNotSpecialized, type);
                }
                else
                {
                    sink->diagnose(specializationParam.loc, Diagnostics::specializationParameterNotSpecialized);
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
            auto targetProgram = program->getTargetProgram(targetReq);
            generateOutput(targetProgram);
        }
    }

    void EndToEndCompileRequest::generateOutput()
    {
        generateOutput(getSpecializedGlobalAndEntryPointsComponentType());

        // If we are in command-line mode, we might be expected to actually
        // write output to one or more files here.

        if (m_isCommandLineCompile)
        {
            auto linkage = getLinkage();
            auto program = getSpecializedGlobalAndEntryPointsComponentType();
            for (auto targetReq : linkage->targets)
            {
                if (targetReq->isWholeProgramRequest())
                {
                    writeWholeProgramResult(
                        targetReq);
                }
                else
                {
                    Index entryPointCount = program->getEntryPointCount();
                    for (Index ee = 0; ee < entryPointCount; ++ee)
                    {
                        writeEntryPointResult(
                            targetReq,
                            ee);
                    }
                }
            }

            maybeCreateContainer();
            maybeWriteContainer(m_containerOutputPath);

            _writeDependencyFile(this);
        }
    }

    // Debug logic for dumping intermediate outputs

    //

    void CodeGenContext::dumpIntermediate(
        void const*     data,
        size_t          size,
        char const*     ext,
        bool            isBinary)
    {
        // Try to generate a unique ID for the file to dump,
        // even in cases where there might be multiple threads
        // doing compilation.
        //
        // This is primarily a debugging aid, so we don't
        // really need/want to do anything too elaborate

        static uint32_t counter = 0;
#ifdef _WIN32
        uint32_t id = InterlockedIncrement(&counter);
#else
        // TODO: actually implement the case for other platforms
        uint32_t id = counter++;
#endif

        String path;
        path.append(getIntermediateDumpPrefix());
        path.append(id);
        path.append(ext);

        FILE* file = fopen(path.getBuffer(), isBinary ? "wb" : "w");
        if (!file) return;

        fwrite(data, size, 1, file);
        fclose(file);
    }

    void CodeGenContext::dumpIntermediateText(
        void const*     data,
        size_t          size,
        char const*     ext)
    {
        dumpIntermediate(data, size, ext, false);
    }

    void CodeGenContext::dumpIntermediateBinary(
        void const*     data,
        size_t          size,
        char const*     ext)
    {
        dumpIntermediate(data, size, ext, true);
    }

    void CodeGenContext::maybeDumpIntermediate(
        DownstreamCompileResult* compileResult)
    {
        if (!shouldDumpIntermediates())
            return;

        ComPtr<ISlangBlob> blob;
        if (SLANG_SUCCEEDED(compileResult->getBinary(blob)))
        {
            maybeDumpIntermediate(blob->getBufferPointer(), blob->getBufferSize());
        }
    }

    static const char* _getTargetExtension(CodeGenTarget target)
    {
        switch (target)
        {
            case CodeGenTarget::HLSL:               return ".hlsl";
            case CodeGenTarget::GLSL:               return ".glsl";
            case CodeGenTarget::SPIRV:              return ".spv";
            case CodeGenTarget::DXBytecode:         return ".dxbc";
            case CodeGenTarget::DXIL:               return ".dxil";
            case CodeGenTarget::SPIRVAssembly:      return ".spv.asm";
            case CodeGenTarget::DXBytecodeAssembly: return ".dxbc.asm";
            case CodeGenTarget::DXILAssembly:       return ".dxil.asm";
            case CodeGenTarget::CSource:            return ".c";
            case CodeGenTarget::CUDASource:         return ".cu";
            case CodeGenTarget::CPPSource:          return ".cpp";
            case CodeGenTarget::HostCPPSource:      return ".cpp";

            // What these should be called is target specific, but just use these exts to make clear for now
            // for now
            case CodeGenTarget::HostExecutable:      
            {
                return ".exe";
            }
            case CodeGenTarget::HostHostCallable:
            case CodeGenTarget::ShaderHostCallable:
            case CodeGenTarget::ShaderSharedLibrary: 
            {
                return ".shared-lib";
            }
            default: break;
        }
        return nullptr;
    }

    void CodeGenContext::maybeDumpIntermediate(
        void const*     data,
        size_t          size)
    {
        if (!shouldDumpIntermediates())
            return;

        auto target = getTargetFormat();
        const auto desc = ArtifactDesc::makeFromCompileTarget(asExternal(target));

        if (desc.kind == ArtifactKind::Text)
        {
            dumpIntermediateText(data, size, _getTargetExtension(target));
            return;
        }
        
        switch (target)
        {
#if 0
            case CodeGenTarget::SlangIRAssembly:
            {
                dumpIntermediateText(compileRequest, data, size, ".slang-ir.asm");
                break;
            }
#endif
            case CodeGenTarget::DXIL:
            case CodeGenTarget::DXBytecode:
            case CodeGenTarget::SPIRV:
            {
                const char* ext = _getTargetExtension(target);
                SLANG_ASSERT(ext);

                dumpIntermediateBinary(data, size, ext);
                ComPtr<ISlangBlob> disassemblyBlob;
                if (SLANG_SUCCEEDED(dissassembleWithDownstream(data, size, disassemblyBlob.writeRef())))
                {
                    StringBuilder buf;
                    buf << ext << ".asm";
                    dumpIntermediateText(disassemblyBlob->getBufferPointer(), disassemblyBlob->getBufferSize(), buf.getBuffer());
                }
                break;
            }

            case CodeGenTarget::HostHostCallable:
            case CodeGenTarget::ShaderHostCallable:
            case CodeGenTarget::ShaderSharedLibrary:
            case CodeGenTarget::HostExecutable:
            {
                dumpIntermediateBinary(data, size, _getTargetExtension(target));
                break;
            }
            default:    break;
        }
    }

    void CodeGenContext::maybeDumpIntermediate(
        char const*     text)
    {
        if (!shouldDumpIntermediates())
            return;

        maybeDumpIntermediate(text, strlen(text));
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
        if (auto endToEndReq = isEndToEndCompile())
        {
            if (endToEndReq->getFrontEndReq()->shouldValidateIR)
                return true;
        }

        return false;
    }

    bool CodeGenContext::shouldDumpIR()
    {
        if (getTargetReq()->getTargetFlags() & SLANG_TARGET_FLAG_DUMP_IR)
            return true;

        if (auto endToEndReq = isEndToEndCompile())
        {
            if (endToEndReq->getFrontEndReq()->shouldDumpIR)
                return true;
        }

        return false;
    }

    bool CodeGenContext::shouldDumpIntermediates()
    {
        if (getTargetReq()->shouldDumpIntermediates())
            return true;
        if (auto endToEndReq = isEndToEndCompile())
        {
            if (endToEndReq->shouldDumpIntermediates)
                return true;
        }
        return false;
    }


    bool CodeGenContext::shouldTrackLiveness()
    {
        auto endToEndReq = isEndToEndCompile();
        return (endToEndReq && endToEndReq->enableLivenessTracking) || 
            getTargetReq()->shouldTrackLiveness();
    }

    String CodeGenContext::getIntermediateDumpPrefix()
    {
        if (auto endToEndReq = isEndToEndCompile())
        {
            return endToEndReq->m_dumpIntermediatePrefix;
        }
        return String();
    }

    bool CodeGenContext::getUseUnknownImageFormatAsDefault()
    {
        if (auto endToEndReq = isEndToEndCompile())
        {
            return endToEndReq->useUnknownImageFormatAsDefault;
        }
        return false;
    }

    bool CodeGenContext::isSpecializationDisabled()
    {
        if (auto endToEndReq = isEndToEndCompile())
        {
            return endToEndReq->disableSpecialization;
        }
        return false;
    }
}
