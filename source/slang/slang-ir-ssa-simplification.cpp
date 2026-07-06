// slang-ir-ssa-simplification.cpp
#include "slang-ir-ssa-simplification.h"

#include "../core/slang-performance-profiler.h"
#include "slang-ir-dce.h"
#include "slang-ir-deduplicate-generic-children.h"
#include "slang-ir-peephole.h"
#include "slang-ir-propagate-func-properties.h"
#include "slang-ir-redundancy-removal.h"
#include "slang-ir-remove-unused-generic-param.h"
#include "slang-ir-sccp.h"
#include "slang-ir-simplify-cfg.h"
#include "slang-ir-ssa.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
IRSimplificationOptions IRSimplificationOptions::getDefault(TargetProgram* targetProgram)
{
    IRSimplificationOptions result;
    result.minimalOptimization =
        targetProgram ? targetProgram->getOptionSet().shouldPerformMinimumOptimizations() : false;
    if (result.minimalOptimization)
        result.cfgOptions = CFGSimplificationOptions::getFast();
    else
        result.cfgOptions = CFGSimplificationOptions::getDefault();
    result.peepholeOptions = PeepholeOptimizationOptions();
    if (targetProgram)
        result.deadCodeElimOptions.keepGlobalParamsAlive =
            targetProgram->getOptionSet().getBoolOption(CompilerOptionName::PreserveParameters);
    result.deadCodeElimOptions.useFastAnalysis = result.minimalOptimization;
    return result;
}

IRSimplificationOptions IRSimplificationOptions::getFast(TargetProgram* targetProgram)
{
    IRSimplificationOptions result;
    result.minimalOptimization =
        targetProgram ? targetProgram->getOptionSet().shouldPerformMinimumOptimizations() : false;
    result.cfgOptions = CFGSimplificationOptions::getFast();
    result.peepholeOptions = PeepholeOptimizationOptions();
    if (targetProgram)
        result.deadCodeElimOptions.keepGlobalParamsAlive =
            targetProgram->getOptionSet().getBoolOption(CompilerOptionName::PreserveParameters);
    result.deadCodeElimOptions.useFastAnalysis = result.minimalOptimization;
    return result;
}

// Run a combination of SSA, SCCP, SimplifyCFG, and DeadCodeElimination pass
// until no more changes are possible.
void simplifyIR(
    IRModule* module,
    TargetProgram* target,
    IRSimplificationOptions options,
    DiagnosticSink* sink)
{
    SLANG_PROFILE;

    // One callee-side-effect memo shared by every DCE invocation in this pass:
    // the per-function fixpoint below runs eliminateDeadCode O(functions x
    // iterations) times, and each uncached invocation re-walks shared callees'
    // use lists (see IRDeadCodeEliminationOptions::calleeSideEffectCache) —
    // quadratic in the call-site count of builtins like `sin`.
    //
    // Sharing one memo across the whole pass is sound because the fixpoint's
    // passes keep the answer monotone: none of them creates an `IRAnnotation`
    // or removes a no-side-effect/read-none decoration (they only remove
    // annotations and add purity decorations), so a cached answer can only go
    // conservatively stale (an inst stays alive one extra round). A pass added
    // to this fixpoint must preserve that contract or must not share the
    // cache; a debug-mode check in `doesCalleeHaveSideEffect` verifies it on
    // every cache hit.
    Dictionary<IRInst*, bool> calleeSideEffectCache;
    if (!options.deadCodeElimOptions.calleeSideEffectCache)
        options.deadCodeElimOptions.calleeSideEffectCache = &calleeSideEffectCache;

    bool changed = true;
    const int kMaxIterations = 8;
    const int kMaxFuncIterations = 16;
    int iterationCounter = 0;

    while (changed && iterationCounter < kMaxIterations)
    {
        if (sink && sink->getErrorCount())
            break;

        changed = false;

        changed |= deduplicateGenericChildren(module);
        changed |= propagateFuncProperties(module);
        changed |= removeUnusedGenericParam(module);
        changed |= applySparseConditionalConstantPropagationForGlobalScope(module, target, sink);
        changed |= peepholeOptimizeGlobalScope(target, module);
        changed |= trimOptimizableTypes(module);

        for (auto inst : module->getGlobalInsts())
        {
            auto func = as<IRGlobalValueWithCode>(inst);
            if (!func)
                continue;
            bool funcChanged = true;
            int funcIterationCount = 0;
            while (funcChanged && funcIterationCount < kMaxFuncIterations)
            {

                eliminateDeadCode(func, options.deadCodeElimOptions);
                funcChanged = false;
                funcChanged |= applySparseConditionalConstantPropagation(func, target, sink);
                funcChanged |= peepholeOptimize(target, func);
                if (options.removeRedundancy)
                    funcChanged |= removeRedundancyInFunc(func, options.hoistLoopInvariantInsts);
                funcChanged |= simplifyCFG(func, options.cfgOptions);
                // Note: we disregard the `changed` state from dead code elimination pass since
                // SCCP pass could be generating temporarily evaluated constant values and never
                // actually use them. DCE will always remove those nearly generated consts and
                // always returns true here. Run eliminate-dead-code twice to ensure optimizations
                // are applied on the dce'd code.
                //
                eliminateDeadCode(func, options.deadCodeElimOptions);
                if (funcIterationCount == 0)
                    funcChanged |= constructSSA(func);
                changed |= funcChanged;
                funcIterationCount++;
            }
        }
        iterationCounter++;
    }
    eliminateDeadCode(module, options.deadCodeElimOptions);
}

