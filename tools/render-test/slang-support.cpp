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
static const char rtEntryPointName[] = "raygenMain";

gfx::StageType translateStage(SlangStage slangStage)
{
    switch(slangStage)
    {
    default:
        SLANG_ASSERT(!"unhandled case");
        return gfx::StageType::Unknown;

#define CASE(FROM, TO) \
    case SLANG_STAGE_##FROM: return gfx::StageType::TO

    CASE(VERTEX,    Vertex);
    CASE(HULL,      Hull);
    CASE(DOMAIN,    Domain);
    CASE(GEOMETRY,  Geometry);
    CASE(FRAGMENT,  Fragment);

    CASE(COMPUTE,   Compute);

    CASE(RAY_GENERATION,    RayGeneration);
    CASE(INTERSECTION,      Intersection);
    CASE(ANY_HIT,           AnyHit);
    CASE(CLOSEST_HIT,       ClosestHit);
    CASE(MISS,              Miss);
    CASE(CALLABLE,          Callable);

#undef CASE
    }
}

void ShaderCompilerUtil::Output::set(
    PipelineType                        pipelineType,
    slang::IComponentType*              inSlangProgram)
{
    slangProgram = inSlangProgram;
    desc.pipelineType = pipelineType;
    desc.slangProgram = inSlangProgram;
}

void ShaderCompilerUtil::Output::reset()
{
    {
        desc.pipelineType = PipelineType::Unknown;
        desc.slangProgram = nullptr;
    }

    if (m_requestForKernels && session)
    {
        spDestroyCompileRequest(m_requestForKernels);
    }
    if (m_extraRequestForReflection && session)
    {
        spDestroyCompileRequest(m_extraRequestForReflection);
    }
    session = nullptr;
    m_requestForKernels = nullptr;
    m_extraRequestForReflection = nullptr;
}

/* static */ SlangResult ShaderCompilerUtil::_compileProgramImpl(SlangSession* session, const Options& options, const Input& input, const ShaderCompileRequest& request, Output& out)
{
    out.reset();

    SlangCompileRequest* slangRequest = spCreateCompileRequest(session);
    out.m_requestForKernels = slangRequest;
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
    case SLANG_SOURCE_LANGUAGE_CUDA:
        spAddPreprocessorDefine(slangRequest, "__CUDA__", "1");
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

    const auto sourceLanguage = input.sourceLanguage;

    int translationUnitIndex = 0;
    {
        translationUnitIndex = spAddTranslationUnit(slangRequest, sourceLanguage, nullptr);
        spAddTranslationUnitSourceString(slangRequest, translationUnitIndex, request.source.path, request.source.dataBegin);
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

   Index explicitEntryPointCount = request.entryPoints.getCount();
   for(Index ee = 0; ee < explicitEntryPointCount; ++ee)
   {
        if(options.dontAddDefaultEntryPoints)
        {
            // If default entry points are not to be added, then
            // the `request.entryPoints` array should have been
            // left empty.
            //
            SLANG_ASSERT(false);
        }

       auto& entryPointInfo = request.entryPoints[ee];
       int entryPointIndex = spAddEntryPoint(
           slangRequest,
           translationUnitIndex,
           entryPointInfo.name,
           entryPointInfo.slangStage);
       SLANG_ASSERT(entryPointIndex == ee);

       setEntryPointSpecializationArgs(entryPointIndex);
   }

    spSetLineDirectiveMode(slangRequest, SLANG_LINE_DIRECTIVE_MODE_NONE);

    const SlangResult res = spCompile(slangRequest);

    if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        fprintf(stderr, "%s", diagnostics);
    }

    SLANG_RETURN_ON_FAIL(res);

    ComPtr<slang::IComponentType> linkedSlangProgram;

    List<ShaderCompileRequest::EntryPoint> actualEntryPoints;
    if(input.passThrough == SLANG_PASS_THROUGH_NONE)
    {
        // In the case where pass-through compilation is not being used,
        // we can use the Slang reflection information to discover what
        // the entry points were, and then use those to drive the
        // loading of code.
        //
        auto reflection = slang::ProgramLayout::get(slangRequest);
        SLANG_RETURN_ON_FAIL(spCompileRequest_getProgramWithEntryPoints(slangRequest, linkedSlangProgram.writeRef()));

        // Get the amount of entry points in reflection
        Index entryPointCount = Index(reflection->getEntryPointCount());

        // We must have at least one entry point (whether explicit or implicit)
        SLANG_ASSERT(entryPointCount);

        for(Index ee = 0; ee < entryPointCount; ++ee)
        {
            auto entryPoint = reflection->getEntryPointByIndex(ee);
            const char* entryPointName = entryPoint->getName();
            SLANG_ASSERT(entryPointName);

            auto slangStage = entryPoint->getStage();

            ShaderCompileRequest::EntryPoint entryPointInfo;
            entryPointInfo.name = entryPointName;
            entryPointInfo.slangStage = slangStage;

            actualEntryPoints.add(entryPointInfo);
        }
    }
    else
    {
        actualEntryPoints = request.entryPoints;
    }

    out.set(input.pipelineType, linkedSlangProgram);

    return SLANG_OK;
}

