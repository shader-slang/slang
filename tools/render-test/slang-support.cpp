// slang-support.cpp

#define SLANG_INCLUDE_IMPLEMENTATION

#include "slang-support.h"

#include <stdio.h>

namespace renderer_test {

struct SlangShaderCompilerWrapper : public ShaderCompiler
{
    ShaderCompiler*     innerCompiler;
    SlangCompileTarget  target;
    SlangSourceLanguage sourceLanguage;

    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override
    {
        SlangSession* slangSession = spCreateSession(NULL);
        SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

        spSetCodeGenTarget(slangRequest, target);

        // Define a macro so that shader code in a test can detect what language we
        // are nominally working with.
        char const* langDefine = nullptr;
        switch (sourceLanguage)
        {
        case SLANG_SOURCE_LANGUAGE_GLSL:    langDefine = "__GLSL__";    break;
        case SLANG_SOURCE_LANGUAGE_HLSL:    langDefine = "__HLSL__";    break;
        case SLANG_SOURCE_LANGUAGE_SLANG:   langDefine = "__SLANG__";   break;
        default:
            assert(!"unexpected");
            break;
        }
        spAddPreprocessorDefine(slangRequest, langDefine, "1");

        int vertexTranslationUnit = 0;
        int fragmentTranslationUnit = 0;
        if( sourceLanguage == SLANG_SOURCE_LANGUAGE_GLSL )
        {
            // GLSL presents unique challenges because, frankly, it got the whole
            // compilation model wrong. One aspect of working around this is that
            // we will compile the same source file multiple times: once per
            // entry point, and we will have different preprocessor definitions
            // active in each case.

            vertexTranslationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
            spAddTranslationUnitSourceString(slangRequest, vertexTranslationUnit, request.source.path, request.source.text);

            spTranslationUnit_addPreprocessorDefine(slangRequest, vertexTranslationUnit, "__GLSL_VERTEX__", "1");

            fragmentTranslationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
            spAddTranslationUnitSourceString(slangRequest, fragmentTranslationUnit, request.source.path, request.source.text);

            spTranslationUnit_addPreprocessorDefine(slangRequest, fragmentTranslationUnit, "__GLSL_FRAGMENT__", "1");
        }
        else
        {
            int translationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
            spAddTranslationUnitSourceString(slangRequest, translationUnit, request.source.path, request.source.text);

            vertexTranslationUnit = translationUnit;
            fragmentTranslationUnit = translationUnit;
        }


        // If we aren't dealing with true Slang input, then don't enable checking.
        if (sourceLanguage != SLANG_SOURCE_LANGUAGE_SLANG)
        {
            spSetCompileFlags(slangRequest, SLANG_COMPILE_FLAG_NO_CHECKING);
        }

        int vertexEntryPoint = spAddTranslationUnitEntryPoint(slangRequest, vertexTranslationUnit, request.vertexShader.name,   spFindProfile(slangSession, request.vertexShader.profile));
        int fragmentEntryPoint = spAddTranslationUnitEntryPoint(slangRequest, fragmentTranslationUnit, request.fragmentShader.name, spFindProfile(slangSession, request.fragmentShader.profile));

        int compileErr = spCompile(slangRequest);
        if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
        {
            // TODO(tfoley): re-enable when I get a logging solution in place
//            OutputDebugStringA(diagnostics);
            fprintf(stderr, "%s", diagnostics);
        }
        if(compileErr)
        {
            return nullptr;
        }


        ShaderCompileRequest innerRequest = request;

        if( sourceLanguage != SLANG_SOURCE_LANGUAGE_GLSL )
        {
            char const* translatedCode = spGetTranslationUnitSource(slangRequest, 0);
            innerRequest.source.text = translatedCode;
        }

        char const* vertexCode = spGetEntryPointSource(slangRequest, vertexTranslationUnit, vertexEntryPoint);
        char const* fragmentCode = spGetEntryPointSource(slangRequest, fragmentTranslationUnit, fragmentEntryPoint);

        innerRequest.vertexShader.source.text = vertexCode;
        innerRequest.fragmentShader.source.text = fragmentCode;


        auto result = innerCompiler->compileProgram(innerRequest);

        // We clean up the Slang compilation context and result *after*
        // we have run the downstream compiler, because Slang
        // owns the memory allocation for the generated text, and will
        // free it when we destroy the compilation result.
        spDestroyCompileRequest(slangRequest);
        spDestroySession(slangSession);

        return result;
    }
};

ShaderCompiler* createSlangShaderCompiler(
    ShaderCompiler*     innerCompiler,
    SlangSourceLanguage sourceLanguage,
    SlangCompileTarget  target)
{
    auto result = new SlangShaderCompilerWrapper();
    result->innerCompiler = innerCompiler;
    result->sourceLanguage = sourceLanguage;
    result->target = target;

    return result;

}


} // renderer_test

//
// In order to actually use Slang in our application, we need to link in its
// implementation. The easiest way to accomplish this is by directly inlcuding
// the (concatenated) Slang source code into our app.
//

#include <slang.h>
