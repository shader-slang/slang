// options.cpp

#include "options.h"

#include "compiler-core/slang-command-line-args.h"
#include "core/slang-list.h"
#include "core/slang-render-api-util.h"
#include "core/slang-string-util.h"
#include "core/slang-type-text-util.h"
#include "core/slang-writer.h"
#include "diagnostics.h"

#include <stdio.h>
#include <stdlib.h>

namespace renderer_test
{
using namespace Slang;

// Returns whether `featureName` is a recognized -render-feature name (an entry in
// SLANG_RHI_FEATURES), not whether the current device supports it. A recognized-but-
// unsupported feature is not rejected here: render-test-main.cpp returns
// SLANG_E_NOT_AVAILABLE at device creation and the harness skips the test.
static bool isValidFeatureName(const UnownedStringSlice& featureName)
{
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

// Returns the CooperativeMatrixComponentType named by `typeName`, using the lowercase
// spellings a shader writes for the component types ("half", "float", "bfloat16", ...).
// Returns false when the token names no component type, so a malformed
// -require-coopmat-shape value is rejected as a test-configuration error rather than
// silently gating the test on a default type.
static bool _getCooperativeMatrixComponentTypeFromName(
    const UnownedStringSlice& typeName,
    CooperativeMatrixComponentType& outType)
{
    struct Entry
    {
        const char* name;
        CooperativeMatrixComponentType type;
    };
    static const Entry kEntries[] = {
        {"half", CooperativeMatrixComponentType::Float16},
        {"float", CooperativeMatrixComponentType::Float32},
        {"bfloat16", CooperativeMatrixComponentType::Bfloat16},
        {"e4m3", CooperativeMatrixComponentType::FloatE4M3},
        {"e5m2", CooperativeMatrixComponentType::FloatE5M2},
        {"int8", CooperativeMatrixComponentType::Int8},
        {"uint8", CooperativeMatrixComponentType::Uint8},
        {"int16", CooperativeMatrixComponentType::Int16},
        {"uint16", CooperativeMatrixComponentType::Uint16},
        {"int32", CooperativeMatrixComponentType::Int32},
        {"uint32", CooperativeMatrixComponentType::Uint32},
    };

    for (const auto& entry : kEntries)
    {
        if (typeName == UnownedStringSlice(entry.name))
        {
            outType = entry.type;
            return true;
        }
    }
    return false;
}

// Returns the CooperativeMatrixScope named by `scopeName` ("subgroup" or "workgroup"), or
// false when the token names no scope.
static bool _getCooperativeMatrixScopeFromName(
    const UnownedStringSlice& scopeName,
    CooperativeMatrixScope& outScope)
{
    if (scopeName == "subgroup")
    {
        outScope = CooperativeMatrixScope::Subgroup;
        return true;
    }
    if (scopeName == "workgroup")
    {
        outScope = CooperativeMatrixScope::Workgroup;
        return true;
    }
    return false;
}

// Parses a -require-coopmat-shape value of the form "<type>:<scope>:<m>:<n>:<k>", for example
// "half:subgroup:8:8:16", into a CooperativeMatrixDesc. The shape is a matmul shape because
// the device query matches it against the cooperative-matrix property entries the device
// lists, and each entry describes a full matmul A (m x k) * B (k x n) = C rather than a
// standalone accumulator; a test whose shader only loads and stores an accumulator therefore
// picks a k that the device lists alongside the shape's other dimensions (16 for the
// half-precision entries current NVIDIA devices list). Diagnoses and returns SLANG_FAIL on a
// malformed value so a typo cannot silently change which devices run the test.
static SlangResult _parseCooperativeMatrixShape(
    const CommandLineArg& shapeArg,
    DiagnosticSink& sink,
    CooperativeMatrixDesc& outDesc)
{
    List<UnownedStringSlice> tokens;
    StringUtil::split(shapeArg.value.getUnownedSlice(), ':', tokens);
    if (tokens.getCount() != 5)
    {
        sink.diagnose(
            shapeArg.loc,
            RenderTestDiagnostics::invalidCooperativeMatrixShape,
            shapeArg.value);
        return SLANG_FAIL;
    }

    CooperativeMatrixDesc desc = {};
    if (!_getCooperativeMatrixComponentTypeFromName(tokens[0], desc.aType))
    {
        sink.diagnose(
            shapeArg.loc,
            RenderTestDiagnostics::invalidCooperativeMatrixShape,
            shapeArg.value);
        return SLANG_FAIL;
    }
    // The type token fills all four component-type slots. Devices list one property entry per
    // component-type combination, and the accumulator shapes a shader declares are validated
    // against whole entries, so the common all-same-type case only needs the one token.
    desc.bType = desc.cType = desc.resultType = desc.aType;

    if (!_getCooperativeMatrixScopeFromName(tokens[1], desc.scope))
    {
        sink.diagnose(
            shapeArg.loc,
            RenderTestDiagnostics::invalidCooperativeMatrixShape,
            shapeArg.value);
        return SLANG_FAIL;
    }

    Int dims[3];
    for (Index i = 0; i < 3; ++i)
    {
        if (SLANG_FAILED(StringUtil::parseInt(tokens[i + 2], dims[i])) || dims[i] < 1)
        {
            sink.diagnose(
                shapeArg.loc,
                RenderTestDiagnostics::invalidCooperativeMatrixShape,
                shapeArg.value);
            return SLANG_FAIL;
        }
    }
    desc.m = uint32_t(dims[0]);
    desc.n = uint32_t(dims[1]);
    desc.k = uint32_t(dims[2]);

    outDesc = desc;
    return SLANG_OK;
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
    case RenderApiType::LLVM:
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

    // Accept every `-X<compiler>` name that slangc accepts (dxc, fxc, glslang, nvrtc, ...),
    // not just `-Xslang`. The default Options() ctor registers only "slang", so `-Xdxc ...`
    // would be rejected as an unknown downstream name. Constructing DownstreamArgs with the
    // command-line context auto-registers all pass-through names (as slangc does in
    // slang-options.cpp); "slang" is not a pass-through enum value, so re-add it explicitly.
    outOptions.downstreamArgs = DownstreamArgs(cmdLineContext);
    outOptions.downstreamArgs.addName("slang");

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
                if (!isValidFeatureName(value))
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
        else if (argValue == "-require-coopmat-shape")
        {
            CommandLineArg shapeArg;
            SLANG_RETURN_ON_FAIL(reader.expectArg(shapeArg));

            CooperativeMatrixDesc desc;
            SLANG_RETURN_ON_FAIL(_parseCooperativeMatrixShape(shapeArg, sink, desc));
            outOptions.requiredCooperativeMatrixShapes.add(desc);
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
        else if (argValue == "-cache-rhi-device")
        {
            outOptions.cacheRhiDevice = true;
        }
        else if (argValue == "-compile-only")
        {
            outOptions.compileOnly = true;
        }
        else
        {
            // Lookup
            Slang::UnownedStringSlice argSlice = arg.value.getUnownedSlice();
            if (argSlice.getLength() && argSlice[0] == '-')
            {
                // Look up the rendering API if set
                UnownedStringSlice argName = argSlice.tail(1);
                RenderApiType renderApi = RenderApiUtil::findApiTypeByName(argName);
                DeviceType deviceType = _toRenderType(renderApi);

                if (deviceType != DeviceType::Default)
                {
                    outOptions.deviceType = deviceType;
                    if (renderApi == RenderApiType::LLVM)
                        outOptions.useLLVMDirectly = true;
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
