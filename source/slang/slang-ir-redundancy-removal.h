// slang-ir-redundancy-removal.h
#pragma once
#include "slang-compiler.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;

// `calleeSideEffectCache` memoizes `doesCalleeHaveSideEffect` queries the load/store redundancy
// walk makes for every `Call` inst it scans past (see `canInstHaveSideEffectAtAddress` in
// slang-ir-util.h/.cpp). It is optional and, when passed, follows the same sharing/staleness
// contract as `IRDeadCodeEliminationOptions::calleeSideEffectCache`: safe to share across every
// function processed in one pass, and across this pass and DCE within the same pass, as long as
// it is cleared whenever a callee's purity could have changed since it was populated (e.g. after
// `propagateFuncProperties`).
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
