// options.h
#pragma once

#include <stdint.h>

#include "../../source/core/slang-result.h"

namespace renderer_test {

typedef intptr_t Int;
typedef uintptr_t UInt;

enum class RendererID
{
    NONE,          
    D3D11,
    D3D12,
    GL,
    VK,
};

enum class InputLanguageID
{
    // Slang being used as an HLSL-ish compiler
    Slang,

    // Raw HLSL or GLSL input, bypassing Slang
    Native,

    // Raw HLSL or GLSL input, passed through the Slang rewriter
    NativeRewrite
};

enum
{
    // maximum number of command-line arguments to pass along to slang
    kMaxSlangArgs = 16,
};

enum class ShaderProgramType
{
	Graphics,
	Compute,
    GraphicsCompute
};

struct Options
{
    char const* appName = "render-test";
    char const* sourcePath = nullptr;
    char const* outputPath = nullptr;
	ShaderProgramType shaderType = ShaderProgramType::Graphics;

    RendererID rendererID = RendererID::NONE;
    InputLanguageID inputLanguageID = InputLanguageID::Slang;

    char const* slangArgs[kMaxSlangArgs];
    int slangArgCount = 0;
};

extern Options gOptions;

extern int gWindowWidth;
extern int gWindowHeight;


SlangResult parseOptions(int* argc, char** argv);

} // renderer_test
