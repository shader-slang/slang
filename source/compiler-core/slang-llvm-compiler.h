#ifndef SLANG_LLVM_COMPILER_UTIL_H
#define SLANG_LLVM_COMPILER_UTIL_H

#include "slang-downstream-compiler-util.h"

#include "../core/slang-platform.h"

namespace Slang
{

struct LLVMDownstreamCompilerUtil
{
    static SlangResult locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
};

}

#endif
