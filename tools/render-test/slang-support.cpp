// slang-support.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "slang-support.h"

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-test-tool-util.h"
#include "options.h"

#include <assert.h>
#include <stdio.h>

namespace renderer_test
{
using namespace Slang;

// Entry point name to use for vertex/fragment shader
static const char vertexEntryPointName[] = "vertexMain";
static const char fragmentEntryPointName[] = "fragmentMain";
static const char computeEntryPointName[] = "computeMain";
static const char rtEntryPointName[] = "raygenMain";
static const char taskEntryPointName[] = "taskMain";
static const char meshEntryPointName[] = "meshMain";

void ShaderCompilerUtil::Output::set(slang::IComponentType* inSlangProgram)
{
    slangProgram = inSlangProgram;
    desc.slangGlobalScope = inSlangProgram;
}

void ShaderCompilerUtil::Output::reset()
{
    {
        desc.slangGlobalScope = nullptr;
    }

    globalSession = nullptr;
    m_session = nullptr;
}

static SlangResult _compileProgramImpl(
    slang::IGlobalSession* globalSession,
    const Options& options,
    const ShaderCompilerUtil::Input& input,
    const ShaderCompileRequest& request,
    ShaderCompilerUtil::Output& out)
{
    out.reset();

    List<const char*> args;
    bool hasRepro = false;
    for (const auto& arg : options.downstreamArgs.getArgsByName("slang"))
    {
        args.add(arg.value.getBuffer());
        if (arg.value == "-load-repro")
            hasRepro = true;
    }

    List<slang::CompilerOptionEntry> sessionOptionEntries;
    slang::TargetDesc sessionTargetDesc = {};
    slang::SessionDesc sessionDesc = {};
    ComPtr<ISlangUnknown> sessionDescMemory;
    // If there are additional args parse them
    if (args.getCount())
    {
        const auto res = globalSession->parseCommandLineArguments(
            int(args.getCount()),
            args.getBuffer(),
            &sessionDesc,
            sessionDescMemory.writeRef());
        // If there is a parse failure and diagnostic, output it
        if (SLANG_FAILED(res))
        {
            fprintf(stderr, "error: Failed to parse command line arguments: %d\n", int(res));
            return res;
        }
        // We're setting the targets ourselves, below.
        // To simplify that, we're currently not expecting targets to be added by the command line
        // arguments.
        if (!hasRepro && (sessionDesc.targetCount > 0))
        {
            fprintf(stderr, "error: Command line arguments added targets.\n");
        }
    }
    List<slang::PreprocessorMacroDesc> macros;
    // Only proceed if the command line arguments are not loading a repro.
    if (!hasRepro)
    {
        // Define a macro so that shader code in a test can detect what language we
        // are nominally working with.
        char const* langDefine = nullptr;
        switch (input.sourceLanguage)
        {
        case SLANG_SOURCE_LANGUAGE_GLSL:
            macros.add({"__GLSL__", "1"});
            break;

        case SLANG_SOURCE_LANGUAGE_SLANG:
            macros.add({"__SLANG__", "1"});
            // fall through
        case SLANG_SOURCE_LANGUAGE_HLSL:
            macros.add({"__HLSL__", "1"});
            break;
        case SLANG_SOURCE_LANGUAGE_C:
            macros.add({"__C__", "1"});
            break;
        case SLANG_SOURCE_LANGUAGE_CPP:
            macros.add({"__CPP__", "1"});
            break;
        case SLANG_SOURCE_LANGUAGE_CUDA:
            macros.add({"__CUDA__", "1"});
            break;
        case SLANG_SOURCE_LANGUAGE_WGSL:
            macros.add({"__WGSL__", "1"});
            break;

        default:
            assert(!"unexpected");
            break;
        }

        {
            slang::CompilerOptionEntry entry;
            entry.name = slang::CompilerOptionName::AllowGLSL;
            entry.value.kind = slang::CompilerOptionValueKind::Int;
            entry.value.intValue0 = int(options.allowGLSL);
            sessionOptionEntries.add(entry);
        }

        {
            slang::CompilerOptionEntry entry;
            entry.name = slang::CompilerOptionName::PassThrough;
            entry.value.kind = slang::CompilerOptionValueKind::Int;
            entry.value.intValue0 = int(input.passThrough);
            sessionOptionEntries.add(entry);
        }

        {
            slang::CompilerOptionEntry entry;
            entry.name = slang::CompilerOptionName::LineDirectiveMode;
            entry.value.kind = slang::CompilerOptionValueKind::Int;
            entry.value.intValue0 = int(SlangLineDirectiveMode::SLANG_LINE_DIRECTIVE_MODE_NONE);
            sessionOptionEntries.add(entry);
        }

        sessionTargetDesc.format = input.target;
        if (input.profile.getLength()) // do not set profile unless requested
            sessionTargetDesc.profile = globalSession->findProfile(input.profile.getBuffer());
        if (options.generateSPIRVDirectly)
            sessionTargetDesc.flags |= SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
        else
            sessionTargetDesc.flags = 0;
    }

    sessionDesc.compilerOptionEntryCount = sessionOptionEntries.getCount();
    sessionDesc.compilerOptionEntries = sessionOptionEntries.getBuffer();

    SLANG_ASSERT(sessionDesc.targetCount == 0);
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &sessionTargetDesc;

    sessionDesc.preprocessorMacroCount = (SlangInt)macros.getCount();
    sessionDesc.preprocessorMacros = macros.getBuffer();

    if (options.generateSPIRVDirectly)
    {
        slang::CompilerOptionEntry entry;
        entry.name = slang::CompilerOptionName::DebugInformation;
        entry.value.kind = slang::CompilerOptionValueKind::Int;
        entry.value.intValue0 =
            int(options.disableDebugInfo ? SlangDebugInfoLevel::SLANG_DEBUG_INFO_LEVEL_NONE
                                         : SlangDebugInfoLevel::SLANG_DEBUG_INFO_LEVEL_STANDARD);
        sessionOptionEntries.add(entry);
    }

    ComPtr<slang::ISession> slangSession = nullptr;
    globalSession->createSession(sessionDesc, slangSession.writeRef());
    out.m_session = slangSession;
    out.globalSession = globalSession;

    String source(request.source.dataBegin, request.source.dataEnd);
    ComPtr<slang::IBlob> diagnostics;
    auto module = slangSession->loadModuleFromSourceString(
        "main",
        "main.slang",
        source.getBuffer(),
        diagnostics.writeRef());
    if (!module)
    {
        fprintf(stderr, "error: Failed to load module: %s", (char*)diagnostics->getBufferPointer());
        return SLANG_FAIL;
    }

    ComPtr<slang::IModule> specializedModule;
    List<ComPtr<slang::IEntryPoint>> specializedEntryPoints;
    if (!hasRepro)
    {
        const int globalSpecializationArgCount = int(request.globalSpecializationArgs.getCount());
        if (globalSpecializationArgCount != module->getSpecializationParamCount())
        {
            fprintf(
                stderr,
                "error: The specialization argument count of the request does not match that of "
                "the module!\n");
            return SLANG_FAIL;
        }
        List<slang::SpecializationArg> moduleSpecializationArgs;
        for (int ii = 0; ii < globalSpecializationArgCount; ++ii)
        {
            String specializedTypeName = request.globalSpecializationArgs[ii].getBuffer();
            slang::TypeReflection* typeReflection =
                module->getLayout()->findTypeByName(specializedTypeName.getBuffer());
            moduleSpecializationArgs.add(slang::SpecializationArg::fromType(typeReflection));
        }

        {
            ComPtr<slang::IBlob> diagnostics;
            auto res = module->specialize(
                moduleSpecializationArgs.getBuffer(),
                moduleSpecializationArgs.getCount(),
                (slang::IComponentType**)specializedModule.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(res))
            {
                fprintf(
                    stderr,
                    "error: Failed to specialize module: %s\n",
                    (char*)diagnostics->getBufferPointer());
                return res;
            }
        }

        Index explicitEntryPointCount = request.entryPoints.getCount();
        for (Index ee = 0; ee < explicitEntryPointCount; ++ee)
        {
            if (options.dontAddDefaultEntryPoints)
            {
                // If default entry points are not to be added, then
                // the `request.entryPoints` array should have been
                // left empty.
                //
                SLANG_ASSERT(false);
            }

            auto& entryPointInfo = request.entryPoints[ee];

            ComPtr<slang::IEntryPoint> entryPoint;
            ComPtr<slang::IBlob> diagnostics;
            auto res = module->findAndCheckEntryPoint(
                entryPointInfo.name,
                entryPointInfo.slangStage,
                entryPoint.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(res))
            {
                fprintf(
                    stderr,
                    "error: Failed to find entry point '%s': %s\n",
                    entryPointInfo.name,
                    (char*)diagnostics->getBufferPointer());
                return res;
            }

            const int entryPointSpecializationArgCount =
                int(request.entryPointSpecializationArgs.getCount());
            if (entryPointSpecializationArgCount != entryPoint->getSpecializationParamCount())
            {
                fprintf(
                    stderr,
                    "error: %s\n",
                    "The specialization argument count of the request does not match that of the "
                    "entry point!");
                return SLANG_FAIL;
            }


            List<slang::SpecializationArg> entryPointSpecializationArgs;
            for (int ii = 0; ii < entryPointSpecializationArgCount; ++ii)
            {
                String specializedTypeName = request.entryPointSpecializationArgs[ii].getBuffer();
                slang::TypeReflection* typeReflection =
                    module->getLayout()->findTypeByName(specializedTypeName.getBuffer());
                entryPointSpecializationArgs.add(
                    slang::SpecializationArg::fromType(typeReflection));
            }

            ComPtr<slang::IEntryPoint> specializedEntryPoint;
            {
                ComPtr<slang::IBlob> diagnostics;
                auto res = entryPoint->specialize(
                    entryPointSpecializationArgs.getBuffer(),
                    entryPointSpecializationArgs.getCount(),
                    (slang::IComponentType**)specializedEntryPoint.writeRef(),
                    diagnostics.writeRef());
                if (SLANG_FAILED(res))
                {
                    fprintf(
                        stderr,
                        "error: Failed to specialize entry point: %s\n",
                        (char*)diagnostics->getBufferPointer());
                    return res;
                }
            }
            specializedEntryPoints.add(specializedEntryPoint);
        }
    }

    ComPtr<slang::IComponentType> linkedSlangProgram;

    List<slang::IComponentType*> componentsRawPtr;
    if (input.passThrough == SLANG_PASS_THROUGH_NONE)
    {
        componentsRawPtr.add(specializedModule);
        for (auto& specializedEntryPoint : specializedEntryPoints)
            componentsRawPtr.add(specializedEntryPoint);
    }

    if (request.typeConformances.getCount())
    {
        List<ComPtr<slang::ITypeConformance>> typeConformanceComponents;
        componentsRawPtr.add(linkedSlangProgram.get());
        auto reflection = module->getLayout();
        ComPtr<ISlangBlob> outDiagnostic;
        for (auto& conformance : request.typeConformances)
        {
            auto derivedType = reflection->findTypeByName(conformance.derivedTypeName.getBuffer());
            auto baseType = reflection->findTypeByName(conformance.baseTypeName.getBuffer());
            ComPtr<slang::ITypeConformance> conformanceComponentType;
            SlangResult res = slangSession->createTypeConformanceComponentType(
                derivedType,
                baseType,
                conformanceComponentType.writeRef(),
                conformance.idOverride,
                outDiagnostic.writeRef());
            if (SLANG_FAILED(res))
            {
                fprintf(stderr, "error: Failed to handle type conformances\n");
                return res;
            }
            typeConformanceComponents.add(conformanceComponentType);
            componentsRawPtr.add(conformanceComponentType);
        }
    }

    if (componentsRawPtr.getCount() > 0)
    {
        ComPtr<slang::IComponentType> newProgram;
        ComPtr<ISlangBlob> outDiagnostic;
        SlangResult res = slangSession->createCompositeComponentType(
            componentsRawPtr.getBuffer(),
            componentsRawPtr.getCount(),
            newProgram.writeRef(),
            outDiagnostic.writeRef());
        if (SLANG_FAILED(res))
        {
            fprintf(stderr, "error: Failed to create linked program\n");
            return res;
        }
        linkedSlangProgram = newProgram;
    }

    out.set(linkedSlangProgram);
    return SLANG_OK;
}

static SlangResult compileProgram(
    slang::IGlobalSession* globalSession,
    const Options& options,
    const ShaderCompilerUtil::Input& input,
    const ShaderCompileRequest& request,
    ShaderCompilerUtil::Output& out)
{
    if (input.passThrough == SLANG_PASS_THROUGH_NONE)
    {
        return _compileProgramImpl(globalSession, options, input, request, out);
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
        ShaderCompilerUtil::Output slangOutput;
        if (canUseSlangForPrecompile)
        {
            ShaderCompilerUtil::Input slangInput = input;
            slangInput.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
            slangInput.passThrough = SLANG_PASS_THROUGH_NONE;
            // TODO: we want to pass along a flag to skip codegen...


            SLANG_RETURN_ON_FAIL(
                _compileProgramImpl(globalSession, options, slangInput, request, slangOutput));
        }

        // Now we have what we need to be able to do the downstream compile better.
        //
        // TODO: We should be able to use the output from the Slang compilation
        // to fill in the actual entry points to be used for this compilation,
        // so that discovery of entry points via `[shader(...)]` attributes will work.
        //
        SLANG_RETURN_ON_FAIL(_compileProgramImpl(globalSession, options, input, request, out));

        out.m_session = slangOutput.m_session;
        out.desc.slangGlobalScope = slangOutput.desc.slangGlobalScope;
        slangOutput.m_session = nullptr;
        return SLANG_OK;
    }
}

// Helper for compileWithLayout
/* static */ SlangResult readSource(const String& inSourcePath, List<char>& outSourceText)
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
    if (fread(outSourceText.getBuffer(), sourceSize, 1, sourceFile) != 1)
    {
        fprintf(stderr, "error: failed to read from '%s'\n", inSourcePath.getBuffer());
        return SLANG_FAIL;
    }
    fclose(sourceFile);
    outSourceText[sourceSize] = 0;

