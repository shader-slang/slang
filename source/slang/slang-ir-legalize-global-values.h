#pragma once

#include "core/slang-dictionary.h"

namespace Slang
{
struct IRBuilder;
struct IRCloneEnv;
struct IRInst;
struct IRModule;
enum class CodeGenTarget;

struct GlobalInstInliningContextGeneric
{
    Dictionary<IRInst*, bool> m_mapGlobalInstToShouldInline;
    bool wrapReferences = true;

    // Target-specific control over how inlining happens
    virtual bool isLegalGlobalInstForTarget(IRInst* inst) = 0;
    virtual bool isInlinableGlobalInstForTarget(IRInst* inst) = 0;
    virtual bool shouldBeInlinedForTarget(IRInst* user) = 0;
    virtual IRInst* getOutsideASM(IRInst* beforeInst) = 0;

    // Inline global values that can't represented by the target to their use sites.
    // If this leaves any global unused, then remove it.
    void inlineGlobalValuesAndRemoveIfUnused(IRModule* module);

    // Returns true for an instruction the target can keep at global scope: a constant, a
    // spec-const-rate value, or one that `isLegalGlobalInstForTarget` accepts.
    // `shouldInlineInstImpl` still inlines such an instruction when one of its operands must be
    // inlined.
    bool isLegalGlobalInst(IRInst* inst);

    // Opcodes that can be inlined into function bodies.
    bool isInlinableGlobalInst(IRInst* inst);

    bool shouldInlineInstImpl(IRInst* inst);

    bool shouldInlineInst(IRInst* inst);

    IRInst* inlineInst(IRBuilder& builder, IRCloneEnv& cloneEnv, IRInst* inst);

    /// Inline `inst` in the local function body so they can be emitted as a local inst.
    ///
    IRInst* maybeInlineGlobalValue(
        IRBuilder& builder,
        IRInst* user,
        IRInst* inst,
        IRCloneEnv& cloneEnv);
};

// For global constant values that are resource typed or struct containing resource types,
// we need to inline their uses to concrete function bodies so they can be legalized during
// resource legalization.
//
// Module-scope calls to member-wise synthesized constructors are first folded into
// `makeStruct`, so that a `static const` struct table can stay a global constant.
//
// For CUDA, a global constant whose type contains `half` is also inlined, because CUDA
// emits a `half` literal as a constructor call, which is a dynamic initializer.
void inlineGlobalConstantsForLegalization(IRModule* module, CodeGenTarget target);


} // namespace Slang
