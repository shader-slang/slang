// slang-ir-ssa-simplification.h
#pragma once

#include "slang-ir-simplify-cfg.h"
#include "slang-ir-peephole.h"

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;
    class DiagnosticSink;
    class TargetProgram;

    struct IRSimplificationOptions
    {
        CFGSimplificationOptions cfgOptions;
        PeepholeOptimizationOptions peepholeOptions;

        static IRSimplificationOptions getDefault()
        {
            IRSimplificationOptions result;
            return result;
        }
        static IRSimplificationOptions getFast()
        {
            IRSimplificationOptions result;
            result.cfgOptions.removeSideEffectFreeLoops = false;
            result.cfgOptions.removeTrivialSingleIterationLoops = false;
            return result;
        }
    };

    // Run a combination of SSA, SCCP, SimplifyCFG, and DeadCodeElimination pass
    // until no more changes are possible.
    void simplifyIR(TargetProgram* target, IRModule* module, IRSimplificationOptions options, DiagnosticSink* sink = nullptr);

    // Run simplifications on IR that is out of SSA form.
    void simplifyNonSSAIR(TargetProgram* target, IRModule* module, IRSimplificationOptions options);

    void simplifyFunc(TargetProgram* target, IRGlobalValueWithCode* func, IRSimplificationOptions options, DiagnosticSink* sink = nullptr);
}
