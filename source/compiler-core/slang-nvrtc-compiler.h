#ifndef SLANG_NVRTC_COMPILER_UTIL_H
#define SLANG_NVRTC_COMPILER_UTIL_H

#include "slang-downstream-compiler-util.h"

#include "../core/slang-platform.h"

namespace Slang
{


struct NVRTCDownstreamCompilerUtil
{
    static SlangResult locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
};

}

#endif
