#ifndef SLANG_NVRTC_COMPILER_UTIL_H
#define SLANG_NVRTC_COMPILER_UTIL_H

#include "core/slang-platform.h"
#include "slang-downstream-compiler-util.h"

namespace Slang
{


struct NVRTCDownstreamCompilerUtil
{
    /// Returns whether the NVRTC adapter created by locateCompilers can retrieve machine code.
    static bool supportsCubin(IDownstreamCompiler* compiler);

    static SlangResult locateCompilers(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        DownstreamCompilerSet* set);
};

} // namespace Slang

#endif
