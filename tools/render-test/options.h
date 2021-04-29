// options.h
#pragma once

#include <stdint.h>

#ifndef SLANG_HANDLE_RESULT_FAIL
#define SLANG_HANDLE_RESULT_FAIL(x) assert(!"failure")
#endif

#include "../../slang-com-helper.h"
#include "../../source/core/slang-writer.h"

#include "../../source/core/slang-process-util.h"

#include "../../slang-gfx.h"

namespace renderer_test {

using namespace gfx;

struct Options
{
    enum
    {
        // maximum number of command-line arguments to pass along to slang
        kMaxSlangArgs = 16,
    };

    enum class InputLanguageID
    {
        // Slang being used as an HLSL-ish compiler
        Slang,

        // Raw HLSL or GLSL input, bypassing Slang
        Native,
    };


    enum class ShaderProgramType
    {
        Graphics,
        Compute,
        GraphicsCompute,
        RayTracing,
    };

    char const* appName = "render-test";
    char const* sourcePath = nullptr;
    char const* outputPath = nullptr;
	ShaderProgramType shaderType = ShaderProgramType::Graphics;

        /// The renderer type inferred from the target language type. Used if a rendererType is not explicitly set.
    DeviceType targetLanguageDeviceType = DeviceType::Unknown;
        /// The set render type
    DeviceType deviceType = DeviceType::Unknown;
    InputLanguageID inputLanguageID = InputLanguageID::Slang;
    SlangSourceLanguage sourceLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;

        /// Can be used for overriding the profile
    const char* profileName = nullptr;

    char const* slangArgs[kMaxSlangArgs];
    int slangArgCount = 0;

    bool outputUsingType = false;

    bool useDXIL = false;
    bool onlyStartup = false;

    bool performanceProfile = false;

    bool dontAddDefaultEntryPoints = false;

    Slang::List<Slang::String> renderFeatures;          /// Required render features for this test to run

    Slang::List<Slang::String> compileArgs;

    Slang::String adapter;                              ///< The adapter to use either name or index

    uint32_t computeDispatchSize[3] = { 1, 1, 1 };

    Slang::String nvapiExtnSlot;                               ///< The nvapiRegister to use.

    static SlangResult parse(int argc, const char*const* argv, Slang::WriterHelper stdError, Options& outOptions);
};

} // renderer_test
