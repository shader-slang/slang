// Compiler.cpp : Defines the entry point for the console application.
//
#include "../core/slang-basic.h"
#include "../core/slang-platform.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"
#include "../core/slang-hex-dump-util.h"
#include "../core/slang-riff.h"

#include "slang-check.h"
#include "slang-compiler.h"
#include "slang-lexer.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-parameter-binding.h"
#include "slang-parser.h"
#include "slang-preprocessor.h"
#include "slang-type-layout.h"
#include "slang-reflection.h"
#include "slang-emit.h"

#include "slang-ir-serialize.h"

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

// NOTE! These must be in the same order as the SlangCompileTarget enum 
#define SLANG_CODE_GEN_TARGETS(x) \
    x("unknown", Unknown) \
    x("none", None) \
    x("glsl", GLSL) \
    x("glsl-vulkan", GLSL_Vulkan) \
    x("glsl-vulkan-one-desc", GLSL_Vulkan_OneDesc) \
    x("hlsl", HLSL) \
    x("spirv", SPIRV) \
    x("spirv-asm,spirv-assembly", SPIRVAssembly) \
    x("dxbc", DXBytecode) \
    x("dxbc-asm,dxbc-assembly", DXBytecodeAssembly) \
    x("dxil", DXIL) \
    x("dxil-asm,dxil-assembly", DXILAssembly) \
    x("c", CSource) \
    x("cpp", CPPSource) \
    x("exe,executable", Executable) \
    x("sharedlib,sharedlibrary,dll", SharedLibrary) \
    x("callable,host-callable", HostCallable) \
    x("cu,cuda", CUDASource) \
    x("ptx", PTX) 

