#pragma once

#include "slang-compiler.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-pairs.h"
#include "slang-ir-autodiff-rev.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-single-return.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-validate.h"
#include "slang-ir.h"

namespace Slang
{

void initializeTranslationDictionary(IRModule* module);

void clearTranslationDictionary(IRModule* module);
struct TranslationContext
{

public:
    TranslationContext(
        TargetProgram* target,
        IRModule* module,
        SpecializationContext* specContext,
        DiagnosticSink* inSink)
        : irModule(module)
        , sink(inSink)
        , targetProgram(target)
        , specContext(specContext)
        , autodiffContext(target, module->getModuleInst())
    {
        initializeTranslationDictionary(module);
    }

    IRInst* maybeTranslateInst(IRInst* inst);

    IRInst* maybeTranslateIdentityRemat(IRInst* inst);

    IRInst* resolveInst(IRInst* inst);

    // Record `inst` as a structural fixed point of `resolveInst`, so subsequent calls can
    // return it in O(1). Precondition: `inst` is an `IRSetBase` (a `set`) that has resolved to
    // itself — see `resolvedStructuralFixedPoints` for why a set is the only kind sound to
    // record. The precondition is release-asserted because the
    // cache's soundness depends on it: a non-set recorded here would later be returned
    // unchanged, masking a resolution the set-only invariant rules out.
    void recordStructuralFixedPoint(IRInst* inst)
    {
        SLANG_RELEASE_ASSERT(as<IRSetBase>(inst));
        resolvedStructuralFixedPoints.add(inst);
    }

    IRModule* getModule() const { return irModule; }

    TargetProgram* getTargetProgram() const { return targetProgram; }

    DiagnosticSink* getSink() const { return sink; }

    SpecializationContext* getSpecializationContext() const { return specContext; }

private:
    IRModule* irModule;

    TargetProgram* targetProgram;

    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink* sink;

    // Shared context.
    AutoDiffSharedContext autodiffContext;
    SpecializationContext* specContext;

    // Memo of `set` insts (`IRSetBase`) that are structural fixed points of `resolveInst`.
    // `IRSetBase` is the base of all four set kinds — `IRTypeSet`, `IRFuncSet`,
    // `IRWitnessTableSet`, `IRGenericSet` — which share the same operand invariants (concrete,
    // de-duplicated, consistently-sorted leaf members; see the `SetBase` spec in
    // slang-ir-insts.lua), so the argument below holds uniformly for each. A set is the
    // O(N)-operand inst at the root of the quadratic: re-resolving it re-walks all N of its
    // members for no net effect, so for a set referenced from many instructions the redundant
    // re-resolution is O(N) per call and O(N^2) over a function. Caching makes subsequent calls
    // O(1) — and caching only the sets suffices, because anything that references a set (e.g. a
    // `TaggedUnion` type) recurses into the cached set and so becomes O(1) too.
    //
    // Why a `set` is sound to cache by pointer (and why nothing else is cached):
    //  - A set's resolution is the identity modulo operand resolution: its members are
    //    concrete witness tables / types that are themselves permanent fixed points, it has
    //    no specialization / translation / fold / witness-lookup form, and `_resolveInstRec`
    //    never transforms it. It can therefore never resolve to anything other than itself,
    //    so skipping its operand re-walk on a later call cannot mask a resolution. (Operand
    //    resolution is driven independently for every global inst by the lowering loop, so
    //    skipping it here does not skip it everywhere.) This is a *whitelist*: other op kinds
    //    that also reach the fall-through but are non-monotonic — `SizeOf` / `AlignOf` /
    //    `GetArrayLength` (simplify once the type concretizes) and a set-specialized
    //    `Specialize` (lowered/deallocated by Phase 2) — are deliberately not cached, and any
    //    future op kind defaults to not cached, which is safe by construction.
    //  - The pointer key stays valid for as long as the cache could return it. A recorded set
    //    *can* be freed mid-run: `_replaceInstUsesWith` (slang-ir.cpp) rebuilds a hoistable set
    //    when one of its elements is RAUW'd, and frees the old set if the rebuild deduplicates to
    //    a different one. But that path calls `replaceUsesWith(newSet)` on the old set *before*
    //    freeing it, redirecting every reference — operands, data-types, and the weak use that
    //    `_resolveInstRec` re-reads via `instRef` — to the live target. So once a set is freed no
    //    live inst references it, it is off the module's inst list, and `resolveInst` is never
    //    called on it again; the cache never returns a freed pointer. (Address reuse is also
    //    impossible: the IR arena does not free individual allocations.) The fast path
    //    `SLANG_RELEASE_ASSERT`s the cached inst is still module-attached, so if a future change
    //    ever did re-query a freed set it fails loudly rather than returning a stale pointer.
    //
    // Lifetime: the cache lives for one `TranslationContext` (a single specialization run) and is
    // never cleared; growth is bounded by the number of set insts in the module (at most one entry
    // per set). An entry for a set that was later deduplicated away is dead (never re-queried, per
    // the previous point) but harmless.
    HashSet<IRInst*> resolvedStructuralFixedPoints;
};

}; // namespace Slang