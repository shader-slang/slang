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

    // EntryPointRequest

    TranslationUnitRequest* EntryPointRequest::getTranslationUnit()
    {
        return compileRequest->translationUnits[translationUnitIndex].Ptr();
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


    SlangResult checkCompileTargetSupport(Session* session, CodeGenTarget target)
    {
        switch (target)
        {
            case CodeGenTarget::None:
            {
                return SLANG_OK;
            }
            case CodeGenTarget::GLSL:
            case CodeGenTarget::GLSL_Vulkan:
            case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                // Can always output GLSL
                return SLANG_OK;
            }
            case CodeGenTarget::HLSL:
            {
                // Can always output HLSL
                return SLANG_OK;
            }
            case CodeGenTarget::SPIRVAssembly:
            case CodeGenTarget::SPIRV:
            {
#if SLANG_ENABLE_GLSLANG_SUPPORT
                return session->getOrLoadSharedLibrary(Slang::SharedLibraryType::Glslang, nullptr) ? SLANG_OK : SLANG_E_NOT_FOUND;
#else
                return SLANG_E_NOT_IMPLEMENTED;
#endif
            }            
            case CodeGenTarget::DXBytecode:
            case CodeGenTarget::DXBytecodeAssembly:
            {
#if SLANG_ENABLE_DXBC_SUPPORT
                // Must have fxc
                return session->getOrLoadSharedLibrary(SharedLibraryType::Fxc, nullptr) ? SLANG_OK : SLANG_E_NOT_FOUND; 
#else
                return SLANG_E_NOT_IMPLEMENTED;
#endif
            }

            case CodeGenTarget::DXIL:
            case CodeGenTarget::DXILAssembly:
            {
#if SLANG_ENABLE_DXIL_SUPPORT
                // Must have dxc
                return (session->getOrLoadSharedLibrary(SharedLibraryType::Dxc, nullptr) &&
                    session->getOrLoadSharedLibrary(SharedLibraryType::Dxil, nullptr)) ? SLANG_OK : SLANG_E_NOT_FOUND;
#else
                return SLANG_E_NOT_IMPLEMENTED;
#endif
            }
            
            default: break;
        }

        SLANG_ASSERT(!"Unhandled target");
        return SLANG_FAIL;
    }

    //

    String emitHLSLForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq)
    {
        auto compileRequest = entryPoint->compileRequest;
        auto translationUnit = entryPoint->getTranslationUnit();
        if (compileRequest->passThrough != PassThroughMode::None)
        {
            // Generate a string that includes the content of
            // the source file(s), along with a line directive
            // to ensure that we get reasonable messages
            // from the downstream compiler when in pass-through
            // mode.

            StringBuilder codeBuilder;
            for(auto sourceFile : translationUnit->sourceFiles)
            {
                codeBuilder << "#line 1 \"";

                const String& path = sourceFile->pathInfo.foundPath;

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

                codeBuilder << sourceFile->content << "\n";
            }

            return codeBuilder.ProduceString();
        }
        else
        {
            return emitEntryPoint(
                entryPoint,
                targetReq->layout.Ptr(),
                CodeGenTarget::HLSL,
                targetReq);
        }
    }

    String emitGLSLForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq)
    {
        auto compileRequest = entryPoint->compileRequest;
        auto translationUnit = entryPoint->getTranslationUnit();

        if (compileRequest->passThrough != PassThroughMode::None)
        {
            // Generate a string that includes the content of
            // the source file(s), along with a line directive
            // to ensure that we get reasonable messages
            // from the downstream compiler when in pass-through
            // mode.

            StringBuilder codeBuilder;
            int translationUnitCounter = 0;
            for(auto sourceFile : translationUnit->sourceFiles)
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
                codeBuilder << sourceFile->content << "\n";
            }

            return codeBuilder.ProduceString();
        }
        else
        {
            // TODO(tfoley): need to pass along the entry point
            // so that we properly emit it as the `main` function.
            return emitEntryPoint(
                entryPoint,
                targetReq->layout.Ptr(),
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

#if SLANG_ENABLE_DXBC_SUPPORT
   
    List<uint8_t> EmitDXBytecodeForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq)
    {
        auto session = entryPoint->compileRequest->mSession;

        auto compileFunc = (pD3DCompile)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Fxc_D3DCompile, &entryPoint->compileRequest->mSink);
        if (!compileFunc)
        {
            return List<uint8_t>();
        }

        auto hlslCode = emitHLSLForEntryPoint(entryPoint, targetReq);
        maybeDumpIntermediate(entryPoint->compileRequest, hlslCode.Buffer(), CodeGenTarget::HLSL);

        auto profile = getEffectiveProfile(entryPoint, targetReq);

        // If we have been invoked in a pass-through mode, then we need to make sure
        // that the downstream compiler sees whatever options were passed to Slang
        // via the command line or API.
        //
        // TODO: more pieces of information should be added here as needed.
        //
        List<D3D_SHADER_MACRO> dxMacrosStorage;
        D3D_SHADER_MACRO const* dxMacros = nullptr;
        if( entryPoint->compileRequest->passThrough != PassThroughMode::None )
        {
            for( auto& define :  entryPoint->compileRequest->preprocessorDefinitions )
            {
                D3D_SHADER_MACRO dxMacro;
                dxMacro.Name = define.Key.Buffer();
                dxMacro.Definition = define.Value.Buffer();
                dxMacrosStorage.Add(dxMacro);
            }
            for( auto& define : entryPoint->getTranslationUnit()->preprocessorDefinitions )
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

        ID3DBlob* codeBlob;
        ID3DBlob* diagnosticsBlob;
        HRESULT hr = compileFunc(
            hlslCode.begin(),
            hlslCode.Length(),
            "slang",
            dxMacros,
            nullptr,
            getText(entryPoint->name).begin(),
            GetHLSLProfileName(profile).Buffer(),
            flags,
            0, // unused: effect flags
            &codeBlob,
            &diagnosticsBlob);

        List<uint8_t> data;
        if (codeBlob)
        {
            data.AddRange((uint8_t const*)codeBlob->GetBufferPointer(), (int)codeBlob->GetBufferSize());
            codeBlob->Release();
        }

        // Note: we will only output diagnostics coming from a downstream
        // compiler in the event of an error (although in that case we will
        // end up including any warning diagnostics that are produced as well).
        //
        // TODO: some day we should aspire to make Slang's output always compile
        // cleanly without warnings on downstream compilers (or else suppress those
        // warnings), but this is difficult to do in practice without a lot of
        // tailoring for the quirks of each compiler (version).
        //
        if (diagnosticsBlob && FAILED(hr))
        {
            // TODO(tfoley): need a better policy for how we translate diagnostics
            // back into the Slang world (although we should always try to generate
            // HLSL that doesn't produce any diagnostics...)
            entryPoint->compileRequest->mSink.diagnoseRaw(
                FAILED(hr) ? Severity::Error : Severity::Warning,
                (char const*) diagnosticsBlob->GetBufferPointer());
            diagnosticsBlob->Release();
        }
        if (FAILED(hr))
        {
            return List<uint8_t>();
        }
        return data;
    }

    SlangResult dissassembleDXBC(
        CompileRequest*     compileRequest,
        void const*         data,
        size_t              size, 
        String& stringOut)
    {
        stringOut = String();

        auto session = compileRequest->mSession;

        auto disassembleFunc = (pD3DDisassemble)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Fxc_D3DDisassemble, &compileRequest->mSink);
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
            char const* codeBegin = (char const*)codeBlob->GetBufferPointer();
            char const* codeEnd = codeBegin + codeBlob->GetBufferSize() - 1;
            stringOut = String(codeBegin, codeEnd);
        }
        if (FAILED(res))
        {
            // TODO(tfoley): need to figure out what to diagnose here...
        }

        return res;
    }

    String EmitDXBytecodeAssemblyForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq)
    {

        List<uint8_t> dxbc = EmitDXBytecodeForEntryPoint(entryPoint, targetReq);
        if (!dxbc.Count())
        {
            return String();
        }

        String result;
        dissassembleDXBC(entryPoint->compileRequest, dxbc.Buffer(), dxbc.Count(), result);

        return result;
    }
