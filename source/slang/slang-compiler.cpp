// Compiler.cpp : Defines the entry point for the console application.
//
#include "../core/slang-basic.h"
#include "../core/slang-platform.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"
#include "../core/slang-hex-dump-util.h"
#include "../core/slang-riff.h"
#include "../core/slang-type-text-util.h"

#include "slang-check.h"
#include "slang-compiler.h"
#include "slang-lexer.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-parameter-binding.h"
#include "slang-parser.h"
#include "slang-preprocessor.h"
#include "slang-type-layout.h"
#include "slang-emit.h"

#include "slang-glsl-extension-tracker.h"
#include "slang-emit-cuda.h"

#include "slang-serialize-container.h"

// Enable calling through to `fxc` or `dxc` to
// generate code on Windows.
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
    #undef WIN32_LEAN_AND_MEAN
    #undef NOMINMAX
    #include <d3dcompiler.h>
    #ifndef SLANG_ENABLE_DXBC_SUPPORT
        #define SLANG_ENABLE_DXBC_SUPPORT 1
    #endif
    #ifndef SLANG_ENABLE_DXIL_SUPPORT
        #define SLANG_ENABLE_DXIL_SUPPORT 1
    #endif
#endif
//
// Otherwise, don't enable DXBC/DXIL by default:
#ifndef SLANG_ENABLE_DXBC_SUPPORT
    #define SLANG_ENABLE_DXBC_SUPPORT 0
#endif
#ifndef SLANG_ENABLE_DXIL_SUPPORT
    #define SLANG_ENABLE_DXIL_SUPPORT 0
#endif

// Enable calling through to `glslang` on
// all platforms.
#ifndef SLANG_ENABLE_GLSLANG_SUPPORT
    #define SLANG_ENABLE_GLSLANG_SUPPORT 1
#endif

#if SLANG_ENABLE_GLSLANG_SUPPORT
#include "../slang-glslang/slang-glslang.h"
#endif

