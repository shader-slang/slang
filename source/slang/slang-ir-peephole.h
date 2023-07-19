// slang-ir-peephole.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRCall;
    struct IRInst;

        /// Apply peephole optimizations.
    bool peepholeOptimize(IRModule* module);
    bool peepholeOptimize(IRInst* func);
    bool peepholeOptimizeGlobalScope(IRModule* module);
    bool tryReplaceInstUsesWithSimplifiedValue(IRModule* module, IRInst* inst);
}
