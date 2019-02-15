// Compiler.cpp : Defines the entry point for the console application.
//
#include "../core/basic.h"
#include "../core/platform.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"

#include "compiler.h"
#include "lexer.h"
#include "lower-to-ir.h"
#include "parameter-binding.h"
#include "parser.h"
#include "preprocessor.h"
#include "syntax-visitors.h"
#include "type-layout.h"
#include "reflection.h"
#include "emit.h"

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

    // CompileResult

    void CompileResult::append(CompileResult const& result)
    {
        // Find which to append to
        ResultFormat appendTo = ResultFormat::None;

        if (format == ResultFormat::None)
        {
            format = result.format;
            appendTo = result.format;
        }
        else if (format == result.format)
        {
            appendTo = format;
        }

        if (appendTo == ResultFormat::Text)
        {
            outputString.append(result.outputString.Buffer());
        }
        else if (appendTo == ResultFormat::Binary)
        {
            outputBinary.AddRange(result.outputBinary.Buffer(), result.outputBinary.Count());
        }
    }

    ComPtr<ISlangBlob> CompileResult::getBlob()
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
                blob = createRawBlob(outputBinary.Buffer(), outputBinary.Count());
                break;
            }
        }
        return blob;
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
        DeclRef<FuncDecl>   funcDeclRef,
        Profile             profile)
    {
        RefPtr<EntryPoint> entryPoint = new EntryPoint(
            funcDeclRef.GetName(),
            profile,
            funcDeclRef);
        return entryPoint;
    }

    RefPtr<EntryPoint> EntryPoint::createDummyForPassThrough(
        Name*       name,
        Profile     profile)
    {
        RefPtr<EntryPoint> entryPoint = new EntryPoint(
            name,
            profile,
            DeclRef<FuncDecl>());
        return entryPoint;
    }

    EntryPoint::EntryPoint(
        Name*               name,
        Profile             profile,
        DeclRef<FuncDecl>   funcDeclRef)
        : m_name(name)
        , m_profile(profile)
        , m_funcDeclRef(funcDeclRef)
    {
        // In order for later code generation to work, we need to track what
        // modules each entry point depends on. We will build up the dependency
        // list here when an `EntryPoint` gets created.
        //
        // We know an entry point depends on the module that declared the
        // entry-point function itself.
        //
        // Note: we are carefully handling the case where `module` could
        // be null, becase of "dummy" entry points created for pass-through
        // compilation.
        //
        if(auto module = getModule())
        {
            m_dependencyList.addDependency(module);
        }
        //
        // TODO: We also need to include the modules needed by any generic
        // arguments in the dependency list, since in the general case they
        // might come from modules other than the one defining the entry point.

        // The following is a bit of a hack.
        //
        // Back-end code generation relies on us having computed layouts for all tagged
        // unions that end up being used in the code, which means we need a way to find
        // all such types that get used in a program (and the stuff it imports).
        //
        // For now we are assuming a tagged union type only comes into existence
        // as a (top-level) argument for a generic type parameter, so that we
        // can check for them here and cache them on the entry point.
        //
        // A longer-term strategy might need to consider any (tagged or untagged)
        // union types that get used inside of a module, and also take
        // those lists into account.
        //
        // An even longer-term strategy would be to allow type layout to
        // be performed on IR types, so taht we don't need to have front-end
        // code worrying about this stuff.
        // 
        for( auto subst = funcDeclRef.substitutions.substitutions; subst; subst = subst->outer )
        {
            if( auto genericSubst = as<GenericSubstitution>(subst) )
            {
                for( auto arg : genericSubst->args )
                {
                    if( auto taggedUnionType = as<TaggedUnionType>(arg) )
                    {
                        m_taggedUnionTypes.Add(taggedUnionType);
                    }
                }
            }
        }
    }

    Module* EntryPoint::getModule()
    {
        return Slang::getModule(getFuncDecl());
    }

    Linkage* EntryPoint::getLinkage()
    {
        return getModule()->getLinkage();
    }

    //

    Profile Profile::LookUp(char const* name)
    {
        #define PROFILE(TAG, NAME, STAGE, VERSION)	if(strcmp(name, #NAME) == 0) return Profile::TAG;
        #define PROFILE_ALIAS(TAG, DEF, NAME)		if(strcmp(name, #NAME) == 0) return Profile::TAG;
        #include "profile-defs.h"

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
        #include "profile-defs.h"
        }
    }

    Stage findStageByName(String const& name)
    {
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

        #include "profile-defs.h"
        };

        for(auto entry : kStages)
        {
            if(name == entry.name)
            {
                return entry.stage;
            }
        }

        return Stage::Unknown;
    }

    SlangResult checkExternalCompilerSupport(Session* session, PassThroughMode passThrough)
    {
        switch (passThrough)
        {
            case PassThroughMode::None:
            {
                // If no pass through -> that will always work!
                return SLANG_OK;
            }
            case PassThroughMode::dxc:
            {
#if SLANG_ENABLE_DXIL_SUPPORT
                // Must have dxc
                return session->getOrLoadSharedLibrary(SharedLibraryType::Dxc, nullptr) ? SLANG_OK : SLANG_E_NOT_FOUND;
#endif
                break;
            }
            case PassThroughMode::fxc:
            {
#if SLANG_ENABLE_DXBC_SUPPORT
                // Must have fxc
                return session->getOrLoadSharedLibrary(SharedLibraryType::Fxc, nullptr) ? SLANG_OK : SLANG_E_NOT_FOUND;
#endif
                break;
            }
            case PassThroughMode::glslang:
            {
#if SLANG_ENABLE_GLSLANG_SUPPORT
                return session->getOrLoadSharedLibrary(Slang::SharedLibraryType::Glslang, nullptr) ? SLANG_OK : SLANG_E_NOT_FOUND;
#endif
                break;
            }
        }
        return SLANG_E_NOT_IMPLEMENTED;
    }

    static PassThroughMode _getExternalCompilerRequiredForTarget(CodeGenTarget target)
    {
        switch (target)
        {
            case CodeGenTarget::None:
            {
                return PassThroughMode::None;
            }
            case CodeGenTarget::GLSL:
            case CodeGenTarget::GLSL_Vulkan:
            case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                // Can always output GLSL
                return PassThroughMode::None; 
            }
            case CodeGenTarget::HLSL:
            {
                // Can always output HLSL
                return PassThroughMode::None;
            }
            case CodeGenTarget::SPIRVAssembly:
            case CodeGenTarget::SPIRV:
            {
                return PassThroughMode::glslang;
            }
            case CodeGenTarget::DXBytecode:
            case CodeGenTarget::DXBytecodeAssembly:
            {
                return PassThroughMode::fxc;
            }
            case CodeGenTarget::DXIL:
            case CodeGenTarget::DXILAssembly:
            {
                return PassThroughMode::dxc;
            }

            default: break;
        }

        SLANG_ASSERT(!"Unhandled target");
        return PassThroughMode::None;
    }

    SlangResult checkCompileTargetSupport(Session* session, CodeGenTarget target)
    {
        const PassThroughMode mode = _getExternalCompilerRequiredForTarget(target);
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

    String emitHLSLForEntryPoint(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
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
            for(auto sourceFile : translationUnit->getSourceFiles())
            {
                codeBuilder << "#line 1 \"";

                const String& path = sourceFile->getPathInfo().foundPath;

                for(auto c : path)
                {
                    char buffer[] = { c, 0 };
                    switch(c)
                    {
                    default:
                        codeBuilder << buffer;
                        break;

                    case '\\':
                        codeBuilder << "\\\\";
                    }
                }
                codeBuilder << "\"\n";

                codeBuilder << sourceFile->getContent() << "\n";
            }

            return codeBuilder.ProduceString();
        }
        else
        {
            return emitEntryPoint(
                compileRequest,
                entryPoint,
                CodeGenTarget::HLSL,
                targetReq);
        }
    }

    String emitGLSLForEntryPoint(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
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
            int translationUnitCounter = 0;
            for(auto sourceFile : translationUnit->getSourceFiles())
            {
                int translationUnitIndex = translationUnitCounter++;

                // We want to output `#line` directives, but we need
                // to skip this for the first file, since otherwise
                // some GLSL implementations will get tripped up by
                // not having the `#version` directive be the first
                // thing in the file.
                if(translationUnitIndex != 0)
                {
                    codeBuilder << "#line 1 " << translationUnitIndex << "\n";
                }
                codeBuilder << sourceFile->getContent() << "\n";
            }

            return codeBuilder.ProduceString();
        }
        else
        {
            // TODO(tfoley): need to pass along the entry point
            // so that we properly emit it as the `main` function.
            return emitEntryPoint(
                compileRequest,
                entryPoint,
                CodeGenTarget::GLSL,
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

    void reportExternalCompileError(const char* compilerName, SlangResult res, const UnownedStringSlice& diagnostic, DiagnosticSink* sink)
    {
        StringBuilder builder;
        if (compilerName)
        {
            builder << compilerName << ": ";
        }

        if (diagnostic.size() > 0)
        {
            builder.Append(diagnostic);
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

        // TODO(tfoley): need a better policy for how we translate diagnostics
        // back into the Slang world (although we should always try to generate
        // HLSL that doesn't produce any diagnostics...)
        sink->diagnoseRaw(SLANG_FAILED(res) ? Severity::Error : Severity::Warning, builder.getUnownedSlice());
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

        auto sink = endToEndReq->getSink();

        const auto& sourceFiles = translationUnitRequest->getSourceFiles();

        const int numSourceFiles = int(sourceFiles.Count());

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

    SlangResult emitDXBytecodeForEntryPoint(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        List<uint8_t>&          byteCodeOut)
    {
        byteCodeOut.Clear();

        auto session = compileRequest->getSession();
        auto sink = compileRequest->getSink();

        auto compileFunc = (pD3DCompile)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Fxc_D3DCompile, sink);
        if (!compileFunc)
        {
            return SLANG_FAIL;
        }

        auto hlslCode = emitHLSLForEntryPoint(compileRequest, entryPoint, entryPointIndex, targetReq, endToEndReq);
        maybeDumpIntermediate(compileRequest, hlslCode.Buffer(), CodeGenTarget::HLSL);

        auto profile = getEffectiveProfile(entryPoint, targetReq);

        // If we have been invoked in a pass-through mode, then we need to make sure
        // that the downstream compiler sees whatever options were passed to Slang
        // via the command line or API.
        //
        // TODO: more pieces of information should be added here as needed.
        //
        List<D3D_SHADER_MACRO> dxMacrosStorage;
        D3D_SHADER_MACRO const* dxMacros = nullptr;
        if(auto translationUnit = findPassThroughTranslationUnit(endToEndReq, entryPointIndex))
        {
            for( auto& define :  translationUnit->compileRequest->preprocessorDefinitions )
            {
                D3D_SHADER_MACRO dxMacro;
                dxMacro.Name = define.Key.Buffer();
                dxMacro.Definition = define.Value.Buffer();
                dxMacrosStorage.Add(dxMacro);
            }
            for( auto& define : translationUnit->preprocessorDefinitions )
            {
                D3D_SHADER_MACRO dxMacro;
                dxMacro.Name = define.Key.Buffer();
                dxMacro.Definition = define.Value.Buffer();
                dxMacrosStorage.Add(dxMacro);
            }
            D3D_SHADER_MACRO nullTerminator = { 0, 0 };
            dxMacrosStorage.Add(nullTerminator);

            dxMacros = dxMacrosStorage.Buffer();
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

        const String sourcePath = "slang-geneated";// calcTranslationUnitSourcePath(entryPoint->getTranslationUnit());

        ComPtr<ID3DBlob> codeBlob;
        ComPtr<ID3DBlob> diagnosticsBlob;
        HRESULT hr = compileFunc(
            hlslCode.begin(),
            hlslCode.Length(),
            sourcePath.Buffer(),
            dxMacros,
            nullptr,
            getText(entryPoint->getName()).begin(),
            GetHLSLProfileName(profile).Buffer(),
            flags,
            0, // unused: effect flags
            codeBlob.writeRef(),
            diagnosticsBlob.writeRef());

        if (codeBlob && SLANG_SUCCEEDED(hr))
        {
            byteCodeOut.AddRange((uint8_t const*)codeBlob->GetBufferPointer(), (int)codeBlob->GetBufferSize());
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
        if (!dxbc.Count())
        {
            return SLANG_FAIL;
        }
        return dissassembleDXBC(compileRequest, dxbc.Buffer(), dxbc.Count(), assemOut);
    }
#endif

#if SLANG_ENABLE_DXIL_SUPPORT

// Implementations in `dxc-support.cpp`

int emitDXILForEntryPointUsingDXC(
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

    SlangResult emitSPIRVForEntryPoint(
        BackEndCompileRequest*  slangRequest,
        EntryPoint*             entryPoint,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        List<uint8_t>&          spirvOut)
    {
        spirvOut.Clear();

        String rawGLSL = emitGLSLForEntryPoint(
            slangRequest,
            entryPoint,
            entryPointIndex,
            targetReq,
            endToEndReq);
        maybeDumpIntermediate(slangRequest, rawGLSL.Buffer(), CodeGenTarget::GLSL);

        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            ((List<uint8_t>*)userData)->AddRange((uint8_t*)data, size);
        };

        const String sourcePath = calcSourcePathForEntryPoint(endToEndReq, entryPointIndex);

        glslang_CompileRequest request;
        request.action = GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV;
        request.sourcePath = sourcePath.Buffer();
        request.slangStage = (SlangStage)entryPoint->getStage();

        request.inputBegin  = rawGLSL.begin();
        request.inputEnd    = rawGLSL.end();

        request.outputFunc = outputFunc;
        request.outputUserData = &spirvOut;

        SLANG_RETURN_ON_FAIL(invokeGLSLCompiler(slangRequest, request));
        return SLANG_OK;
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

        if (spirv.Count() == 0)
            return SLANG_FAIL;

        return dissassembleSPIRV(slangRequest, spirv.begin(), spirv.Count(), assemblyOut);
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
        case CodeGenTarget::HLSL:
            {
                String code = emitHLSLForEntryPoint(
                    compileRequest,
                    entryPoint,
                    entryPointIndex,
                    targetReq,
                    endToEndReq);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::GLSL:
            {
                String code = emitGLSLForEntryPoint(
                    compileRequest,
                    entryPoint,
                    entryPointIndex,
                    targetReq,
                    endToEndReq);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
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
                    maybeDumpIntermediate(compileRequest, code.Buffer(), code.Count(), target);
                    result = CompileResult(code);
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
                    maybeDumpIntermediate(compileRequest, code.Buffer(), target);
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
                    maybeDumpIntermediate(compileRequest, code.Buffer(), code.Count(), target);
                    result = CompileResult(code);
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
                        code.Buffer(),
                        code.Count(), 
                        assembly);

                    maybeDumpIntermediate(compileRequest, assembly.Buffer(), target);

                    result = CompileResult(assembly);
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
                    maybeDumpIntermediate(compileRequest, code.Buffer(), code.Count(), target);
                    result = CompileResult(code);
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
                    maybeDumpIntermediate(compileRequest, code.Buffer(), target);
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
            path.Buffer(),
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
                auto& data = result.outputBinary;
                writeOutputFile(compileRequest,
                    outputPath,
                    data.begin(),
                    data.end() - data.begin(),
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
        writer->write(text.Buffer(), text.Length());
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
                auto& data = result.outputBinary;
                
                if (writer->isConsole())
                {
                    // Writing to console, so we need to generate text output.

                    switch (targetReq->target)
                    {
                #if SLANG_ENABLE_DXBC_SUPPORT
                    case CodeGenTarget::DXBytecode:
                        {
                            String assembly;
                            dissassembleDXBC(backEndReq,
                                data.begin(),
                                data.end() - data.begin(), assembly);
                            writeOutputToConsole(writer, assembly);
                        }
                        break;
                #endif

                #if SLANG_ENABLE_DXIL_SUPPORT
                    case CodeGenTarget::DXIL:
                        {
                            String assembly; 
                            dissassembleDXILUsingDXC(backEndReq,
                                data.begin(),
                                data.end() - data.begin(), 
                                assembly);
                            writeOutputToConsole(writer, assembly);
                        }
                        break;
                #endif

                    case CodeGenTarget::SPIRV:
                        {
                            String assembly;
                            dissassembleSPIRV(backEndReq,
                                data.begin(),
                                data.end() - data.begin(), assembly);
                            writeOutputToConsole(writer, assembly);
                        }
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
                        data.begin(),
                        data.end() - data.begin());
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
        auto program = compileRequest->getSpecializedProgram();
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
        for(UInt ii = 0; ii < entryPointCount; ++ii)
        {
            auto entryPoint = program->getEntryPoint(ii);
            CompileResult entryPointResult = emitEntryPoint(
                compileReq,
                entryPoint,
                ii,
                targetReq,
                endToEndReq);
            targetProgram->setEntryPointResult(ii, entryPointResult);
        }
    }

    static void _generateOutput(
        BackEndCompileRequest* compileRequest,
        EndToEndCompileRequest* endToEndReq)
    {
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
            auto program = compileRequest->getSpecializedProgram();
            for (auto targetReq : linkage->targets)
            {
                UInt entryPointCount = program->getEntryPointCount();
                for (UInt ee = 0; ee < entryPointCount; ++ee)
                {
                    writeEntryPointResult(
                        compileRequest,
                        program->getEntryPoint(ee),
                        targetReq,
                        ee);
                }
            }
        }
    }

    // Debug logic for dumping intermediate outputs

    //

    void dumpIntermediate(
        BackEndCompileRequest*,
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
        path.append("slang-dump-");
        path.append(id);
        path.append(ext);

        FILE* file = fopen(path.Buffer(), isBinary ? "wb" : "w");
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
                dumpIntermediateText(compileRequest, spirvAssembly.begin(), spirvAssembly.Length(), ".spv.asm");
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
                dumpIntermediateText(compileRequest, dxbcAssembly.begin(), dxbcAssembly.Length(), ".dxbc.asm");
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
                dumpIntermediateText(compileRequest, dxilAssembly.begin(), dxilAssembly.Length(), ".dxil.asm");
            }
            break;
    #endif
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
