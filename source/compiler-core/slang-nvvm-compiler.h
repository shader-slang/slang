#ifndef SLANG_NVVM_COMPILER_H
#define SLANG_NVVM_COMPILER_H

#include "core/slang-platform.h"
#include "slang-downstream-compiler-util.h"

namespace Slang
{

/// Locates dynamically loadable libNVVM downstream compilers.
struct NVVMDownstreamCompilerUtil
{
    static SlangResult locateCompilers(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        DownstreamCompilerSet* set);
};

} // namespace Slang

#endif
