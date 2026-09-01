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

    // Optional memo for the per-callee side-effect query: without it, each
    // call-site query re-walks the callee's use list, which is quadratic in
    // the number of call sites to a shared callee (see
    // `doesCalleeHaveSideEffect` in slang-ir-util.h).
    //
    // This is a non-owning pointer on purpose: null means "not shared" (each
    // `eliminateDeadCode` invocation uses its own fresh cache), while a
    // caller-owned dictionary lets a simplification fixpoint share one memo
    // across its many DCE invocations — which is where the quadratic cost
    // actually accrues. An owned (non-pointer) member could not express that
    // sharing. A shared cache is sound only while no pass creates an
    // `IRAnnotation` or removes a purity decoration; a stale entry is then
    // conservative (keeps a call alive, never wrongly eliminates one).
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
