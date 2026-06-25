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
    // can return it in O(1). Called by `_resolveInstRec` only at the point where an
    // inst has been determined to have no specialization/translation/fold form — see
    // `resolvedStructuralFixedPoints` for why that is the only sound place to record.
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

    // Memo of insts that are *structural* fixed points of `resolveInst`: module-scope
    // insts for which `_resolveInstRec` reached its final fall-through, i.e. the inst has
    // no specialization, translation, constant-fold, or witness-lookup form and its
    // resolution is purely "resolve operands, then return self" (e.g. a witness-table /
    // type `set` or other structural type). Re-resolving such an inst re-walks all of its
    // operands for no net effect, so for a set/tagged-union with N members referenced from
    // many instructions the redundant re-resolution is O(N) per call and O(N^2) over a
    // function. Caching makes subsequent calls O(1).
    //
    // Why caching at the structural fall-through is sound by construction (and why a
    // pointer key is safe):
    //  - A structural fixed point's `resolveInst` result is the identity modulo operand
    //    resolution. It can never become resolvable to something *other* than itself, so
    //    skipping its operand re-walk on a later call cannot mask a resolution. (Operand
    //    resolution is driven independently for every global inst by the lowering loop, so
    //    skipping it here does not skip it everywhere.) The non-monotonic kinds that *do*
    //    become resolvable when their operands concretize — `Specialize` and
    //    `LookupWitnessMethod` — return through their own branches in `_resolveInstRec` and
    //    never reach the fall-through, so they are structurally excluded from this cache.
    //  - These insts are module-scope (the fall-through is guarded by an `IRModuleInst`
    //    parent check) and are not freed while the cache is live, so the pointer key cannot
    //    be invalidated by an inst being deallocated and its address reused. `resolveInst`
    //    asserts the cached inst is still module-attached on every hit to catch any future
    //    caller that violates this.
    HashSet<IRInst*> resolvedStructuralFixedPoints;
};

}; // namespace Slang