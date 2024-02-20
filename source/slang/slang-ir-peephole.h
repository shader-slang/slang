// slang-ir-peephole.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRCall;
    struct IRInst;
    class TargetProgram;

        /// Apply peephole optimizations.
    bool peepholeOptimize(TargetProgram* target, IRModule* module);
    bool peepholeOptimize(TargetProgram* target, IRInst* func);
    bool peepholeOptimizeGlobalScope(TargetProgram* target, IRModule* module);
    bool tryReplaceInstUsesWithSimplifiedValue(TargetProgram* target, IRModule* module, IRInst* inst);
}
