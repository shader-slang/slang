// slang-ir-ssa-simplification.cpp
#pragma once

#include "slang-ir.h"
#include "slang-ir-ssa.h"
#include "slang-ir-sccp.h"
#include "slang-ir-dce.h"

namespace Slang
{
    struct IRModule;

    // Run a combination of SSA, SCCP, SimplifyCFG, and DeadCodeElimination pass
    // until no more changes are possible.
    void simplifyIR(IRModule* module)
    {
        bool changed = true;
        int iterationCounter = 8;
        while (changed && iterationCounter > 0)
        {
            changed = false;
            changed |= applySparseConditionalConstantPropagation(module);
            eliminateDeadCode(module);
            constructSSA(module);
            iterationCounter--;
        }
    }
}
