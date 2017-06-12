// slang-support.cpp

#define SLANG_INCLUDE_IMPLEMENTATION

#include "slang-support.h"

#include <stdio.h>

namespace renderer_test {

struct SlangShaderCompilerWrapper : public ShaderCompiler
{
    ShaderCompiler*     innerCompiler;
    SlangCompileTarget  target;

    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override
    {
        SlangSession* slangSession = spCreateSession(NULL);
        SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

        spSetCodeGenTarget(slangRequest, target);

        // Define a macro so that shader code in a test can detect when it is being
        // compiled as Slang source code.
        spAddPreprocessorDefine(slangRequest, "__SLANG__", "1");

        int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

        spAddTranslationUnitSourceString(slangRequest, translationUnitIndex, request.source.path, request.source.text);

        int vertexEntryPoint = spAddTranslationUnitEntryPoint(slangRequest, translationUnitIndex, request.vertexShader.name,   spFindProfile(slangSession, request.vertexShader.profile));
        int fragmentEntryPoint = spAddTranslationUnitEntryPoint(slangRequest, translationUnitIndex, request.fragmentShader.name, spFindProfile(slangSession, request.fragmentShader.profile));

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

        char const* translatedCode = spGetTranslationUnitSource(slangRequest, translationUnitIndex);
        char const* vertexCode = spGetEntryPointSource(slangRequest, translationUnitIndex, vertexEntryPoint);
        char const* fragmentCode = spGetEntryPointSource(slangRequest, translationUnitIndex, fragmentEntryPoint);

        ShaderCompileRequest innerRequest = request;
        innerRequest.source.text = translatedCode;
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

ShaderCompiler* createSlangShaderCompiler(ShaderCompiler* innerCompiler, SlangCompileTarget target)
{
    auto result = new SlangShaderCompilerWrapper();
    result->innerCompiler = innerCompiler;
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
