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

    // Callee-side-effect memo shared by every DCE invocation in this pass, and also by the
    // removeRedundancyInFunc call below (see IRDeadCodeEliminationOptions::calleeSideEffectCache
    // for the sharing contract, and slang-ir-redundancy-removal.h for why staleness is safe for
    // that second consumer). Cleared each outer iteration so both consumers see the purity facts
    // propagateFuncProperties proves that iteration. removeRedundancyInFunc itself runs inside
    // the inner per-func loop, up to kMaxFuncIterations times per outer iteration, without an
    // additional clear -- safe because propagateFuncProperties (the only thing in this pass that
    // can change a callee's purity) runs once per outer iteration, before the inner loop starts.
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
    // slang-ir-redundancy-removal.h for why that consumer needs the cache cleared more eagerly
    // than DCE's own staleness tolerance would require. Cleared every iteration, mirroring
    // simplifyIR's own per-iteration clear.
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
    // See the identical comment in simplifyNonSSAIR above: this cache is shared with
    // removeRedundancyInFunc below, not just eliminateDeadCode, so it is cleared every
    // iteration rather than relying on this loop never mutating callee purity.
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
