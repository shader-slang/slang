#ifndef SHADER_COMPILER_STD_LIB_H
#define SHADER_COMPILER_STD_LIB_H

#include "../core/basic.h"

namespace Slang
{
    class SlangStdLib
    {
    private:
        static String code;
    public:
        static String GetCode();
        static void Finalize();
    };

    String getGLSLLibraryCode();
}

#endif