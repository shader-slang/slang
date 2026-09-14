// slang-ir-redundancy-removal.h
#pragma once
#include "slang-compiler.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;

// `calleeSideEffectCache` memoizes `doesCalleeHaveSideEffect` queries the load/store redundancy
// walk makes for every `Call` inst it scans past (see `canInstHaveSideEffectAtAddress` in
// slang-ir-util.h/.cpp). It is optional; when shared with DCE (see the sharing/staleness
// contract on `IRDeadCodeEliminationOptions::calleeSideEffectCache` in slang-ir-dce.h -- the
// single place that contract is stated, not paraphrased here), callers must clear it whenever a
// callee's purity could have changed since it was populated. That contract's "a stale entry is
// conservative" reasoning is DCE-specific (a stale "no side effect" only keeps dead code alive);
// for this consumer a stale "no side effect" instead lets `canInstHaveSideEffectAtAddress`
// license forwarding a load/store across that call, which is not automatically safe -- callers
// sharing a cache with this pass must clear it at least as often as DCE would need to, not rely
// on DCE's weaker justification.
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
