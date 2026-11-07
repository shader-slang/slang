#ifndef SLANG_VKD3D_COMPILER_H
#define SLANG_VKD3D_COMPILER_H

#include "slang-downstream-compiler-util.h"

namespace Slang
{

struct Vkd3dDownstreamCompilerUtil
{
    static SlangResult locateCompilers(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        DownstreamCompilerSet* set);
};

}

#endif
