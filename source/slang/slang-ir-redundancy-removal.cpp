#include "slang-ir-redundancy-removal.h"
#include "slang-ir-dominators.h"
#include "slang-ir-util.h"

namespace Slang
{

struct RedundancyRemovalContext
{
    RefPtr<IRDominatorTree> dom;
    bool removeRedundancyInBlock(DeduplicateContext& deduplicateContext, IRBlock* block)
    {
        bool result = false;
        for (auto instP : block->getChildren())
        {
            auto resultInst = deduplicateContext.deduplicate(instP, [&](IRInst* inst)
                {
                    auto parentBlock = as<IRBlock>(inst->getParent());
                    if (!parentBlock)
                        return false;
                    if (dom->isUnreachable(parentBlock))
                        return false;

                    switch (inst->getOp())
                    {
                    case kIROp_Add:
                    case kIROp_Sub:
                    case kIROp_Mul:
                    case kIROp_Div:
                    case kIROp_Module:
                    case kIROp_Lsh:
                    case kIROp_Rsh:
                    case kIROp_And:
                    case kIROp_Or:
                    case kIROp_Not:
                    case kIROp_FieldExtract:
                    case kIROp_FieldAddress:
                    case kIROp_GetElement:
                    case kIROp_GetElementPtr:
                    case kIROp_UpdateElement:
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
                    case kIROp_Reinterpret:
                    case kIROp_Greater:
                    case kIROp_Less:
                    case kIROp_Geq:
                    case kIROp_Leq:
                    case kIROp_Neq:
                    case kIROp_Eql:
                        return true;
                    case kIROp_Call:
                        return isPureFunctionalCall(as<IRCall>(inst));
                    default:
                        return false;
                    }
                });
            if (resultInst != instP)
                result = true;
        }
        for (auto child : dom->getImmediatelyDominatedBlocks(block))
        {
            DeduplicateContext subContext;
            subContext.deduplicateMap = deduplicateContext.deduplicateMap;
            result |= removeRedundancyInBlock(subContext, child);
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
            changed |= eliminateRedundantLoadStore(func);
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
    return context.removeRedundancyInBlock(deduplicateCtx, root);
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
                // We perform a quick and conservative check:
                // A store is redundant if it is followed by another store to the same address in
                // the same basic block, and there are no instructions that may use any addresses
                // related to this address.
                bool hasAddrUse = false;
                bool hasOverridingStore = false;

                // Stores to global variables will never get removed.
                if (!isChildInstOf(store->getPtr(), func))
                    hasAddrUse = true;

                for (auto next = store->getNextInst(); next; next = next->getNextInst())
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
                }

                if (!hasAddrUse && hasOverridingStore)
                {
                    store->removeAndDeallocate();
                    changed = true;
                }
            }
            inst = nextInst;
        }
    }
    return changed;
}

}
