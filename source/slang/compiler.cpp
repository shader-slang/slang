// Compiler.cpp : Defines the entry point for the console application.
//
#include "../core/basic.h"
#include "../core/slang-io.h"
#include "compiler.h"
#include "lexer.h"
#include "parameter-binding.h"
#include "parser.h"
#include "preprocessor.h"
#include "syntax-visitors.h"
#include "slang-stdlib.h"

#include "reflection.h"
#include "emit.h"

// Utilities for pass-through modes
#include "../slang-glslang/slang-glslang.h"


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#include <d3dcompiler.h>
#endif

#include <io.h>
#include <fcntl.h> 

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

    //

    String emitHLSLForEntryPoint(
        EntryPointRequest*  entryPoint)
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
                compileRequest->layout.Ptr(),
                CodeGenTarget::HLSL);
        }
    }

    String emitGLSLForEntryPoint(
        EntryPointRequest*  entryPoint)
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
                compileRequest->layout.Ptr(),
                CodeGenTarget::GLSL);
        }
    }

    char const* GetHLSLProfileName(Profile profile)
    {
        switch(profile.raw)
        {
        #define PROFILE(TAG, NAME, STAGE, VERSION) case Profile::TAG: return #NAME;
        #include "profile-defs.h"

        default:
            // TODO: emit an error here!
            return "unknown";
        }
    }