#endif

#if SLANG_ENABLE_DXIL_SUPPORT

// Implementations in `dxc-support.cpp`

int emitDXILForEntryPointUsingDXC(
    EntryPointRequest*  entryPoint,
    TargetRequest*      targetReq,
    List<uint8_t>&      outCode);

SlangResult dissassembleDXILUsingDXC(
    CompileRequest*     compileRequest,
    void const*         data,
    size_t              size, 
    String&             stringOut);

#endif

#if SLANG_ENABLE_GLSLANG_SUPPORT
    int invokeGLSLCompiler(
        CompileRequest*             slangCompileRequest,
        glslang_CompileRequest&     request)
    {
        Session* session = slangCompileRequest->mSession;

        auto glslang_compile = (glslang_CompileFunc)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Glslang_Compile, &slangCompileRequest->mSink);
        if (!glslang_compile)
        {
            return 1;
        }

        String diagnosticOutput;
        auto diagnosticOutputFunc = [](void const* data, size_t size, void* userData)
        {
            (*(String*)userData).append((char const*)data, (char const*)data + size);
        };

        request.diagnosticFunc = diagnosticOutputFunc;
        request.diagnosticUserData = &diagnosticOutput;

        int err = glslang_compile(&request);

        if (err)
        {
            slangCompileRequest->mSink.diagnoseRaw(
                Severity::Error,
                diagnosticOutput.begin());
            return err;
        }

        return 0;
    }

    SlangResult dissassembleSPIRV(
        CompileRequest*     slangRequest,
        void const*         data,
        size_t              size, 
        String&             stringOut)
    {
        stringOut = String();

        String output;
        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            (*(String*)userData).append((char const*)data, (char const*)data + size);
        };

        glslang_CompileRequest request;
        request.action = GLSLANG_ACTION_DISSASSEMBLE_SPIRV;

        request.inputBegin  = data;
        request.inputEnd    = (char*)data + size;

        request.outputFunc = outputFunc;
        request.outputUserData = &output;

        int err = invokeGLSLCompiler(slangRequest, request);
        if (err)
        {
            return SLANG_FAIL;
        }

        stringOut = output;
        return SLANG_OK;
    }

    List<uint8_t> emitSPIRVForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq)
    {
        String rawGLSL = emitGLSLForEntryPoint(entryPoint, targetReq);
        maybeDumpIntermediate(entryPoint->compileRequest, rawGLSL.Buffer(), CodeGenTarget::GLSL);

        List<uint8_t> output;
        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            ((List<uint8_t>*)userData)->AddRange((uint8_t*)data, size);
        };

        glslang_CompileRequest request;
        request.action = GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV;
        request.sourcePath = "slang";
        request.slangStage = (SlangStage)entryPoint->getStage();

        request.inputBegin  = rawGLSL.begin();
        request.inputEnd    = rawGLSL.end();

        request.outputFunc = outputFunc;
        request.outputUserData = &output;

        int err = invokeGLSLCompiler(entryPoint->compileRequest, request);

        if (err)
        {
            return List<uint8_t>();
        }

        return output;
    }

    String emitSPIRVAssemblyForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq)
    {
        List<uint8_t> spirv = emitSPIRVForEntryPoint(entryPoint, targetReq);
        if (spirv.Count() == 0)
            return String();

        String result;
        dissassembleSPIRV(entryPoint->compileRequest, spirv.begin(), spirv.Count(), result);
        return result;
    }
