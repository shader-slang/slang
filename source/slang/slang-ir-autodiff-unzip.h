// slang-ir-autodiff-unzip.h
#pragma once

#include "slang-compiler.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-primal-hoist.h"
#include "slang-ir-autodiff-region.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-insts.h"
#include "slang-ir-ssa.h"
#include "slang-ir-validate.h"
#include "slang-ir.h"

namespace Slang
{
void unzipDiffInsts(AutoDiffSharedContext* context, IRFunc* func);

IRFunc* splitApplyAndPropFuncs(
    AutoDiffSharedContext* autodiffContext,
    IRFunc* func,
    IRFunc* originalFunc,
    HoistedPrimalsInfo* primalsInfo,
    IRInst*& intermediateType,
    IRFunc*& getValFunc);

} // namespace Slang
