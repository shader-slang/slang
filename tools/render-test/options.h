// options.h
#pragma once

#include <stdint.h>

#ifndef SLANG_HANDLE_RESULT_FAIL
#define SLANG_HANDLE_RESULT_FAIL(x) SLANG_ASSERT(!"failure")
#endif

#include "compiler-core/slang-command-line-args.h"
#include "core/slang-process-util.h"
#include "core/slang-writer.h"
#include "slang-com-helper.h"

#include <slang-rhi.h>

namespace renderer_test
{

using namespace rhi;

struct Options
{
    enum class InputLanguageID
    {
        // Slang being used as an HLSL-ish compiler
        Slang,

        // Raw HLSL or GLSL input, bypassing Slang
        Native,
    };

    enum class ShaderProgramType
    {
        // Vertex and Fragment shader, writing an image out
        Graphics,
        // Compute shader, writing buffer contents out
        Compute,
        // Vertex and Fragment shader, writing buffer contents out
        GraphicsCompute,
        // Ray tracing shaders, writing buffer contents out
        RayTracing,
        // Mesh and Fragment shader, writing buffer contents out
        GraphicsMeshCompute,
        // Task, Mesh and Fragment shader, writing buffer contents out
        GraphicsTaskMeshCompute,
    };

    Slang::String appName = "render-test";
    Slang::String sourcePath;
    Slang::String outputPath;
    ShaderProgramType shaderType = ShaderProgramType::Graphics;

    /// The renderer type inferred from the target language type. Used if a rendererType is not
    /// explicitly set.
    DeviceType targetLanguageDeviceType = DeviceType::Default;
    /// The set render type
    DeviceType deviceType = DeviceType::Default;
    InputLanguageID inputLanguageID = InputLanguageID::Slang;
    SlangSourceLanguage sourceLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;

    /// Can be used for overriding the profile
    Slang::String profileName;

    bool outputUsingType = false;

    bool useDXBC = false;

    bool onlyStartup = false;

    bool performanceProfile = false;

    bool dontAddDefaultEntryPoints = false;

    bool disableDebugInfo = false;

    bool allowGLSL = false;

    Slang::String entryPointName;

    Slang::List<Slang::String> renderFeatures; /// Required render features for this test to run

    uint32_t computeDispatchSize[3] = {1, 1, 1};

    Slang::String nvapiExtnSlot; ///< The nvapiRegister to use.

    Slang::DownstreamArgs downstreamArgs; ///< Args to downstream tools. Here it's just slang

    bool generateSPIRVDirectly = true;

    bool enableDebugLayers = false;

    bool dx12Experimental = false;

    bool showAdapterInfo = false;

    bool skipSPIRVValidation = false;

    // Whether to enable RHI device caching (default: false in render-test)
    bool cacheRhiDevice = false;

    bool useLLVMDirectly = false;

    bool compileOnly = false;

    Slang::List<Slang::String> capabilities;

    Options() { downstreamArgs.addName("slang"); }

    static SlangResult parse(
        int argc,
        const char* const* argv,
        Slang::WriterHelper stdError,
        Options& outOptions);
};

/// Return the `rhi::Feature` a `-render-feature` name refers to, or `rhi::Feature::_Count` if the
/// name is not recognized.
///
/// This is the single place that maps a test's feature name onto an RHI feature, so that option
/// parsing (which rejects unknown names) and the runtime requirement check (which decides whether
/// to skip a test) can never disagree. Besides the names generated from `SLANG_RHI_FEATURES`, it
/// resolves the individual `VK_NV_cooperative_matrix2` sub-feature names used by tests -- slang-rhi
/// exposes that extension only as the single `cooperative-matrix-2` feature, so each sub-feature
/// name maps onto it.
rhi::Feature getRenderFeatureFromName(const Slang::UnownedStringSlice& featureName);

} // namespace renderer_test
