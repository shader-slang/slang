// slang-ir-simplify-cfg.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;

    struct CFGSimplificationOptions
    {
        bool removeTrivialSingleIterationLoops = true;
        bool removeSideEffectFreeLoops = true;
        static CFGSimplificationOptions getDefault() { return CFGSimplificationOptions(); }
        static CFGSimplificationOptions getFast() { return CFGSimplificationOptions{ false, false }; }
    };

        /// Simplifies control flow graph by merging basic blocks that
        /// forms a simple linear chain.
        /// Returns true if changed.
    bool simplifyCFG(IRModule* module, CFGSimplificationOptions options);

    bool simplifyCFG(IRGlobalValueWithCode* func, CFGSimplificationOptions options);

}
