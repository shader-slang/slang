// slang-support.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "slang-support.h"

#include "options.h"

#include <assert.h>
#include <stdio.h>

namespace renderer_test {

RefPtr<ShaderProgram> ShaderCompiler::compileProgram(
    ShaderCompileRequest const& request)
{
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

    spSetCodeGenTarget(slangRequest, target);
    spSetTargetProfile(slangRequest, 0,
        spFindProfile(slangSession, profile));

    // Define a macro so that shader code in a test can detect what language we
    // are nominally working with.
    char const* langDefine = nullptr;
    switch (sourceLanguage)
    {
    case SLANG_SOURCE_LANGUAGE_GLSL:
        spAddPreprocessorDefine(slangRequest, "__GLSL__", "1");
        break;

    case SLANG_SOURCE_LANGUAGE_SLANG:
        spAddPreprocessorDefine(slangRequest, "__SLANG__", "1");
        // fall through
    case SLANG_SOURCE_LANGUAGE_HLSL:
        spAddPreprocessorDefine(slangRequest, "__HLSL__", "1");
        break;

    default:
        assert(!"unexpected");
        break;
    }

    if (passThrough != SLANG_PASS_THROUGH_NONE)
    {
        spSetPassThrough(slangRequest, passThrough);
    }

    // Process any additional command-line options specified for Slang using
    // the `-xslang <arg>` option to `render-test`.
    SLANG_RETURN_NULL_ON_FAIL(spProcessCommandLineArguments(slangRequest, &gOptions.slangArgs[0], gOptions.slangArgCount));

    int computeTranslationUnit = 0;
    int vertexTranslationUnit = 0;
    int fragmentTranslationUnit = 0;
    char const* vertexEntryPointName = request.vertexShader.name;
    char const* fragmentEntryPointName = request.fragmentShader.name;
    char const* computeEntryPointName = request.computeShader.name;

    if (sourceLanguage == SLANG_SOURCE_LANGUAGE_GLSL)
    {
        // GLSL presents unique challenges because, frankly, it got the whole
        // compilation model wrong. One aspect of working around this is that
        // we will compile the same source file multiple times: once per
        // entry point, and we will have different preprocessor definitions
        // active in each case.

        vertexTranslationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
        spAddTranslationUnitSourceString(slangRequest, vertexTranslationUnit, request.source.path, request.source.dataBegin);
        spTranslationUnit_addPreprocessorDefine(slangRequest, vertexTranslationUnit, "__GLSL_VERTEX__", "1");
        vertexEntryPointName = "main";

        fragmentTranslationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
        spAddTranslationUnitSourceString(slangRequest, fragmentTranslationUnit, request.source.path, request.source.dataBegin);
        spTranslationUnit_addPreprocessorDefine(slangRequest, fragmentTranslationUnit, "__GLSL_FRAGMENT__", "1");
        fragmentEntryPointName = "main";

        computeTranslationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
        spAddTranslationUnitSourceString(slangRequest, computeTranslationUnit, request.source.path, request.source.dataBegin);
        spTranslationUnit_addPreprocessorDefine(slangRequest, computeTranslationUnit, "__GLSL_COMPUTE__", "1");
        computeEntryPointName = "main";
    }
    else
    {
        int translationUnit = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
        spAddTranslationUnitSourceString(slangRequest, translationUnit, request.source.path, request.source.dataBegin);

        vertexTranslationUnit = translationUnit;
        fragmentTranslationUnit = translationUnit;
        computeTranslationUnit = translationUnit;
    }


    RefPtr<ShaderProgram> shaderProgram;

    Slang::List<const char*> rawGlobalTypeNames;
    for (auto typeName : request.globalTypeArguments)
        rawGlobalTypeNames.Add(typeName.Buffer());
    spSetGlobalGenericArgs(
        slangRequest,
        (int)rawGlobalTypeNames.Count(),
        rawGlobalTypeNames.Buffer());

    Slang::List<const char*> rawEntryPointTypeNames;
    for (auto typeName : request.entryPointTypeArguments)
        rawEntryPointTypeNames.Add(typeName.Buffer());
    if (request.computeShader.name)
    {
        int computeEntryPoint = spAddEntryPointEx(slangRequest, computeTranslationUnit, 
            computeEntryPointName,
            SLANG_STAGE_COMPUTE,
            (int)rawEntryPointTypeNames.Count(),
            rawEntryPointTypeNames.Buffer());

        spSetLineDirectiveMode(slangRequest, SLANG_LINE_DIRECTIVE_MODE_NONE);
        const SlangResult res = spCompile(slangRequest);
        if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
        {
            fprintf(stderr, "%s", diagnostics);
        }
        if (SLANG_SUCCEEDED(res))
        {
            size_t codeSize = 0;
            char const* code = (char const*) spGetEntryPointCode(slangRequest, computeEntryPoint, &codeSize);

            ShaderProgram::KernelDesc kernelDesc;
            kernelDesc.stage = StageType::Compute;
            kernelDesc.codeBegin = code;
            kernelDesc.codeEnd = code + codeSize;

            ShaderProgram::Desc desc;
            desc.pipelineType = PipelineType::Compute;
            desc.kernels = &kernelDesc;
            desc.kernelCount = 1;

            shaderProgram = renderer->createProgram(desc);
        }
    }
    else
    {
        int vertexEntryPoint = spAddEntryPointEx(slangRequest, vertexTranslationUnit, vertexEntryPointName, SLANG_STAGE_VERTEX, (int)rawEntryPointTypeNames.Count(), rawEntryPointTypeNames.Buffer());
        int fragmentEntryPoint = spAddEntryPointEx(slangRequest, fragmentTranslationUnit, fragmentEntryPointName, SLANG_STAGE_FRAGMENT, (int)rawEntryPointTypeNames.Count(), rawEntryPointTypeNames.Buffer());

        const SlangResult res = spCompile(slangRequest);
        if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
        {
            // TODO(tfoley): re-enable when I get a logging solution in place
//            OutputDebugStringA(diagnostics);
            fprintf(stderr, "%s", diagnostics);
        }
        if (SLANG_SUCCEEDED(res))
        {
            size_t vertexCodeSize = 0;
            char const* vertexCode = (char const*) spGetEntryPointCode(slangRequest, vertexEntryPoint, &vertexCodeSize);

            size_t fragmentCodeSize = 0;
            char const* fragmentCode = (char const*) spGetEntryPointCode(slangRequest, fragmentEntryPoint, &fragmentCodeSize);

            static const int kDescCount = 2;

            ShaderProgram::KernelDesc kernelDescs[kDescCount];

            kernelDescs[0].stage = StageType::Vertex;
            kernelDescs[0].codeBegin = vertexCode;
            kernelDescs[0].codeEnd = vertexCode + vertexCodeSize;

            kernelDescs[1].stage = StageType::Fragment;
            kernelDescs[1].codeBegin = fragmentCode;
            kernelDescs[1].codeEnd = fragmentCode + fragmentCodeSize;

            ShaderProgram::Desc desc;
            desc.pipelineType = PipelineType::Graphics;
            desc.kernels = &kernelDescs[0];
            desc.kernelCount = kDescCount;

            shaderProgram = renderer->createProgram(desc);
        }
    }
    // We clean up the Slang compilation context and result *after*
    // we have run the downstream compiler, because Slang
    // owns the memory allocation for the generated text, and will
    // free it when we destroy the compilation result.
    spDestroyCompileRequest(slangRequest);
    
    return shaderProgram;
}

} // renderer_test
