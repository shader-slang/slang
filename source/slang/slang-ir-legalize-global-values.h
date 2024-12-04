#pragma once

#include "core/slang-dictionary.h"

namespace Slang
{
struct IRBuilder;
struct IRCloneEnv;
struct IRInst;
struct IRModule;

struct GlobalInstInliningContextGeneric
{
    Dictionary<IRInst*, bool> m_mapGlobalInstToShouldInline;

    // Target-specific control over how inlining happens
    virtual bool isLegalGlobalInstForTarget(IRInst* inst) =0;
    virtual bool isInlinableGlobalInstForTarget(IRInst* inst) =0;
    virtual bool shouldBeInlinedForTarget(IRInst* user) =0;
    virtual IRInst* getOutsideASM(IRInst* beforeInst) =0;

    // Inline global values that can't represented by the target to their use sites.
    void inlineGlobalValues(IRModule * module);

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
} // namespace Slang