#ifdef _WIN32
    HMODULE loadD3DCompilerDLL(CompileRequest* request)
    {
        char const* libraryName = "d3dcompiler_47";
        HMODULE d3dCompiler =  LoadLibraryA(libraryName);
        if (!d3dCompiler)
        {
            request->mSink.diagnose(CodePosition(), Diagnostics::failedToLoadDynamicLibrary, libraryName);
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
        EntryPointRequest*  entryPoint)
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

        auto hlslCode = emitHLSLForEntryPoint(entryPoint);
        maybeDumpIntermediate(entryPoint->compileRequest, hlslCode.Buffer(), CodeGenTarget::HLSL);

        ID3DBlob* codeBlob;
        ID3DBlob* diagnosticsBlob;
        HRESULT hr = D3DCompile_(
            hlslCode.begin(),
            hlslCode.Length(),
            "slang",
            nullptr,
            nullptr,
            entryPoint->name.begin(),
            GetHLSLProfileName(entryPoint->profile),
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
        if (diagnosticsBlob)
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

#if 0
    List<uint8_t> EmitDXBytecode(
        ExtraContext&				context)
    {
        if(context.getTranslationUnitOptions().entryPoints.Count() != 1)
        {
            if(context.getTranslationUnitOptions().entryPoints.Count() == 0)
            {
                // TODO(tfoley): need to write diagnostics into this whole thing...
                fprintf(stderr, "no entry point specified\n");
            }
            else
            {
                fprintf(stderr, "multiple entry points specified\n");
            }
            return List<uint8_t>();
        }

        return EmitDXBytecodeForEntryPoint(context, context.getTranslationUnitOptions().entryPoints[0]);
    }
#endif

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
        EntryPointRequest*  entryPoint)
    {

        List<uint8_t> dxbc = EmitDXBytecodeForEntryPoint(entryPoint);
        if (!dxbc.Count())
        {
            return String();
        }

        String result = dissassembleDXBC(entryPoint->compileRequest, dxbc.Buffer(), dxbc.Count());

        return result;
    }

#if 0
    String EmitDXBytecodeAssembly(
        ExtraContext&				context)
    {
        if(context.getTranslationUnitOptions().entryPoints.Count() == 0)
        {
            // TODO(tfoley): need to write diagnostics into this whole thing...
            fprintf(stderr, "no entry point specified\n");
            return "";
        }

        StringBuilder sb;
        for (auto entryPoint : context.getTranslationUnitOptions().entryPoints)
        {
            sb << EmitDXBytecodeAssemblyForEntryPoint(context, entryPoint);
        }
        return sb.ProduceString();
    }
#endif

    HMODULE loadGLSLCompilerDLL(CompileRequest* request)
    {
        char const* libraryName = "slang-glslang";
        // TODO(tfoley): let user specify version of glslang DLL to use.
        HMODULE glslCompiler =  LoadLibraryA(libraryName);
        if (!glslCompiler)
        {
            request->mSink.diagnose(CodePosition(), Diagnostics::failedToLoadDynamicLibrary, libraryName);
        }
        return glslCompiler;
    }

    HMODULE getGLSLCompilerDLL(CompileRequest* request)
    {
        static HMODULE glslCompiler =  loadGLSLCompilerDLL(request);
        return glslCompiler;
    }


    int invokeGLSLCompiler(
        CompileRequest*             slangCompileRequest,
        glslang_CompileRequest&     request)
    {

        static glslang_CompileFunc glslang_compile = nullptr;
        if (!glslang_compile)
        {
            HMODULE glslCompiler = getGLSLCompilerDLL(slangCompileRequest);
            if (!glslCompiler)
                return 1;

            glslang_compile = (glslang_CompileFunc)GetProcAddress(glslCompiler, "glslang_compile");
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
        EntryPointRequest*  entryPoint)
    {
        String rawGLSL = emitGLSLForEntryPoint(entryPoint);
        maybeDumpIntermediate(entryPoint->compileRequest, rawGLSL.Buffer(), CodeGenTarget::GLSL);

        List<uint8_t> output;
        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            ((List<uint8_t>*)userData)->AddRange((uint8_t*)data, size);
        };

        glslang_CompileRequest request;
        request.action = GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV;
        request.sourcePath = "slang";
        request.slangStage = (SlangStage)entryPoint->profile.GetStage();

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
        EntryPointRequest*  entryPoint)
    {
        List<uint8_t> spirv = emitSPIRVForEntryPoint(entryPoint);
        if (spirv.Count() == 0)
            return String();

        String result = dissassembleSPIRV(entryPoint->compileRequest, spirv.begin(), spirv.Count());
        return result;
    }
#endif

#if 0
    String emitSPIRVAssembly(
        ExtraContext&				context)
    {
        if(context.getTranslationUnitOptions().entryPoints.Count() == 0)
        {
            // TODO(tfoley): need to write diagnostics into this whole thing...
            fprintf(stderr, "no entry point specified\n");
            return "";
        }

        StringBuilder sb;
        for (auto entryPoint : context.getTranslationUnitOptions().entryPoints)
        {
            sb << emitSPIRVAssemblyForEntryPoint(context, entryPoint);
        }
        return sb.ProduceString();
    }
#endif

    // Do emit logic for a single entry point
    CompileResult emitEntryPoint(
        EntryPointRequest*  entryPoint)
    {
        CompileResult result;

        auto compileRequest = entryPoint->compileRequest;
        auto target = compileRequest->Target;

        switch (target)
        {
        case CodeGenTarget::HLSL:
            {
                String code = emitHLSLForEntryPoint(entryPoint);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::GLSL:
            {
                String code = emitGLSLForEntryPoint(entryPoint);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::DXBytecode:
            {
                List<uint8_t> code = EmitDXBytecodeForEntryPoint(entryPoint);
                maybeDumpIntermediate(compileRequest, code.Buffer(), code.Count(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::DXBytecodeAssembly:
            {
                String code = EmitDXBytecodeAssemblyForEntryPoint(entryPoint);
                maybeDumpIntermediate(compileRequest, code.Buffer(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::SPIRV:
            {
                List<uint8_t> code = emitSPIRVForEntryPoint(entryPoint);
                maybeDumpIntermediate(compileRequest, code.Buffer(), code.Count(), target);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::SPIRVAssembly:
            {
                String code = emitSPIRVAssemblyForEntryPoint(entryPoint);
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
                CodePosition(),
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
                CodePosition(),
                Diagnostics::cannotWriteOutputFile,
                path);
            return;
        }

        writeOutputFile(compileRequest, file, path, data, size);
        fclose(file);
    }

    static void writeEntryPointResultToFile(
        EntryPointRequest*  entryPoint)
    {
        auto compileRequest = entryPoint->compileRequest;
        auto outputPath = entryPoint->outputPath;
        auto result = entryPoint->result;
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
        EntryPointRequest*  entryPoint)
    {
        auto compileRequest = entryPoint->compileRequest;
        auto result = entryPoint->result;

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

                    switch (compileRequest->Target)
                    {
                    case CodeGenTarget::DXBytecode:
                        {
                            String assembly = dissassembleDXBC(compileRequest,
                                data.begin(),
                                data.end() - data.begin());
                            writeOutputToConsole(compileRequest, assembly);
                        }
                        break;

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
                    _setmode(stdoutFileDesc, _O_BINARY);
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
        EntryPointRequest*  entryPoint)
    {
        // Skip the case with no output
        if (entryPoint->result.format == ResultFormat::None)
            return;

        if (entryPoint->outputPath.Length())
        {
            writeEntryPointResultToFile(entryPoint);
        }
        else
        {
            writeEntryPointResultToStandardOutput(entryPoint);
        }
    }

    CompileResult emitTranslationUnitEntryPoints(
        TranslationUnitRequest* translationUnit)
    {
        CompileResult result;

        for (auto& entryPoint : translationUnit->entryPoints)
        {
            CompileResult entryPointResult = emitEntryPoint(entryPoint.Ptr());

            entryPoint->result = entryPointResult;
        }

        // The result for the translation unit will just be the concatenation
        // of the results for each entry point. This doesn't actually make
        // much sense, but it is good enough for now.
        //
        // TODO: Replace this with a packaged JSON and/or binary format.
        for (auto& entryPoint : translationUnit->entryPoints)
        {
            result.append(entryPoint->result);
        }

        return result;
    }

    // Do emit logic for an entire translation unit, which might
    // have zero or more entry points
    CompileResult emitTranslationUnit(
        TranslationUnitRequest* translationUnit)
    {
        return emitTranslationUnitEntryPoints(translationUnit);
    }

#if 0
    TranslationUnitResult generateOutput(ExtraContext& context)
    {
        TranslationUnitResult result = emitTranslationUnit(context);
        return result;
    }
#endif

    void generateOutput(
        CompileRequest* compileRequest)
    {
        // Start of with per-translation-unit and per-entry-point lowering
        for( auto translationUnit : compileRequest->translationUnits )
        {
            CompileResult translationUnitResult = emitTranslationUnit(translationUnit.Ptr());
            translationUnit->result = translationUnitResult;
        }


        // Allow for an "extra" target to verride things before we finish.
        String extraResult;
        switch (compileRequest->extraTarget)
        {
        case CodeGenTarget::ReflectionJSON:
            {
                String reflectionJSON = emitReflectionJSON(compileRequest->layout.Ptr());

                // Clobber existing output so we don't have to deal with it
                for( auto translationUnit : compileRequest->translationUnits )
                {
                    translationUnit->result = CompileResult();
                }
                for( auto entryPoint : compileRequest->entryPoints )
                {
                    entryPoint->result = CompileResult();
                }

                extraResult = reflectionJSON;
            }
            break;

        default:
            break;
        }

        // If we are in command-line mode, we might be expected to actually
        // write output to one or more files here.

        if (compileRequest->isCommandLineCompile)
        {
            switch (compileRequest->extraTarget)
            {
            case CodeGenTarget::ReflectionJSON:
                fprintf(stdout, "%s", extraResult.begin());
                break;

            default:
                for( auto entryPoint : compileRequest->entryPoints )
                {
                    writeEntryPointResult(entryPoint);
                }
                break;
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
        static int counter = 0;
        int id = counter++;

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

        case CodeGenTarget::DXBytecodeAssembly:
            dumpIntermediateText(compileRequest, data, size, ".dxbc.asm");
            break;

        case CodeGenTarget::SPIRV:
            dumpIntermediateBinary(compileRequest, data, size, ".spv");
            {
                String spirvAssembly = dissassembleSPIRV(compileRequest, data, size);
                dumpIntermediateText(compileRequest, spirvAssembly.begin(), spirvAssembly.Length(), ".spv.asm");
            }
            break;

        case CodeGenTarget::DXBytecode:
            dumpIntermediateBinary(compileRequest, data, size, ".dxbc");
            {
                String dxbcAssembly = dissassembleDXBC(compileRequest, data, size);
                dumpIntermediateText(compileRequest, dxbcAssembly.begin(), dxbcAssembly.Length(), ".dxbc.asm");
            }
            break;
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