/* static */ SlangResult ShaderCompilerUtil::compileProgram(SlangSession* session, const Options& options, const Input& input, const ShaderCompileRequest& request, Output& out)
{
    if( input.passThrough == SLANG_PASS_THROUGH_NONE )
    {
        return _compileProgramImpl(session, options, input, request, out);
    }
    else
    {
        bool canUseSlangForPrecompile = false;
        switch (input.passThrough)
        {
        case SLANG_PASS_THROUGH_DXC:
        case SLANG_PASS_THROUGH_FXC:
            canUseSlangForPrecompile = true;
            break;
        default:
            break;
        }
        // If we are doing a HLSL pass-through compilation, then we can't rely
        // on the downstream compiler for the reflection information that
        // will drive all of our parameter binding. As such, we will first
        // compile with Slang to get reflection information, and then
        // compile in another pass using the desired downstream compiler
        // so that we can get the refleciton information we need.
        //
        Output slangOutput;
        if (canUseSlangForPrecompile)
        {
            ShaderCompilerUtil::Input slangInput = input;
            slangInput.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
            slangInput.passThrough = SLANG_PASS_THROUGH_NONE;
            // TODO: we want to pass along a flag to skip codegen...


            SLANG_RETURN_ON_FAIL(_compileProgramImpl(session, options, slangInput, request, slangOutput));
        }

        // Now we have what we need to be able to do the downstream compile better.
        //
        // TODO: We should be able to use the output from the Slang compilation
        // to fill in the actual entry points to be used for this compilation,
        // so that discovery of entry points via `[shader(...)]` attributes will work.
        //
        SLANG_RETURN_ON_FAIL(_compileProgramImpl(session, options, input, request, out));

        out.m_extraRequestForReflection = slangOutput.getRequestForReflection();
        out.desc.slangProgram = slangOutput.desc.slangProgram;
        slangOutput.m_requestForKernels = nullptr;

        return SLANG_OK;
    }
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

/* static */SlangResult ShaderCompilerUtil::compileWithLayout(SlangSession* session, const Options& options, const ShaderCompilerUtil::Input& input, OutputAndLayout& output)
{
    String sourcePath = options.sourcePath;
    auto& compileArgs = options.compileArgs;
    auto shaderType = options.shaderType;

    List<char> sourceText;
    SLANG_RETURN_ON_FAIL(readSource(sourcePath, sourceText));

    if (input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_C)
    {
        // Add an include of the prelude
        ComPtr<ISlangBlob> prelude;
        session->getLanguagePrelude(input.sourceLanguage, prelude.writeRef());

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
        case Options::ShaderProgramType::RayTracing:
            layout.numRenderTargets = 0;
            break;
    }

    // Deterministic random generator
    RefPtr<RandomGenerator> rand = RandomGenerator::create(0x34234);

    // Parse the layout
    layout.parse(rand, sourceText.getBuffer());

    // Setup SourceInfo
    ShaderCompileRequest::SourceInfo sourceInfo;
    sourceInfo.path = sourcePath.getBuffer();
    sourceInfo.dataBegin = sourceText.getBuffer();
    // Subtract 1 because it's zero terminated
    sourceInfo.dataEnd = sourceText.getBuffer() + sourceText.getCount() - 1;

    ShaderCompileRequest compileRequest;

    compileRequest.compileArgs = compileArgs;

    compileRequest.source = sourceInfo;

    // Now we will add the "default" entry point names/stages that
    // are appropriate to the pipeline type being targetted, *unless*
    // the options specify that we should leave out the default
    // entry points and instead rely on the Slang compiler's built-in
    // mechanisms for discovering entry points (e.g., `[shader(...)]`
    // attributes).
    //
    if( !options.dontAddDefaultEntryPoints )
    {
        if (shaderType == Options::ShaderProgramType::Graphics || shaderType == Options::ShaderProgramType::GraphicsCompute)
        {
            ShaderCompileRequest::EntryPoint vertexEntryPoint;
            vertexEntryPoint.name = vertexEntryPointName;
            vertexEntryPoint.slangStage = SLANG_STAGE_VERTEX;
            compileRequest.entryPoints.add(vertexEntryPoint);

            ShaderCompileRequest::EntryPoint fragmentEntryPoint;
            fragmentEntryPoint.name = fragmentEntryPointName;
            fragmentEntryPoint.slangStage = SLANG_STAGE_FRAGMENT;
            compileRequest.entryPoints.add(fragmentEntryPoint);
        }
        else if( shaderType == Options::ShaderProgramType::RayTracing )
        {
            // Note: Current GPU ray tracing pipelines allow for an
            // almost arbitrary mix of entry points for different stages
            // to be used together (e.g., a single "program" might
            // have multiple any-hit shaders, multiple miss shaders, etc.)
            //
            // Rather than try to define a fixed set of entry point
            // names and stages that the testing will support, we will
            // instead rely on `[shader(...)]` annotations to tell us
            // what entry points are present in the input code.
        }
        else
        {
            ShaderCompileRequest::EntryPoint computeEntryPoint;
            computeEntryPoint.name = computeEntryPointName;
            computeEntryPoint.slangStage = SLANG_STAGE_COMPUTE;
            compileRequest.entryPoints.add(computeEntryPoint);
        }
    }
    compileRequest.globalSpecializationArgs = layout.globalSpecializationArgs;
    compileRequest.entryPointSpecializationArgs = layout.entryPointSpecializationArgs;

    return ShaderCompilerUtil::compileProgram(session, options, input, compileRequest, output.output);
}

} // renderer_test
