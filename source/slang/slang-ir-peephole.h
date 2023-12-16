// slang-ir-peephole.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRCall;
    struct IRInst;
    class TargetRequest;

        /// Apply peephole optimizations.
    bool peepholeOptimize(TargetRequest* target, IRModule* module);
    bool peepholeOptimize(TargetRequest* target, IRInst* func);
    bool peepholeOptimizeGlobalScope(TargetRequest* target, IRModule* module);
    bool tryReplaceInstUsesWithSimplifiedValue(TargetRequest* target, IRModule* module, IRInst* inst);
}
