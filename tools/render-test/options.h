// options.h
#pragma once

#include <stdint.h>

namespace renderer_test {

typedef intptr_t Int;
typedef uintptr_t UInt;

enum class Mode
{
    Slang,
    HLSL,
    GLSL,
    GLSLCrossCompile,
};

struct Options
{
    char const* appName = "render-test";
    char const* sourcePath = nullptr;
    char const* outputPath = nullptr;
    Mode mode = Mode::Slang;
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
