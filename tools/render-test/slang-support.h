// slang-support.h
#pragma once

#include "render.h"

#include <slang.h>

namespace renderer_test {

ShaderCompiler* createSlangShaderCompiler(
    ShaderCompiler*     innerCompiler,
    SlangSourceLanguage sourceLanguage,
    SlangCompileTarget  target);

} // renderer_test
