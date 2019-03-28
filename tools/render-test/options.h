// options.h
#pragma once

#include <stdint.h>

#include "../../slang-com-helper.h"
#include "../../source/core/slang-writer.h"

#include "render.h"

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
        GraphicsCompute
    };

    char const* appName = "render-test";
    char const* sourcePath = nullptr;
    char const* outputPath = nullptr;
	ShaderProgramType shaderType = ShaderProgramType::Graphics;

        /// The renderer type inferred from the target language type. Used if a rendererType is not explicitly set.
    RendererType targetLanguageRendererType = RendererType::Unknown;
        /// The set render type
    RendererType rendererType = RendererType::Unknown;
    InputLanguageID inputLanguageID = InputLanguageID::Slang;

        /// Can be used for overriding the profile
    const char* profileName = nullptr;

    char const* slangArgs[kMaxSlangArgs];
    int slangArgCount = 0;

    bool useDXIL = false;
    bool onlyStartup = false;

    Slang::List<Slang::String> renderFeatures;          /// Required render features for this test to run

    Slang::String adapter;                              ///< The adapter to use either name or index
};

extern Options gOptions;

SlangResult parseOptions(int argc, const char*const* argv, Slang::WriterHelper stdError);

} // renderer_test