#define SLANG_CODE_GEN_INFO(names, e) \
    { CodeGenTarget::e, UnownedStringSlice::fromLiteral(names) },

    struct CodeGenTargetInfo
    {
        CodeGenTarget target;
        UnownedStringSlice names;
    };

    static const CodeGenTargetInfo s_codeGenTargetInfos[] = 
    {
        SLANG_CODE_GEN_TARGETS(SLANG_CODE_GEN_INFO)
    };

    CodeGenTarget calcCodeGenTargetFromName(const UnownedStringSlice& name)
    {
        for (int i = 0; i < SLANG_COUNT_OF(s_codeGenTargetInfos); ++i)
        {
            const auto& info = s_codeGenTargetInfos[i];

            // If this assert fails, then the SLANG_CODE_GEN_TARGETS macro has the wrong order
            SLANG_ASSERT(i == int(info.target));

            if (StringUtil::indexOfInSplit(info.names, ',', name) >= 0)
            {
                return info.target;
            }
        }
        return CodeGenTarget::Unknown;
    }
    UnownedStringSlice getCodeGenTargetName(CodeGenTarget target)
    {
        // Return the first name
        return StringUtil::getAtInSplit(s_codeGenTargetInfos[int(target)].names, ',', 0);
    }

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
                case ResultFormat::None:
                default:
                    break;

                case ResultFormat::Text:
                    blob = StringUtil::createStringBlob(outputString);
                    break;
                case ResultFormat::Binary:
                {
                    if (downstreamResult)
                    {
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

    RefPtr<EntryPoint> EntryPoint::create(
        Linkage*            linkage,
        DeclRef<FuncDecl>   funcDeclRef,
        Profile             profile)
    {
        RefPtr<EntryPoint> entryPoint = new EntryPoint(
            linkage,
            funcDeclRef.GetName(),
            profile,
            funcDeclRef);
        entryPoint->m_mangledName = getMangledName(funcDeclRef);
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

    Profile Profile::LookUp(char const* name)
    {
        #define PROFILE(TAG, NAME, STAGE, VERSION)	if(strcmp(name, #NAME) == 0) return Profile::TAG;
        #define PROFILE_ALIAS(TAG, DEF, NAME)		if(strcmp(name, #NAME) == 0) return Profile::TAG;
        #include "slang-profile-defs.h"

        return Profile::Unknown;
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

    static UnownedStringSlice _getPassThroughAsText(PassThroughMode mode)
    {
        switch (mode)
        {
            case PassThroughMode::None:     return UnownedStringSlice::fromLiteral("None");
            case PassThroughMode::Dxc:      return UnownedStringSlice::fromLiteral("Dxc");
            case PassThroughMode::Fxc:      return UnownedStringSlice::fromLiteral("Fxc");
            case PassThroughMode::Glslang:  return UnownedStringSlice::fromLiteral("Glslang");
            case PassThroughMode::Clang:    return UnownedStringSlice::fromLiteral("Clang");
            case PassThroughMode::VisualStudio: return UnownedStringSlice::fromLiteral("VisualStudio");
            case PassThroughMode::Gcc:      return UnownedStringSlice::fromLiteral("GCC");
            case PassThroughMode::GenericCCpp:  return UnownedStringSlice::fromLiteral("Generic C/C++ Compiler");
            default:                        return UnownedStringSlice::fromLiteral("Unknown");
        }
    }

    static const struct
    {
        char const*     name;
        SourceLanguage  sourceLanguage;
    } kSourceLanguages[] =
    {
        { "slang", SourceLanguage::Slang },
        { "hlsl", SourceLanguage::HLSL },
        { "glsl", SourceLanguage::GLSL },
        { "c", SourceLanguage::C },
        { "cxx", SourceLanguage::CPP },
        { "cuda", SourceLanguage::CUDA },
    };

    SourceLanguage findSourceLanguageByName(String const& name)
    {
        for (auto entry : kSourceLanguages)
        {
            if (name == entry.name)
            {
                return entry.sourceLanguage;
            }
        }

        return SourceLanguage::Unknown;
    };

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

    PassThroughMode getDownstreamCompilerRequiredForTarget(CodeGenTarget target)
    {
        switch (target)
        {
            case CodeGenTarget::None:
            {
                return PassThroughMode::None;
            }
            case CodeGenTarget::GLSL:
            {
                // Can always output GLSL
                return PassThroughMode::None; 
            }
            case CodeGenTarget::HLSL:
            {
                // Can always output HLSL
                return PassThroughMode::None;
            }
            case CodeGenTarget::CUDASource:
            {
                // Can always output CUDA
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
            case CodeGenTarget::CPPSource:
            case CodeGenTarget::CSource:
            {
                // Don't need an external compiler to output C and C++ code
                return PassThroughMode::None;
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

    SlangResult checkCompileTargetSupport(Session* session, CodeGenTarget target)
    {
        const PassThroughMode mode = getDownstreamCompilerRequiredForTarget(target);
        return (mode != PassThroughMode::None) ?
            checkExternalCompilerSupport(session, mode) :
            SLANG_OK;
    }

    //

        /// If there is a pass-through compile going on, find the translation unit for the given entry point.
    TranslationUnitRequest* findPassThroughTranslationUnit(
        EndToEndCompileRequest* endToEndReq,
        Int                     entryPointIndex)
    {
        // If there isn't an end-to-end compile going on,
        // there can be no pass-through.
        //
        if(!endToEndReq) return nullptr;

        // And if pass-through isn't set, we don't need
        // access to the translation unit.
        //
        if(endToEndReq->passThrough == PassThroughMode::None) return nullptr;

        auto frontEndReq = endToEndReq->getFrontEndReq();
        auto entryPointReq = frontEndReq->getEntryPointReq(entryPointIndex);
        auto translationUnit = entryPointReq->getTranslationUnit();
        return translationUnit;
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

    String emitEntryPointSource(
        BackEndCompileRequest*  compileRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        CodeGenTarget           target,
        EndToEndCompileRequest* endToEndReq)
    {
        if(auto translationUnit = findPassThroughTranslationUnit(endToEndReq, entryPointIndex))
        {
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
                for(auto sourceFile : translationUnit->getSourceFiles())
                {
                    _appendCodeWithPath(sourceFile->getPathInfo().foundPath.getUnownedSlice(), sourceFile->getContent(), codeBuilder);
                }
            }
            return codeBuilder.ProduceString();
        }
        else
        {
            return emitEntryPointSourceFromIR(
                compileRequest,
                entryPointIndex,
                target,
                targetReq);
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
        switch( profile.GetStage() )
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
    #undef CASE
        }

        char const* versionSuffix = nullptr;
        switch(profile.GetVersion())
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

        if (diagnostic.size() > 0)
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
        if (sink->flags & DiagnosticSink::Flag::VerbosePath)
        {
            return sourceFile->calcVerbosePath();
        }
        else
        {
            return sourceFile->getPathInfo().foundPath;
        }
    }

    String calcSourcePathForEntryPoint(
        EndToEndCompileRequest* endToEndReq,
        UInt                    entryPointIndex)
    {
        auto translationUnitRequest = findPassThroughTranslationUnit(endToEndReq, entryPointIndex);
        if(!translationUnitRequest)
            return "slang-generated";

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

        /// Read a file in the context of handling a preprocessor directive
    static SlangResult readFile(
        Linkage*        linkage,
        String const&   path,
        ISlangBlob**    outBlob)
    {
        // The actual file loading will be handled by the file system
        // associated with the parent linkage.
        //
        auto fileSystemExt = linkage->getFileSystemExt();
        SLANG_RETURN_ON_FAIL(fileSystemExt->loadFile(path.getBuffer(), outBlob));

        return SLANG_OK;
    }

    struct FxcIncludeHandler : ID3DInclude
    {
        Linkage*        linkage;
        DiagnosticSink* sink;
        IncludeHandler* includeHandler;
        PathInfo        rootPathInfo;

        STDMETHOD(Open)(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override
        {
            SLANG_UNUSED(IncludeType);
            SLANG_UNUSED(pParentData);

            String path(pFileName);

            SourceLoc loc;

            PathInfo includedFromPathInfo = rootPathInfo;

            if (!includeHandler)
            {
                return SLANG_E_NOT_IMPLEMENTED;
            }

            // Find the path relative to the foundPath
            PathInfo filePathInfo;
            if (SLANG_FAILED(includeHandler->findFile(path, includedFromPathInfo.foundPath, filePathInfo)))
            {
                return SLANG_E_CANNOT_OPEN;
            }

            // We must have a uniqueIdentity to be compare
            if (!filePathInfo.hasUniqueIdentity())
            {
                return SLANG_E_ABORT;
            }

            // Simplify the path
            filePathInfo.foundPath = includeHandler->simplifyPath(filePathInfo.foundPath);

            // See if this an already loaded source file
            auto sourceManager = linkage->getSourceManager();
            SourceFile* sourceFile = sourceManager->findSourceFileRecursively(filePathInfo.uniqueIdentity);

            // If not create a new one, and add to the list of known source files
            if (!sourceFile)
            {
                ComPtr<ISlangBlob> foundSourceBlob;
                if (SLANG_FAILED(readFile(linkage, filePathInfo.foundPath, foundSourceBlob.writeRef())))
                {
                    return SLANG_E_CANNOT_OPEN;
                }

                sourceFile = sourceManager->createSourceFileWithBlob(filePathInfo, foundSourceBlob);
                sourceManager->addSourceFile(filePathInfo.uniqueIdentity, sourceFile);
            }

            // This is a new parse (even if it's a pre-existing source file), so create a new SourceUnit
            SourceView* sourceView = sourceManager->createSourceView(sourceFile, &filePathInfo);

            *ppData = sourceView->getContent().begin();
            *pBytes = (UINT) sourceView->getContentSize();

            return S_OK;
        }

        STDMETHOD(Close)(LPCVOID pData) override
        {
            SLANG_UNUSED(pData);
            return S_OK;
        }
    };

    SlangResult emitDXBytecodeForEntryPoint(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
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

        auto hlslCode = emitEntryPointSource(compileRequest, entryPointIndex, targetReq, CodeGenTarget::HLSL, endToEndReq);
        maybeDumpIntermediate(compileRequest, hlslCode.getBuffer(), CodeGenTarget::HLSL);

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

        IncludeHandlerImpl includeHandler;
        includeHandler.linkage = linkage;
        includeHandler.searchDirectories = &linkage->searchDirectories;

        FxcIncludeHandler fxcIncludeHandlerStorage;
        FxcIncludeHandler* fxcIncludeHandler = nullptr;

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
            fxcIncludeHandler->linkage = linkage;
            fxcIncludeHandler->sink = compileRequest->getSink();
            fxcIncludeHandler->includeHandler = &includeHandler;
            fxcIncludeHandler->rootPathInfo = translationUnit->m_sourceFiles[0]->getPathInfo();
        }

        DWORD flags = 0;

        switch( targetReq->floatingPointMode )
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
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        String&                 assemOut)
    {

        List<uint8_t> dxbc;
        SLANG_RETURN_ON_FAIL(emitDXBytecodeForEntryPoint(
            compileRequest,
            entryPoint,
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
    BackEndCompileRequest*  compileRequest,
    EntryPoint*             entryPoint,
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
        glslang_CompileRequest&     request)
    {
        Session* session = slangCompileRequest->getSession();
        auto sink = slangCompileRequest->getSink();
        auto linkage = slangCompileRequest->getLinkage();

        auto glslang_compile = (glslang_CompileFunc)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Glslang_Compile, sink);
        if (!glslang_compile)
        {
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

        int err = glslang_compile(&request);

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

        glslang_CompileRequest request;
        request.action = GLSLANG_ACTION_DISSASSEMBLE_SPIRV;

        request.sourcePath = nullptr;

        request.inputBegin  = data;
        request.inputEnd    = (char*)data + size;

        request.outputFunc = outputFunc;
        request.outputUserData = &output;

        SLANG_RETURN_ON_FAIL(invokeGLSLCompiler(slangRequest, request));

        stringOut = output;
        return SLANG_OK;
    }

    SlangResult emitWithDownstreamForEntryPoint(
        BackEndCompileRequest*  slangRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        RefPtr<DownstreamCompileResult>& outResult)
    {
        outResult.setNull();

        auto sink = slangRequest->getSink();

        auto session = slangRequest->getSession();

        const String originalSourcePath = calcSourcePathForEntryPoint(endToEndReq, entryPointIndex);

        CodeGenTarget sourceTarget = CodeGenTarget::None;
        SourceLanguage sourceLanguage = SourceLanguage::Unknown;

        PassThroughMode downstreamCompiler = endToEndReq->passThrough;

        // If we are not in pass through, lookup the default compiler for the emitted source type
        if (downstreamCompiler == PassThroughMode::None)
        {
            auto target = targetReq->target;
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
            auto compilerName = _getPassThroughAsText(downstreamCompiler);
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
        if (auto translationUnit = findPassThroughTranslationUnit(endToEndReq, entryPointIndex))
        {
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
                options.sourceContents = emitEntryPointSource(slangRequest, entryPointIndex, targetReq, sourceTarget, endToEndReq);
            }
        }
        else
        {
            options.sourceContents = emitEntryPointSource(slangRequest, entryPointIndex, targetReq, sourceTarget, endToEndReq);

            maybeDumpIntermediate(slangRequest, options.sourceContents.getBuffer(), sourceTarget);
        }

        // Set the source type
        options.sourceLanguage = SlangSourceLanguage(sourceLanguage);

        // Disable exceptions and security checks
        options.flags &= ~(CompileOptions::Flag::EnableExceptionHandling | CompileOptions::Flag::EnableSecurityChecks);

        // Set what kind of target we should build
        switch (targetReq->target)
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

            switch( targetReq->floatingPointMode )
            {
                case FloatingPointMode::Default:    options.floatingPointMode = DownstreamCompiler::FloatingPointMode::Default; break;
                case FloatingPointMode::Precise:    options.floatingPointMode = DownstreamCompiler::FloatingPointMode::Precise; break;
                case FloatingPointMode::Fast:       options.floatingPointMode = DownstreamCompiler::FloatingPointMode::Fast; break;
                default: SLANG_ASSERT(!"Unhandled floating point mode");
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
                
                switch (diagnostic.type)
                {
                    case Diagnostic::Type::Unknown:
                    case Diagnostic::Type::Error:
                    {
                        severity = Severity::Error;
                        builder << "error";
                        break;
                    }
                    case Diagnostic::Type::Warning:
                    {
                        severity = Severity::Warning;
                        builder << "warning";
                        break;
                    }
                    case Diagnostic::Type::Info:
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
        if (diagnostics.has(DownstreamDiagnostic::Type::Error))
        {
            return SLANG_FAIL;
        }

        outResult = downstreamCompileResult;
        return SLANG_OK;
    }

    SlangResult emitSPIRVForEntryPointDirectly(
        BackEndCompileRequest*  compileRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        List<uint8_t>&          spirvOut);

    SlangResult emitSPIRVForEntryPointViaGLSL(
        BackEndCompileRequest*  slangRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        List<uint8_t>&          spirvOut)
    {
        spirvOut.clear();

        String rawGLSL = emitEntryPointSource(
            slangRequest,
            entryPointIndex,
            targetReq,
            CodeGenTarget::GLSL,
            endToEndReq);
        maybeDumpIntermediate(slangRequest, rawGLSL.getBuffer(), CodeGenTarget::GLSL);

        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            ((List<uint8_t>*)userData)->addRange((uint8_t*)data, size);
        };

        const String sourcePath = calcSourcePathForEntryPoint(endToEndReq, entryPointIndex);

        glslang_CompileRequest request;
        request.action = GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV;
        request.sourcePath = sourcePath.getBuffer();
        request.slangStage = (SlangStage)entryPoint->getStage();

        request.inputBegin  = rawGLSL.begin();
        request.inputEnd    = rawGLSL.end();

        request.outputFunc = outputFunc;
        request.outputUserData = &spirvOut;

        SLANG_RETURN_ON_FAIL(invokeGLSLCompiler(slangRequest, request));
        return SLANG_OK;
    }

    SlangResult emitSPIRVForEntryPoint(
        BackEndCompileRequest*  slangRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        List<uint8_t>&          spirvOut)
    {
        if( slangRequest->shouldEmitSPIRVDirectly )
        {
            return emitSPIRVForEntryPointDirectly(
                slangRequest,
                entryPointIndex,
                targetReq,
                spirvOut);
        }
        else
        {
            return emitSPIRVForEntryPointViaGLSL(
                slangRequest,
                entryPoint,
                entryPointIndex,
                targetReq,
                endToEndReq,
                spirvOut);
        }
    }

    SlangResult emitSPIRVAssemblyForEntryPoint(
        BackEndCompileRequest*  slangRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        String&                 assemblyOut)
    {
        List<uint8_t> spirv;
        SLANG_RETURN_ON_FAIL(emitSPIRVForEntryPoint(
            slangRequest,
            entryPoint,
            entryPointIndex,
            targetReq,
            endToEndReq,
            spirv));

        if (spirv.getCount() == 0)
            return SLANG_FAIL;

        return dissassembleSPIRV(slangRequest, spirv.begin(), spirv.getCount(), assemblyOut);
    }
#endif

    // Do emit logic for a single entry point
    CompileResult emitEntryPoint(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq)
    {
        CompileResult result;

        auto target = targetReq->target;

        switch (target)
        {
        case CodeGenTarget::PTX:
        case CodeGenTarget::HostCallable:
        case CodeGenTarget::SharedLibrary:
        case CodeGenTarget::Executable:
            {
                RefPtr<DownstreamCompileResult> downstreamResult;

                if (SLANG_SUCCEEDED(emitWithDownstreamForEntryPoint(
                    compileRequest,
                    entryPointIndex,
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
                String code = emitEntryPointSource(
                    compileRequest,
                    entryPointIndex,
                    targetReq,
                    target,
                    endToEndReq);
                maybeDumpIntermediate(compileRequest, code.getBuffer(), target);
                result = CompileResult(code);
            }
            break;

#if SLANG_ENABLE_DXBC_SUPPORT
        case CodeGenTarget::DXBytecode:
            {
                List<uint8_t> code;
                if (SLANG_SUCCEEDED(emitDXBytecodeForEntryPoint(
                    compileRequest,
                    entryPoint,
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
                String code;
                if (SLANG_SUCCEEDED(emitDXBytecodeAssemblyForEntryPoint(
                    compileRequest,
                    entryPoint,
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
                if (SLANG_SUCCEEDED(emitDXILForEntryPointUsingDXC(
                    compileRequest,
                    entryPoint,
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
                if (SLANG_SUCCEEDED(emitDXILForEntryPointUsingDXC(
                    compileRequest,
                    entryPoint,
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

                    // Write as text, even if stored in uint8_t array
                    result = CompileResult(UnownedStringSlice((const char*)code.getBuffer(), code.getCount()));
                }
            }
            break;
#endif

        case CodeGenTarget::SPIRV:
            {
                List<uint8_t> code;
                if (SLANG_SUCCEEDED(emitSPIRVForEntryPoint(
                    compileRequest,
                    entryPoint,
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

        case CodeGenTarget::SPIRVAssembly:
            {
                String code;
                if (SLANG_SUCCEEDED(emitSPIRVAssemblyForEntryPoint(
                    compileRequest,
                    entryPoint,
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

    static void writeEntryPointResultToFile(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        String const&           outputPath,
        CompileResult const&    result)
    {
        SLANG_UNUSED(entryPoint);

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
                result.getBlob(blob);
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

    static void writeOutputToConsole(
        ISlangWriter* writer,
        String const&   text)
    {
        writer->write(text.getBuffer(), text.getLength());
    }

    static void writeEntryPointResultToStandardOutput(
        EndToEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        TargetRequest*          targetReq,
        CompileResult const&    result)
    {
        SLANG_UNUSED(entryPoint);

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
                    return;
                }

                const void* blobData = blob->getBufferPointer();
                size_t blobSize = blob->getBufferSize();

                if (writer->isConsole())
                {
                    // Writing to console, so we need to generate text output.

                    switch (targetReq->target)
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

    static void writeEntryPointResult(
        EndToEndCompileRequest* compileRequest,
        EntryPoint*             entryPoint,
        TargetRequest*          targetReq,
        Int                     entryPointIndex)
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
        if(compileRequest->targetInfos.TryGetValue(targetReq, targetInfo))
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

        auto entryPoint = m_program->getEntryPoint(entryPointIndex);

        auto& result = m_entryPointResults[entryPointIndex];
        result = emitEntryPoint(
            backEndRequest,
            entryPoint,
            entryPointIndex,
            m_targetReq,
            endToEndRequest);

        return result;

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
        for(Index ii = 0; ii < entryPointCount; ++ii)
        {
            targetProgram->_createEntryPointResult(
                ii,
                compileReq,
                endToEndReq);
        }
    }

    
    SlangResult EndToEndCompileRequest::writeContainerToStream(Stream* stream)
    {
        RiffContainer container;

        const IRSerialBinary::CompressionType compressionType = getLinkage()->irCompressionType;

        {
            // Module list
            RiffContainer::ScopeChunk listScope(&container, RiffContainer::Chunk::Kind::List, IRSerialBinary::kSlangModuleListFourCc);

            auto linkage = getLinkage();
            auto sink = getSink();
            auto frontEndReq = getFrontEndReq();

            IRSerialWriter::OptionFlags optionFlags = 0;

            if (linkage->debugInfoLevel != DebugInfoLevel::None)
            {
                optionFlags |= IRSerialWriter::OptionFlag::DebugInfo;
            }

            SourceManager* sourceManager = frontEndReq->getSourceManager();

            for (auto translationUnit : frontEndReq->translationUnits)
            {
                auto module = translationUnit->module;
                auto irModule = module->getIRModule();

                // Okay, we need to serialize this module to our container file.
                // We currently don't serialize it's name..., but support for that could be added.

                IRSerialData serialData;
                IRSerialWriter writer;
                SLANG_RETURN_ON_FAIL(writer.write(irModule, sourceManager, optionFlags, &serialData));
                SLANG_RETURN_ON_FAIL(IRSerialWriter::writeContainer(serialData, compressionType, &container));
            }

            auto program = getSpecializedGlobalAndEntryPointsComponentType();

            // TODO: in the case where we have specialization, we might need
            // to serialize IR related to `program`...

            for (auto target : linkage->targets)
            {
                auto targetProgram = program->getTargetProgram(target);
                auto irModule = targetProgram->getOrCreateIRModuleForLayout(sink);

                // Okay, we need to serialize this target program and its IR too...
                IRSerialData serialData;
                IRSerialWriter writer;
                SLANG_RETURN_ON_FAIL(writer.write(irModule, sourceManager, optionFlags, &serialData));
                SLANG_RETURN_ON_FAIL(IRSerialWriter::writeContainer(serialData, compressionType, &container));
            }

            auto entryPointCount = program->getEntryPointCount();
            for( Index ii = 0; ii < entryPointCount; ++ii )
            {
                auto entryPoint = program->getEntryPoint(ii);
                auto entryPointMangledName = program->getEntryPointMangledName(ii);

                RiffContainer::ScopeChunk entryPointScope(&container, RiffContainer::Chunk::Kind::Data, IRSerialBinary::kEntryPointFourCc);

                auto writeString = [&](String const& str)
                {
                    uint32_t length = (uint32_t) str.getLength();
                    container.write(&length, sizeof(length));
                    container.write(str.getBuffer(), length+1);
                };

                writeString(entryPoint->getName()->text);

                Profile profile = entryPoint->getProfile();
                container.write(&profile, sizeof(profile));

                writeString(entryPointMangledName);
            }
        }

        // We now write the RiffContainer to the stream
        SLANG_RETURN_ON_FAIL(RiffUtil::write(container.getRoot(), true, stream));

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
        catch (IOException&)
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
        // If we are about to generate output code, but we still
        // have unspecialized generic/existential parameters,
        // then there is a problem.
        //
        auto program = compileRequest->getProgram();
        auto specializationParamCount = program->getSpecializationParamCount();
        if( specializationParamCount != 0 )
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

        if (compileRequest->isCommandLineCompile)
        {
            auto linkage = compileRequest->getLinkage();
            auto program = compileRequest->getSpecializedGlobalAndEntryPointsComponentType();
            for (auto targetReq : linkage->targets)
            {
                Index entryPointCount = program->getEntryPointCount();
                for (Index ee = 0; ee < entryPointCount; ++ee)
                {
                    writeEntryPointResult(
                        compileRequest,
                        program->getEntryPoint(ee),
                        targetReq,
                        ee);
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
