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

    RendererType rendererType = RendererType::Unknown;
    InputLanguageID inputLanguageID = InputLanguageID::Slang;

    char const* slangArgs[kMaxSlangArgs];
    int slangArgCount = 0;

    bool useDXIL = false;
};

extern Options gOptions;

SlangResult parseOptions(int argc, const char*const* argv, Slang::WriterHelper stdError);

} // renderer_test
