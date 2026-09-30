#include "slang-ir-specialize-target-switch.h"

#include "slang-capability.h"
#include "slang-compiler.h"
#include "slang-ir-dce.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
// Select available branches first. Switches needing an incompatible-profile diagnostic remain
// in their original IR form until module-wide DCE determines whether the program reaches them.
static void specializeTargetSwitchInCode(
    TargetRequest* target,
    IRGlobalValueWithCode* code,
    DiagnosticSink* sink,
    bool diagnoseUnavailableTargets)
{
    if (auto gen = as<IRGeneric>(code))
    {
        auto retVal = findGenericReturnVal(gen);
        if (auto innerCode = as<IRGlobalValueWithCode>(retVal))
        {
            specializeTargetSwitchInCode(target, innerCode, sink, diagnoseUnavailableTargets);
            return;
        }
    }

    bool changed = false;
    for (auto block : code->getBlocks())
    {
        bool failedImplies = false;
        if (auto targetSwitch = as<IRTargetSwitch>(block->getTerminator()))
        {
            bool isEqual;
            CapabilitySet bestCapSet = CapabilitySet::makeInvalid();
            IRBlock* targetBlock = nullptr;
            CapabilitySet::ImpliesReturnFlags impliesReturnType =
                CapabilitySet::ImpliesReturnFlags::NotImplied;
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
                bool isBetterForTarget =
                    capSet.isBetterForTarget(bestCapSet, target->getTargetCaps(), isEqual);
                if (isBetterForTarget)
                {
                    impliesReturnType = target->getTargetCaps().atLeastOneSetImpliedInOther(capSet);
                    bool targetImpliesCapSet =
                        ((int)impliesReturnType & (int)CapabilitySet::ImpliesReturnFlags::Implied ||
                         capSet.isEmpty());
                    if (targetImpliesCapSet)
                    {
                        // Now check if bestCapSet contains targetCaps. If it does not then this is
                        // an invalid target
                        targetBlock = targetSwitch->getCaseBlock(i);
                        bestCapSet = capSet;
                    }
                    else
                        failedImplies = true;
                }
            }
            if (!targetBlock && failedImplies && !diagnoseUnavailableTargets)
                continue;

            IRBuilder builder(targetSwitch);
            builder.setInsertBefore(targetSwitch);
            if (targetBlock)
            {
                builder.emitBranch(targetBlock);
            }
            else
            {
                // only error if we have the chance of setting a valid target switch, but did not
                // due to incompatability within same `target` atom. Otherwise we will have an issue
                // when we process a `__target_switch() { case metal: return; }` for glsl targets.
                if (failedImplies)
                {
                    StringBuilder profileSb;
                    printDiagnosticArg(profileSb, target->getTargetCaps());
                    sink->diagnose(Diagnostics::ProfileIncompatibleWithTargetSwitch{
                        .profile = profileSb.produceString(),
                        .location = targetSwitch->sourceLoc,
                    });
                }
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
            specializeTargetSwitchInCode(target, code, sink, false);
        }
    }

    // Consider a helper called only from `case nvvm` in another function's target switch.
    // Linking clones both functions before selecting the caller's CUDA branch. Diagnosing the
    // helper at that point would reject a function that the selected program never calls.
    // Prune discarded blocks and their dependencies before diagnosing the remaining switches.
    // Keeping each failed IRTargetSwitch intact also avoids retaining pointers across DCE.
    eliminateDeadCode(module);
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto code = as<IRGlobalValueWithCode>(globalInst))
        {
            specializeTargetSwitchInCode(target, code, sink, true);
        }
    }
}

} // namespace Slang