    return SLANG_OK;
}

/* static */ SlangResult ShaderCompilerUtil::compileWithLayout(
    slang::IGlobalSession* globalSession,
    const Options& options,
    const Input& input,
    ShaderCompilerUtil::OutputAndLayout& output)
{
    String sourcePath = options.sourcePath;
    auto shaderType = options.shaderType;

    List<char> sourceText;
    SLANG_RETURN_ON_FAIL(readSource(sourcePath, sourceText));

    if (input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP ||
        input.sourceLanguage == SLANG_SOURCE_LANGUAGE_C)
    {
        // Add an include of the prelude
        ComPtr<ISlangBlob> prelude;
        globalSession->getLanguagePrelude(input.sourceLanguage, prelude.writeRef());

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

    compileRequest.source = sourceInfo;

    // Now we will add the "default" entry point names/stages that
    // are appropriate to the pipeline type being targetted, *unless*
    // the options specify that we should leave out the default
    // entry points and instead rely on the Slang compiler's built-in
    // mechanisms for discovering entry points (e.g., `[shader(...)]`
    // attributes).
    //
    if (!options.dontAddDefaultEntryPoints)
    {
        switch (shaderType)
        {
        case Options::ShaderProgramType::Graphics:
        case Options::ShaderProgramType::GraphicsCompute:
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
            break;
        case Options::ShaderProgramType::GraphicsTaskMeshCompute:
            {
                ShaderCompileRequest::EntryPoint taskEntryPoint;
                taskEntryPoint.name = taskEntryPointName;
                taskEntryPoint.slangStage = SLANG_STAGE_AMPLIFICATION;
                compileRequest.entryPoints.add(taskEntryPoint);
            }
            [[fallthrough]];
        case Options::ShaderProgramType::GraphicsMeshCompute:
            {
                ShaderCompileRequest::EntryPoint meshEntryPoint;
                meshEntryPoint.name = meshEntryPointName;
                meshEntryPoint.slangStage = SLANG_STAGE_MESH;
                compileRequest.entryPoints.add(meshEntryPoint);

                ShaderCompileRequest::EntryPoint fragmentEntryPoint;
                fragmentEntryPoint.name = fragmentEntryPointName;
                fragmentEntryPoint.slangStage = SLANG_STAGE_FRAGMENT;
                compileRequest.entryPoints.add(fragmentEntryPoint);
            }
            break;
        case Options::ShaderProgramType::RayTracing:
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
            break;
        default:
            {
                ShaderCompileRequest::EntryPoint computeEntryPoint;
                computeEntryPoint.name = computeEntryPointName;
                computeEntryPoint.slangStage = SLANG_STAGE_COMPUTE;
                compileRequest.entryPoints.add(computeEntryPoint);
            }
        }
    }
    compileRequest.globalSpecializationArgs = layout.globalSpecializationArgs;
    compileRequest.entryPointSpecializationArgs = layout.entryPointSpecializationArgs;
    for (auto conformance : layout.typeConformances)
    {
        ShaderCompileRequest::TypeConformance c;
        c.derivedTypeName = conformance.derivedTypeName;
        c.baseTypeName = conformance.baseTypeName;
        c.idOverride = conformance.idOverride;
        compileRequest.typeConformances.add(c);
    }
    return compileProgram(globalSession, options, input, compileRequest, output.output);
}

} // namespace renderer_test
