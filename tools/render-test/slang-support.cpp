// slang-support.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "slang-support.h"

#include "options.h"

#include <assert.h>
#include <stdio.h>

#include "../../source/core/slang-string-util.h"

namespace renderer_test {
using namespace Slang;

// Entry point name to use for vertex/fragment shader
static const char vertexEntryPointName[] = "vertexMain";
static const char fragmentEntryPointName[] = "fragmentMain";
static const char computeEntryPointName[] = "computeMain";

/* static */ SlangResult ShaderCompilerUtil::compileProgram(SlangSession* session, const Input& input, const ShaderCompileRequest& request, Output& out)
{
    out.reset();

    SlangCompileRequest* slangRequest = spCreateCompileRequest(session);
    out.request = slangRequest;
    out.session = session;

    // Parse all the extra args
    if (request.compileArgs.getCount() > 0)
    {
        List<const char*> args;
        for (const auto& arg : request.compileArgs)
        {
            args.add(arg.value.getBuffer());
        }
        SLANG_RETURN_ON_FAIL(spProcessCommandLineArguments(slangRequest, args.getBuffer(), int(args.getCount())));
    }

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
    case SLANG_SOURCE_LANGUAGE_C:
        spAddPreprocessorDefine(slangRequest, "__C__", "1");
        break;
    case SLANG_SOURCE_LANGUAGE_CPP:
        spAddPreprocessorDefine(slangRequest, "__CPP__", "1");
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

    const int globalSpecializationArgCount = int(request.globalSpecializationArgs.getCount());
    for(int ii = 0; ii < globalSpecializationArgCount; ++ii )
    {
        spSetTypeNameForGlobalExistentialTypeParam(slangRequest, ii, request.globalSpecializationArgs[ii].getBuffer());
    }

    const int entryPointSpecializationArgCount = int(request.entryPointSpecializationArgs.getCount());
    auto setEntryPointSpecializationArgs = [&](int entryPoint)
    {
        for( int ii = 0; ii < entryPointSpecializationArgCount; ++ii )
        {
            spSetTypeNameForEntryPointExistentialTypeParam(slangRequest, entryPoint, ii, request.entryPointSpecializationArgs[ii].getBuffer());
        }
    };

    if (request.computeShader.name)
    {
        int computeEntryPointIndex = 0;
        if(!gOptions.dontAddDefaultEntryPoints)
        {
            computeEntryPointIndex = spAddEntryPoint(slangRequest, computeTranslationUnit,
                computeEntryPointName,
                SLANG_STAGE_COMPUTE);

            setEntryPointSpecializationArgs(computeEntryPointIndex);
        }

        spSetLineDirectiveMode(slangRequest, SLANG_LINE_DIRECTIVE_MODE_NONE);

        const SlangResult res = spCompile(slangRequest);

        if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
        {
            fprintf(stderr, "%s", diagnostics);
        }

        SLANG_RETURN_ON_FAIL(res);

        // We are going to get the entry point count... lets check what we have
        {
            auto reflection = spGetReflection(slangRequest);
            // Get the amount of entry points in reflection
            const int entryPointCount = int(spReflection_getEntryPointCount(reflection));

            // Above code assumes there is an entry point
            SLANG_ASSERT(entryPointCount && computeEntryPointIndex < entryPointCount);

            auto entryPoint = spReflection_getEntryPointByIndex(reflection, computeEntryPointIndex);

            // Get the entry point name
            const char* entryPointName = spReflectionEntryPoint_getName(entryPoint);

            SLANG_ASSERT(entryPointName);
        }

        {
            size_t codeSize = 0;
            char const* code = (char const*) spGetEntryPointCode(slangRequest, computeEntryPointIndex, &codeSize);

            ShaderProgram::KernelDesc kernelDesc;
            kernelDesc.stage = StageType::Compute;
            kernelDesc.codeBegin = code;
            kernelDesc.codeEnd = code + codeSize;

            out.set(PipelineType::Compute, &kernelDesc, 1);
        }
    }
    else
    {
        int vertexEntryPoint = 0;
        int fragmentEntryPoint = 1;
        if( !gOptions.dontAddDefaultEntryPoints )
        {
            vertexEntryPoint = spAddEntryPoint(slangRequest, vertexTranslationUnit, vertexEntryPointName, SLANG_STAGE_VERTEX);
            fragmentEntryPoint = spAddEntryPoint(slangRequest, fragmentTranslationUnit, fragmentEntryPointName, SLANG_STAGE_FRAGMENT);

            setEntryPointSpecializationArgs(vertexEntryPoint);
            setEntryPointSpecializationArgs(fragmentEntryPoint);
        }

        const SlangResult res = spCompile(slangRequest);
        if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
        {
            // TODO(tfoley): re-enable when I get a logging solution in place
//            OutputDebugStringA(diagnostics);
            fprintf(stderr, "%s", diagnostics);
        }

        SLANG_RETURN_ON_FAIL(res);

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

/* static */SlangResult ShaderCompilerUtil::readSource(const String& inSourcePath, List<char>& outSourceText)
{
    // Read in the source code
    FILE* sourceFile = fopen(inSourcePath.getBuffer(), "rb");
    if (!sourceFile)
    {
        fprintf(stderr, "error: failed to open '%s' for reading\n", inSourcePath.getBuffer());
        return SLANG_FAIL;
    }
    fseek(sourceFile, 0, SEEK_END);
    size_t sourceSize = ftell(sourceFile);
    fseek(sourceFile, 0, SEEK_SET);

    outSourceText.setCount(sourceSize + 1);
    fread(outSourceText.getBuffer(), sourceSize, 1, sourceFile);
    fclose(sourceFile);
    outSourceText[sourceSize] = 0;

    return SLANG_OK;
}

/* static */SlangResult ShaderCompilerUtil::compileWithLayout(SlangSession* session, const String& sourcePath, const Slang::List<Slang::CommandLine::Arg>& compileArgs, Options::ShaderProgramType shaderType, const ShaderCompilerUtil::Input& input, OutputAndLayout& output)
{
    List<char> sourceText;
    SLANG_RETURN_ON_FAIL(readSource(sourcePath, sourceText));

    if (input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_C)
    {
        // Add an include of the prelude
        ComPtr<ISlangBlob> prelude;
        session->getDownstreamCompilerPrelude(SLANG_PASS_THROUGH_GENERIC_C_CPP, prelude.writeRef());

        String preludeString = StringUtil::getString(prelude);

        // Add the prelude 
        StringBuilder builder;
        builder << preludeString << "\n";
        builder << UnownedStringSlice(sourceText.getBuffer(), sourceText.getCount());

        sourceText.setCount(builder.getLength());
        memcpy(sourceText.getBuffer(), builder.getBuffer(), builder.getLength());
    }

    output.sourcePath = sourcePath;

    auto& layout = output.layout;

    // Default the amount of renderTargets based on shader type
    switch (shaderType)
    {
        default:
            layout.numRenderTargets = 1;
            break;

        case Options::ShaderProgramType::Compute:
            layout.numRenderTargets = 0;
            break;
    }

    // Deterministic random generator
    RefPtr<RandomGenerator> rand = RandomGenerator::create(0x34234);

    // Parse the layout
    layout.parse(rand, sourceText.getBuffer());
    layout.updateForTarget(input.target);

    // Setup SourceInfo
    ShaderCompileRequest::SourceInfo sourceInfo;
    sourceInfo.path = sourcePath.getBuffer();
    sourceInfo.dataBegin = sourceText.getBuffer();
    // Subtract 1 because it's zero terminated
    sourceInfo.dataEnd = sourceText.getBuffer() + sourceText.getCount() - 1;

    ShaderCompileRequest compileRequest;

    compileRequest.compileArgs = compileArgs;

    compileRequest.source = sourceInfo;
    if (shaderType == Options::ShaderProgramType::Graphics || shaderType == Options::ShaderProgramType::GraphicsCompute)
    {
        compileRequest.vertexShader.source = sourceInfo;
        compileRequest.vertexShader.name = vertexEntryPointName;
        compileRequest.fragmentShader.source = sourceInfo;
        compileRequest.fragmentShader.name = fragmentEntryPointName;
    }
    else
    {
        compileRequest.computeShader.source = sourceInfo;
        compileRequest.computeShader.name = computeEntryPointName;
    }
    compileRequest.globalSpecializationArgs = layout.globalSpecializationArgs;
    compileRequest.entryPointSpecializationArgs = layout.entryPointSpecializationArgs;

    return ShaderCompilerUtil::compileProgram(session, input, compileRequest, output.output);
}

} // renderer_test
