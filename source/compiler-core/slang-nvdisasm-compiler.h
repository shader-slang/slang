#pragma once

#include "compiler-core/slang-downstream-compiler-util.h"

namespace Slang
{
struct NvdisasmDownstreamCompilerUtil
{
    static SlangResult locateCompilers(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        DownstreamCompilerSet* set);
};
} // namespace Slang
