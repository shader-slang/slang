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
    // sharing.
    //
    // THE authoritative staleness contract for this cache -- every other comment referencing
    // "the staleness contract" or "the sharing contract" (slang-ir-redundancy-removal.h,
    // slang-ir-util.h's `canInstHaveSideEffectAtAddress`, slang-ir-ssa-simplification.cpp,
    // slang-ir-defer-buffer-load.cpp) means this paragraph, not a re-derivation of it:
    //
    // A shared cache is sound only while purity is monotonic toward pure within the window it is
    // shared over -- i.e. no pass in that window creates an `IRAnnotation` (which
    // `doesCalleeHaveSideEffect` reads via `forEachAssociatedCallee` to find associated callees,
    // e.g. autodiff-generated derivative functions) or removes an `IRNoSideEffectDecoration`/
    // `IRReadNoneDecoration`. `propagateFuncProperties` (slang-ir-propagate-func-properties.cpp)
    // is the only pass sharing this cache that touches those decorations, and it only ever adds
    // them, never removes one -- so the only staleness that can arise is a cached entry that was
    // `true` (impure) becoming `false` (pure) with the fresher, un-cached answer. That direction
    // is safe for BOTH consumers that read this cache, not just DCE: DCE just keeps a now-pure
    // call alive an iteration longer, and load/store redundancy removal
    // (slang-ir-redundancy-removal.h) just declines to forward a load/store it could safely
    // forward. The unsafe direction -- a cached `false` (pure) that should have become `true`
    // (impure) -- cannot arise from monotonic-toward-pure purity changes, so it is not a live
    // concern for either consumer as long as the monotonicity invariant above holds. A caller
    // that shares this cache across a purity-decreasing operation this invariant doesn't cover
    // must clear it first.
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
