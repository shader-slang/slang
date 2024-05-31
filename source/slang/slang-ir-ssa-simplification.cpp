// slang-ir-ssa-simplification.cpp
#include "slang-ir-ssa-simplification.h"
#include "slang-ir.h"
#include "slang-ir-ssa.h"
#include "slang-ir-sccp.h"
#include "slang-ir-dce.h"
#include "slang-ir-simplify-cfg.h"
#include "slang-ir-peephole.h"
#include "slang-ir-hoist-constants.h"
#include "slang-ir-deduplicate-generic-children.h"
#include "slang-ir-remove-unused-generic-param.h"
#include "../core/slang-performance-profiler.h"

namespace Slang
{
    // Run a combination of SSA, SCCP, SimplifyCFG, and DeadCodeElimination pass
    // until no more changes are possible.
    void simplifyIR(IRModule* module)
    {
        SLANG_PROFILE;

        bool changed = true;
        const int kMaxIterations = 8;
        int iterationCounter = 0;
        while (changed && iterationCounter < kMaxIterations)
        {
            changed = false;
            changed |= hoistConstants(module);
            changed |= deduplicateGenericChildren(module);
            changed |= applySparseConditionalConstantPropagation(module);
            changed |= peepholeOptimize(module);
            changed |= simplifyCFG(module);

            // Note: we disregard the `changed` state from dead code elimination pass since
            // SCCP pass could be generating temporarily evaluated constant values and never actually use them.
            // DCE will always remove those nearly generated consts and always returns true here.
            eliminateDeadCode(module);

            changed |= constructSSA(module);
            changed |= removeUnusedGenericParam(module);
            iterationCounter++;
        }
    }

    void simplifyFunc(IRGlobalValueWithCode* func)
    {
        bool changed = true;
        const int kMaxIterations = 8;
        int iterationCounter = 0;
        while (changed && iterationCounter < kMaxIterations)
        {
            changed = false;
            changed |= applySparseConditionalConstantPropagation(func);
            changed |= peepholeOptimize(func);
            changed |= simplifyCFG(func);

            // Note: we disregard the `changed` state from dead code elimination pass since
            // SCCP pass could be generating temporarily evaluated constant values and never actually use them.
            // DCE will always remove those nearly generated consts and always returns true here.
            eliminateDeadCode(func);

            changed |= constructSSA(func);

            iterationCounter++;
        }
    }
}
