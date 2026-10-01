// slang-ir-redundancy-removal.h
#pragma once
#include "slang-compiler.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;

// `calleeSideEffectCache` memoizes `doesCalleeHaveSideEffect` queries made by the load/store
// redundancy walk (see `canInstHaveSideEffectAtAddress`). Optional; may be the same cache a
// caller shares with DCE -- see `IRDeadCodeEliminationOptions::calleeSideEffectCache` in
// slang-ir-dce.h for the authoritative staleness contract both consumers share. This consumer's
// specific consequence of a stale entry: a stale-pure entry (unsafe, but excluded by that
// contract's monotonicity invariant) would let `canInstHaveSideEffectAtAddress` forward a
// load/store across a call that has since become impure; a stale-impure entry (the only direction
// that can actually occur) just declines to forward a load/store it could safely forward.
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
