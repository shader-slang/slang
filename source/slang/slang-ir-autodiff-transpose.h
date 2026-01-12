// slang-ir-autodiff-transpose.h
#pragma once
#include "slang-ir-autodiff.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
void transposeDiffBlocksInFunc(AutoDiffSharedContext* sharedContext, IRFunc* propagateFunc);
} // namespace Slang
