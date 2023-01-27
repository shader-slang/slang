// slang-ir-ssa-register-allocate.cpp
#include "slang-ir-ssa-register-allocate.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-dominators.h"


namespace Slang {

// A context for computing and caching reachability between blocks on the CFG.
struct ReachabilityContext
{
    struct BlockPair
    {
        IRBlock* first;
        IRBlock* second;
        HashCode getHashCode()
        {
            Hasher h;
            h.hashValue(first);
            h.hashValue(second);
            return h.getResult();
        }
        bool operator == (const BlockPair& other)
        {
            return first == other.first && second == other.second;
        }
    };
    Dictionary<BlockPair, bool> reachabilityResults;

    List<IRBlock*> workList;
    HashSet<IRBlock*> reachableBlocks;

    // Computes whether block1 can reach block2.
    // A block is considered not reachable from itself unless there is a backedge in the CFG.
    bool computeReachability(IRBlock* block1, IRBlock* block2)
    {
        workList.clear();
        reachableBlocks.Clear();
        workList.add(block1);
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto src = workList[i];
            for (auto successor : src->getSuccessors())
            {
                if (successor == block2)
                    return true;
                if (reachableBlocks.Add(successor))
                    workList.add(successor);
            }
        }
        return false;
    }

    bool isBlockReachable(IRBlock* from, IRBlock* to)
    {
        BlockPair pair;
        pair.first = from;
        pair.second = to;
        bool result = false;
        if (reachabilityResults.TryGetValue(pair, result))
            return result;
        result = computeReachability(from, to);
        reachabilityResults[pair] = result;
        return result;
    }

    bool isInstReachable(IRInst* inst1, IRInst* inst2)
    {
        if (isBlockReachable(as<IRBlock>(inst1->getParent()), as<IRBlock>(inst2->getParent())))
            return true;

        // If the parent blocks are not reachable, but inst1 and inst2 are in the same block,
        // we test if inst2 appears after inst1.
        if (inst1->getParent() == inst2->getParent())
        {
            for (auto inst = inst1->getNextInst(); inst; inst = inst->getNextInst())
            {
                if (inst == inst2)
                    return true;
            }
        }

        return false;
    }
};

struct RegisterAllocateContext
{
    OrderedDictionary<IRType*, List<RefPtr<RegisterInfo>>> mapTypeToRegisterList;
    List<RefPtr<RegisterInfo>>& getRegisterListForType(IRType* type)
    {
        if (auto list = mapTypeToRegisterList.TryGetValue(type))
        {
            return *list;
        }
        mapTypeToRegisterList[type] = List<RefPtr<RegisterInfo>>();
        return mapTypeToRegisterList[type].GetValue();
    }

    void assignInstToNewRegister(List<RefPtr<RegisterInfo>>& regList, IRInst* inst)
    {
        auto reg = new RegisterInfo();
        reg->type = inst->getFullType();
        reg->insts.add(inst);
        regList.add(reg);
    }

    bool areInstsPreferredToBeCoalescedImpl(IRInst* inst0, IRInst* inst1)
    {
        switch (inst1->getOp())
        {
        case kIROp_UpdateElement:
            if (inst0 == inst1->getOperand(0))
                return true;
            break;
        default:
            break;
        }

        // If isnts have the same name, prefer to coalesce them.
        auto name1 = inst0->findDecoration<IRNameHintDecoration>();
        auto name2 = inst1->findDecoration<IRNameHintDecoration>();
        if (name1 && name2 && name1->getName() == name2->getName())
            return true;

        return false;
    }
    bool areInstsPreferredToBeCoalesced(IRInst* inst0, IRInst* inst1)
    {
        return areInstsPreferredToBeCoalescedImpl(inst0, inst1) ||
               areInstsPreferredToBeCoalescedImpl(inst1, inst0);
    }

    bool isRegisterPreferred(RegisterInfo* existingRegister, RegisterInfo* newRegister, IRInst* inst)
    {
        int preferredCountExistingReg = 0;
        int preferredCountNewReg = 0;
        for (auto existingInst : existingRegister->insts)
        {
            if (areInstsPreferredToBeCoalesced(existingInst, inst))
                preferredCountExistingReg++;
        }
        for (auto existingInst : newRegister->insts)
        {
            if (areInstsPreferredToBeCoalesced(existingInst, inst))
                preferredCountNewReg++;
        }
        return preferredCountNewReg > preferredCountExistingReg;
    }

    bool canCoalesce(IRInst* inst1, IRInst* inst2)
    {
        // If two insts are coming from two separate user defined names, don't coalesce them into
        // the same register.
        auto name1 = inst1->findDecoration<IRNameHintDecoration>();
        auto name2 = inst2->findDecoration<IRNameHintDecoration>();
        
        if (name1 && !name2 || !name1 && name2)
            return false;

        if (!name1 || !name2)
            return true;
        if (name1->getName() != name2->getName())
            return false;
        return true;
    }

