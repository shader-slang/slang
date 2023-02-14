#include "slang-ir-insts.h"

namespace Slang
{
    void SharedIRBuilder::deduplicateAndRebuildGlobalNumberingMap()
    {
    }

    void SharedIRBuilder::replaceGlobalInst(IRInst* oldInst, IRInst* newInst)
    {
        oldInst->replaceUsesWith(newInst);
    }

    void SharedIRBuilder::removeHoistableInstFromGlobalNumberingMap(IRInst* instToRemove)
    {
        HashSet<IRInst*> userWorkListSet;
        List<IRInst*> userWorkList;
        auto addToWorkList = [&](IRInst* i)
        {
            if (userWorkListSet.Add(i))
                userWorkList.add(i);
        };
        addToWorkList(instToRemove);
        for (Index i = 0; i < userWorkList.getCount(); i++)
        {
            auto inst = userWorkList[i];
            if (getIROpInfo(inst->getOp()).isHoistable())
            {
                _removeGlobalNumberingEntry(inst);
                for (auto use = inst->firstUse; use; use = use->nextUse)
                {
                    addToWorkList(use->getUser());
                }
            }
        }
    }

    void addHoistableInst(
        IRBuilder* builder,
        IRInst* inst);

    void SharedIRBuilder::tryHoistInst(IRInst* inst)
    {
        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;
        workList.add(inst);
        workListSet.Add(inst);
        IRBuilder builder(inst->getModule());

        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto item = workList[i];

            // Does inst no longer depend on anything defined locally?
            // If so we should hoist it.
            bool shouldHoist = false;
            for (UInt a = 0; a < item->getOperandCount(); a++)
            {
                auto opParent = item->getOperand(a)->getParent();
                if (opParent != item->getParent())
                {
                    shouldHoist = true;
                    break;
                }
            }

            // Hoisting this inst 
            if (shouldHoist)
            {
                item->removeFromParent();
                addHoistableInst(&builder, item);

                // Continue to consider all users for hoisting.
                for (auto use = item->firstUse; use; use = use->nextUse)
                {
                    if (getIROpInfo(use->getUser()->getOp()).isHoistable())
                    {
                        if (workListSet.Add(use->getUser()))
                            workList.add(use->getUser());
                    }
                }
            }
        }
    }
}
