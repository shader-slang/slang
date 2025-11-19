// options.cpp

#include "options.h"

#include "../../source/compiler-core/slang-command-line-args.h"
#include "../../source/core/slang-list.h"
#include "../../source/core/slang-render-api-util.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-type-text-util.h"
#include "../../source/core/slang-writer.h"
#include "diagnostics.h"

#include <stdio.h>
#include <stdlib.h>

namespace renderer_test
{
using namespace Slang;

// Helper function to check if a feature name is valid
static bool isValidFeatureName(
    const UnownedStringSlice& featureName,
    DiagnosticSink* sink,
    SourceLoc loc)
{
    // WAR: Accept cooperative-matrix-2 sub-features until RHI backend supports them
    // These features will be gracefully skipped at runtime if hardware doesn't support them
    if (featureName.startsWith("cooperative-matrix-"))
    {
        if (sink)
        {
            sink->diagnoseRaw(
                Severity::Warning,
                "Using cooperative-matrix-2 feature that is not yet fully supported "
                "in RHI backend. "
                "Test will be skipped if hardware doesn't support it.");
        }
        return true;
    }

#define SLANG_RHI_FEATURES_X(id, name) name,
    static const char* kValidFeatureNames[] = {SLANG_RHI_FEATURES(SLANG_RHI_FEATURES_X)};
#undef SLANG_RHI_FEATURES_X

    static const int kFeatureCount = sizeof(kValidFeatureNames) / sizeof(kValidFeatureNames[0]);

    for (int i = 0; i < kFeatureCount; i++)
    {
        if (featureName == UnownedStringSlice(kValidFeatureNames[i]))
        {
            return true;
        }
    }
    return false;
}

static rhi::DeviceType _toRenderType(Slang::RenderApiType apiType)
{
    using namespace Slang;
    switch (apiType)
    {
    case RenderApiType::D3D11:
        return rhi::DeviceType::D3D11;
    case RenderApiType::D3D12:
        return rhi::DeviceType::D3D12;
    case RenderApiType::Vulkan:
        return rhi::DeviceType::Vulkan;
    case RenderApiType::Metal:
        return rhi::DeviceType::Metal;
    case RenderApiType::CPU:
        return rhi::DeviceType::CPU;
    case RenderApiType::CUDA:
        return rhi::DeviceType::CUDA;
    case RenderApiType::WebGPU:
        return rhi::DeviceType::WGPU;
    default:
        return rhi::DeviceType::Default;
    }
}

/* static */ SlangResult Options::parse(
    int argc,
    const char* const* argv,
    Slang::WriterHelper stdError,
    Options& outOptions)
{
    using namespace Slang;

    RefPtr<CommandLineContext> cmdLineContext(new CommandLineContext);

    DiagnosticSink sink(cmdLineContext->getSourceManager(), nullptr);
    sink.writer = stdError.getWriter();
    sink.setFlag(DiagnosticSink::Flag::SourceLocationLine);

    outOptions = Options();

    CommandLineArgs args(cmdLineContext);

    if (argc > 0)
    {
        // first argument is the application name
        outOptions.appName = argv[0];
        args.setArgs(argv + 1, argc - 1);
    }
    else
    {
        args.setArgs(argv, argc);
    }
    SLANG_RETURN_ON_FAIL(outOptions.downstreamArgs.stripDownstreamArgs(args, 0, &sink));

    CommandLineReader reader(&args, &sink);

    List<CommandLineArg> positionalArgs;

    typedef Options::ShaderProgramType ShaderProgramType;
    typedef Options::InputLanguageID InputLanguageID;

    // now iterate over arguments to collect options
    while (reader.hasArg())
    {
        CommandLineArg arg = reader.getArgAndAdvance();
        const auto& argValue = arg.value;

        if (!argValue.startsWith("-"))
        {
            positionalArgs.add(arg);
            continue;
        }

        if (argValue == "--")
        {
            while (reader.hasArg())
            {
                positionalArgs.add(reader.getArgAndAdvance());
            }
            break;
        }
        else if (argValue == "-o")
        {
            SLANG_RETURN_ON_FAIL(reader.expectArg(outOptions.outputPath));
        }
        else if (argValue == "-profile")
        {
            SLANG_RETURN_ON_FAIL(reader.expectArg(outOptions.profileName));
        }
        else if (argValue == "-render-features" || argValue == "-render-feature")
        {
            CommandLineArg featuresArg;
            SLANG_RETURN_ON_FAIL(reader.expectArg(featuresArg));

            List<UnownedStringSlice> values;
            StringUtil::split(featuresArg.value.getUnownedSlice(), ',', values);

            for (const auto& value : values)
            {
                // Validate that the feature name is recognized
                if (!isValidFeatureName(value, &sink, featuresArg.loc))
                {
                    sink.diagnose(
                        featuresArg.loc,
                        RenderTestDiagnostics::invalidRenderFeature,
                        value);
                    return SLANG_FAIL;
                }
                outOptions.renderFeatures.add(value);
            }
        }
        else if (argValue == "-xslang" || argValue == "-compile-arg")
        {
            // This is legacy support, should use -Xslang now
            // This is an option that we want to pass along to Slang
            CommandLineArg slangArg;
            SLANG_RETURN_ON_FAIL(reader.expectArg(slangArg));
            outOptions.downstreamArgs.getArgsByName("slang").add(slangArg);
        }
        else if (argValue == "-compute")
        {
            outOptions.shaderType = ShaderProgramType::Compute;
        }
        else if (argValue == "-graphics")
        {
            outOptions.shaderType = ShaderProgramType::Graphics;
        }
        else if (argValue == "-gcompute")
        {
            outOptions.shaderType = ShaderProgramType::GraphicsCompute;
        }
        else if (argValue == "-rt")
        {
            outOptions.shaderType = ShaderProgramType::RayTracing;
        }
        else if (argValue == "-mesh")
        {
            outOptions.shaderType = ShaderProgramType::GraphicsMeshCompute;
        }
        else if (argValue == "-task")
        {
            outOptions.shaderType = ShaderProgramType::GraphicsTaskMeshCompute;
        }
        else if (argValue == "-use-dxbc")
        {
            outOptions.useDXBC = true;
        }
        else if (argValue == "-skip-spirv-validation")
        {
            outOptions.skipSPIRVValidation = true;
        }
        else if (argValue == "-emit-spirv-directly")
        {
            outOptions.generateSPIRVDirectly = true;
        }
        else if (argValue == "-emit-spirv-via-glsl")
        {
            outOptions.generateSPIRVDirectly = false;
        }
        else if (argValue == "-capability" || argValue == "-capabilities")
        {
            String capabilities;
            SLANG_RETURN_ON_FAIL(reader.expectArg(capabilities));

            List<UnownedStringSlice> values;
            StringUtil::split(capabilities.getUnownedSlice(), ',', values);

            for (const auto& value : values)
            {
                outOptions.capabilities.add(value);
            }
        }
        else if (argValue == "-only-startup")
        {
            outOptions.onlyStartup = true;
        }
        else if (argValue == "-performance-profile")
        {
            outOptions.performanceProfile = true;
        }
        else if (argValue == "-output-using-type")
        {
            outOptions.outputUsingType = true;
        }
        else if (argValue == "-compute-dispatch")
        {
            CommandLineArg dispatchSize;
            SLANG_RETURN_ON_FAIL(reader.expectArg(dispatchSize));

            List<UnownedStringSlice> slices;
            StringUtil::split(dispatchSize.value.getUnownedSlice(), ',', slices);
            if (slices.getCount() != 3)
            {
                sink.diagnose(
                    dispatchSize.loc,
                    RenderTestDiagnostics::expectingCommaComputeDispatch);
                return SLANG_FAIL;
            }

            String string;
            for (Index i = 0; i < 3; ++i)
            {
                string = slices[i];
                int v = stringToInt(string);
                if (v < 1)
                {
                    sink.diagnose(
                        dispatchSize.loc,
                        RenderTestDiagnostics::expectingPositiveComputeDispatch);
                    return SLANG_FAIL;
                }
                outOptions.computeDispatchSize[i] = v;
            }
        }
        else if (argValue == "-source-language")
        {
            CommandLineArg sourceLanguageName;
            SLANG_RETURN_ON_FAIL(reader.expectArg(sourceLanguageName));

            const SlangSourceLanguage sourceLanguage =
                TypeTextUtil::findSourceLanguage(sourceLanguageName.value.getUnownedSlice());
            if (sourceLanguage == SLANG_SOURCE_LANGUAGE_UNKNOWN)
            {
                sink.diagnose(sourceLanguageName.loc, RenderTestDiagnostics::unknownSourceLanguage);
                return SLANG_FAIL;
            }

            outOptions.sourceLanguage = sourceLanguage;
        }
        else if (argValue == "-no-default-entry-point")
        {
            outOptions.dontAddDefaultEntryPoints = true;
        }
        else if (argValue == "-nvapi-slot")
        {
            SLANG_RETURN_ON_FAIL(reader.expectArg(outOptions.nvapiExtnSlot));
        }
        else if (argValue == "-shaderobj")
        {
            // Note: We ignore this option because it is always enabled now.
            //
            // TODO: At some point we could warn/error and deprecate this option.
        }
        else if (argValue == "-g0")
        {
            outOptions.disableDebugInfo = true;
        }
        else if (argValue == "-allow-glsl")
        {
            outOptions.allowGLSL = true;
        }
        else if (argValue == "-entry")
        {
            SLANG_RETURN_ON_FAIL(reader.expectArg(outOptions.entryPointName));
        }
        else if (argValue == "-enable-debug-layers")
        {
            outOptions.enableDebugLayers = true;
        }
        else if (argValue == "-dx12-experimental")
        {
            outOptions.dx12Experimental = true;
        }
        else if (argValue == "-show-adapter-info")
        {
            outOptions.showAdapterInfo = true;
        }
        else if (argValue == "-ignore-abort-msg")
        {
#ifdef _MSC_VER
            _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
        }
        else if (argValue == "-cache-rhi-device")
        {
            outOptions.cacheRhiDevice = true;
        }
        else
        {
            // Lookup
            Slang::UnownedStringSlice argSlice = arg.value.getUnownedSlice();
            if (argSlice.getLength() && argSlice[0] == '-')
            {
                // Look up the rendering API if set
                UnownedStringSlice argName = argSlice.tail(1);
                DeviceType deviceType = _toRenderType(RenderApiUtil::findApiTypeByName(argName));

                if (deviceType != DeviceType::Default)
                {
                    outOptions.deviceType = deviceType;
                    continue;
                }

                // Lookup the target language type
                DeviceType targetLanguageDeviceType =
                    _toRenderType(RenderApiUtil::findImplicitLanguageRenderApiType(argName));

                if (targetLanguageDeviceType != DeviceType::Default || argName == "glsl")
                {
                    outOptions.targetLanguageDeviceType = targetLanguageDeviceType;
                    outOptions.inputLanguageID =
                        (argName == "hlsl" || argName == "glsl" || argName == "cpp" ||
                         argName == "cxx" || argName == "c")
                            ? InputLanguageID::Native
                            : InputLanguageID::Slang;
                    continue;
                }
            }
            sink.diagnose(arg.loc, RenderTestDiagnostics::unknownCommandLineOption, arg.value);
            return SLANG_FAIL;
        }
    }

    // If a render option isn't set use defaultRenderType
    outOptions.deviceType = (outOptions.deviceType == DeviceType::Default)
                                ? outOptions.targetLanguageDeviceType
                                : outOptions.deviceType;

    // first positional argument is source shader path
    if (positionalArgs.getCount())
    {
        outOptions.sourcePath = positionalArgs[0].value;
        positionalArgs.removeAt(0);
    }

    // any remaining arguments represent an error
    if (positionalArgs.getCount() != 0)
    {
        sink.diagnose(positionalArgs[0].loc, RenderTestDiagnostics::unexpectedPositionalArg);
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

} // namespace renderer_test