    RegisterAllocationResult allocateRegisters(IRGlobalValueWithCode* func, RefPtr<IRDominatorTree>& inOutDom)
    {
        ReachabilityContext reachabilityContext;
        mapTypeToRegisterList.Clear();

        auto dom = computeDominatorTree(func);
        inOutDom = dom;

        // Note that if inst A does not dominate inst B, then A can't be alive at B.
        // Therefore we only need to test interference against insts that dominates the
        // current inst.
        // 
        // We can visit the dominance tree in pre-order and assign insts to registers.
        // This order allows us to easily track what is dominating the current inst.

        // We track the insts dominating the current location in a stack.
        List<IRInst*> dominatingInsts;
        HashSet<IRInst*> dominatingInstSet;

        struct WorkStackItem
        {
            IRBlock* block;
            Index dominatingInstCount;
            WorkStackItem() = default;
            WorkStackItem(IRBlock* inBlock, Index inDominatingInstCount)
            {
                block = inBlock;
                dominatingInstCount = inDominatingInstCount;
            }
        };
        List<WorkStackItem> workStack;
        workStack.add(WorkStackItem(func->getFirstBlock(), 0));

        while (workStack.getCount())
        {
            auto item = workStack.getLast();
            workStack.removeLast();

            // Pop dominatingInst stack to correct location.
            for (Index i = item.dominatingInstCount; i < dominatingInsts.getCount(); i++)
                dominatingInstSet.Remove(dominatingInsts[i]);
            dominatingInsts.setCount(item.dominatingInstCount);

            for (auto inst : item.block->getChildren())
            {
                if (!instNeedsProcessing(func, inst))
                    continue;
                // This is an inst we need to allocate register for.
                // Find register list for this type.
                auto& registers = getRegisterListForType(inst->getFullType());
                RegisterInfo* allocatedReg = nullptr;
                for (auto reg : registers)
                {
                    // Can we assign inst to this reg?
                    // We answer this by checking if any insts already assigned
                    // to this register is alive. If none are alive we can assign
                    // the register.
                    bool hasInterference = false;
                    for (auto existingInst : reg->insts)
                    {
                        // If `existingInst` does not dominate `inst`, it
                        // can't be alive here and during the entire life-time of the `inst`.
                        // This means that `inst` and `existingInst` won't interfere.
                        if (!dominatingInstSet.Contains(existingInst))
                            continue;

                        // If `existingInst` does dominate `inst`, we need to check all
                        // its use sites U to see if there is a path from `inst` to U.
                        // The idea is that is `existingInst` is never used anywhere after
                        // `inst`, then its lifetime ended before `inst` is defined, so it
                        // is still fine to place them in the same register.
                        for (auto use = existingInst->firstUse; use; use = use->nextUse)
                        {
                            if (use->getUser() == inst)
                                continue;

                            if (!canCoalesce(existingInst, inst) ||
                                reachabilityContext.isInstReachable(inst, use->getUser()))
                            {
                                hasInterference = true;
                                goto endRegInstCheck;
                            }
                        }
                    }
                endRegInstCheck:;
                    if (!hasInterference)
                    {
                        if (!allocatedReg || isRegisterPreferred(allocatedReg, reg, inst))
                        {
                            allocatedReg = reg;
                        }
                    }
                }
                if (!allocatedReg)
                {
                    assignInstToNewRegister(registers, inst);
                }
                else
                {
                    allocatedReg->insts.add(inst);
                }
                dominatingInsts.add(inst);
                dominatingInstSet.Add(inst);
            }

            // Recursively visit idom children.
            for (auto idomChild : dom->getImmediatelyDominatedBlocks(item.block))
            {
                workStack.add(WorkStackItem(idomChild, dominatingInsts.getCount()));
            }
        }

        RegisterAllocationResult result;
        result.mapTypeToRegisterList = _Move(mapTypeToRegisterList);
        for (auto& regList : result.mapTypeToRegisterList)
        {
            for (auto reg : regList.Value)
            {
                for (auto inst : reg->insts)
                {
                    result.mapInstToRegister[inst] = reg;
                }
            }
        }
        return result;
    }
    bool instNeedsProcessing(IRGlobalValueWithCode* func, IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_Param:
            if (inst->getParent() == func->getFirstBlock())
                return false;
            return true;
        case kIROp_UpdateElement:
            return true;
        default:
            return false;
        }
    }
    bool needProcessing(IRGlobalValueWithCode* func)
    {
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (instNeedsProcessing(func, inst))
                    return true;
            }
        }
        return false;
    }
};

RegisterAllocationResult allocateRegistersForFunc(IRGlobalValueWithCode* func, RefPtr<IRDominatorTree>& inOutDom)
{
    RegisterAllocateContext context;
    if (context.needProcessing(func))
        return context.allocateRegisters(func, inOutDom);
    return RegisterAllocationResult();
}

}
