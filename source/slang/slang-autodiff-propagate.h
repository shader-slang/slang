// slang-ir-autodiff-propagate.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

#include "slang-ir-autodiff.h"

namespace Slang
{

bool isDifferentialInst(IRInst* inst)
{
    return inst->findDecoration<IRDifferentialInstDecoration>();
}

struct DiffPropagationPass : InstPassBase
{
    AutoDiffSharedContext*                  autodiffContext;

    DiffPropagationPass(AutoDiffSharedContext* autodiffContext) : 
        autodiffContext(autodiffContext),
        InstPassBase(autodiffContext->moduleInst->getModule())
    {

    }


    bool shouldInstBeMarkedDifferential(IRInst* inst)
    {
        for (UIndex ii = 0; ii < inst->getOperandCount(); ii ++)
        {
            if (isDifferentialInst(inst->getOperand(ii)))
            {
                return true;   
            }
        }

        return false;
    }

    void addPendingUsersToWorkList(IRInst* inst)
    {
        auto use = inst->firstUse;
        while (use)
        {
            if (!isDifferentialInst(use->getUser()))
            {
                addToWorkList(use->getUser());
            }
            use = use->nextUse;
        }
    }

    // Propagate IRDifferentialInstDecoration for all children of instWithChildren.
    void propagateDiffInstDecoration(IRBuilder* builder, IRInst* instWithChildren)
    {
        // Mark 'GetDifferential' insts as differential.
        processChildInstsOfType<IRDifferentialPairGetDifferential>(
            kIROp_DifferentialPairGetDifferential, 
            instWithChildren, 
            [&](IRDifferentialPairGetDifferential* getDifferentialInst)
            {
                builder->markInstAsDifferential(getDifferentialInst);
            });


        workList.clear();
        workListSet.Clear();

        // Add the marked insts to the work list.
        for (auto child = instWithChildren->getFirstChild(); child; child = child->getNextInst())
        {
            // Look for insts marked as differential.
            if (isDifferentialInst(child))
                addPendingUsersToWorkList(child);
        }

        // Propagate to all users..
        while (workList.getCount() != 0)
        {
            IRInst* inst = pop();

            // Skip if this is already a differential inst.
            if (isDifferentialInst(inst))
            {
                continue;
            }

            if (shouldInstBeMarkedDifferential(inst))
            {
                builder->markInstAsDifferential(inst);
                addPendingUsersToWorkList(inst);
            }
        }
    }
};

}