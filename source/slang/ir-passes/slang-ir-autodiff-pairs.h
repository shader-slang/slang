// slang-ir-autodiff-pairs.h
#pragma once

#include "slang-ir-autodiff.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-inst-pass-base.h"
#include "slang/ir/slang-ir-clone.h"
#include "slang/ir/slang-ir-insts.h"
#include "slang/ir/slang-ir-util.h"
#include "slang/ir/slang-ir.h"
#include "slang/slang-compiler.h"

namespace Slang
{

bool processPairTypes(AutoDiffSharedContext* context);

} // namespace Slang
