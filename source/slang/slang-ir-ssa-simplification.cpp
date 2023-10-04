// slang-ir-ssa-simplification.cpp
#include "slang-ir-ssa-simplification.h"
#include "slang-ir.h"
#include "slang-ir-ssa.h"
#include "slang-ir-sccp.h"
#include "slang-ir-dce.h"
#include "slang-ir-simplify-cfg.h"
#include "slang-ir-peephole.h"
#include "slang-ir-deduplicate-generic-children.h"
#include "slang-ir-remove-unused-generic-param.h"
#include "slang-ir-redundancy-removal.h"
#include "slang-ir-propagate-func-properties.h"
#include "../core/slang-performance-profiler.h"
#include "slang-ir-util.h"

namespace Slang
{
    // Run a combination of SSA, SCCP, SimplifyCFG, and DeadCodeElimination pass
    // until no more changes are possible.
    void simplifyIR(IRModule* module, IRSimplificationOptions options, DiagnosticSink* sink)
    {
        SLANG_PROFILE;
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
            changed |= applySparseConditionalConstantPropagationForGlobalScope(module, sink);
            changed |= peepholeOptimizeGlobalScope(module);

            for (auto inst : module->getGlobalInsts())
            {
                auto func = as<IRGlobalValueWithCode>(inst);
                if (!func)
                    continue;
                bool funcChanged = true;
                int funcIterationCount = 0;
                while (funcChanged && funcIterationCount < kMaxFuncIterations)
                {
                    funcChanged = false;
                    funcChanged |= applySparseConditionalConstantPropagation(func, sink);
                    funcChanged |= peepholeOptimize(func);
                    funcChanged |= removeRedundancyInFunc(func);
                    funcChanged |= simplifyCFG(func, options.cfgOptions);
                    eliminateDeadCode(func);
                    funcChanged |= constructSSA(func);
                    changed |= funcChanged;
                    funcIterationCount++;
                }
            }

            // Note: we disregard the `changed` state from dead code elimination pass since
            // SCCP pass could be generating temporarily evaluated constant values and never actually use them.
            // DCE will always remove those nearly generated consts and always returns true here.
            eliminateDeadCode(module);

            iterationCounter++;
        }
    }

    void simplifyNonSSAIR(IRModule* module, IRSimplificationOptions options)
    {
        bool changed = true;
        const int kMaxIterations = 8;
        int iterationCounter = 0;
        while (changed && iterationCounter < kMaxIterations)
        {
            changed = false;
            changed |= peepholeOptimize(module);

            changed |= removeRedundancy(module);
            changed |= simplifyCFG(module, options.cfgOptions);

            // Note: we disregard the `changed` state from dead code elimination pass since
            // SCCP pass could be generating temporarily evaluated constant values and never actually use them.
            // DCE will always remove those nearly generated consts and always returns true here.
            eliminateDeadCode(module);
            iterationCounter++;
        }
    }


    void simplifyFunc(IRGlobalValueWithCode* func, IRSimplificationOptions options, DiagnosticSink* sink)
    {
        bool changed = true;
        const int kMaxIterations = 8;
        int iterationCounter = 0;
        while (changed && iterationCounter < kMaxIterations)
        {
            if (sink && sink->getErrorCount())
                break;

            changed = false;
            changed |= applySparseConditionalConstantPropagation(func, sink);
            changed |= peepholeOptimize(func);
            changed |= removeRedundancyInFunc(func);
            changed |= simplifyCFG(func, options.cfgOptions);

            // Note: we disregard the `changed` state from dead code elimination pass since
            // SCCP pass could be generating temporarily evaluated constant values and never actually use them.
            // DCE will always remove those nearly generated consts and always returns true here.
            eliminateDeadCode(func);

            changed |= constructSSA(func);

            iterationCounter++;

        }
    }
}
