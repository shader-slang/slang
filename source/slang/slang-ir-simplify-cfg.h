// slang-ir-simplify-cfg.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;

        /// Simplifies control flow graph by merging basic blocks that
        /// forms a simple linear chain.
        /// Returns true if changed.
    bool simplifyCFG(IRModule* module);

    bool simplifyCFG(IRGlobalValueWithCode* func);

}
