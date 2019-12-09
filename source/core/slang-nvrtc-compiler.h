#ifndef SLANG_NVRTC_COMPILER_UTIL_H
#define SLANG_NVRTC_COMPILER_UTIL_H

#include "slang-downstream-compiler.h"

#include "../core/slang-platform.h"

namespace Slang
{


struct NVRTCDownstreamCompilerUtil
{
    static SlangResult createCompiler(SharedLibrary::Handle handle, RefPtr<DownstreamCompiler>& outCompiler);

};

}

#endif
