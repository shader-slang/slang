#include "slang-ir-redundancy-removal.h"
#include "slang-ir-dominators.h"
#include "slang-ir-util.h"

namespace Slang
{

struct RedundancyRemovalContext
{
    RefPtr<IRDominatorTree> dom;
    bool isMovableInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Div:
        case kIROp_FRem:
        case kIROp_IRem:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_Not:
        case kIROp_Neg:
        case kIROp_FieldExtract:
        case kIROp_FieldAddress:
        case kIROp_GetElement:
        case kIROp_GetElementPtr:
        case kIROp_UpdateElement:
        case kIROp_Specialize:
        case kIROp_LookupWitness:
        case kIROp_OptionalHasValue:
        case kIROp_GetOptionalValue:
        case kIROp_MakeOptionalValue:
        case kIROp_MakeTuple:
        case kIROp_GetTupleElement:
        case kIROp_MakeStruct:
        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
        case kIROp_MakeVector:
        case kIROp_MakeMatrix:
        case kIROp_MakeMatrixFromScalar:
        case kIROp_MakeVectorFromScalar:
        case kIROp_swizzle:
        case kIROp_swizzleSet:
        case kIROp_MatrixReshape:
        case kIROp_MakeString:
        case kIROp_MakeResultError:
        case kIROp_MakeResultValue:
        case kIROp_GetResultError:
        case kIROp_GetResultValue:
        case kIROp_CastFloatToInt:
        case kIROp_CastIntToFloat:
        case kIROp_CastIntToPtr:
        case kIROp_CastPtrToBool:
        case kIROp_CastPtrToInt:
        case kIROp_BitAnd:
        case kIROp_BitNot:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_BitCast:
        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_Reinterpret:
        case kIROp_Greater:
        case kIROp_Less:
        case kIROp_Geq:
        case kIROp_Leq:
        case kIROp_Neq:
        case kIROp_Eql:
        case kIROp_ExtractExistentialType:
        case kIROp_ExtractExistentialValue:
        case kIROp_ExtractExistentialWitnessTable:
        case kIROp_PtrType:
        case kIROp_ArrayType:
        case kIROp_FuncType:
        case kIROp_InOutType:
        case kIROp_OutType:
            return true;
        case kIROp_Call:
            return isPureFunctionalCall(as<IRCall>(inst));
        case kIROp_Load:
            // Load is generally not movable, an exception is loading a global constant buffer.
            if (auto load = as<IRLoad>(inst))
            {
                auto addrType = load->getPtr()->getDataType();
                switch (addrType->getOp())
                {
                case kIROp_ConstantBufferType:
                case kIROp_ParameterBlockType:
                    return true;
                default:
                    break;
                }
            }
            return false;
        default:
            return false;
        }
    }

    bool tryHoistInstToOuterMostLoop(IRGlobalValueWithCode* func, IRInst* inst)
    {
        bool changed = false;
        for (auto parentBlock = dom->getImmediateDominator(as<IRBlock>(inst->getParent()));
             parentBlock;
             parentBlock = dom->getImmediateDominator(parentBlock))
        {
            auto terminatorInst = parentBlock->getTerminator();
            if (terminatorInst->getOp() == kIROp_loop)
            {
                // Consider hoisting the inst into this block.
                // This is only possible if all operands of the inst are dominating `parentBlock`.
                bool canHoist = true;
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    auto operand = inst->getOperand(i);
                    if (getParentFunc(operand) != func)
                    {
                        // Global value won't prevent hoisting.
                        continue;
                    }
                    auto operandParent = as<IRBlock>(operand->getParent());
                    if (!operandParent)
                    {
                        canHoist = false;
                        break;
                    }
                    canHoist = dom->dominates(operandParent, parentBlock);
                    if (!canHoist)
                        break;
                }
                if (!canHoist)
                    break;

                // Move inst to parentBlock.
                inst->insertBefore(terminatorInst);
                changed = true;

                // Continue to consider outer hoisting positions.
            }
        }
        return changed;
    }

    bool removeRedundancyInBlock(DeduplicateContext& deduplicateContext, IRGlobalValueWithCode* func, IRBlock* block)
    {
        bool result = false;
        for (auto instP : block->getModifiableChildren())
        {
            auto resultInst = deduplicateContext.deduplicate(instP, [&](IRInst* inst)
                {
                    auto parentBlock = as<IRBlock>(inst->getParent());
                    if (!parentBlock)
                        return false;
                    if (dom->isUnreachable(parentBlock))
                        return false;
                    return isMovableInst(inst);
                });
            if (resultInst != instP)
            {
                instP->replaceUsesWith(resultInst);
                instP->removeAndDeallocate();
                result = true;
            }
            else if (isMovableInst(resultInst))
            {
                // This inst is unique, we should consider hoisting it
                // if it is inside a loop.
                result |= tryHoistInstToOuterMostLoop(func, resultInst);
            }
        }
        for (auto child : dom->getImmediatelyDominatedBlocks(block))
        {
            DeduplicateContext subContext;
            subContext.deduplicateMap = deduplicateContext.deduplicateMap;
            result |= removeRedundancyInBlock(subContext, func, child);
        }
        return result;
    }
};

bool removeRedundancy(IRModule* module)
{
    bool changed = false;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto genericInst = as<IRGeneric>(inst))
        {
            removeRedundancyInFunc(genericInst);
            inst = findGenericReturnVal(genericInst);
        }
        if (auto func = as<IRFunc>(inst))
        {
            changed |= removeRedundancyInFunc(func);
        }
    }
    return changed;
}

