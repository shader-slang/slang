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
        /// ioLocations is optional - pass nullptr if not required. 
        /// If set, will have liveness location information appended to the list
    void eliminatePhis(CodeGenContext* context, List<LivenessLocation>* ioLocations, IRModule* module);
}
