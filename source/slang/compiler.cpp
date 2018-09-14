// Compiler.cpp : Defines the entry point for the console application.
//
#include "../core/basic.h"
#include "../core/platform.h"
#include "../core/slang-io.h"
#include "bytecode.h"
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
                blob = createStringBlob(outputString);
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
                for(auto c : sourceFile->path)
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

    char const* GetHLSLProfileName(Profile profile)
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

        switch(profile.raw)
        {
        #define PROFILE(TAG, NAME, STAGE, VERSION) case Profile::TAG: return #NAME;
        #include "profile-defs.h"

        default:
            // TODO: emit an error here!
            return "unknown";
        }
    }

#if SLANG_ENABLE_DXBC_SUPPORT
    HMODULE loadD3DCompilerDLL(CompileRequest* request)
    {
        char const* libraryName = "d3dcompiler_47";
        HMODULE d3dCompiler =  LoadLibraryA(libraryName);
        if (!d3dCompiler)
        {
            request->mSink.diagnose(SourceLoc(), Diagnostics::failedToLoadDynamicLibrary, libraryName);
        }
        return d3dCompiler;
    }

    HMODULE getD3DCompilerDLL(CompileRequest* request)
    {
        // TODO(tfoley): let user specify version of d3dcompiler DLL to use.
        static HMODULE d3dCompiler = loadD3DCompilerDLL(request);
        return d3dCompiler;
    }

    List<uint8_t> EmitDXBytecodeForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq)
    {
        static pD3DCompile D3DCompile_ = nullptr;
        if (!D3DCompile_)
        {
            HMODULE d3dCompiler = getD3DCompilerDLL(entryPoint->compileRequest);
            if (!d3dCompiler)
                return List<uint8_t>();

            D3DCompile_ = (pD3DCompile)GetProcAddress(d3dCompiler, "D3DCompile");
            if (!D3DCompile_)
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

        ID3DBlob* codeBlob;
        ID3DBlob* diagnosticsBlob;
        HRESULT hr = D3DCompile_(
            hlslCode.begin(),
            hlslCode.Length(),
            "slang",
            dxMacros,
            nullptr,
            getText(entryPoint->name).begin(),
            GetHLSLProfileName(profile),
            0,
            0,
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

    String dissassembleDXBC(
        CompileRequest*     compileRequest,
        void const*         data,
        size_t              size)
    {
        static pD3DDisassemble D3DDisassemble_ = nullptr;
        if (!D3DDisassemble_)
        {
            HMODULE d3dCompiler = getD3DCompilerDLL(compileRequest);
            if (!d3dCompiler)
                return String();

            D3DDisassemble_ = (pD3DDisassemble)GetProcAddress(d3dCompiler, "D3DDisassemble");
            if (!D3DDisassemble_)
                return String();
        }

        if (!data || !size)
        {
            return String();
        }

        ID3DBlob* codeBlob;
        HRESULT hr = D3DDisassemble_(
            data,
            size,
            0,
            nullptr,
            &codeBlob);

        String result;
        if (codeBlob)
        {
            char const* codeBegin = (char const*)codeBlob->GetBufferPointer();
            char const* codeEnd = codeBegin + codeBlob->GetBufferSize() - 1;
            result.append(codeBegin, codeEnd);
            codeBlob->Release();
        }
        if (FAILED(hr))
        {
            // TODO(tfoley): need to figure out what to diagnose here...
        }
        return result;
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

        String result = dissassembleDXBC(entryPoint->compileRequest, dxbc.Buffer(), dxbc.Count());

        return result;
    }
#endif

#if SLANG_ENABLE_DXIL_SUPPORT

// Implementations in `dxc-support.cpp`

int emitDXILForEntryPointUsingDXC(
    EntryPointRequest*  entryPoint,
    TargetRequest*      targetReq,
    List<uint8_t>&      outCode);

String dissassembleDXILUsingDXC(
    CompileRequest*     compileRequest,
    void const*         data,
    size_t              size);

#endif

#if SLANG_ENABLE_GLSLANG_SUPPORT

    SharedLibrary loadGLSLCompilerDLL(CompileRequest* request)
    {
        char const* libraryName = "slang-glslang";
        // TODO(tfoley): let user specify version of glslang DLL to use.

        SharedLibrary glslCompiler = SharedLibrary::load(libraryName);
        if (!glslCompiler)
        {
            request->mSink.diagnose(SourceLoc(), Diagnostics::failedToLoadDynamicLibrary, libraryName);
        }
        return glslCompiler;
    }

    SharedLibrary getGLSLCompilerDLL(CompileRequest* request)
    {
        static SharedLibrary glslCompiler =  loadGLSLCompilerDLL(request);
        return glslCompiler;
    }


    int invokeGLSLCompiler(
        CompileRequest*             slangCompileRequest,
        glslang_CompileRequest&     request)
    {

        static glslang_CompileFunc glslang_compile = nullptr;
        if (!glslang_compile)
        {
            SharedLibrary glslCompiler = getGLSLCompilerDLL(slangCompileRequest);
            if (!glslCompiler)
                return 1;

            glslang_compile = (glslang_CompileFunc) glslCompiler.findFuncByName("glslang_compile");
            if (!glslang_compile)
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

    String dissassembleSPIRV(
        CompileRequest*     slangRequest,
        void const*         data,
        size_t              size)
    {
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
            String();
        }

        return output;
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

        String result = dissassembleSPIRV(entryPoint->compileRequest, spirv.begin(), spirv.Count());
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
                    String assembly = dissassembleDXILUsingDXC(
                        compileRequest,
                        code.Buffer(),
                        code.Count());

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
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq,
        UInt                entryPointIndex)
    {
        auto compileRequest = entryPoint->compileRequest;
        auto outputPath = entryPoint->outputPath;
        auto result = targetReq->entryPointResults[entryPointIndex];
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
        CompileRequest*,
        String const&   text)
    {
        fwrite(
            text.begin(),
            text.end() - text.begin(),
            1,
            stdout);
    }

    static void writeEntryPointResultToStandardOutput(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq,
        UInt                entryPointIndex)
    {
        auto compileRequest = entryPoint->compileRequest;
        auto& result = targetReq->entryPointResults[entryPointIndex];

        switch (result.format)
        {
        case ResultFormat::Text:
            writeOutputToConsole(compileRequest, result.outputString);
            break;

        case ResultFormat::Binary:
            {
                auto& data = result.outputBinary;
                int stdoutFileDesc = _fileno(stdout);
                if (_isatty(stdoutFileDesc))
                {
                    // Writing to console, so we need to generate text output.

                    switch (targetReq->target)
                    {
                #if SLANG_ENABLE_DXBC_SUPPORT
                    case CodeGenTarget::DXBytecode:
                        {
                            String assembly = dissassembleDXBC(compileRequest,
                                data.begin(),
                                data.end() - data.begin());
                            writeOutputToConsole(compileRequest, assembly);
                        }
                        break;
                #endif

                #if SLANG_ENABLE_DXIL_SUPPORT
                    case CodeGenTarget::DXIL:
                        {
                            String assembly = dissassembleDXILUsingDXC(compileRequest,
                                data.begin(),
                                data.end() - data.begin());
                            writeOutputToConsole(compileRequest, assembly);
                        }
                        break;
                #endif

                    case CodeGenTarget::SPIRV:
                        {
                            String assembly = dissassembleSPIRV(compileRequest,
                                data.begin(),
                                data.end() - data.begin());
                            writeOutputToConsole(compileRequest, assembly);
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
                #ifdef _WIN32
                    _setmode(stdoutFileDesc, _O_BINARY);
                #endif
                    writeOutputFile(
                        compileRequest,
                        stdout,
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
        auto& result = targetReq->entryPointResults[entryPointIndex];

        // Skip the case with no output
        if (result.format == ResultFormat::None)
            return;

        if (entryPoint->outputPath.Length())
        {
            writeEntryPointResultToFile(entryPoint, targetReq, entryPointIndex);
        }
        else
        {
            writeEntryPointResultToStandardOutput(entryPoint, targetReq, entryPointIndex);
        }
    }

    void emitEntryPoints(
        TargetRequest*          /*targetReq*/)
    {

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

        // If we are being asked to generate code in a container
        // format, then we are now in a position to do so.
        switch (compileRequest->containerFormat)
        {
        default:
            break;

        case ContainerFormat::SlangModule:
            generateBytecodeForCompileRequest(compileRequest);
            break;
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
                String spirvAssembly = dissassembleSPIRV(compileRequest, data, size);
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
                String dxbcAssembly = dissassembleDXBC(compileRequest, data, size);
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
                String dxilAssembly = dissassembleDXILUsingDXC(compileRequest, data, size);
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
