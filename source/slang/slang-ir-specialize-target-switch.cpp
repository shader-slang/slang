#include "slang-ir-specialize-target-switch.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"
#include "slang-capability.h"
#include "slang-ir-dce.h"

namespace Slang
{
    void specializeTargetSwitch(TargetRequest* target, IRGlobalValueWithCode* code)
    {
        bool changed = false;
        for (auto block : code->getBlocks())
        {
            if (auto targetSwitch = as<IRTargetSwitch>(block->getTerminator()))
            {
                CapabilitySet bestCapSet = CapabilitySet::makeInvalid();
                IRBlock* targetBlock = nullptr;
                for (UInt i = 0; i < targetSwitch->getCaseCount(); i++)
                {
                    auto cap = (CapabilityName)getIntVal(targetSwitch->getCaseValue(i));
                    if (target->getTargetCaps().isIncompatibleWith(cap))
                        continue;
                    CapabilitySet capSet;
                    if (cap == CapabilityName::Invalid)
                        capSet = CapabilitySet::makeEmpty();
                    else
                        capSet = CapabilitySet(cap);
                    if (capSet.isBetterForTarget(bestCapSet, target->getTargetCaps()))
                    {
                        targetBlock = targetSwitch->getCaseBlock(i);
                        bestCapSet = capSet;
                    }
                }
                IRBuilder builder(targetSwitch);
                builder.setInsertBefore(targetSwitch);
                if (targetBlock)
                {
                    builder.emitBranch(targetBlock);
                }
                else
                {
                    builder.emitMissingReturn();
                }
                targetSwitch->removeAndDeallocate();
                changed = true;
            }
        }
        if (changed)
        {
            // Remove unreachable blocks after specialization.
            eliminateDeadCode(code);
        }
    }

    void specializeTargetSwitch(TargetRequest* target, IRModule* module)
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto code = as<IRGlobalValueWithCode>(globalInst))
            {
                specializeTargetSwitch(target, code);
                if (auto gen = as<IRGeneric>(code))
                {
                    auto retVal = findGenericReturnVal(gen);
                    if (auto innerCode = as<IRGlobalValueWithCode>(retVal))
                    {
                        specializeTargetSwitch(target, innerCode);
                    }
                }
            }
        }
    }

}
