#include "slang-ir-propagate-func-properties.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"


namespace Slang
{
class FuncPropertyPropagationContext
{
public:
    virtual bool canProcess(IRFunc* f) = 0;
    virtual bool isInitialFunc(IRFunc* f) = 0;
    virtual bool propagate(IRBuilder& builder, IRFunc* func) = 0;
};

/// Return whether function-property propagation analyzes `op` with rules beyond the generic
/// side-effect query.
static bool doesOpRequireCustomFuncPropertyAnalysis(IROp op)
{
    switch (op)
    {
    case kIROp_IfElse:
    case kIROp_UnconditionalBranch:
    case kIROp_Switch:
    case kIROp_Return:
    case kIROp_Loop:
    case kIROp_Call:
    case kIROp_Param:
    case kIROp_Unreachable:
    case kIROp_Store:
    case kIROp_SwizzledStore:
        return true;
    default:
        return false;
    }
}

/// Return whether an operand of `inst` may name mutable storage outside `func`.
static bool doesInstUseGlobalOrUnknownMutableAddress(IRFunc* func, IRInst* inst)
{
    // Constants and types do not name mutable storage. We ask the shared address classifier about
    // every other operand because it follows local address calculations to their underlying
    // storage.
    for (UInt operandIndex = 0; operandIndex < inst->getOperandCount(); ++operandIndex)
    {
        auto operand = inst->getOperand(operandIndex);
        if (as<IRConstant>(operand) || as<IRType>(operand))
            continue;
        if (isGlobalOrUnknownMutableAddress(func, operand))
            return true;
    }
    return false;
}

class ReadNoneFuncPropertyPropagationContext : public FuncPropertyPropagationContext
{
public:
    virtual bool isInitialFunc(IRFunc* f) override
    {
        // If the func has already been marked with any decorations, skip.
        for (auto decoration : f->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_ReadNoneDecoration:
                return true;
            }
        }
        return false;
    }
    virtual bool canProcess(IRFunc* f) override
    {
        // If the func has already been marked with any decorations, skip.
        for (auto decoration : f->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_ReadNoneDecoration:
            case kIROp_TargetIntrinsicDecoration:
                return false;
            }
        }
        return true;
    }

    virtual bool propagate(IRBuilder& builder, IRFunc* f) override
    {
        // A `ReadNone` function cannot read or modify mutable program state. We reject ordinary
        // instructions that report a side effect or read resource contents, require every callee to
        // be `ReadNone`, and reject any instruction that uses an address into global or otherwise
        // unknown mutable storage. We preserve the existing conservative treatment of in-block
        // debug instructions by leaving them to the ordinary side-effect test. Dead-code
        // elimination treats `ReadNone` as permission to remove a call, but it has no separate rule
        // for preserving the debug records associated with that call.
        bool preventsReadNone = false;
        for (auto block : f->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                // The generic side-effect query recognizes instructions such as buffer stores and
                // discard, but it deliberately does not classify ordinary loads as side effects.
                // The address check below detects loads from mutable storage. Dedicated resource
                // operations need a separate check because their resource operands are values, not
                // addresses into the resource's contents.
                if (!doesOpRequireCustomFuncPropertyAnalysis(inst->getOp()))
                {
                    if (inst->mightHaveSideEffects() || doesOpReadResourceContents(inst->getOp()))
                    {
                        preventsReadNone = true;
                        break;
                    }
                }

                if (auto call = as<IRCall>(inst))
                {
                    if (!isReadNoneCallee(call->getCallee()))
                    {
                        preventsReadNone = true;
                        break;
                    }
                }

                if (doesInstUseGlobalOrUnknownMutableAddress(f, inst))
                {
                    preventsReadNone = true;
                    break;
                }
            }
            if (preventsReadNone)
                break;
        }
        if (!preventsReadNone)
        {
            builder.addDecoration(f, kIROp_ReadNoneDecoration);
            return true;
        }
        return false;
    }
};

