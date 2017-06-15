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
    //

    Profile Profile::LookUp(char const* name)
    {
        #define PROFILE(TAG, NAME, STAGE, VERSION)	if(strcmp(name, #NAME) == 0) return Profile::TAG;
        #define PROFILE_ALIAS(TAG, NAME)			if(strcmp(name, #NAME) == 0) return Profile::TAG;
        #include "profile-defs.h"

        return Profile::Unknown;
    }



    //

    String EmitHLSL(ExtraContext& context)
    {
        if (context.getOptions().passThrough != PassThroughMode::None)
        {
            return context.sourceText;
        }
        else
        {
            // TODO(tfoley): probably need a way to customize the emit logic...
            return emitProgram(
                context.programSyntax.Ptr(),
                context.programLayout,
                CodeGenTarget::HLSL);
        }
    }

    String emitGLSLForEntryPoint(ExtraContext& context, EntryPointOption const& /*entryPoint*/)
    {
        if (context.getOptions().passThrough != PassThroughMode::None)
        {
            return context.sourceText;
        }
        else
        {
            // TODO(tfoley): probably need a way to customize the emit logic...
            return emitProgram(
                context.programSyntax.Ptr(),
                context.programLayout,
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
        ExtraContext&				context,
        EntryPointOption const&		entryPoint)
    {
        static pD3DCompile D3DCompile_ = nullptr;
        if (!D3DCompile_)
        {
            HMODULE d3dCompiler = (HMODULE)GetD3DCompilerDLL();
            assert(d3dCompiler);

            D3DCompile_ = (pD3DCompile)GetProcAddress(d3dCompiler, "D3DCompile");
            assert(D3DCompile_);
        }

        // The HLSL compiler will try to "canonicalize" our input file path,
        // and we don't want it to do that, because they it won't report
        // the same locations on error messages that we would.
        //
        // To work around that, we prepend a custom `#line` directive.

        String rawHlslCode = EmitHLSL(context);

        StringBuilder hlslCodeBuilder;
        hlslCodeBuilder << "#line 1 \"";
        for(auto c : context.sourcePath)
        {
            char buffer[] = { c, 0 };
            switch(c)
            {
            default:
                hlslCodeBuilder << buffer;
                break;

            case '\\':
                hlslCodeBuilder << "\\\\";
            }
        }
        hlslCodeBuilder << "\"\n";
        hlslCodeBuilder << rawHlslCode;

        auto hlslCode = hlslCodeBuilder.ProduceString();

        ID3DBlob* codeBlob;
        ID3DBlob* diagnosticsBlob;
        HRESULT hr = D3DCompile_(
            hlslCode.begin(),
            hlslCode.Length(),
            context.sourcePath.begin(),
            nullptr,
            nullptr,
            entryPoint.name.begin(),
            GetHLSLProfileName(entryPoint.profile),
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
            String diagnostics = (char const*) diagnosticsBlob->GetBufferPointer();
            fprintf(stderr, "%s", diagnostics.begin());
            OutputDebugStringA(diagnostics.begin());
            diagnosticsBlob->Release();
        }
        if (FAILED(hr))
        {
            // TODO(tfoley): What to do on failure?
        }
        return data;
    }

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

    String EmitDXBytecodeAssemblyForEntryPoint(
        ExtraContext&				context,
        EntryPointOption const&		entryPoint)
    {
        static pD3DDisassemble D3DDisassemble_ = nullptr;
        if (!D3DDisassemble_)
        {
            HMODULE d3dCompiler = (HMODULE)GetD3DCompilerDLL();
            assert(d3dCompiler);

            D3DDisassemble_ = (pD3DDisassemble)GetProcAddress(d3dCompiler, "D3DDisassemble");
            assert(D3DDisassemble_);
        }

        List<uint8_t> dxbc = EmitDXBytecodeForEntryPoint(context, entryPoint);
        if (!dxbc.Count())
        {
            return "";
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
            result = String((char const*) codeBlob->GetBufferPointer());
            codeBlob->Release();
        }
        if (FAILED(hr))
        {
            // TODO(tfoley): need to figure out what to diagnose here...
        }
        return result;
    }


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


    HMODULE getGLSLCompilerDLL()
    {
        // TODO(tfoley): let user specify version of glslang DLL to use.
        static HMODULE glslCompiler =  LoadLibraryA("glslang");
        // TODO(tfoley): handle case where we can't find it gracefully
        assert(glslCompiler);
        return glslCompiler;
    }


    String emitSPIRVAssemblyForEntryPoint(
        ExtraContext&				context,
        EntryPointOption const&		entryPoint)
    {
        String rawGLSL = emitGLSLForEntryPoint(context, entryPoint);

        static glslang_CompileFunc glslang_compile = nullptr;
        if (!glslang_compile)
        {
            HMODULE glslCompiler = getGLSLCompilerDLL();
            assert(glslCompiler);

            glslang_compile = (glslang_CompileFunc)GetProcAddress(glslCompiler, "glslang_compile");
            assert(glslang_compile);
        }

        StringBuilder diagnosticBuilder;
        StringBuilder outputBuilder;

        auto outputFunc = [](char const* text, void* userData)
        {
            *(StringBuilder*)userData << text;
        };

        glslang_CompileRequest request;
        request.sourcePath = context.sourcePath.begin();
        request.sourceText = rawGLSL.begin();
        request.slangStage = (SlangStage) entryPoint.profile.GetStage();

        request.diagnosticFunc = outputFunc;
        request.diagnosticUserData = &diagnosticBuilder;

        request.outputFunc = outputFunc;
        request.outputUserData = &outputBuilder;

        int err = glslang_compile(&request);

        String diagnostics = diagnosticBuilder.ProduceString();
        String output = outputBuilder.ProduceString();

        if(err)
        {
            OutputDebugStringA(diagnostics.Buffer());
            fprintf(stderr, "%s", diagnostics.Buffer());
            exit(1);
        }

        return output;
    }
#endif

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

    // Do emit logic for a single entry point
    EntryPointResult emitEntryPoint(ExtraContext& context, EntryPointOption& entryPoint)
    {
        EntryPointResult result;

        switch (context.getOptions().Target)
        {
        case CodeGenTarget::GLSL:
            {
                String code = emitGLSLForEntryPoint(context, entryPoint);
                result.outputSource = code;
            }
            break;

        case CodeGenTarget::DXBytecode:
            {
                auto code = EmitDXBytecodeForEntryPoint(context, entryPoint);

                // TODO(tfoley): Need to figure out an appropriate interface
                // for returning binary code, in addition to source.
#if 0
                if (context.compileResult)
                {
                    StringBuilder sb;
                    sb.Append((char*) code.begin(), code.Count());

                    String codeString = sb.ProduceString();
                    result.outputSource = codeString;
                }
                else
#endif
                {
                    int col = 0;
                    for(auto ii : code)
                    {
                        if(col != 0) fputs(" ", stdout);
                        fprintf(stdout, "%02X", ii);
                        col++;
                        if(col == 8)
                        {
                            fputs("\n", stdout);
                            col = 0;
                        }
                    }
                    if(col != 0)
                    {
                        fputs("\n", stdout);
                    }
                }
                return result;
            }
            break;

        case CodeGenTarget::DXBytecodeAssembly:
            {
                String code = EmitDXBytecodeAssemblyForEntryPoint(context, entryPoint);
                result.outputSource = code;
            }
            break;

        case CodeGenTarget::SPIRVAssembly:
            {
                String code = emitSPIRVAssemblyForEntryPoint(context, entryPoint);
                result.outputSource = code;
            }
            break;

        // Note(tfoley): We currently hit this case when compiling the stdlib
        case CodeGenTarget::Unknown:
            break;

        default:
            throw "unimplemented";
        }

        return result;


    }

    TranslationUnitResult emitTranslationUnitEntryPoints(ExtraContext& context)
    {
        TranslationUnitResult result;

        for (auto& entryPoint : context.getTranslationUnitOptions().entryPoints)
        {
            EntryPointResult entryPointResult = emitEntryPoint(context, entryPoint);

            result.entryPoints.Add(entryPointResult);
        }

        // The result for the translation unit will just be the concatenation
        // of the results for each entry point. This doesn't actually make
        // much sense, but it is good enough for now.
        StringBuilder sb;
        for (auto& entryPointResult : result.entryPoints)
        {
            sb << entryPointResult.outputSource;
        }

        result.outputSource = sb.ProduceString();

        return result;
    }

    // Do emit logic for an entire translation unit, which might
    // have zero or more entry points
    TranslationUnitResult emitTranslationUnit(ExtraContext& context)
    {
        // Most of our code generation targets will require us
        // to proceed through one entry point at a time, but
        // in some cases we can emit an entire translation unit
        // in one go.

        switch (context.getOptions().Target)
        {
        default:
            // The default behavior is going to loop over all the entry
            // points, and then collect an aggregate result.
            return emitTranslationUnitEntryPoints(context);

        case CodeGenTarget::HLSL:
            // When targetting HLSL, we can emit the entire translation unit
            // as a single HLSL program, and include all the entry points.
            {

                String hlsl = EmitHLSL(context);

                TranslationUnitResult result;
                result.outputSource = hlsl;

                // Because the user might ask for per-entry-point source,
                // we will just attach the same string as the result for
                // each entry point.
                for( auto& entryPoint : context.getTranslationUnitOptions().entryPoints )
                {
                    (void)entryPoint;

                    EntryPointResult entryPointResult;
                    entryPointResult.outputSource = hlsl;
                    result.entryPoints.Add(entryPointResult);
                }

                return result;
            }
            break;
        }
    }

    TranslationUnitResult generateOutput(ExtraContext& context)
    {
        TranslationUnitResult result = emitTranslationUnit(context);
        return result;
    }

    void generateOutput(
        ExtraContext&                   context,
        CollectionOfTranslationUnits*   collectionOfTranslationUnits)
    {
        switch (context.getOptions().Target)
        {
        default:
            // For most targets, we will do things per-translation-unit
            for( auto translationUnit : collectionOfTranslationUnits->translationUnits )
            {
                ExtraContext innerContext = context;
                innerContext.translationUnitOptions = &translationUnit.options;
                innerContext.programSyntax = translationUnit.SyntaxNode;
                innerContext.sourcePath = "slang"; // don't have this any more!
                innerContext.sourceText = "";

                TranslationUnitResult translationUnitResult = generateOutput(innerContext);
                context.compileResult->translationUnits.Add(translationUnitResult);
            }
            break;

        case CodeGenTarget::ReflectionJSON:
            {
                String reflectionJSON = emitReflectionJSON(context.programLayout);

                // HACK(tfoley): just print it out since that is what people probably expect.
                // TODO: need a way to control where output gets routed across all possible targets.
                fprintf(stdout, "%s", reflectionJSON.begin());
            }
            break;
        }
    }

    TranslationUnitResult passThrough(
        String const&			sourceText,
        String const&			sourcePath,
        const CompileOptions &	options,
        TranslationUnitOptions const& translationUnitOptions)
    {
        ExtraContext extra;
        extra.options = &options;
        extra.translationUnitOptions = &translationUnitOptions;
        extra.sourcePath = sourcePath;
        extra.sourceText = sourceText;

        return generateOutput(extra);
    }

}
