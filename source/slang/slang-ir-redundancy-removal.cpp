#include "slang-ir-redundancy-removal.h"

#include "slang-ir-dominators.h"
#include "slang-ir-simplify-cfg.h"
#include "slang-ir-util.h"

namespace Slang
{

struct RedundancyRemovalContext
{
    RefPtr<IRDominatorTree> dom;

    bool tryHoistInstToOuterMostLoop(IRGlobalValueWithCode* func, IRInst* inst)
    {
        bool changed = false;
        for (auto parentBlock = dom->getImmediateDominator(as<IRBlock>(inst->getParent()));
             parentBlock;
             parentBlock = dom->getImmediateDominator(parentBlock))
        {
            auto terminatorInst = parentBlock->getTerminator();
            if (auto loop = as<IRLoop>(terminatorInst))
            {
                // Don't bother hoisting if a loop has only a single trivial iteration.
                if (isTrivialSingleIterationLoop(dom, func, loop))
                    continue;

                // If `inst` is outside of the loop region, don't hoist it into the loop.
                if (dom->dominates(loop->getBreakBlock(), inst))
                    continue;

                // Consider hoisting the inst into this block.
                // This is only possible if all operands of the inst are dominating
                // `parentBlock`.
                bool canHoist = true;
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    auto operand = inst->getOperand(i);
                    if (!hasDescendent(func, operand))
                    {
                        // Only prevent hoisting from operands local to this function
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

    bool removeRedundancyInBlock(
        Dictionary<IRBlock*, DeduplicateContext>& mapBlockToDedupContext,
        IRGlobalValueWithCode* func,
        IRBlock* block,
        bool hoistLoopInvariantInsts)
    {
        bool result = false;
        auto& deduplicateContext = mapBlockToDedupContext.getValue(block);
        for (auto instP : block->getModifiableChildren())
        {
            auto resultInst = deduplicateContext.deduplicate(
                instP,
                [&](IRInst* inst)
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
                if (hoistLoopInvariantInsts)
                    result |= tryHoistInstToOuterMostLoop(func, resultInst);
            }
        }
        for (auto child : dom->getImmediatelyDominatedBlocks(block))
        {
            DeduplicateContext& subContext = mapBlockToDedupContext.getValue(child);
            subContext.deduplicateMap = deduplicateContext.deduplicateMap;
        }
        return result;
    }
};

bool removeRedundancy(IRModule* module, bool hoistLoopInvariantInsts)
{
    bool changed = false;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto genericInst = as<IRGeneric>(inst))
        {
            removeRedundancyInFunc(genericInst, hoistLoopInvariantInsts);
            inst = findGenericReturnVal(genericInst);
        }
        if (auto func = as<IRFunc>(inst))
        {
            changed |= removeRedundancyInFunc(func, hoistLoopInvariantInsts);
        }
    }
    return changed;
}

bool isAddressMutable(IRInst* inst)
{
    auto rootAddr = getRootAddr(inst);
    return !isPointerToImmutableLocation(rootAddr);
}

/// Eliminate redundant temporary variable copies in load-store patterns.
/// This optimization looks for patterns where a value is loaded from memory
/// and immediately stored to a temporary variable, which is then only used
/// in read-only contexts. In such cases, the temporary variable and the
/// load-store indirection can be eliminated by using the original memory
/// location directly.
/// Returns true if any changes were made.
static bool eliminateRedundantTemporaryCopyInFunc(IRFunc* func)
{
    // Consider the following IR pattern:
    // ```
    // let %temp = var
    // let %value = load(%sourcePtr)
    // store(%temp, %value)
    // ```
    // We can replace "%temp" with "%sourcePtr" without the load and store indirection
    // if "%temp" is used only in read-only contexts.

    bool overallChanged = false;
    for (bool changed = true; changed;)
    {
        changed = false;

        HashSet<IRInst*> toRemove;

        for (auto block : func->getBlocks())
        {
            for (auto blockInst : block->getChildren())
            {
                auto storeInst = as<IRStore>(blockInst);
                if (!storeInst)
                {
                    // We are interested only in IRStore.
                    continue;
                }

                auto storedValue = storeInst->getVal();
                auto destPtr = storeInst->getPtr();

                if (destPtr->getOp() != kIROp_Var)
                {
                    // Only optimize temporary variable.
                    // Don't optimize stores to permanent memory locations.
                    continue;
                }

                if (destPtr->findDecorationImpl(kIROp_DisableCopyEliminationDecoration))
                    continue;

                // Check if we're storing a load result
                auto loadInst = as<IRLoad>(storedValue);
                if (!loadInst)
                {
                    // Skip because only IRLoad is expected for the optimization.
                    continue;
                }

                auto loadPtr = loadInst->getPtr();

                if (isAddressMutable(loadPtr))
                {
                    // If the input is mutable, we cannot optimize,
                    // because any function calls may alter the content of the input
                    // and we cannot replace the temporary copy with a memory pointer.
                    continue;
                }

                // Storing address-sapce for later use.
                AddressSpace loadAddressSpace = AddressSpace::Generic;
                if (auto rootPtrType = as<IRPtrTypeBase>(getRootAddr(loadPtr)->getDataType()))
                {
                    loadAddressSpace = rootPtrType->getAddressSpace();
                }

                // Do not optimize loads from semantic parameters because some semantics have
                // builtin types that are vector types but pretend to be scalar types (e.g.,
                // SV_DispatchThreadID is used as 'int id' but maps to 'float3
                // gl_GlobalInvocationID'). The legalization step must remove the load instruction
                // to maintain this pretense, which breaks our load/store optimization assumptions.
                // Skip optimization when loading from semantics to let legalization handle the load
                // removal.
                if (auto param = as<IRParam>(loadPtr))
                    if (param->findDecoration<IRSemanticDecoration>())
                        continue;

                // Check all uses of the destination variable
                for (auto use = destPtr->firstUse; use; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (user == storeInst)
                    {
                        // Skip the store itself
                        continue; // check the next use
                    }

                    if (as<IRStore>(use->getUser()))
                    {
                        // We cannot optimize when the variable is reused
                        // with another store.
                        goto unsafeToOptimize;
                    }

                    if (as<IRLoad>(user))
                    {
                        // Allow loads because IRLoad is read-only operation
                        continue; // Check the next use
                    }

                    // For function calls, check if the pointer is treated as immutable.
                    if (auto call = as<IRCall>(user))
                    {
                        auto callee = call->getCallee();
                        auto funcInst = as<IRFunc>(callee);
                        if (!funcInst)
                            goto unsafeToOptimize;

                        UIndex argIndex = (UIndex)(use - call->getArgs());
                        SLANG_ASSERT(argIndex < call->getArgCount());
                        SLANG_ASSERT(call->getArg(argIndex) == destPtr);

                        IRParam* param = funcInst->getFirstParam();
                        for (UIndex i = 0; i < argIndex; i++)
                        {
                            if (param)
                                param = param->getNextParam();
                        }
                        if (nullptr == param)
                            goto unsafeToOptimize; // IRFunc might be incomplete yet

                        if (auto paramPtrType = as<IRBorrowInParamType>(param->getFullType()))
                        {
                            if (paramPtrType->getAddressSpace() != loadAddressSpace)
                                goto unsafeToOptimize; // incompatible address space

                            continue; // safe so far and check the next use
                        }
                        goto unsafeToOptimize; // must be const-ref
                    }

                    // TODO: there might be more cases that is safe to optimize
                    // We need to add more cases here as needed.

                    // If we get here, the pointer is used with an unexpected IR.
                    goto unsafeToOptimize;
                }

                // If we get here, all uses are safe to optimize.

                // Replace all uses of destPtr with loadedPtr
                destPtr->replaceUsesWith(loadPtr);

                // Mark instructions for removal
                toRemove.add(storeInst);
                toRemove.add(destPtr);

                // Note: loadInst might be still in use.
                // We need to rely on DCE to delete it if unused.

                changed = true;
                overallChanged = true;

            unsafeToOptimize:;
            }
        }

        // Remove marked instructions
        for (auto instToRemove : toRemove)
        {
            instToRemove->removeAndDeallocate();
        }
    }

    return overallChanged;
}

bool removeRedundancyInFunc(IRGlobalValueWithCode* func, bool hoistLoopInvariantInsts)
{
    auto root = func->getFirstBlock();
    if (!root)
        return false;

    RedundancyRemovalContext context;
    context.dom = computeDominatorTree(func);
    Dictionary<IRBlock*, DeduplicateContext> mapBlockToDeduplicateContext;
    for (auto block : func->getBlocks())
    {
        mapBlockToDeduplicateContext[block] = DeduplicateContext();
    }
    List<IRBlock*> workList, pendingWorkList;
    workList.add(root);
    bool result = false;
    while (workList.getCount())
    {
        for (auto block : workList)
        {
            result |= context.removeRedundancyInBlock(
                mapBlockToDeduplicateContext,
                func,
                block,
                hoistLoopInvariantInsts);

            for (auto child : context.dom->getImmediatelyDominatedBlocks(block))
            {
                pendingWorkList.add(child);
            }
        }
        workList.swapWith(pendingWorkList);
        pendingWorkList.clear();
    }
    if (auto normalFunc = as<IRFunc>(func))
    {
        result |= eliminateRedundantLoadStore(normalFunc);
        result |= eliminateRedundantTemporaryCopyInFunc(normalFunc);
    }
    return result;
}

// Remove IR definitions from all AvailableInDownstreamIR functions where the
// languages match what we're currently targetting,  as these functions are
// already defined in the embedded precompiled library.
void removeAvailableInDownstreamModuleDecorations(CodeGenTarget target, IRModule* module)
{
    List<IRInst*> toRemove;
    auto builder = IRBuilder(module);
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto funcInst = as<IRFunc>(globalInst))
        {
            if (auto dec = globalInst->findDecoration<IRAvailableInDownstreamIRDecoration>())
            {
                if ((dec->getTarget() == CodeGenTarget::DXIL && target == CodeGenTarget::HLSL) ||
                    (dec->getTarget() == target))
                {
                    // Gut the function definition, turning it into a declaration
                    for (auto block : funcInst->getBlocks())
                    {
                        toRemove.add(block);
                    }
                    builder.addDecoration(funcInst, kIROp_DownstreamModuleImportDecoration);
                }
            }
        }
    }
    for (auto inst : toRemove)
    {
        inst->removeAndDeallocate();
    }
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

// 0 is the most broad scope
static int getMemoryScopeOrder(MemoryScope scope)
{
    switch (scope)
    {
    case MemoryScope::CrossDevice:
        return 7;
    case MemoryScope::Device:
        return 6;
    case MemoryScope::QueueFamily:
        // https://docs.vulkan.org/spec/latest/chapters/shaders.html#shaders-scope-queue-family
        return 5;
    case MemoryScope::ShaderCall:
        // https://docs.vulkan.org/spec/latest/chapters/shaders.html#shaders-scope-shadercall
        return 4;
    case MemoryScope::Workgroup:
        return 3;
    case MemoryScope::Subgroup:
        return 2;
    case MemoryScope::Invocation:
    default:
        return 1;
    }
}

// Returns if MemoryScope x is a sub-set of y
static bool isMemoryScopeSubsetOf(MemoryScope x, MemoryScope y)
{
    return getMemoryScopeOrder(x) <= getMemoryScopeOrder(y);
}

// Inst's are relative to a memory scope, get that memory scope.
static MemoryScope getMemoryScopeOfLoadStore(IRInst* inst)
{
    SLANG_ASSERT(as<IRLoad>(inst) || as<IRStoreBase>(inst));
    auto memoryScope = inst->findAttr<IRMemoryScopeAttr>();
    if (!memoryScope)
        return MemoryScope::Invocation;
    return (MemoryScope)getIntVal(memoryScope->getMemoryScope());
}

bool tryRemoveRedundantStore(IRGlobalValueWithCode* func, IRStoreBase* store)
{
    // We perform a quick and conservative check:
    // A store is redundant if it is followed by another store to the same address in
    // the same basic block, and there are no instructions that may use any addresses
    // related to this address.
    bool hasAddrUse = false;
    bool hasOverridingStore = false;

    // Generally, we do not remove stores to global variables.
    // A special case is if there is a store into a thread-local global var,
    // and there are no other uses of the global var other than the store, we should be able to
    // eliminate the global var. This special case optimization is needed so we don't generate
    // code that stores a non-applicable builtin value into a thread-local global var that is not
    // actually used (e.g. sv_instanceindex).
    auto rootVar = _getRootVar(store->getPtr());
    if (!isChildInstOf(rootVar, func))
    {
        if (auto globalVar = as<IRGlobalVar>(store->getPtr()))
        {
            if (auto ptrType = globalVar->getDataType())
            {
                switch (ptrType->getAddressSpace())
                {
                case AddressSpace::ThreadLocal:
                    for (auto use = globalVar->firstUse; use; use = use->nextUse)
                    {
                        if (use->getUser() != store)
                            return false;
                    }
                    store->removeAndDeallocate();
                    globalVar->removeAndDeallocate();
                    return true;
                }
            }
        }
        return false;
    }

    // A store can be removed if it stores into a local variable
    // that has no other uses than store.
    if (const auto varInst = as<IRVar>(rootVar))
    {
        bool hasNonStoreUse = false;
        // If the entire access chain doesn't non-store use, we can safely remove it.
        InstHashSet knownAccessChain(func->getModule());
        for (auto accessChain = store->getPtr(); accessChain;)
        {
            knownAccessChain.add(accessChain);
            for (auto use = accessChain->firstUse; use; use = use->nextUse)
            {
                if (as<IRDecoration>(use->getUser()))
                    continue;
                if (knownAccessChain.contains(use->getUser()))
                    continue;
                if (use->getUser()->getOp() == kIROp_Store && use == use->getUser()->getOperands())
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

    // This store can be removed if there are subsequent stores to the same variable,
    // and there are no insts in between the stores that can read the variable.
    // Additionally, MemoryScope of the `store` must be a sub-set of `nextStore`,
    // otherwise we can not be certain that `nextStore` completely overwrites `store`.
    MemoryScope memoryScopeOfStore = getMemoryScopeOfLoadStore(store);
    HashSet<IRBlock*> visitedBlocks;
    for (auto next = store->getNextInst(); next;)
    {
        if (auto nextStore = as<IRStore>(next))
        {
            if (nextStore->getPtr() == store->getPtr() &&
                isMemoryScopeSubsetOf(memoryScopeOfStore, getMemoryScopeOfLoadStore(nextStore)))
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

// Checks if we can change or have a modified rootVar
// at some point.
bool isExternallyModifiableAddr(IRInst* rootVar)
{
    if (!rootVar)
        return false;

    auto ptr = as<IRBorrowInParamType>(rootVar->getDataType());
    if (!ptr)
        return true;

    // Only a UserPointer can potentially be modified and changed to point to a different address
    // if constRef. This may happen from a different thread even if constref to the current thread.
    auto addrSpace = ptr->getAddressSpace();
    if (addrSpace == AddressSpace::UserPointer)
        return true;

    return false;
}

bool tryRemoveRedundantLoad(IRGlobalValueWithCode* func, IRLoad* load)
{
    bool changed = false;

    // Get the memory scope we are operating on.
    MemoryScope memoryScopeOfLoad = getMemoryScopeOfLoadStore(load);

    // We can replace a load with a `Store->getVal()` if that store is a super-set
    // memory scope to our load.
    // Ex 1: Store into Workgroup, load from Invocation. Load will be equal to the Store.
    //
    // Ex 2: Store into Invocation, load from Workgroup. Load may/may-not be equal to the Store
    // since the cache managing the Workgroup scope may contain different data than the invocation.
    for (auto prev = load->getPrevInst(); prev; prev = prev->getPrevInst())
    {
        if (auto store = as<IRStore>(prev))
        {
            if (store->getPtr() == load->getPtr() &&
                isMemoryScopeSubsetOf(memoryScopeOfLoad, getMemoryScopeOfLoadStore(store)))
            {
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

    return changed;
}

bool eliminateRedundantLoadStore(IRGlobalValueWithCode* func)
{
    bool changed = false;
    for (auto block : func->getBlocks())
    {
        IRInst* nextInst = nullptr;
        for (auto inst = block->getFirstInst(); inst; inst = nextInst)
        {
            nextInst = inst->getNextInst();
            if (auto load = as<IRLoad>(inst))
            {
                changed |= tryRemoveRedundantLoad(func, load);
            }
            else if (auto store = as<IRStoreBase>(inst))
            {
                changed |= tryRemoveRedundantStore(func, store);
            }
            else if (auto getElementPtr = as<IRGetElementPtr>(inst))
            {
                auto rootAddr = getRootAddr(getElementPtr);
                if (isExternallyModifiableAddr(rootAddr))
                    continue;

                // GetElement(Load(GetElementPtr(x)))) ==> Load(GetElementPtr(GetElementPtr(x)))
                // The benefit is that any GetAddr(Load(...)) can then transitively be optimized
                // out.
                // This can only be done if we have no side-effects. `constref` never has
                // single-invocation side-effects.
                traverseUsers<IRLoad>(
                    getElementPtr,
                    [&](IRLoad* load)
                    {
                        traverseUsers<IRGetElement>(
                            load,
                            [&](IRGetElement* getElement)
                            {
                                // Only optimize if the load
                                // is the base
                                if (getElement->getBase() != load)
                                    return;

                                IRBuilder builder(getElement);
                                builder.setInsertBefore(getElement);
                                auto newGetElementPtr = builder.emitElementAddress(
                                    getElementPtr,
                                    getElement->getIndex());
                                auto newLoad = builder.emitLoad(newGetElementPtr);
                                getElement->replaceUsesWith(newLoad);
                                changed = true;
                            });
                    });
            }
            else if (auto fieldAddress = as<IRFieldAddress>(inst))
            {
                auto rootAddr = getRootAddr(fieldAddress);
                if (isExternallyModifiableAddr(rootAddr))
                    continue;

                // ExtractField(Load(GetFieldAddr(x)))) ==> Load(GetFieldAddr(GetFieldAddr(x)))
                // The benefit is that any GetAddr(Load(...)) can then transitively be optimized
                // out.
                // This can only be done if we have no side-effects. `constref` never has
                // single-invocation side-effects.
                traverseUsers<IRLoad>(
                    fieldAddress,
                    [&](IRLoad* load)
                    {
                        traverseUsers<IRFieldExtract>(
                            load,
                            [&](IRFieldExtract* fieldExtract)
                            {
                                // Only optimize if the load
                                // is the base; not strictly
                                // needed for field extract,
                                // but it will prevent future
                                // regressions if a field ever
                                // becomes a non-struct-key
                                if (fieldExtract->getBase() != load)
                                    return;

                                IRBuilder builder(fieldExtract);
                                builder.setInsertBefore(fieldExtract);
                                auto newGetFieldAddress = builder.emitFieldAddress(
                                    fieldAddress,
                                    fieldExtract->getField());
                                auto newLoad = builder.emitLoad(newGetFieldAddress);
                                fieldExtract->replaceUsesWith(newLoad);
                                changed = true;
                            });
                    });
            }
        }
    }
    return changed;
}

} // namespace Slang
