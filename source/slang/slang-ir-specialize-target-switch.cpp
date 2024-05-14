#include "slang-ir-specialize-target-switch.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"
#include "slang-capability.h"
#include "slang-ir-dce.h"

namespace Slang
{
    void specializeTargetSwitch(TargetRequest* target, IRGlobalValueWithCode* code, DiagnosticSink* sink)
    {
        bool changed = false;
        for (auto block : code->getBlocks())
        {
            if (auto targetSwitch = as<IRTargetSwitch>(block->getTerminator()))
            {
                bool isEqual;
                CapabilitySet bestCapSet = CapabilitySet::makeInvalid();
                IRBlock* targetBlock = nullptr;
                for (UInt i = 0; i < targetSwitch->getCaseCount(); i++)
                {
                    auto cap = (CapabilityName)getIntVal(targetSwitch->getCaseValue(i));
                    if (target->getTargetCaps().isIncompatibleWith(cap))
                        continue;
                    CapabilitySet capSet;
                    if (cap == CapabilityName::Invalid) // `default` case
                        capSet = CapabilitySet::makeEmpty();
                    else
                        capSet = CapabilitySet(cap);
                    bool isBetterForTarget = capSet.isBetterForTarget(bestCapSet, target->getTargetCaps(), isEqual);
                    if (isBetterForTarget && target->getTargetCaps().implies(capSet))
                    {
                        // Now check if bestCapSet contains targetCaps. If it does not then this is an invalid target
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
                    sink->diagnose(targetSwitch->sourceLoc, Diagnostics::profileIncompatibleWithTargetSwitch, target->getTargetCaps());
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

    void specializeTargetSwitch(TargetRequest* target, IRModule* module, DiagnosticSink* sink)
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto code = as<IRGlobalValueWithCode>(globalInst))
            {
                specializeTargetSwitch(target, code, sink);
                if (auto gen = as<IRGeneric>(code))
                {
                    auto retVal = findGenericReturnVal(gen);
                    if (auto innerCode = as<IRGlobalValueWithCode>(retVal))
                    {
                        specializeTargetSwitch(target, innerCode, sink);
                    }
                }
            }
        }
    }

}
