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

    // Record `inst` (an `IRSetBase`, asserted) as a structural fixed point of `resolveInst`, so
    // subsequent calls return it in O(1). See `resolvedStructuralFixedPoints` for why only sets
    // are sound to record.
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

    // Memo of `set` insts (any `IRSetBase`: type/func/witness-table/generic set) that
    // `resolveInst` returned unchanged. A set has one operand per dispatch member, so re-walking
    // those N operands on each of `resolveInst`'s O(N) calls is the O(N^2) cost removed here; a
    // hit is O(1). Caching only sets suffices — anything referencing a set (e.g. a `TaggedUnion`)
    // recurses into the cached set.
    //
    // Sound by construction: a set has no specialization/fold/lookup form, so `resolveInst` always
    // returns it unchanged — skipping the re-walk cannot mask a resolution. This is a whitelist;
    // the other kinds reaching the same fall-through *are* non-monotonic (`SizeOf`/`AlignOf`/
    // `GetArrayLength`, set-specialized `Specialize`) and are excluded.
    //
    // Pointer-key safety: a set can be freed mid-run (set dedup in `_replaceInstUsesWith`), but the
    // free RAUWs every reference to the dedup target before freeing, so a freed set is never
    // re-queried and its address is never reused (the IR arena has no per-allocation free); the
    // fast path release-asserts module-attachment as a tripwire. Lives for one
    // `TranslationContext`.
    HashSet<IRInst*> resolvedStructuralFixedPoints;
};

}; // namespace Slang