// options.h
#pragma once

#include <stdint.h>

#ifndef SLANG_HANDLE_RESULT_FAIL
#define SLANG_HANDLE_RESULT_FAIL(x) assert(!"failure")
#endif

#include "../../slang-com-helper.h"
#include "../../source/core/slang-writer.h"

#include "../../source/core/slang-process-util.h"

#include "../../source/compiler-core/slang-command-line-args.h"

#include "../../slang-gfx.h"

namespace renderer_test {

using namespace gfx;

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

        /// The renderer type inferred from the target language type. Used if a rendererType is not explicitly set.
    DeviceType targetLanguageDeviceType = DeviceType::Unknown;
        /// The set render type
    DeviceType deviceType = DeviceType::Unknown;
    InputLanguageID inputLanguageID = InputLanguageID::Slang;
    SlangSourceLanguage sourceLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;

        /// Can be used for overriding the profile
    Slang::String profileName;

    bool outputUsingType = false;

    bool useDXIL = false;
    bool onlyStartup = false;

    bool performanceProfile = false;

    bool dontAddDefaultEntryPoints = false;

    Slang::List<Slang::String> renderFeatures;          /// Required render features for this test to run

    uint32_t computeDispatchSize[3] = { 1, 1, 1 };

    Slang::String nvapiExtnSlot;                               ///< The nvapiRegister to use.

    Slang::DownstreamArgs downstreamArgs;                    ///< Args to downstream tools. Here it's just slang

#if defined(SLANG_CONFIG_DEFAULT_SPIRV_DIRECT)
    bool generateSPIRVDirectly = true;
#else
    bool generateSPIRVDirectly = false;
#endif

    Options() { downstreamArgs.addName("slang"); }

    static SlangResult parse(int argc, const char*const* argv, Slang::WriterHelper stdError, Options& outOptions);
};

} // renderer_test