void simplifyNonSSAIR(
    IRModule* module,
    TargetProgram* target,
    IRSimplificationOptions options,
    DiagnosticSink* sink)
{
    // See the matching memo in simplifyIR for why and when this is sound.
    Dictionary<IRInst*, bool> calleeSideEffectCache;
    if (!options.deadCodeElimOptions.calleeSideEffectCache)
        options.deadCodeElimOptions.calleeSideEffectCache = &calleeSideEffectCache;

    bool changed = true;
    const int kMaxIterations = 8;
    int iterationCounter = 0;

    while (changed && iterationCounter < kMaxIterations)
    {
        changed = false;
        changed |= applySparseConditionalConstantPropagationForGlobalScope(module, target, sink);
        changed |= peepholeOptimize(target, module, options.peepholeOptions);

        if (!options.minimalOptimization)
            changed |= removeRedundancy(module, options.hoistLoopInvariantInsts);
        changed |= simplifyCFG(module, options.cfgOptions);

        // Note: we disregard the `changed` state from dead code elimination pass since
        // SCCP pass could be generating temporarily evaluated constant values and never actually
        // use them. DCE will always remove those nearly generated consts and always returns true
        // here.
        eliminateDeadCode(module, options.deadCodeElimOptions);
        iterationCounter++;
    }
}


void simplifyFunc(
    TargetProgram* target,
    IRGlobalValueWithCode* func,
    IRSimplificationOptions options,
    DiagnosticSink* sink)
{
    // See the matching memo in simplifyIR for why and when this is sound.
    Dictionary<IRInst*, bool> calleeSideEffectCache;
    if (!options.deadCodeElimOptions.calleeSideEffectCache)
        options.deadCodeElimOptions.calleeSideEffectCache = &calleeSideEffectCache;

    bool changed = true;
    const int kMaxIterations = 8;
    int iterationCounter = 0;
    while (changed && iterationCounter < kMaxIterations)
    {
        if (sink && sink->getErrorCount())
            break;

        changed = false;
        changed |= applySparseConditionalConstantPropagation(func, target, sink);
        changed |= peepholeOptimize(target, func);
        if (!options.minimalOptimization)
            changed |= removeRedundancyInFunc(func, options.hoistLoopInvariantInsts);
        changed |= simplifyCFG(func, options.cfgOptions);

        // Note: we disregard the `changed` state from dead code elimination pass since
        // SCCP pass could be generating temporarily evaluated constant values and never actually
        // use them. DCE will always remove those nearly generated consts and always returns true
        // here.
        eliminateDeadCode(func, options.deadCodeElimOptions);

        changed |= constructSSA(func);

        iterationCounter++;
    }
}
} // namespace Slang
