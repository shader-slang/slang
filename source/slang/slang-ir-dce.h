// slang-ir-dce.h
#pragma once

#include "slang-ir-insts.h"

namespace Slang
{
struct IRModule;

struct IRDeadCodeEliminationOptions
{
    bool keepExportsAlive = false;
    bool keepLayoutsAlive = false;
    bool useFastAnalysis = false;
    bool keepGlobalParamsAlive = true;

    // Optional memo for the per-callee side-effect query, shared across every
    // liveness check in one DCE invocation. Without it, each call-site query
    // re-walks the callee's use list to find associated-function annotations,
    // which is quadratic in the number of call sites to a shared callee (see
    // `doesCalleeHaveSideEffect` in slang-ir-util.h). `eliminateDeadCode`
    // supplies a fresh cache automatically; setting this is only needed to
    // share one cache across multiple DCE invocations.
    Dictionary<IRInst*, bool>* calleeSideEffectCache = nullptr;
};

/// Eliminate "dead" code from the given IR module.
///
/// This pass is primarily designed for flow-insensitive
/// "global" dead code elimination (DCE), such as removing
/// types that are unused, functions that are never called,
/// etc.
/// Returns true if changed.
bool eliminateDeadCode(
    IRModule* module,
    IRDeadCodeEliminationOptions const& options = IRDeadCodeEliminationOptions());

bool eliminateDeadCode(
    IRInst* root,
    IRDeadCodeEliminationOptions const& options = IRDeadCodeEliminationOptions());

bool shouldInstBeLiveIfParentIsLive(IRInst* inst, IRDeadCodeEliminationOptions options);

bool isWeakReferenceOperand(IRInst* inst, UInt operandIndex);

bool trimOptimizableTypes(IRModule* module);

} // namespace Slang