#endif

    // Do emit logic for a single entry point
    CompileResult emitEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq)
    {
        CompileResult result;

        auto compileRequest = entryPoint->compileRequest;
        auto target = targetReq->target;

        switch (target)
        {
        case CodeGenTarget::HLSL:
            {
                String code = emitHLSLForEntryPoint(entryPoint, targetReq);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::GLSL:
            {
                String code = emitGLSLForEntryPoint(entryPoint, targetReq);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
                result = CompileResult(code);
            }
            break;

#if SLANG_ENABLE_DXBC_SUPPORT
        case CodeGenTarget::DXBytecode:
            {
                List<uint8_t> code = EmitDXBytecodeForEntryPoint(entryPoint, targetReq);
                maybeDumpIntermediate(compileRequest, code.Buffer(), code.Count(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::DXBytecodeAssembly:
            {
                String code = EmitDXBytecodeAssemblyForEntryPoint(entryPoint, targetReq);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
                result = CompileResult(code);
            }
            break;
#endif

#if SLANG_ENABLE_DXIL_SUPPORT
        case CodeGenTarget::DXIL:
            {
                List<uint8_t> code;
                int err = emitDXILForEntryPointUsingDXC(entryPoint, targetReq, code);
                if (!err)
                {
                    maybeDumpIntermediate(compileRequest, code.Buffer(), code.Count(), target);
                    result = CompileResult(code);
                }
            }
            break;

        case CodeGenTarget::DXILAssembly:
            {
                List<uint8_t> code;
                int err = emitDXILForEntryPointUsingDXC(entryPoint, targetReq, code);
                if (!err)
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
                List<uint8_t> code = emitSPIRVForEntryPoint(entryPoint, targetReq);
                maybeDumpIntermediate(compileRequest, code.Buffer(), code.Count(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::SPIRVAssembly:
            {
                String code = emitSPIRVAssemblyForEntryPoint(entryPoint, targetReq);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
                result = CompileResult(code);
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
        CompileRequest* compileRequest,
        FILE*           file,
        String const&   path,
        void const*     data,
        size_t          size)
    {
        size_t count = fwrite(data, size, 1, file);
        if (count != 1)
        {
            compileRequest->mSink.diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
        }
    }

    static void writeOutputFile(
        CompileRequest* compileRequest,
        ISlangWriter*   writer, 
        String const&   path,
        void const*     data,
        size_t          size)
    {

        if (SLANG_FAILED(writer->write((const char*)data, size)))
        {
            compileRequest->mSink.diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
        }
    }

    static void writeOutputFile(
        CompileRequest* compileRequest,
        String const&   path,
        void const*     data,
        size_t          size,
        OutputFileKind  kind)
    {
        FILE* file = fopen(
            path.Buffer(),
            kind == OutputFileKind::Binary ? "wb" : "w");
        if (!file)
        {
            compileRequest->mSink.diagnose(
                SourceLoc(),
                Diagnostics::cannotWriteOutputFile,
                path);
            return;
        }

        writeOutputFile(compileRequest, file, path, data, size);
        fclose(file);
    }

    static void writeEntryPointResultToFile(
        EntryPointRequest*      entryPoint,
        String const&           outputPath,
        CompileResult const&    result)
    {
        auto compileRequest = entryPoint->compileRequest;

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
        EntryPointRequest*      entryPoint,
        TargetRequest*          targetReq,
        CompileResult const&    result)
    {
        auto compileRequest = entryPoint->compileRequest;

        ISlangWriter* writer = compileRequest->getWriter(WriterChannel::StdOutput);

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
                            dissassembleDXBC(compileRequest,
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
                            dissassembleDXILUsingDXC(compileRequest,
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
                            dissassembleSPIRV(compileRequest,
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
                        compileRequest,
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
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq,
        UInt                entryPointIndex)
    {
        // It is possible that we are dynamically discovering entry
        // points (using `[shader(...)]` attributes), so that the
        // number of entry points on the compile request does not
        // match the number of entries in teh `entryPointOutputPaths`
        // array.
        //
        String outputPath;
        if( entryPointIndex < targetReq->entryPointOutputPaths.Count() )
        {
            outputPath = targetReq->entryPointOutputPaths[entryPointIndex];
        }

        auto& result = targetReq->entryPointResults[entryPointIndex];

        // Skip the case with no output
        if (result.format == ResultFormat::None)
            return;

        if (outputPath.Length())
        {
            writeEntryPointResultToFile(entryPoint, outputPath, result);
        }
        else
        {
            writeEntryPointResultToStandardOutput(entryPoint, targetReq, result);
        }
    }

    void generateOutputForTarget(
        TargetRequest*  targetReq)
    {
        CompileRequest* compileReq = targetReq->compileRequest;

        // Generate target code any entry points that
        // have been requested for compilation.
        for (auto& entryPoint : compileReq->entryPoints)
        {
            CompileResult entryPointResult = emitEntryPoint(entryPoint, targetReq);
            targetReq->entryPointResults.Add(entryPointResult);
        }
    }

    void generateOutput(
        CompileRequest* compileRequest)
    {
        // Go through the code-generation targets that the user
        // has specified, and generate code for each of them.
        //
        for (auto targetReq : compileRequest->targets)
        {
            generateOutputForTarget(targetReq);
        }

        // If we are in command-line mode, we might be expected to actually
        // write output to one or more files here.

        if (compileRequest->isCommandLineCompile)
        {
            for (auto targetReq : compileRequest->targets)
            {
                UInt entryPointCount = compileRequest->entryPoints.Count();
                for (UInt ee = 0; ee < entryPointCount; ++ee)
                {
                    writeEntryPointResult(
                        compileRequest->entryPoints[ee],
                        targetReq,
                        ee);
                }
            }

            if (compileRequest->containerOutputPath.Length() != 0)
            {
                auto& data = compileRequest->generatedBytecode;
                writeOutputFile(compileRequest,
                    compileRequest->containerOutputPath,
                    data.begin(),
                    data.end() - data.begin(),
                    OutputFileKind::Binary);
            }
        }
    }

    // Debug logic for dumping intermediate outputs

    //

    void dumpIntermediate(
        CompileRequest*,
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
        CompileRequest* compileRequest,
        void const*     data,
        size_t          size,
        char const*     ext)
    {
        dumpIntermediate(compileRequest, data, size, ext, false);
    }

    void dumpIntermediateBinary(
        CompileRequest* compileRequest,
        void const*     data,
        size_t          size,
        char const*     ext)
    {
        dumpIntermediate(compileRequest, data, size, ext, true);
    }

    void maybeDumpIntermediate(
        CompileRequest* compileRequest,
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
        CompileRequest* compileRequest,
        char const*     text,
        CodeGenTarget   target)
    {
        if (!compileRequest->shouldDumpIntermediates)
            return;

        maybeDumpIntermediate(compileRequest, text, strlen(text), target);
    }

}
