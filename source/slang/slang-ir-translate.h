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

    // Record `inst` as a structural fixed point of `resolveInst` so subsequent calls
    // can return it in O(1). Called by `_resolveInstRec` only for witness-table / type
    // `set` insts — see `resolvedStructuralFixedPoints` for why those are the only insts
    // it is sound to record.
    void recordStructuralFixedPoint(IRInst* inst) { resolvedStructuralFixedPoints.add(inst); }

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

    // Memo of witness-table / type `set` insts (`IRSetBase`) that are structural fixed
    // points of `resolveInst`. A set is the O(N)-operand inst at the root of the quadratic:
    // re-resolving it re-walks all N of its members for no net effect, so for a set
    // referenced from many instructions the redundant re-resolution is O(N) per call and
    // O(N^2) over a function. Caching makes subsequent calls O(1) — and caching only the
    // sets suffices, because anything that references a set (e.g. a `TaggedUnion` type)
    // recurses into the cached set and so becomes O(1) too.
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
    //  - Sets are module-scope and are not freed while the cache is live, so the pointer key
    //    cannot be invalidated by an inst being deallocated and its address reused.
    //    `resolveInst` additionally `SLANG_RELEASE_ASSERT`s the cached inst is still
    //    module-attached on every hit, so any future caller that violates this fails loudly
    //    in every build instead of reading a stale pointer.
    HashSet<IRInst*> resolvedStructuralFixedPoints;
};

}; // namespace Slang