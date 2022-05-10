// slang-ir-eliminate-phis.h
#pragma once

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
    void eliminatePhis(CodeGenContext* context, IRModule* module);
}
