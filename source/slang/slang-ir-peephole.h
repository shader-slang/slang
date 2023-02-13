// slang-ir-peephole.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRCall;
    struct IRInst;
    struct SharedIRBuilder;

        /// Apply peephole optimizations.
    bool peepholeOptimize(IRModule* module);
    bool peepholeOptimize(IRInst* func);
    bool tryReplaceInstUsesWithSimplifiedValue(SharedIRBuilder* sharedBuilder, IRInst* inst);
}
