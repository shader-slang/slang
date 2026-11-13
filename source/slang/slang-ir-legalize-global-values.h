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

    // Copy each global value that the target can't represent into every function, or global
    // variable initializer, that uses it. Then remove the global, and the operands that were
    // copied with it, once they are left without uses.
    void inlineGlobalValuesAndRemoveIfUnused(IRModule* module);

    // Opcodes that can exist in global scope, as long as the operands are.
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
