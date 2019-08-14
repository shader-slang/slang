// slang-support.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "slang-support.h"

#include "options.h"

#include <assert.h>
#include <stdio.h>

namespace renderer_test {

/* static */ SlangResult ShaderCompilerUtil::compileProgram(SlangSession* session, const Input& input, const ShaderCompileRequest& request, Output& out)
{
    out.reset();

    SlangCompileRequest* slangRequest = spCreateCompileRequest(session);
    out.request = slangRequest;
    out.session = session;

    spSetCodeGenTarget(slangRequest, input.target);
    spSetTargetProfile(slangRequest, 0, spFindProfile(session, input.profile));

    // Define a macro so that shader code in a test can detect what language we
    // are nominally working with.
    char const* langDefine = nullptr;
    switch (input.sourceLanguage)
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

    if (input.passThrough != SLANG_PASS_THROUGH_NONE)
    {
        spSetPassThrough(slangRequest, input.passThrough);
    }

    // Process any additional command-line options specified for Slang using
    // the `-xslang <arg>` option to `render-test`.
    SLANG_RETURN_ON_FAIL(spProcessCommandLineArguments(slangRequest, input.args, input.argCount)); 

    int computeTranslationUnit = 0;
    int vertexTranslationUnit = 0;
    int fragmentTranslationUnit = 0;
    char const* vertexEntryPointName = request.vertexShader.name;
    char const* fragmentEntryPointName = request.fragmentShader.name;
    char const* computeEntryPointName = request.computeShader.name;

    const auto sourceLanguage = input.sourceLanguage;

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


    Slang::List<const char*> rawGlobalTypeNames;
    for (auto typeName : request.globalGenericTypeArguments)
        rawGlobalTypeNames.add(typeName.getBuffer());
    spSetGlobalGenericArgs(
        slangRequest,
        (int)rawGlobalTypeNames.getCount(),
        rawGlobalTypeNames.getBuffer());

    Slang::List<const char*> rawEntryPointTypeNames;
    for (auto typeName : request.entryPointGenericTypeArguments)
        rawEntryPointTypeNames.add(typeName.getBuffer());

    const int globalExistentialTypeCount = int(request.globalExistentialTypeArguments.getCount());
    for(int ii = 0; ii < globalExistentialTypeCount; ++ii )
    {
        spSetTypeNameForGlobalExistentialTypeParam(slangRequest, ii, request.globalExistentialTypeArguments[ii].getBuffer());
    }

    const int entryPointExistentialTypeCount = int(request.entryPointExistentialTypeArguments.getCount());
    auto setEntryPointExistentialTypeArgs = [&](int entryPoint)
    {
        for( int ii = 0; ii < entryPointExistentialTypeCount; ++ii )
        {
            spSetTypeNameForEntryPointExistentialTypeParam(slangRequest, entryPoint, ii, request.entryPointExistentialTypeArguments[ii].getBuffer());
        }
    };

    if (request.computeShader.name)
    {
        int computeEntryPoint = spAddEntryPointEx(slangRequest, computeTranslationUnit, 
            computeEntryPointName,
            SLANG_STAGE_COMPUTE,
            (int)rawEntryPointTypeNames.getCount(),
            rawEntryPointTypeNames.getBuffer());

        setEntryPointExistentialTypeArgs(computeEntryPoint);

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

            out.set(PipelineType::Compute, &kernelDesc, 1);
        }
    }
    else
    {
        int vertexEntryPoint = spAddEntryPointEx(slangRequest, vertexTranslationUnit, vertexEntryPointName, SLANG_STAGE_VERTEX, (int)rawEntryPointTypeNames.getCount(), rawEntryPointTypeNames.getBuffer());
        int fragmentEntryPoint = spAddEntryPointEx(slangRequest, fragmentTranslationUnit, fragmentEntryPointName, SLANG_STAGE_FRAGMENT, (int)rawEntryPointTypeNames.getCount(), rawEntryPointTypeNames.getBuffer());

        setEntryPointExistentialTypeArgs(vertexEntryPoint);
        setEntryPointExistentialTypeArgs(fragmentEntryPoint);

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

            out.set(PipelineType::Graphics, kernelDescs, kDescCount);
        }
    }

    return SLANG_OK;
}

} // renderer_test
