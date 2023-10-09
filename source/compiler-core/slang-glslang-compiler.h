#ifndef SLANG_GLSLANG_COMPILER_UTIL_H
#define SLANG_GLSLANG_COMPILER_UTIL_H

#include "slang-downstream-compiler-util.h"

#include "../core/slang-platform.h"

namespace Slang
{

struct GlslangDownstreamCompilerUtil
{
    static SlangResult locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
};


struct SpirvOptDownstreamCompilerUtil
{
    static SlangResult locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
};

struct SpirvDisDownstreamCompilerUtil
{
    static SlangResult locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
};

}

#endif
