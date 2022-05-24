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
        /// If liveness information is needed LivenessOptions must be enabled. If it is LiveRangeStarts will be inserted
    void eliminatePhis(CodeGenContext* context, const LivenessOptions& options, IRModule* module);
}
