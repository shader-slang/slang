// options.h
#pragma once

#include <stdint.h>

namespace renderer_test {

typedef intptr_t Int;
typedef uintptr_t UInt;

enum class Mode
{
    // Slang being used as an HLSL-ish compiler
    Slang,

    // Raw HLSL or GLSL input, bypassing Slang
    HLSL,
    GLSL,

    // Raw HLSL or GLSL input, passed through the Slang rewriter
    HLSLRewrite,
    GLSLRewrite,

    // Slang/HLSL input -> GLSL output
    GLSLCrossCompile,
};

enum
{
    // maximum number of command-line arguments to pass along to slang
    kMaxSlangArgs = 16,
};

struct Options
{
    char const* appName = "render-test";
    char const* sourcePath = nullptr;
    char const* outputPath = nullptr;
    Mode mode = Mode::Slang;

    char const* slangArgs[kMaxSlangArgs];
    int slangArgCount = 0;
};

extern Options gOptions;

extern int gWindowWidth;
extern int gWindowHeight;


void parseOptions(int* argc, char** argv);

enum class Error
{
    None = 0,
    InvalidParam,
    Unexpected,
};

} // renderer_test
