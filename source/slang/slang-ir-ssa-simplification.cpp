// slang-ir-ssa-simplification.cpp
#include "slang-ir-ssa-simplification.h"

#include "core/slang-performance-profiler.h"
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

    // Callee-side-effect memo shared by DCE and the removeRedundancyInFunc call below -- see
    // IRDeadCodeEliminationOptions::calleeSideEffectCache in slang-ir-dce.h for the authoritative
    // staleness contract. `propagateFuncProperties`, the only purity mutator this contract's
    // monotonicity invariant is about, runs once per outer iteration below, so clearing here
    // (before it runs) is a correctness requirement for that iteration. Unlike the sibling
    // simplifyNonSSAIR/simplifyFunc, this deliberately omits a second clear inside the inner
    // per-function loop (up to `kMaxFuncIterations` calls to `removeRedundancyInFunc` per
    // function, per outer iteration): safe because `propagateFuncProperties` already ran before
    // that inner loop starts, and nothing inside it mutates purity.
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
        options.deadCodeElimOptions.calleeSideEffectCache->clear();

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
                    funcChanged |= removeRedundancyInFunc(
                        func,
                        options.hoistLoopInvariantInsts,
                        options.deadCodeElimOptions.calleeSideEffectCache);
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
    // Shared with removeRedundancy below, not just eliminateDeadCode -- see
    // IRDeadCodeEliminationOptions::calleeSideEffectCache in slang-ir-dce.h for the authoritative
    // staleness contract. Unlike simplifyIR, no step in this loop calls `propagateFuncProperties`
    // or otherwise mutates callee purity, so the per-iteration `clear()` below is NOT a
    // correctness requirement -- it is a deliberate, defensive choice mirroring simplifyIR's
    // clear for symmetry, at the cost of discarding cross-iteration cache reuse (including a
    // caller-supplied cache's entries, if one was passed in via `options`) every iteration.
    Dictionary<IRInst*, bool> calleeSideEffectCache;
    if (!options.deadCodeElimOptions.calleeSideEffectCache)
        options.deadCodeElimOptions.calleeSideEffectCache = &calleeSideEffectCache;

    bool changed = true;
    const int kMaxIterations = 8;
    int iterationCounter = 0;

    while (changed && iterationCounter < kMaxIterations)
    {
        changed = false;
        options.deadCodeElimOptions.calleeSideEffectCache->clear();
        changed |= applySparseConditionalConstantPropagationForGlobalScope(module, target, sink);
        changed |= peepholeOptimize(target, module, options.peepholeOptions);

        if (!options.minimalOptimization)
            changed |= removeRedundancy(
                module,
                options.hoistLoopInvariantInsts,
                options.deadCodeElimOptions.calleeSideEffectCache);
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
    // Same situation as simplifyNonSSAIR above: shared with removeRedundancyInFunc below, and
    // cleared every iteration as a defensive choice, not a correctness requirement -- see that
    // function's comment and IRDeadCodeEliminationOptions::calleeSideEffectCache in
    // slang-ir-dce.h.
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
        options.deadCodeElimOptions.calleeSideEffectCache->clear();
        changed |= applySparseConditionalConstantPropagation(func, target, sink);
        changed |= peepholeOptimize(target, func);
        if (!options.minimalOptimization)
            changed |= removeRedundancyInFunc(
                func,
                options.hoistLoopInvariantInsts,
                options.deadCodeElimOptions.calleeSideEffectCache);
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