// Includes to allow us to control console
// output when writing assembly dumps.
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace Slang
{

    // !!!!!!!!!!!!!!!!!!!!!! free functions for DiagnosicSink !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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

    static const Guid IID_IEntryPoint = SLANG_UUID_IEntryPoint;

    ISlangUnknown* EntryPoint::getInterface(const Guid& guid)
    {
        if(guid == IID_IEntryPoint)
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
        switch (passThrough)
        {
            case PassThroughMode::None:
            {
                // If no pass through -> that will always work!
                return SLANG_OK;
            }
#if !SLANG_ENABLE_DXIL_SUPPORT
            case PassThroughMode::Dxc: return SLANG_E_NOT_IMPLEMENTED;
#endif
#if !SLANG_ENABLE_DXBC_SUPPORT
            case PassThroughMode::Fxc: return SLANG_E_NOT_IMPLEMENTED;
#endif
#if !SLANG_ENABLE_GLSLANG_SUPPORT
            case PassThroughMode::Glslang: return SLANG_E_NOT_IMPLEMENTED;
#endif

            default: break;
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
            case CodeGenTarget::HostCallable:
            case CodeGenTarget::SharedLibrary:
            case CodeGenTarget::Executable:
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

    bool isPassThroughEnabled(
        EndToEndCompileRequest* endToEndReq)
    {        // If there isn't an end-to-end compile going on,
        // there can be no pass-through.
        //
        if (!endToEndReq) return false;

        // And if pass-through isn't set, we don't need
        // access to the translation unit.
        //
        if(endToEndReq->m_passThrough == PassThroughMode::None) return false;
        return true;
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

    TranslationUnitRequest* findPassThroughTranslationUnit(
        EndToEndCompileRequest* endToEndReq,
        Int                     entryPointIndex)
    {
        if (isPassThroughEnabled(endToEndReq))
            return getPassThroughTranslationUnit(endToEndReq, entryPointIndex);
        return nullptr;
    }

    static void _appendEscapedPath(const UnownedStringSlice& path, StringBuilder& outBuilder)
    {
        for (auto c : path)
        {
            // TODO(JS): Probably want more sophisticated handling... 
            if (c == '\\')
            {
                outBuilder.appendChar(c);
            }
            outBuilder.appendChar(c);
        }
    }

    static void _appendCodeWithPath(const UnownedStringSlice& filePath, const UnownedStringSlice& fileContent, StringBuilder& outCodeBuilder)
    {
        outCodeBuilder << "#line 1 \"";
        _appendEscapedPath(filePath, outCodeBuilder);
        outCodeBuilder << "\"\n";
        outCodeBuilder << fileContent << "\n";
    }

    SlangResult emitEntryPointsSource(
        BackEndCompileRequest*  compileRequest,
        const List<Int>&        entryPointIndices,
        TargetRequest*          targetReq,
        CodeGenTarget           target,
        EndToEndCompileRequest* endToEndReq,
        SourceResult&           outSource)
    {
        outSource.reset();

        if(isPassThroughEnabled(endToEndReq))
        {
            for (auto entryPointIndex : entryPointIndices)
            {
                auto translationUnit = getPassThroughTranslationUnit(endToEndReq, entryPointIndex);
                SLANG_ASSERT(translationUnit);
                // Generate a string that includes the content of
                // the source file(s), along with a line directive
                // to ensure that we get reasonable messages
                // from the downstream compiler when in pass-through
                // mode.

                StringBuilder codeBuilder;
                if (target == CodeGenTarget::GLSL)
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

                outSource.source = codeBuilder.ProduceString();
            }
            return SLANG_OK;
        }
        else
        {
            return emitEntryPointsSourceFromIR(
                compileRequest,
                entryPointIndices,
                target,
                targetReq,
                outSource);
        }
    }

    SlangResult emitEntryPointSource(
        BackEndCompileRequest*  compileRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        CodeGenTarget           target,
        EndToEndCompileRequest* endToEndReq,
        SourceResult&           outSource)
    {
        List<Int> entryPointIndices;
        entryPointIndices.add(entryPointIndex);
        return emitEntryPointsSource(compileRequest, entryPointIndices, targetReq,
            target, endToEndReq, outSource);
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

    String calcSourcePathForEntryPoints(
        EndToEndCompileRequest* endToEndReq,
        const List<Int>&        entryPointIndices)
    {
        String failureMode = "slang-generated";
        if (entryPointIndices.getCount() != 1)
            return failureMode;
        auto entryPointIndex = entryPointIndices[0];
        auto translationUnitRequest = findPassThroughTranslationUnit(endToEndReq, entryPointIndex);
        if (!translationUnitRequest)
            return failureMode;

        const auto& sourceFiles = translationUnitRequest->getSourceFiles();

        auto sink = endToEndReq->getSink();

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

    String calcSourcePathForEntryPoint(
        EndToEndCompileRequest* endToEndReq,
        Int                     entryPointIndex)
    {
        List<Int> entryPointIndices;
        entryPointIndices.add(entryPointIndex);
        return calcSourcePathForEntryPoints(endToEndReq, entryPointIndices);
    }

    // Helper function for cases where we can assume a single entry point
    Int assertSingleEntryPoint(List<Int> const& entryPointIndices) {
        SLANG_ASSERT(entryPointIndices.getCount() == 1);
        return *entryPointIndices.begin();
    }

#if SLANG_ENABLE_DXBC_SUPPORT

    static UnownedStringSlice _getSlice(ID3DBlob* blob)
    {
        if (blob)
        {
            const char* chars = (const char*)blob->GetBufferPointer();
            size_t len = blob->GetBufferSize();
            len -= size_t(len > 0 && chars[len - 1] == 0);
            return UnownedStringSlice(chars, len);
        }
        return UnownedStringSlice();
    }

    struct FxcIncludeHandler : ID3DInclude
    {
        
        STDMETHOD(Open)(D3D_INCLUDE_TYPE includeType, LPCSTR fileName, LPCVOID parentData, LPCVOID* outData, UINT* outSize) override
        {
            SLANG_UNUSED(includeType);
            // NOTE! The pParentData means the *text* of any previous include.
            // In order to work out what *path* that came from, we need to seach which source file it came from, and
            // use it's path

            // Assume the root pathInfo initially 
            PathInfo includedFromPathInfo = m_rootPathInfo;

            // Lets try and find the parent source if there is any
            if (parentData)
            {
                SourceFile* foundSourceFile = m_system.getSourceManager()->findSourceFileByContentRecursively((const char*)parentData);
                if (foundSourceFile)
                {
                    includedFromPathInfo = foundSourceFile->getPathInfo();
                }
            }

            String path(fileName);
            PathInfo pathInfo;
            ComPtr<ISlangBlob> blob;

            SLANG_RETURN_ON_FAIL(m_system.findAndLoadFile(path, includedFromPathInfo.foundPath, pathInfo, blob));

            // Return the data
            *outData = blob->getBufferPointer();
            *outSize = (UINT) blob->getBufferSize();
            
            return S_OK;
        }

        STDMETHOD(Close)(LPCVOID pData) override
        {
            SLANG_UNUSED(pData);
            return S_OK;
        }
        FxcIncludeHandler(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt, SourceManager* sourceManager):
            m_system(searchDirectories, fileSystemExt, sourceManager)
        {
        }

        PathInfo m_rootPathInfo;
        IncludeSystem m_system;
    };

    SlangResult emitDXBytecodeForEntryPoint(
        ComponentType*          program,
        BackEndCompileRequest*  compileRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        List<uint8_t>&          byteCodeOut)
    {
        byteCodeOut.clear();

        auto session = compileRequest->getSession();
        auto sink = compileRequest->getSink();

        auto compileFunc = (pD3DCompile)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Fxc_D3DCompile, sink);
        if (!compileFunc)
        {
            return SLANG_FAIL;
        }

        SourceResult source;
        SLANG_RETURN_ON_FAIL(emitEntryPointSource(compileRequest, entryPointIndex, targetReq, CodeGenTarget::HLSL, endToEndReq, source));

        const auto& hlslCode = source.source;
        maybeDumpIntermediate(compileRequest, hlslCode.getBuffer(), CodeGenTarget::HLSL);

        auto entryPoint = program->getEntryPoint(entryPointIndex);
        auto profile = getEffectiveProfile(entryPoint, targetReq);

        auto linkage = compileRequest->getLinkage();

        // If we have been invoked in a pass-through mode, then we need to make sure
        // that the downstream compiler sees whatever options were passed to Slang
        // via the command line or API.
        //
        // TODO: more pieces of information should be added here as needed.
        //
        List<D3D_SHADER_MACRO> dxMacrosStorage;
        D3D_SHADER_MACRO const* dxMacros = nullptr;

        FxcIncludeHandler fxcIncludeHandlerStorage(&linkage->searchDirectories, linkage->getFileSystemExt(), sink->getSourceManager());
        FxcIncludeHandler* fxcIncludeHandler = &fxcIncludeHandlerStorage;
         
        if(auto translationUnit = findPassThroughTranslationUnit(endToEndReq, entryPointIndex))
        {
            for( auto& define :  translationUnit->compileRequest->preprocessorDefinitions )
            {
                D3D_SHADER_MACRO dxMacro;
                dxMacro.Name = define.Key.getBuffer();
                dxMacro.Definition = define.Value.getBuffer();
                dxMacrosStorage.add(dxMacro);
            }
            for( auto& define : translationUnit->preprocessorDefinitions )
            {
                D3D_SHADER_MACRO dxMacro;
                dxMacro.Name = define.Key.getBuffer();
                dxMacro.Definition = define.Value.getBuffer();
                dxMacrosStorage.add(dxMacro);
            }
            D3D_SHADER_MACRO nullTerminator = { 0, 0 };
            dxMacrosStorage.add(nullTerminator);

            dxMacros = dxMacrosStorage.getBuffer();

            fxcIncludeHandler = &fxcIncludeHandlerStorage;
            fxcIncludeHandlerStorage.m_rootPathInfo = translationUnit->m_sourceFiles[0]->getPathInfo();
        }

        DWORD flags = 0;

        switch( targetReq->getFloatingPointMode() )
        {
        default:
            break;

        case FloatingPointMode::Precise:
            flags |= D3DCOMPILE_IEEE_STRICTNESS;
            break;
        }

        // Some of the `D3DCOMPILE_*` constants aren't available in all
        // versions of `d3dcompiler.h`, so we define them here just in case
        #ifndef D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES
        #define D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES (1 << 20)
        #endif

        #ifndef D3DCOMPILE_ALL_RESOURCES_BOUND
        #define D3DCOMPILE_ALL_RESOURCES_BOUND (1 << 21)
        #endif

        flags |= D3DCOMPILE_ENABLE_STRICTNESS;
        flags |= D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;

        switch( linkage->optimizationLevel )
        {
        default:
            break;

        case OptimizationLevel::None:       flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0; break;
        case OptimizationLevel::Default:    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL1; break;
        case OptimizationLevel::High:       flags |= D3DCOMPILE_OPTIMIZATION_LEVEL2; break;
        case OptimizationLevel::Maximal:    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3; break;
        }

        switch( linkage->debugInfoLevel )
        {
        case DebugInfoLevel::None:
            break;
        
        default:
            flags |= D3DCOMPILE_DEBUG;
            break;
        }

        const String sourcePath = calcSourcePathForEntryPoint(endToEndReq, entryPointIndex);

        ComPtr<ID3DBlob> codeBlob;
        ComPtr<ID3DBlob> diagnosticsBlob;
        HRESULT hr = compileFunc(
            hlslCode.begin(),
            hlslCode.getLength(),
            sourcePath.getBuffer(),
            dxMacros,
            fxcIncludeHandler,
            getText(entryPoint->getName()).begin(),
            GetHLSLProfileName(profile).getBuffer(),
            flags,
            0, // unused: effect flags
            codeBlob.writeRef(),
            diagnosticsBlob.writeRef());

        if (codeBlob && SLANG_SUCCEEDED(hr))
        {
            byteCodeOut.addRange((uint8_t const*)codeBlob->GetBufferPointer(), (int)codeBlob->GetBufferSize());
        }

        if (FAILED(hr))
        {
            reportExternalCompileError("fxc", hr, _getSlice(diagnosticsBlob), sink);
        }
                
        return hr;
    }

    SlangResult dissassembleDXBC(
        BackEndCompileRequest*  compileRequest,
        void const*             data,
        size_t                  size, 
        String&                 assemOut)
    {
        assemOut = String();

        auto session = compileRequest->getSession();
        auto sink = compileRequest->getSink();

        auto disassembleFunc = (pD3DDisassemble)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Fxc_D3DDisassemble, sink);
        if (!disassembleFunc)
        {
            return SLANG_E_NOT_FOUND;
        }

        if (!data || !size)
        {
            return SLANG_FAIL;
        }

        ComPtr<ID3DBlob> codeBlob;
        SlangResult res = disassembleFunc(data, size, 0, nullptr, codeBlob.writeRef());

        if (codeBlob)
        {
            assemOut = _getSlice(codeBlob);
        }
        if (FAILED(res))
        {
            // TODO(tfoley): need to figure out what to diagnose here...
            reportExternalCompileError("fxc", res, UnownedStringSlice(), sink);
        }

        return res;
    }

    SlangResult emitDXBytecodeAssemblyForEntryPoint(
        ComponentType*          program,
        BackEndCompileRequest*  compileRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        String&                 assemOut)
    {

        List<uint8_t> dxbc;
        SLANG_RETURN_ON_FAIL(emitDXBytecodeForEntryPoint(
            program,
            compileRequest,
            entryPointIndex,
            targetReq,
            endToEndReq,
            dxbc));
        if (!dxbc.getCount())
        {
            return SLANG_FAIL;
        }
        return dissassembleDXBC(compileRequest, dxbc.getBuffer(), dxbc.getCount(), assemOut);
    }
#endif

#if SLANG_ENABLE_DXIL_SUPPORT

// Implementations in `dxc-support.cpp`

SlangResult emitDXILForEntryPointUsingDXC(
    ComponentType*          program,
    BackEndCompileRequest*  compileRequest,
    Int                     entryPointIndex,
    TargetRequest*          targetReq,
    EndToEndCompileRequest* endToEndReq,
    List<uint8_t>&          outCode);

SlangResult dissassembleDXILUsingDXC(
    BackEndCompileRequest*  compileRequest,
    void const*             data,
    size_t                  size, 
    String&                 stringOut);

#endif

#if SLANG_ENABLE_GLSLANG_SUPPORT
    SlangResult invokeGLSLCompiler(
        BackEndCompileRequest*      slangCompileRequest,
        glslang_CompileRequest_1_1&     request)
    {
        Session* session = slangCompileRequest->getSession();
        auto sink = slangCompileRequest->getSink();
        auto linkage = slangCompileRequest->getLinkage();

        auto glslang_compile_1_0 = (glslang_CompileFunc_1_0)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Glslang_Compile_1_0, nullptr);
        auto glslang_compile_1_1 = (glslang_CompileFunc_1_1)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Glslang_Compile_1_1, nullptr);

        if(glslang_compile_1_0 == nullptr && glslang_compile_1_1 == nullptr)
        {
            // Try again and put diagnostic to the sink
            session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Glslang_Compile_1_0, sink);
            session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Glslang_Compile_1_1, sink);
            return SLANG_FAIL;
        }

        StringBuilder diagnosticOutput;
        
        auto diagnosticOutputFunc = [](void const* data, size_t size, void* userData)
        {
            (*(StringBuilder*)userData).append((char const*)data, (char const*)data + size);
        };

        request.diagnosticFunc = diagnosticOutputFunc;
        request.diagnosticUserData = &diagnosticOutput;

        request.optimizationLevel = (unsigned)linkage->optimizationLevel;
        request.debugInfoType = (unsigned)linkage->debugInfoLevel;

        int err = 1;
        if (glslang_compile_1_1)
        {
            err = glslang_compile_1_1(&request);   
        }
        else if (glslang_compile_1_0)
        {
            glslang_CompileRequest_1_0 request_1_0;
            request_1_0.set(request);
            err = glslang_compile_1_0(&request_1_0);   
        }

        if (err)
        {
            reportExternalCompileError("glslang", SLANG_FAIL, diagnosticOutput.getUnownedSlice(), sink);
            return SLANG_FAIL;
        }

        return SLANG_OK;
    }

    SlangResult dissassembleSPIRV(
        BackEndCompileRequest*  slangRequest,
        void const*             data,
        size_t                  size, 
        String&                 stringOut)
    {
        stringOut = String();

        String output;
        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            (*(String*)userData).append((char const*)data, (char const*)data + size);
        };

        glslang_CompileRequest_1_1 request;
        memset(&request, 0, sizeof(request));
        request.sizeInBytes = sizeof(request);

            
        request.action = GLSLANG_ACTION_DISSASSEMBLE_SPIRV;

        request.sourcePath = nullptr;

        request.inputBegin = data;
        request.inputEnd = (char*)data + size;

        request.outputFunc = outputFunc;
        request.outputUserData = &output;

        SLANG_RETURN_ON_FAIL(invokeGLSLCompiler(slangRequest, request));

        stringOut = output;
        return SLANG_OK;
    }

    SlangResult emitWithDownstreamForEntryPoints(
        BackEndCompileRequest*  slangRequest,
        const List<Int>&        entryPointIndices,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        RefPtr<DownstreamCompileResult>& outResult)
    {
        outResult.setNull();

        auto sink = slangRequest->getSink();

        auto session = slangRequest->getSession();

        const String originalSourcePath = calcSourcePathForEntryPoints(endToEndReq, entryPointIndices);

        CodeGenTarget sourceTarget = CodeGenTarget::None;
        SourceLanguage sourceLanguage = SourceLanguage::Unknown;

        PassThroughMode downstreamCompiler = endToEndReq ? endToEndReq->m_passThrough : PassThroughMode::None;

        // If we are not in pass through, lookup the default compiler for the emitted source type
        if (downstreamCompiler == PassThroughMode::None)
        {
            auto target = targetReq->getTarget();
            switch (target)
            {
                case CodeGenTarget::PTX:
                {
                    sourceTarget = CodeGenTarget::CUDASource;
                    sourceLanguage = SourceLanguage::CUDA;
                    break;
                }
                case CodeGenTarget::HostCallable:
                case CodeGenTarget::SharedLibrary:
                case CodeGenTarget::Executable:
                {
                    sourceTarget = CodeGenTarget::CPPSource;
                    sourceLanguage = SourceLanguage::CPP;
                    break;
                }
                default: break;
            }

            downstreamCompiler = PassThroughMode(session->getDefaultDownstreamCompiler(SlangSourceLanguage(sourceLanguage)));
        }
        
        // Get the required downstream compiler
        DownstreamCompiler* compiler = session->getOrLoadDownstreamCompiler(downstreamCompiler, sink);

        if (!compiler)
        {
            auto compilerName = TypeTextUtil::getPassThroughAsHumanText((SlangPassThrough)downstreamCompiler);
            if (downstreamCompiler != PassThroughMode::None)
            {
                sink->diagnose(SourceLoc(), Diagnostics::passThroughCompilerNotFound, compilerName);
            }
            else
            {
                sink->diagnose(SourceLoc(), Diagnostics::cppCompilerNotFound, compilerName);
            }
            return SLANG_FAIL;
        }

        Dictionary<String, String> preprocessorDefinitions;
        List<String> includePaths;

        typedef DownstreamCompiler::CompileOptions CompileOptions;
        CompileOptions options;

        /* This is more convoluted than the other scenarios, because when we invoke C/C++ compiler we would ideally like
        to use the original file. We want to do this because we want includes relative to the source file to work, and
        for that to work most easily we want to use the original file, if there is one */
        if (isPassThroughEnabled(endToEndReq))
        {
            // TODO(DG): Review this assertion later
            SLANG_ASSERT(entryPointIndices.getCount() == 1);
            auto translationUnit = getPassThroughTranslationUnit(endToEndReq, entryPointIndices[0]);
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
                auto linkage = targetReq->getLinkage();
                for (auto& define : linkage->preprocessorDefinitions)
                {
                    preprocessorDefinitions.Add(define.Key, define.Value);
                }
            }

            {
                /* TODO(JS): Not totally clear what options should be set here. If we are using the pass through - then using say the defines/includes
                all makes total sense. If we are generating C++ code from slang, then should we really be using these values -> aren't they what is
                being set for the *slang* source, not for the C++ generated code. That being the case it implies that there needs to be a mechanism
                (if there isn't already) to specify such information on a particular pass/pass through etc.

                On invoking DXC for example include paths do not appear to be set at all (even with pass-through).
                */ 

                auto linkage = targetReq->getLinkage();

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

            // We are just passing thru, so it's whatever it originally was
            sourceLanguage = translationUnit->sourceLanguage;
            sourceTarget = CodeGenTarget(DownstreamCompiler::getCompileTarget(SlangSourceLanguage(sourceLanguage)));

            // Special case if we have a single file, so that we pass the path, and the contents
            const auto& sourceFiles = translationUnit->getSourceFiles();
            if (sourceFiles.getCount() == 1)
            {
                const SourceFile* sourceFile = sourceFiles[0];
                const PathInfo& pathInfo = sourceFile->getPathInfo();
                if (pathInfo.type == PathInfo::Type::FoundPath || pathInfo.type == PathInfo::Type::Normal)
                {
                    options.sourceContentsPath = pathInfo.foundPath;
                }
                options.sourceContents = sourceFile->getContent();
            }
            else
            {
                SourceResult source;
                SLANG_RETURN_ON_FAIL(emitEntryPointsSource(slangRequest, entryPointIndices, targetReq, sourceTarget, endToEndReq, source));

                options.sourceContents = source.source;
            }
        }
        else
        {
            SourceResult source;
            SLANG_RETURN_ON_FAIL(emitEntryPointsSource(slangRequest, entryPointIndices, targetReq, sourceTarget, endToEndReq, source));

            // Look for the version
            if (auto cudaTracker = as<CUDAExtensionTracker>(source.extensionTracker))
            {
                if (cudaTracker->m_smVersion.isSet())
                {
                    DownstreamCompiler::CapabilityVersion version;
                    version.kind = DownstreamCompiler::CapabilityVersion::Kind::CUDASM;
                    version.version = cudaTracker->m_smVersion;

                    options.requiredCapabilityVersions.add(version);
                }
            }

            options.sourceContents = source.source;
            
            maybeDumpIntermediate(slangRequest, options.sourceContents.getBuffer(), sourceTarget);
        }

        // Set the source type
        options.sourceLanguage = SlangSourceLanguage(sourceLanguage);

        // Disable exceptions and security checks
        options.flags &= ~(CompileOptions::Flag::EnableExceptionHandling | CompileOptions::Flag::EnableSecurityChecks);

        // Set what kind of target we should build
        switch (targetReq->getTarget())
        {
            case CodeGenTarget::HostCallable:
            case CodeGenTarget::SharedLibrary:
            {
                options.targetType = DownstreamCompiler::TargetType::SharedLibrary;
                break;
            }
            case CodeGenTarget::Executable:
            {
                options.targetType = DownstreamCompiler::TargetType::Executable;
                break;
            }
            case CodeGenTarget::PTX:
            {
                // TODO(JS): Not clear what to do here.
                // For example should 'Kernel' be distinct from 'Executable'. For now just use executable.
                options.targetType = DownstreamCompiler::TargetType::Executable;
                break;
            }
            default: break;
        }

        // Need to configure for the compilation

        {
            auto linkage = targetReq->getLinkage();

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

            switch( targetReq->getFloatingPointMode() )
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
                Index entryPointCount = slangRequest->getProgram()->getEntryPointCount();
                for(Index ee = 0; ee < entryPointCount; ++ee)
                {
                    auto stage = slangRequest->getProgram()->getEntryPoint(ee)->getStage();
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
        }

        // Compile
        RefPtr<DownstreamCompileResult> downstreamCompileResult;
        SLANG_RETURN_ON_FAIL(compiler->compile(options, downstreamCompileResult));
        
        const auto& diagnostics = downstreamCompileResult->getDiagnostics();

        {
            StringBuilder compilerText;
            compiler->getDesc().appendAsText(compilerText);

            StringBuilder builder;

            typedef DownstreamDiagnostic Diagnostic;

            for (const auto& diagnostic : diagnostics.diagnostics)
            {
                builder.Clear();

                builder << diagnostic.filePath << "(" << diagnostic.fileLine <<"): ";

                if (diagnostic.stage == Diagnostic::Stage::Link)
                {
                    builder << "link ";
                }

                // 
                Severity severity = Severity::Error;
                
                switch (diagnostic.severity)
                {
                    case Diagnostic::Severity::Unknown:
                    case Diagnostic::Severity::Error:
                    {
                        severity = Severity::Error;
                        builder << "error";
                        break;
                    }
                    case Diagnostic::Severity::Warning:
                    {
                        severity = Severity::Warning;
                        builder << "warning";
                        break;
                    }
                    case Diagnostic::Severity::Info:
                    {
                        severity = Severity::Note;
                        builder << "info";
                        break;
                    }
                    default: break;
                }

                builder << " " << diagnostic.code << ": " << diagnostic.text;

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

    SlangResult emitSPIRVForEntryPointsDirectly(
        BackEndCompileRequest*  compileRequest,
        const List<Int>&        entryPointIndices,
        TargetRequest*          targetReq,
        List<uint8_t>&          spirvOut);

    SlangResult emitSPIRVForEntryPointsViaGLSL(
        ComponentType*                  program,
        BackEndCompileRequest*          slangRequest,
        const List<Int>&                entryPointIndices,
        TargetRequest*                  targetReq,
        EndToEndCompileRequest*         endToEndReq,
        List<uint8_t>&                  spirvOut)
    {
        spirvOut.clear();

        SourceResult source;

        SLANG_RETURN_ON_FAIL(emitEntryPointsSource(slangRequest, entryPointIndices, targetReq, CodeGenTarget::GLSL, endToEndReq, source));

        const auto& rawGLSL = source.source;

        maybeDumpIntermediate(slangRequest, rawGLSL.getBuffer(), CodeGenTarget::GLSL);

        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            ((List<uint8_t>*)userData)->addRange((uint8_t*)data, size);
        };

        const String sourcePath = calcSourcePathForEntryPoint(endToEndReq, assertSingleEntryPoint(entryPointIndices));

        glslang_CompileRequest_1_1 request;
        memset(&request, 0, sizeof(request));
        request.sizeInBytes = sizeof(request);

        request.action = GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV;
        request.sourcePath = sourcePath.getBuffer();
        auto entryPoint = program->getEntryPoint(assertSingleEntryPoint(entryPointIndices));
        request.slangStage = (SlangStage)entryPoint->getStage();

        request.inputBegin = rawGLSL.begin();
        request.inputEnd = rawGLSL.end();

        if (GLSLExtensionTracker* tracker = as<GLSLExtensionTracker>(source.extensionTracker.Ptr()))
        {
            request.spirvTargetName = nullptr;
            auto spirvLanguageVersion = tracker->getSPIRVVersion();

            request.spirvVersion.major = spirvLanguageVersion.m_major;
            request.spirvVersion.minor = spirvLanguageVersion.m_minor;
            request.spirvVersion.patch = spirvLanguageVersion.m_patch;
        }
        else
        {
            // HACK: look at the requested capabilities of the target,
            // and see if they specify a SPIR-V version that we should
            // pass down.
            //
            auto targetCaps = targetReq->getTargetCaps();
            if(targetCaps.implies(CapabilityAtom::SPIRV_1_4))
            {
                request.spirvVersion.major = 1;
                request.spirvVersion.minor = 4;
                request.spirvVersion.patch = 0;
            }
        }

        request.outputFunc = outputFunc;
        request.outputUserData = &spirvOut;

        SLANG_RETURN_ON_FAIL(invokeGLSLCompiler(slangRequest, request));
        return SLANG_OK;
    }

    SlangResult emitSPIRVForEntryPoints(
        ComponentType*                  program,
        BackEndCompileRequest*          slangRequest,
        const List<Int>&                entryPointIndices,
        TargetRequest*                  targetReq,
        EndToEndCompileRequest*         endToEndReq,
        List<uint8_t>&                  spirvOut)
    {
        if( slangRequest->shouldEmitSPIRVDirectly )
        {
            return emitSPIRVForEntryPointsDirectly(
                slangRequest,
                entryPointIndices,
                targetReq,
                spirvOut);
        }
        else
        {
            return emitSPIRVForEntryPointsViaGLSL(
                program,
                slangRequest,
                entryPointIndices,
                targetReq,
                endToEndReq,
                spirvOut);
        }
    }

    SlangResult emitSPIRVAssemblyForEntryPoints(
        ComponentType*                  program,
        BackEndCompileRequest*          slangRequest,
        const List<Int>&                entryPointIndices,
        TargetRequest*                  targetReq,
        EndToEndCompileRequest*         endToEndReq,
        String&                         assemblyOut)
    {
        List<uint8_t> spirv;
        SLANG_RETURN_ON_FAIL(emitSPIRVForEntryPoints(
            program,
            slangRequest,
            entryPointIndices,
            targetReq,
            endToEndReq,
            spirv));

        if (spirv.getCount() == 0)
            return SLANG_FAIL;

        return dissassembleSPIRV(slangRequest, spirv.begin(), spirv.getCount(), assemblyOut);
    }
#endif

    // Do emit logic for a zero or more entry points
    CompileResult emitEntryPoints(
        ComponentType*                  program,
        BackEndCompileRequest*          compileRequest,
        const List<Int>&                entryPointIndices,
        TargetRequest*                  targetReq,
        EndToEndCompileRequest*         endToEndReq)
    {
        CompileResult result;

        auto target = targetReq->getTarget();

        switch (target)
        {
        case CodeGenTarget::PTX:
        case CodeGenTarget::HostCallable:
        case CodeGenTarget::SharedLibrary:
        case CodeGenTarget::Executable:
            {
                RefPtr<DownstreamCompileResult> downstreamResult;

                if (SLANG_SUCCEEDED(emitWithDownstreamForEntryPoints(
                    compileRequest,
                    entryPointIndices,
                    targetReq,
                    endToEndReq,
                    downstreamResult)))
                {
                    maybeDumpIntermediate(compileRequest, downstreamResult, target);

                    result = CompileResult(downstreamResult);
                }
            }
            break;
        case CodeGenTarget::GLSL:
        case CodeGenTarget::HLSL:
        case CodeGenTarget::CUDASource:
        case CodeGenTarget::CPPSource:
        case CodeGenTarget::CSource:
            {
                SourceResult source;
                if (SLANG_FAILED(emitEntryPointsSource(compileRequest, entryPointIndices,
                    targetReq, target, endToEndReq, source)))
                {
                    return result;
                }

                const auto& code = source.source;
                maybeDumpIntermediate(compileRequest, code.getBuffer(), target);
                result = CompileResult(code);
            }
            break;

#if SLANG_ENABLE_DXBC_SUPPORT
        case CodeGenTarget::DXBytecode:
            {
                // Assert only one entry point case -- move out of this function
                List<uint8_t> code;
                auto entryPointIndex = assertSingleEntryPoint(entryPointIndices);
                if (SLANG_SUCCEEDED(emitDXBytecodeForEntryPoint(
                    program,
                    compileRequest,
                    entryPointIndex,
                    targetReq,
                    endToEndReq,
                    code)))
                {
                    maybeDumpIntermediate(compileRequest, code.getBuffer(), code.getCount(), target);

                    result = CompileResult(ListBlob::moveCreate(code));
                }
            }
            break;

        case CodeGenTarget::DXBytecodeAssembly:
            {
                // Assert only one entry point case
                String code;
                auto entryPointIndex = assertSingleEntryPoint(entryPointIndices);
                if (SLANG_SUCCEEDED(emitDXBytecodeAssemblyForEntryPoint(
                    program,
                    compileRequest,
                    entryPointIndex,
                    targetReq,
                    endToEndReq,
                    code)))
                {
                    maybeDumpIntermediate(compileRequest, code.getBuffer(), target);
                    result = CompileResult(code);
                }
            }
            break;
#endif

#if SLANG_ENABLE_DXIL_SUPPORT
        case CodeGenTarget::DXIL:
            {
                List<uint8_t> code;
                auto entryPointIndex = assertSingleEntryPoint(entryPointIndices);
                if (SLANG_SUCCEEDED(emitDXILForEntryPointUsingDXC(
                    program,
                    compileRequest,
                    entryPointIndex,
                    targetReq,
                    endToEndReq,
                    code)))
                {
                    maybeDumpIntermediate(compileRequest, code.getBuffer(), code.getCount(), target);
                    result = CompileResult(ListBlob::moveCreate(code));
                }
            }
            break;

        case CodeGenTarget::DXILAssembly:
            {
                List<uint8_t> code;
                Int entryPointIndex = assertSingleEntryPoint(entryPointIndices);
                if (SLANG_SUCCEEDED(emitDXILForEntryPointUsingDXC(
                    program,
                    compileRequest,
                    entryPointIndex,
                    targetReq,
                    endToEndReq,
                    code)))
                {
                    String assembly;
                    dissassembleDXILUsingDXC(
                        compileRequest,
                        code.getBuffer(),
                        code.getCount(),
                        assembly);

                    maybeDumpIntermediate(compileRequest, assembly.getBuffer(), target);
                    result = CompileResult(assembly);
                }
            }
            break;
#endif

        case CodeGenTarget::SPIRV:
            {
                List<uint8_t> code;
                if (SLANG_SUCCEEDED(emitSPIRVForEntryPoints(
                    program,
                    compileRequest,
                    entryPointIndices,
                    targetReq,
                    endToEndReq,
                    code)))
                {
                    maybeDumpIntermediate(compileRequest, code.getBuffer(), code.getCount(), target);
                    result = CompileResult(ListBlob::moveCreate(code));
                }
            }
            break;

        case CodeGenTarget::SPIRVAssembly:
            {
                String code;
                if (SLANG_SUCCEEDED(emitSPIRVAssemblyForEntryPoints(
                    program,
                    compileRequest,
                    entryPointIndices,
                    targetReq,
                    endToEndReq,
                    code)))
                {
                    maybeDumpIntermediate(compileRequest, code.getBuffer(), target);
                    result = CompileResult(code);
                }
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

    // Do emit logic for a single entry point
    CompileResult emitEntryPoint(
        ComponentType*          program,
        BackEndCompileRequest*  compileRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq)
    {
        List<Int> entryPointIndices;
        entryPointIndices.add(entryPointIndex);
        return emitEntryPoints(program, compileRequest, entryPointIndices, targetReq, endToEndReq);
    }

    enum class OutputFileKind
    {
        Text,
        Binary,
    };

    static void writeOutputFile(
        BackEndCompileRequest*  compileRequest,
        FILE*                   file,
        String const&           path,
        void const*             data,
        size_t                  size)
    {
        size_t count = fwrite(data, size, 1, file);
        if (count != 1)
        {
            compileRequest->getSink()->diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
        }
    }

    static void writeOutputFile(
        BackEndCompileRequest*  compileRequest,
        ISlangWriter*           writer,
        String const&           path,
        void const*             data,
        size_t                  size)
    {

        if (SLANG_FAILED(writer->write((const char*)data, size)))
        {
            compileRequest->getSink()->diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
        }
    }

    static void writeOutputFile(
        BackEndCompileRequest*  compileRequest,
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
            compileRequest->getSink()->diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
            return;
        }

        writeOutputFile(compileRequest, file, path, data, size);
        fclose(file);
    }

    static void writeCompileResultToFile(
        BackEndCompileRequest* compileRequest,
        String const& outputPath,
        CompileResult const& result)
    {
        switch (result.format)
        {
        case ResultFormat::Text:
            {
                auto text = result.outputString;
                writeOutputFile(compileRequest,
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
                writeOutputFile(compileRequest,
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

    static void writeEntryPointResultToFile(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        String const&           outputPath,
        CompileResult const&    result)
    {
        SLANG_UNUSED(entryPoint);
        writeCompileResultToFile(compileRequest, outputPath, result);
    }

    static void writeOutputToConsole(
        ISlangWriter* writer,
        String const&   text)
    {
        writer->write(text.getBuffer(), text.getLength());
    }

    static void writeCompileRequestToStandardOutput(
        EndToEndCompileRequest* compileRequest,
        TargetRequest* targetReq,
        CompileResult const& result)
    {
        ISlangWriter* writer = compileRequest->getWriter(WriterChannel::StdOutput);
        auto backEndReq = compileRequest->getBackEndReq();

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
                #if SLANG_ENABLE_DXBC_SUPPORT
                    case CodeGenTarget::DXBytecode:
                        {
                            String assembly;
                            dissassembleDXBC(backEndReq, blobData, blobSize, assembly);
                            writeOutputToConsole(writer, assembly);
                        }
                        break;
                #endif

                #if SLANG_ENABLE_DXIL_SUPPORT
                    case CodeGenTarget::DXIL:
                        {
                            String assembly;
                            dissassembleDXILUsingDXC(backEndReq, blobData, blobSize, assembly);
                            writeOutputToConsole(writer, assembly);
                        }
                        break;
                #endif

                    case CodeGenTarget::SPIRV:
                        {
                            String assembly;
                            dissassembleSPIRV(backEndReq, blobData, blobSize, assembly);
                            writeOutputToConsole(writer, assembly);
                        }
                        break;

                    case CodeGenTarget::PTX:
                        // For now we just dump PTX out as hex

                    case CodeGenTarget::HostCallable:
                    case CodeGenTarget::SharedLibrary:
                    case CodeGenTarget::Executable:
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
                    backEndReq,
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

    static void writeEntryPointResultToStandardOutput(
        EndToEndCompileRequest*     compileRequest,
        EntryPoint*                 entryPoint,
        TargetRequest*              targetReq,
        CompileResult const&        result)
    {
        SLANG_UNUSED(entryPoint);
        writeCompileRequestToStandardOutput(compileRequest, targetReq, result);
    }

    static void writeWholeProgramResult(
        EndToEndCompileRequest* compileRequest,
        TargetRequest*          targetReq)
    {
        auto program = compileRequest->getSpecializedGlobalAndEntryPointsComponentType();
        auto targetProgram = program->getTargetProgram(targetReq);
        auto backEndReq = compileRequest->getBackEndReq();

        auto& result = targetProgram->getExistingWholeProgramResult();

        // Skip the case with no output
        if (result.format == ResultFormat::None)
            return;

        // It is possible that we are dynamically discovering entry
        // points (using `[shader(...)]` attributes), so that there
        // might be entry points added to the program that did not
        // get paths specified via command-line options.
        //
        RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
        if (compileRequest->m_targetInfos.TryGetValue(targetReq, targetInfo))
        {
            String outputPath = targetInfo->wholeTargetOutputPath;
            if (outputPath != "")
            {
                writeCompileResultToFile(backEndReq, outputPath, result);
                return;
            }
        }

        writeCompileRequestToStandardOutput(compileRequest, targetReq, result);
    }

    static void writeEntryPointResult(
        ComponentType*          currentProgram,
        EndToEndCompileRequest* compileRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq)
    {
        auto program = compileRequest->getSpecializedGlobalAndEntryPointsComponentType();
        auto targetProgram = program->getTargetProgram(targetReq);
        auto backEndReq = compileRequest->getBackEndReq();

        auto& result = targetProgram->getExistingEntryPointResult(entryPointIndex);

        // Skip the case with no output
        if (result.format == ResultFormat::None)
            return;

        // It is possible that we are dynamically discovering entry
        // points (using `[shader(...)]` attributes), so that there
        // might be entry points added to the program that did not
        // get paths specified via command-line options.
        //
        RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
        auto entryPoint = currentProgram->getEntryPoint(entryPointIndex);
        if(compileRequest->m_targetInfos.TryGetValue(targetReq, targetInfo))
        {
            String outputPath;
            if(targetInfo->entryPointOutputPaths.TryGetValue(entryPointIndex, outputPath))
            {
                writeEntryPointResultToFile(backEndReq, entryPoint, outputPath, result);
                return;
            }
        }

        writeEntryPointResultToStandardOutput(compileRequest, entryPoint, targetReq, result);
    }

    CompileResult& TargetProgram::_createWholeProgramResult(
        BackEndCompileRequest*  backEndRequest,
        EndToEndCompileRequest* endToEndRequest)
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
        result = emitEntryPoints(
            m_program,
            backEndRequest,
            entryPointIndices,
            m_targetReq,
            endToEndRequest);

        return result;
    }

    CompileResult& TargetProgram::_createEntryPointResult(
        Int                     entryPointIndex,
        BackEndCompileRequest*  backEndRequest,
        EndToEndCompileRequest* endToEndRequest)
    {
        // It is possible that entry points goot added to the `Program`
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
        result = emitEntryPoint(
            m_program,
            backEndRequest,
            entryPointIndex,
            m_targetReq,
            endToEndRequest);

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

        RefPtr<BackEndCompileRequest> backEndRequest = new BackEndCompileRequest(
            m_program->getLinkage(),
            sink,
            m_program);

        return _createWholeProgramResult(
            backEndRequest,
            nullptr);
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

        RefPtr<BackEndCompileRequest> backEndRequest = new BackEndCompileRequest(
            m_program->getLinkage(),
            sink,
            m_program);

        return _createEntryPointResult(
            entryPointIndex,
            backEndRequest,
            nullptr);
    }

    void generateOutputForTarget(
        BackEndCompileRequest*  compileReq,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq)
    {
        auto program = compileReq->getProgram();
        auto targetProgram = program->getTargetProgram(targetReq);

        // Generate target code any entry points that
        // have been requested for compilation.
        auto entryPointCount = program->getEntryPointCount();
        if (targetReq->isWholeProgramRequest())
        {
            targetProgram->_createWholeProgramResult(
                compileReq,
                endToEndReq);
        }
        else
        {
            for (Index ii = 0; ii < entryPointCount; ++ii)
            {
                targetProgram->_createEntryPointResult(
                    ii,
                    compileReq,
                    endToEndReq);
            }
        }
    }

    
    SlangResult EndToEndCompileRequest::writeContainerToStream(Stream* stream)
    {
        auto linkage = getLinkage();

        // Set up options
        SerialContainerUtil::WriteOptions options;

        options.compressionType = linkage->serialCompressionType;
        if (linkage->debugInfoLevel != DebugInfoLevel::None)
        {
            options.optionFlags |= SerialOptionFlag::SourceLocation;
        }
        if (linkage->m_obfuscateCode)
        {
            // If code is obfuscated, we *disable* AST output as it is not obfuscated and will reveal
            // too much about IR.
            // Also currently only IR is needed.
            options.optionFlags &= ~SerialOptionFlag::ASTModule;
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

        FileStream stream(fileName, FileMode::Create, FileAccess::Write, FileShare::ReadWrite);
        try
        {
            stream.write(m_containerBlob->getBufferPointer(), m_containerBlob->getBufferSize());
        }
        catch (const IOException&)
        {
            // Unable to write
            return SLANG_FAIL;
        }
        return SLANG_OK;
    }


    static void _generateOutput(
        BackEndCompileRequest* compileRequest,
        EndToEndCompileRequest* endToEndReq)
    {
        // When dynamic dispatch is disabled, the program must
        // be fully specialized by now. So we check if we still
        // have unspecialized generic/existential parameters,
        // and report them as an error.
        //
        auto program = compileRequest->getProgram();
        auto specializationParamCount = program->getSpecializationParamCount();
        if (compileRequest->disableDynamicDispatch && specializationParamCount != 0)
        {
            auto sink = compileRequest->getSink();
            
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
        auto linkage = compileRequest->getLinkage();
        for (auto targetReq : linkage->targets)
        {
            generateOutputForTarget(compileRequest, targetReq, endToEndReq);
        }
    }

    void generateOutput(
        BackEndCompileRequest* compileRequest)
    {
        _generateOutput(compileRequest, nullptr);
    }

    void generateOutput(
        EndToEndCompileRequest* compileRequest)
    {
        _generateOutput(compileRequest->getBackEndReq(), compileRequest);

        // If we are in command-line mode, we might be expected to actually
        // write output to one or more files here.

        if (compileRequest->m_isCommandLineCompile)
        {
            auto linkage = compileRequest->getLinkage();
            auto program = compileRequest->getSpecializedGlobalAndEntryPointsComponentType();
            for (auto targetReq : linkage->targets)
            {
                Index entryPointCount = program->getEntryPointCount();
                if (targetReq->isWholeProgramRequest()) {
                    writeWholeProgramResult(
                        compileRequest,
                        targetReq);
                }
                else
                {
                    for (Index ee = 0; ee < entryPointCount; ++ee)
                    {
                        writeEntryPointResult(
                            program,
                            compileRequest,
                            ee,
                            targetReq);
                    }
                }
            }

            compileRequest->maybeCreateContainer();
            compileRequest->maybeWriteContainer(compileRequest->m_containerOutputPath);
        }
    }

    // Debug logic for dumping intermediate outputs

    //

    void dumpIntermediate(
        BackEndCompileRequest* request,
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
#ifdef WIN32
        uint32_t id = InterlockedIncrement(&counter);
#else
        // TODO: actually implement the case for other platforms
        uint32_t id = counter++;
#endif

        String path;
        path.append(request->m_dumpIntermediatePrefix);
        path.append(id);
        path.append(ext);

        FILE* file = fopen(path.getBuffer(), isBinary ? "wb" : "w");
        if (!file) return;

        fwrite(data, size, 1, file);
        fclose(file);
    }

    void dumpIntermediateText(
        BackEndCompileRequest* compileRequest,
        void const*     data,
        size_t          size,
        char const*     ext)
    {
        dumpIntermediate(compileRequest, data, size, ext, false);
    }

    void dumpIntermediateBinary(
        BackEndCompileRequest* compileRequest,
        void const*     data,
        size_t          size,
        char const*     ext)
    {
        dumpIntermediate(compileRequest, data, size, ext, true);
    }

    void maybeDumpIntermediate(
        BackEndCompileRequest* compileRequest,
        DownstreamCompileResult* compileResult,
        CodeGenTarget   target)
    {
        if (!compileRequest->shouldDumpIntermediates)
            return;

        ComPtr<ISlangBlob> blob;
        if (SLANG_SUCCEEDED(compileResult->getBinary(blob)))
        {
            maybeDumpIntermediate(compileRequest, blob->getBufferPointer(), blob->getBufferSize(), target);
        }
    }

    void maybeDumpIntermediate(
        BackEndCompileRequest* compileRequest,
        void const*     data,
        size_t          size,
        CodeGenTarget   target)
    {
        if (!compileRequest->shouldDumpIntermediates)
            return;

        switch (target)
        {
        default:
            break;

        case CodeGenTarget::HLSL:
            dumpIntermediateText(compileRequest, data, size, ".hlsl");
            break;

        case CodeGenTarget::GLSL:
            dumpIntermediateText(compileRequest, data, size, ".glsl");
            break;

        case CodeGenTarget::SPIRVAssembly:
            dumpIntermediateText(compileRequest, data, size, ".spv.asm");
            break;

#if 0
        case CodeGenTarget::SlangIRAssembly:
            dumpIntermediateText(compileRequest, data, size, ".slang-ir.asm");
            break;
#endif

        case CodeGenTarget::SPIRV:
            dumpIntermediateBinary(compileRequest, data, size, ".spv");
            {
                String spirvAssembly;
                dissassembleSPIRV(compileRequest, data, size, spirvAssembly);
                dumpIntermediateText(compileRequest, spirvAssembly.begin(), spirvAssembly.getLength(), ".spv.asm");
            }
            break;

    #if SLANG_ENABLE_DXBC_SUPPORT
        case CodeGenTarget::DXBytecodeAssembly:
            dumpIntermediateText(compileRequest, data, size, ".dxbc.asm");
            break;

        case CodeGenTarget::DXBytecode:
            dumpIntermediateBinary(compileRequest, data, size, ".dxbc");
            {
                String dxbcAssembly;
                dissassembleDXBC(compileRequest, data, size, dxbcAssembly);
                dumpIntermediateText(compileRequest, dxbcAssembly.begin(), dxbcAssembly.getLength(), ".dxbc.asm");
            }
            break;
    #endif

    #if SLANG_ENABLE_DXIL_SUPPORT
        case CodeGenTarget::DXILAssembly:
            dumpIntermediateText(compileRequest, data, size, ".dxil.asm");
            break;

        case CodeGenTarget::DXIL:
            dumpIntermediateBinary(compileRequest, data, size, ".dxil");
            {
                String dxilAssembly;
                dissassembleDXILUsingDXC(compileRequest, data, size, dxilAssembly);
                dumpIntermediateText(compileRequest, dxilAssembly.begin(), dxilAssembly.getLength(), ".dxil.asm");
            }
            break;
    #endif

        case CodeGenTarget::CSource:
            dumpIntermediateText(compileRequest, data, size, ".c");
            break;
        case CodeGenTarget::CPPSource:
            dumpIntermediateText(compileRequest, data, size, ".cpp");
            break;

        case CodeGenTarget::Executable:
            // What these should be called is target specific, but just use these exts to make clear for now
            // for now
            dumpIntermediateBinary(compileRequest, data, size, ".exe");
            break;
        case CodeGenTarget::HostCallable:
        case CodeGenTarget::SharedLibrary:
            dumpIntermediateBinary(compileRequest, data, size, ".shared-lib");
            break;
        }
    }

    void maybeDumpIntermediate(
        BackEndCompileRequest* compileRequest,
        char const*     text,
        CodeGenTarget   target)
    {
        if (!compileRequest->shouldDumpIntermediates)
            return;

        maybeDumpIntermediate(compileRequest, text, strlen(text), target);
    }
}
