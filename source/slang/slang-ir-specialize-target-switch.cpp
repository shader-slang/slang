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
// Mark `missingReturn`, which replaces `targetSwitch` in `code`, as having no case for the
// target, and give it the switch's location so that E41037 points at the switch (or, once `code`
// is inlined, at the call). We record the function name as a string because it is needed after
// `code` has been inlined into its callers.
static void markMissingReturnAsNoTargetCase(
    IRBuilder& builder,
    IRMissingReturn* missingReturn,
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
                // A switch with no case at all is not diagnosed here: the function may be one
                // that linking pulled in only through another target's case, such as
                // `__sincos_metal`, referenced from the `metal` case of `sincos`, which DCE
                // removes once that switch is resolved. The `failedImplies` switch is left
                // unmarked because E41011 above is an error and `linkAndOptimizeIR` stops after
                // linking when there are errors.
                auto missingReturn = cast<IRMissingReturn>(builder.emitMissingReturn());
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

IRNoTargetCaseDecoration* findNoTargetCaseDecoration(IRMissingReturn* missingReturn)
{
    return missingReturn->findDecoration<IRNoTargetCaseDecoration>();
}

void diagnoseNoTargetCase(
    IRMissingReturn* missingReturn,
    IRNoTargetCaseDecoration* noTargetCase,
    CodeGenTarget target,
    DiagnosticSink* sink)
{
    sink->diagnose(Diagnostics::TargetSwitchNoCaseForTarget{
        .funcName = noTargetCase->getFuncNameOperand()->getStringSlice(),
        .targetName = TypeTextUtil::getCompileTargetName(SlangCompileTarget(target)),
        .location = missingReturn->sourceLoc,
    });
    diagnoseCallStack(missingReturn, sink);
}

void diagnoseRemainingNoTargetCases(IRModule* module, CodeGenTarget target, DiagnosticSink* sink)
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
            if (auto noTargetCase = findNoTargetCaseDecoration(missingReturn))
                diagnoseNoTargetCase(missingReturn, noTargetCase, target, sink);
        }
    }
}

} // namespace Slang
