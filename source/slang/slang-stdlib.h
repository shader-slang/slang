#ifndef SHADER_COMPILER_STD_LIB_H
#define SHADER_COMPILER_STD_LIB_H

#include "../core/basic.h"

namespace Slang
{
    class SlangStdLib
    {
    private:
        static CoreLib::String code;
    public:
        static CoreLib::String GetCode();
        static void Finalize();
    };

    CoreLib::String getGLSLLibraryCode();
}

#endif