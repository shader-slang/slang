// slang-ir-eliminate-phis.h
#pragma once

#include "slang-ir-liveness.h"

namespace Slang
{
    struct CodeGenContext;
    struct IRModule;

        /// Eliminate all "phi nodes" from the given `module`.
        ///
        /// This process moves the code in `module` *out* of SSA form,
        /// so that it is more suitable for emission on targets that
        /// are not themselves based on an SSA representation.
        ///
        /// If livenessMode is enabled LiveRangeStarts will be inserted into the module.
    void eliminatePhis(CodeGenContext* context, LivenessMode livenessMode, IRModule* module);
}