bool removeRedundancyInFunc(IRGlobalValueWithCode* func)
{
    auto root = func->getFirstBlock();
    if (!root)
        return false;

    RedundancyRemovalContext context;
    context.dom = computeDominatorTree(func);
    DeduplicateContext deduplicateCtx;
    bool result = context.removeRedundancyInBlock(deduplicateCtx, func, root);
    if (auto normalFunc = as<IRFunc>(func))
    {
        result |= eliminateRedundantLoadStore(normalFunc);
    }
    return result;
}

static IRInst* _getRootVar(IRInst* inst)
{
    while (inst)
    {
        switch (inst->getOp())
        {
        case kIROp_FieldAddress:
        case kIROp_GetElementPtr:
            inst = inst->getOperand(0);
            break;
        default:
            return inst;
        }
    }
    return inst;
}

bool tryRemoveRedundantStore(IRGlobalValueWithCode* func, IRStore* store)
{
    // We perform a quick and conservative check:
    // A store is redundant if it is followed by another store to the same address in
    // the same basic block, and there are no instructions that may use any addresses
    // related to this address.
    bool hasAddrUse = false;
    bool hasOverridingStore = false;

    // Stores to global variables will never get removed.
    auto rootVar = _getRootVar(store->getPtr());
    if (!isChildInstOf(rootVar, func))
        return false;

    // A store can be removed if it stores into a local variable
    // that has no other uses than store.
    if (const auto varInst = as<IRVar>(rootVar))
    {
        bool hasNonStoreUse = false;
        // If the entire access chain doesn't non-store use, we can safely remove it.
        HashSet<IRInst*> knownAccessChain;
        for (auto accessChain = store->getPtr(); accessChain;)
        {
            knownAccessChain.add(accessChain);
            for (auto use = accessChain->firstUse; use; use = use->nextUse)
            {
                if (as<IRDecoration>(use->getUser()))
                    continue;
                if (knownAccessChain.contains(use->getUser()))
                    continue;
                if (use->getUser()->getOp() == kIROp_Store && 
                    use == use->getUser()->getOperands())
                {
                    continue;
                }
                hasNonStoreUse = true;
                break;
            }
            if (hasNonStoreUse)
                break;
            switch (accessChain->getOp())
            {
            case kIROp_GetElementPtr:
            case kIROp_FieldAddress:
                accessChain = accessChain->getOperand(0);
                continue;
            default:
                break;
            }
            break;
        }
        if (!hasNonStoreUse)
        {
            store->removeAndDeallocate();
            return true;
        }
    }

    // A store can be removed if there are subsequent stores to the same variable,
    // and there are no insts in between the stores that can read the variable.

    HashSet<IRBlock*> visitedBlocks;
    for (auto next = store->getNextInst(); next;)
    {
        if (auto nextStore = as<IRStore>(next))
        {
            if (nextStore->getPtr() == store->getPtr())
            {
                hasOverridingStore = true;
                break;
            }
        }

        // If we see any insts that have reads or modifies the address before seeing
        // an overriding store, don't remove the store.
        // We can make the test more accurate by collecting all addresses related to
        // the target address first, and only bail out if any of the related addresses
        // are involved.
        switch (next->getOp())
        {
        case kIROp_Load:
            if (canAddressesPotentiallyAlias(func, next->getOperand(0), store->getPtr()))
            {
                hasAddrUse = true;
            }
            break;
        default:
            if (canInstHaveSideEffectAtAddress(func, next, store->getPtr()))
            {
                hasAddrUse = true;
            }
            break;
        }
        if (hasAddrUse)
            break;

        // If we are at the end of the current block and see a unconditional branch,
        // we can follow the path and check the subsequent block.
        if (auto branch = as<IRUnconditionalBranch>(next))
        {
            auto nextBlock = branch->getTargetBlock();
            if (visitedBlocks.add(nextBlock))
            {
                next = nextBlock->getFirstInst();
                continue;
            }
        }
        next = next->getNextInst();
    }

    if (!hasAddrUse && hasOverridingStore)
    {
        store->removeAndDeallocate();
        return true;
    }

    // A store can be removed if it is a store into the same var, and there are
    // no side effects between the load of the var and the store of the var.
    if (auto load = as<IRLoad>(store->getVal()))
    {
        if (load->getPtr() == store->getPtr())
        {
            if (load->getParent() == store->getParent())
            {
                bool valueMayChange = false;
                for (auto inst = load->next; inst; inst = inst->next)
                {
                    if (inst == store)
                        break;
                    if (canInstHaveSideEffectAtAddress(func, inst, store->getPtr()))
                    {
                        valueMayChange = true;
                        break;
                    }
                }
                if (!valueMayChange)
                {
                    store->removeAndDeallocate();
                    return true;
                }
            }
        }
    }
    return false;
}

bool eliminateRedundantLoadStore(IRGlobalValueWithCode* func)
{
    bool changed = false;
    for (auto block : func->getBlocks())
    {
        for (auto inst = block->getFirstInst(); inst;)
        {
            auto nextInst = inst->getNextInst();
            if (auto load = as<IRLoad>(inst))
            {
                for (auto prev = inst->getPrevInst(); prev; prev = prev->getPrevInst())
                {
                    if (auto store = as<IRStore>(prev))
                    {
                        if (store->getPtr() == load->getPtr())
                        {
                            // If the load is preceeded by a store without any side-effect insts in-between, remove the load.
                            auto value = store->getVal();
                            load->replaceUsesWith(value);
                            load->removeAndDeallocate();
                            changed = true;
                            break;
                        }
                    }

                    if (canInstHaveSideEffectAtAddress(func, prev, load->getPtr()))
                    {
                        break;
                    }
                }
            }
            else if (auto store = as<IRStore>(inst))
            {
                changed |= tryRemoveRedundantStore(func, store);
            }
            inst = nextInst;
        }
    }
    return changed;
}

}
