#ifndef SHADER_COMPILER_STD_LIB_H
#define SHADER_COMPILER_STD_LIB_H

#include "../core/basic.h"

namespace Slang
{
    String getCoreLibraryCode();
    String getHLSLLibraryCode();
    String getGLSLLibraryCode();

    void finalizeShaderLibrary();
}

#endif