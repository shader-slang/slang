// slang-support.h
#pragma once

#include "render.h"

#include <slang.h>

#include "shader-input-layout.h"

namespace renderer_test {

struct ShaderCompiler
{
    RefPtr<Renderer>        renderer;
    SlangCompileTarget      target;
    SlangSourceLanguage     sourceLanguage;
    SlangPassThrough        passThrough;
    char const*             profile;
    SlangSession*           slangSession;
    
    RefPtr<ShaderProgram> compileProgram(    
        ShaderCompileRequest const& request);
};


} // renderer_test
