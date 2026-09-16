// slang-ir-redundancy-removal.h
#pragma once
#include "slang-compiler.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;

// `calleeSideEffectCache` memoizes `doesCalleeHaveSideEffect` queries made by the load/store
// redundancy walk (see `canInstHaveSideEffectAtAddress`). Optional; may be the same cache a
// caller shares with DCE (`IRDeadCodeEliminationOptions::calleeSideEffectCache` in
// slang-ir-dce.h). DCE tolerates a stale "no side effect" entry because it only keeps dead code
// alive; here a stale entry lets `canInstHaveSideEffectAtAddress` forward a load/store across a
// call that has since become impure, which is not conservative. Clear a shared cache whenever a
// purity-changing pass (e.g. `propagateFuncProperties`) has run since the last clear.
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
