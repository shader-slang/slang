#pragma once

#include "slang-downstream-compiler-util.h"
#include "../core/slang-platform.h"

namespace Slang
{

    struct TintDownstreamCompilerUtil
    {
        static
        SlangResult
        locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
    };

}
