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
#include "../../tools/glslang/glslang.h"


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#include <d3dcompiler.h>
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
        #define PROFILE_ALIAS(TAG, NAME)			if(strcmp(name, #NAME) == 0) return Profile::TAG;
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
    void* GetD3DCompilerDLL()
    {
        // TODO(tfoley): let user specify version of d3dcompiler DLL to use.
        static HMODULE d3dCompiler =  LoadLibraryA("d3dcompiler_47");
        // TODO(tfoley): handle case where we can't find it gracefully
        assert(d3dCompiler);
        return d3dCompiler;
    }

    List<uint8_t> EmitDXBytecodeForEntryPoint(
        EntryPointRequest*  entryPoint)
    {
        static pD3DCompile D3DCompile_ = nullptr;
        if (!D3DCompile_)
        {
            HMODULE d3dCompiler = (HMODULE)GetD3DCompilerDLL();
            assert(d3dCompiler);

            D3DCompile_ = (pD3DCompile)GetProcAddress(d3dCompiler, "D3DCompile");
            assert(D3DCompile_);
        }

        auto hlslCode = emitHLSLForEntryPoint(entryPoint);

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

    String EmitDXBytecodeAssemblyForEntryPoint(
        EntryPointRequest*  entryPoint)
    {
        static pD3DDisassemble D3DDisassemble_ = nullptr;
        if (!D3DDisassemble_)
        {
            HMODULE d3dCompiler = (HMODULE)GetD3DCompilerDLL();
            assert(d3dCompiler);

            D3DDisassemble_ = (pD3DDisassemble)GetProcAddress(d3dCompiler, "D3DDisassemble");
            assert(D3DDisassemble_);
        }

        List<uint8_t> dxbc = EmitDXBytecodeForEntryPoint(entryPoint);
        if (!dxbc.Count())
        {
            return String();
        }

        ID3DBlob* codeBlob;
        HRESULT hr = D3DDisassemble_(
            &dxbc[0],
            dxbc.Count(),
            0,
            nullptr,
            &codeBlob);

        String result;
        if (codeBlob)
        {
            char const* codeBegin = (char const*)codeBlob->GetBufferPointer();
            char const* codeEnd = codeBegin + codeBlob->GetBufferSize();
            result.append(codeBegin, codeEnd);
            codeBlob->Release();
        }
        if (FAILED(hr))
        {
            // TODO(tfoley): need to figure out what to diagnose here...
        }
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


    HMODULE getGLSLCompilerDLL()
    {
        // TODO(tfoley): let user specify version of glslang DLL to use.
        static HMODULE glslCompiler =  LoadLibraryA("glslang");
        // TODO(tfoley): handle case where we can't find it gracefully
        assert(glslCompiler);
        return glslCompiler;
    }

    int invokeGLSLCompilerForEntryPoint(
        EntryPointRequest*          entryPoint,
        glslang_CompileRequest&     request)
    {
        String rawGLSL = emitGLSLForEntryPoint(entryPoint);

        static glslang_CompileFunc glslang_compile = nullptr;
        if (!glslang_compile)
        {
            HMODULE glslCompiler = getGLSLCompilerDLL();
            assert(glslCompiler);

            glslang_compile = (glslang_CompileFunc)GetProcAddress(glslCompiler, "glslang_compile");
            assert(glslang_compile);
        }

        String diagnosticOutput;
        auto diagnosticOutputFunc = [](void const* data, size_t size, void* userData)
        {
            (*(String*)userData).append((char const*)data, (char const*)data + size);
        };

        request.sourcePath = "slang";
        request.sourceText = rawGLSL.begin();
        request.slangStage = (SlangStage)entryPoint->profile.GetStage();

        request.diagnosticFunc = diagnosticOutputFunc;
        request.diagnosticUserData = &diagnosticOutput;

        int err = glslang_compile(&request);

        if (err)
        {
            entryPoint->compileRequest->mSink.diagnoseRaw(
                Severity::Error,
                diagnosticOutput.begin());
            return err;
        }

        return 0;
    }


    List<uint8_t> emitSPIRVForEntryPoint(
        EntryPointRequest*  entryPoint)
    {
        List<uint8_t> output;
        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            ((List<uint8_t>*)userData)->AddRange((uint8_t*)data, size);
        };

        glslang_CompileRequest request;
        request.outputFunc = outputFunc;
        request.outputUserData = &output;
        request.disassembleResult = false;

        int err = invokeGLSLCompilerForEntryPoint(entryPoint, request);

        if (err)
        {
            return List<uint8_t>();
        }

        return output;
    }

    String emitSPIRVAssemblyForEntryPoint(
        EntryPointRequest*  entryPoint)
    {
        String output;
        auto outputFunc = [](void const* data, size_t size, void* userData)
        {
            (*(String*)userData).append((char const*)data, (char const*)data + size);
        };

        glslang_CompileRequest request;
        request.outputFunc = outputFunc;
        request.outputUserData = &output;
        request.disassembleResult = true;

        int err = invokeGLSLCompilerForEntryPoint(entryPoint, request);

        if (err)
        {
            String();
        }

        return output;
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

        switch (compileRequest->Target)
        {
        case CodeGenTarget::HLSL:
            {
                String code = emitHLSLForEntryPoint(entryPoint);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::GLSL:
            {
                String code = emitGLSLForEntryPoint(entryPoint);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::DXBytecode:
            {
                List<uint8_t> code = EmitDXBytecodeForEntryPoint(entryPoint);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::DXBytecodeAssembly:
            {
                String code = EmitDXBytecodeAssemblyForEntryPoint(entryPoint);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::SPIRV:
            {
                List<uint8_t> code = emitSPIRVForEntryPoint(entryPoint);
                result = CompileResult(code);
            }
            break;

        case CodeGenTarget::SPIRVAssembly:
            {
                String code = emitSPIRVAssemblyForEntryPoint(entryPoint);
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
            throw "unimplemented";
        }

        return result;
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

                // HACK(tfoley): just print it out since that is what people probably expect.
                // TODO: need a way to control where output gets routed across all possible targets.
                fprintf(stdout, "%s", reflectionJSON.begin());

                return;
            }
            break;

        default:
            break;
        }

    }

}
