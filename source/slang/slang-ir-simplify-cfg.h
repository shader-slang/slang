// slang-ir-simplify-cfg.h
#pragma once

namespace Slang
{
    struct IRModule;

        /// Simplifies control flow graph by merging basic blocks that
        /// forms a simple linear chain.
        ///
    void simplifyCFG(IRModule* module);
}
