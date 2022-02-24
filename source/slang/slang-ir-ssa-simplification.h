// slang-ir-ssa-simplification.h
#pragma once

namespace Slang
{
    struct IRModule;

    // Run a combination of SSA, SCCP, SimplifyCFG, and DeadCodeElimination pass
    // until no more changes are possible.
    void simplifyIR(IRModule* module);
}
