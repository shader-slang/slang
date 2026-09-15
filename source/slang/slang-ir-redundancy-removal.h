// slang-ir-redundancy-removal.h
#pragma once
#include "slang-compiler.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;

// `calleeSideEffectCache` memoizes `doesCalleeHaveSideEffect` queries the load/store redundancy
// walk makes for every `Call` inst it scans past (see `canInstHaveSideEffectAtAddress` in
// slang-ir-util.h/.cpp). It is optional, and may be the same cache a caller shares with DCE (see
// `IRDeadCodeEliminationOptions::calleeSideEffectCache` in slang-ir-dce.h for the DCE-side
// contract). This is the single place the contract for *this* consumer is stated; callers should
// reference it rather than restate it. DCE's "a stale entry is conservative" reasoning does not
// transfer here unchanged: DCE's stale "no side effect" only keeps dead code alive, but here it
// lets `canInstHaveSideEffectAtAddress` license forwarding a load/store across a call that has
// since become impure, which is not conservative. So a cache shared with this pass must be
// cleared whenever a purity-changing pass (e.g. `propagateFuncProperties`) could have run since
// it was last cleared, even if DCE's own staleness tolerance would have allowed reusing it.
bool removeRedundancy(
    IRModule* module,
    bool hoistLoopInvariantInsts,
    Dictionary<IRInst*, bool>* calleeSideEffectCache = nullptr);
bool removeRedundancyInFunc(
    IRGlobalValueWithCode* func,
    bool hoistLoopInvariantInsts,
    Dictionary<IRInst*, bool>* calleeSideEffectCache = nullptr);

bool eliminateRedundantLoadStore(
    IRGlobalValueWithCode* func,
    Dictionary<IRInst*, bool>* calleeSideEffectCache = nullptr);

void removeAvailableInDownstreamModuleDecorations(IRModule* module, CodeGenTarget target);
} // namespace Slang
