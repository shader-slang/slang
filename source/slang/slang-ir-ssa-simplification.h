// slang-ir-ssa-simplification.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;

    // Run a combination of SSA, SCCP, SimplifyCFG, and DeadCodeElimination pass
    // until no more changes are possible.
    void simplifyIR(IRModule* module);

    // Run simplifications on IR that is out of SSA form.
    void simplifyNonSSAIR(IRModule* module);

    void simplifyFunc(IRGlobalValueWithCode* func);
}
