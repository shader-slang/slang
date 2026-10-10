#include "slang-ir-specialize-target-switch.h"

#include "core/slang-type-text-util.h"
#include "slang-capability.h"
#include "slang-compiler.h"
#include "slang-ir-dce.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
// A `__target_switch` with no case for the current target is legal as long as the code that
// contains it is never generated: the core module links such functions into every target and
// relies on DCE to remove the ones that are not called (e.g. `__sincos_metal` on CUDA). We
// therefore do not diagnose here. Instead we mark the `missingReturn` that replaces the switch,
// recording the switch location and the name of its function (which is lost once the function is
// inlined), so that `diagnoseReachableNoTargetCase` can report it if it survives to code
// generation.
static void markMissingReturnAsNoTargetCase(
    IRBuilder& builder,
    IRInst* missingReturn,
    IRGlobalValueWithCode* code,
    IRTargetSwitch* targetSwitch)
{
    StringBuilder funcName;
    printDiagnosticArg(funcName, code);
    missingReturn->sourceLoc = targetSwitch->sourceLoc;
    builder.addDecoration(
        missingReturn,
        kIROp_NoTargetCaseDecoration,
        builder.getStringValue(funcName.getUnownedSlice()));
}

void specializeTargetSwitch(
    TargetRequest* target,
    IRGlobalValueWithCode* code,
    DiagnosticSink* sink)
{
    if (auto gen = as<IRGeneric>(code))
    {
        auto retVal = findGenericReturnVal(gen);
        if (auto innerCode = as<IRGlobalValueWithCode>(retVal))
        {
            specializeTargetSwitch(target, innerCode, sink);
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
                auto missingReturn = builder.emitMissingReturn();
                if (!failedImplies)
                    markMissingReturnAsNoTargetCase(builder, missingReturn, code, targetSwitch);
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
        }
    }
}

void diagnoseReachableNoTargetCase(IRModule* module, TargetRequest* target, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        auto code = as<IRGlobalValueWithCode>(globalInst);
        if (!code)
            continue;
        for (auto block : code->getBlocks())
        {
            auto missingReturn = as<IRMissingReturn>(block->getTerminator());
            if (!missingReturn)
                continue;
            auto noTargetCase = missingReturn->findDecoration<IRNoTargetCaseDecoration>();
            if (!noTargetCase)
                continue;
            sink->diagnose(Diagnostics::TargetSwitchNoCaseForTarget{
                .funcName = noTargetCase->getFuncNameOperand()->getStringSlice(),
                .targetName =
                    TypeTextUtil::getCompileTargetName(SlangCompileTarget(target->getTarget())),
                .location = missingReturn->sourceLoc,
            });
            diagnoseCallStack(missingReturn, sink);
        }
    }
}

} // namespace Slang