bool propagateFuncPropertiesImpl(IRModule* module, FuncPropertyPropagationContext* context)
{
    bool result = false;
    List<IRFunc*> workList;
    HashSet<IRFunc*> workListSet;

    auto addToWorkList = [&](IRFunc* f)
    {
        if (workListSet.add(f))
            workList.add(f);
    };
    auto addCallersToWorkList = [&](IRFunc* f)
    {
        if (auto g = findOuterGeneric(f))
        {
            for (auto use = g->firstUse; use; use = use->nextUse)
            {
                if (use->getUser()->getOp() == kIROp_Specialize)
                {
                    auto specialize = use->getUser();
                    for (auto iuse = specialize->firstUse; iuse; iuse = iuse->nextUse)
                    {
                        if (auto userFunc = getParentFunc(iuse->getUser()))
                            addToWorkList(userFunc);
                    }
                }
            }
            return;
        }
        for (auto use = f->firstUse; use; use = use->nextUse)
        {
            if (use->getUser()->getOp() == kIROp_Call)
            {
                if (auto userFunc = getParentFunc(use->getUser()))
                    addToWorkList(userFunc);
            }
        }
    };
    for (;;)
    {
        bool changed = false;
        workList.clear();
        workListSet.clear();

        // Add side effect free functions and their transitive callers to work list.
        for (auto inst : module->getGlobalInsts())
        {
            auto genericInst = as<IRGeneric>(inst);
            if (genericInst)
            {
                inst = findGenericReturnVal(genericInst);
            }
            if (auto func = as<IRFunc>(inst))
            {
                if (context->isInitialFunc(func))
                {
                    addCallersToWorkList(func);
                }
            }
        }

        // Add remaining functions to work list.
        for (auto inst : module->getGlobalInsts())
        {
            auto genericInst = as<IRGeneric>(inst);
            if (genericInst)
            {
                inst = findGenericReturnVal(genericInst);
            }
            if (auto func = as<IRFunc>(inst))
            {
                addToWorkList(func);
            }
        }

        IRBuilder builder(module);

        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto f = workList[i];
            if (!context->canProcess(f))
                continue;

            // Never propagate to functions without a body.
            if (f->getFirstBlock() == nullptr)
                continue;

            if (context->propagate(builder, f))
            {
                addCallersToWorkList(f);
                changed = true;
            }
        }
        result |= changed;
        if (!changed)
            break;
    }
    return result;
}

class NoSideEffectFuncPropertyPropagationContext : public FuncPropertyPropagationContext
{
public:
    virtual bool canProcess(IRFunc* f) override
    {
        // If the func has already been marked with any decorations, skip.
        for (auto decoration : f->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_ReadNoneDecoration:
            case kIROp_NoSideEffectDecoration:
            case kIROp_TargetIntrinsicDecoration:
                return false;
            }
        }
        return true;
    }
    virtual bool isInitialFunc(IRFunc* f) override
    {
        // If the func has already been marked with any decorations, skip.
        for (auto decoration : f->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_ReadNoneDecoration:
            case kIROp_NoSideEffectDecoration:
                return true;
            }
        }
        return false;
    }
    virtual bool propagate(IRBuilder& builder, IRFunc* f) override
    {
        // A `NoSideEffect` function may read program state but cannot modify state visible outside
        // the function. We use the generic side-effect query for ordinary instructions. For the
        // instructions that need custom handling, we require each call to target a `NoSideEffect`
        // callee and retain the conservative rule that their operands cannot name global or
        // otherwise unknown mutable storage. As in the `ReadNone` analysis, we preserve the
        // existing conservative treatment of in-block debug instructions because dead-code
        // elimination has no separate debug-liveness rule.
        bool preventsNoSideEffect = false;
        for (auto block : f->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (!doesOpRequireCustomFuncPropertyAnalysis(inst->getOp()))
                {
                    if (inst->mightHaveSideEffects())
                    {
                        preventsNoSideEffect = true;
                        break;
                    }
                    continue;
                }

                if (auto call = as<IRCall>(inst))
                {
                    if (!isNoSideEffectCallee(call->getCallee()))
                    {
                        preventsNoSideEffect = true;
                        break;
                    }
                }

                if (doesInstUseGlobalOrUnknownMutableAddress(f, inst))
                {
                    preventsNoSideEffect = true;
                    break;
                }
            }
            if (preventsNoSideEffect)
                break;
        }
        if (!preventsNoSideEffect)
        {
            builder.addDecoration(f, kIROp_NoSideEffectDecoration);
            return true;
        }
        return false;
    }
};

bool propagateFuncProperties(IRModule* module)
{
    ReadNoneFuncPropertyPropagationContext readNoneContext;
    bool changed = propagateFuncPropertiesImpl(module, &readNoneContext);

    NoSideEffectFuncPropertyPropagationContext noSideEffectContext;
    changed |= propagateFuncPropertiesImpl(module, &noSideEffectContext);

    return changed;
}

bool propagatePropertiesForSingleFunc(IRModule* module, IRFunc* f)
{
    ReadNoneFuncPropertyPropagationContext readNoneContext;
    bool changed = false;
    IRBuilder builder(module);
    if (readNoneContext.canProcess(f))
        changed |= readNoneContext.propagate(builder, f);

    NoSideEffectFuncPropertyPropagationContext noSideEffectContext;
    if (noSideEffectContext.canProcess(f))
        changed |= noSideEffectContext.propagate(builder, f);

    return changed;
}
} // namespace Slang
