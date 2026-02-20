// slang-ir-autodiff-pairs.h
#pragma once

#include "slang-compiler.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

bool processPairTypes(AutoDiffSharedContext* context);

} // namespace Slang
