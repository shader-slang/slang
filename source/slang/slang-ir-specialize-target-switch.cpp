#include "slang-ir-specialize-target-switch.h"

#include "slang-capability.h"
#include "slang-compiler.h"
#include "slang-ir-dce.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

// Describes a __target_switch that had no compatible case for the current target
// (the `failedImplies` path), collected during the first pass of specialization.
struct FailedTargetSwitch
{
    SourceLoc loc;
    // The function (or inner function of a generic) that contains the failed switch.
    IRGlobalValueWithCode* code;
    // The basic block that contained the failed switch. After per-function DCE this
    // pointer is checked: if the block was in an unreachable branch (e.g. inside a
    // dead `case _sm_6_10:` arm of an outer __target_switch) DCE removes it and sets
    // its parent to nullptr, indicating the failure is moot.
    IRBlock* block;
    String profile;
};

// Specialize all __target_switch instructions in `code` for `target`.
// Failed switches (no compatible case, but also not just an incompatible target family) are
// appended to `outFailed` rather than emitted immediately as diagnostics.
static void specializeTargetSwitchForCode(
    TargetRequest* target,
    IRGlobalValueWithCode* code,
    List<FailedTargetSwitch>& outFailed)
{
    if (auto gen = as<IRGeneric>(code))
    {
        auto retVal = findGenericReturnVal(gen);
        if (auto innerCode = as<IRGlobalValueWithCode>(retVal))
        {
            specializeTargetSwitchForCode(target, innerCode, outFailed);
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
                // Only error if we have the chance of setting a valid target switch, but did not
                // due to incompatability within same `target` atom. Otherwise we will have an issue
                // when we process a `__target_switch() { case metal: return; }` for glsl targets.
                if (failedImplies)
                {
                    StringBuilder profileSb;
                    printDiagnosticArg(profileSb, target->getTargetCaps());
                    outFailed.add(FailedTargetSwitch{
                        targetSwitch->sourceLoc,
                        code,
                        block,
                        profileSb.produceString(),
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


void specializeTargetSwitch(
    TargetRequest* target,
    IRGlobalValueWithCode* code,
    DiagnosticSink* sink)
{
    List<FailedTargetSwitch> failed;
    specializeTargetSwitchForCode(target, code, failed);
    for (auto& f : failed)
        sink->diagnose(Diagnostics::ProfileIncompatibleWithTargetSwitch{
            .profile = f.profile,
            .location = f.loc,
        });
}

void specializeTargetSwitch(TargetRequest* target, IRModule* module, DiagnosticSink* sink)
{
    // Phase 1: specialize all __target_switch instructions in every function, applying
    // per-function DCE after each. Failures are collected rather than emitted immediately.
    //
    // Per-function DCE removes dead blocks (e.g. the `case _sm_6_10:` block when target
    // is cs_6_9) along with the call instructions inside them. This causes any helper
    // functions that were only called from those dead blocks to lose their call sites.
    List<FailedTargetSwitch> allFailed;
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto code = as<IRGlobalValueWithCode>(globalInst))
        {
            specializeTargetSwitchForCode(target, code, allFailed);
        }
    }

    // Phase 2: emit errors only for failed switches whose containing block survived DCE.
    //
    // A failed switch that was inside a dead branch (e.g. `case _sm_6_10:` in a function
    // compiled for cs_6_9) will have had its containing block eliminated by the per-function
    // DCE in Phase 1.  After DCE the block's parent pointer is null — we use that as a
    // signal that the failure is moot and should not produce an error.
    //
    // Concretely this covers two scenarios:
    //   1. A helper function G (possibly generic) is only called from a dead branch of an
    //      outer __target_switch in caller F.  When F is specialized and DCE'd, the call to
    //      G is removed.  G's own target switch is recorded as failed, but G (or its outer
    //      generic) has no remaining call sites — the block pointer, however, is inside G
    //      itself and survives (G is a separate global), so we additionally need scenario 2.
    //   2. An inlined helper's __target_switch lives directly inside the dead branch block
    //      of the calling function (e.g. __MatMul_linalg force-inlined into
    //      __coopVecMatMulPacked_impl).  After per-function DCE the dead branch block is
    //      removed, setting its parent to nullptr — so the block-parent check fires.
    //
    // For case 1 (non-inlined helpers that are still separate globals), we additionally
    // suppress the error when the containing function's block is alive but the function
    // itself has no remaining callers: we check whether the top-level entity (function or
    // its enclosing generic) is still referenced by any live IRSpecialize.
    for (auto& f : allFailed)
    {
        // Check if the block containing the failed switch was eliminated by DCE.
        // IRBlock::getParent() returns nullptr after removeAndDeallocate().
        if (f.block->getParent() == nullptr)
            continue;

        // The block is still alive.  Now check whether the enclosing function (or its
        // generic) is still reachable for this target.  This handles the case where a
        // separate helper function G has its __target_switch as the *only* thing in G's
        // body — the block is G's entry block (always alive inside G), but G may have no
        // remaining call sites after the caller's dead branch was DCE'd.
        IRInst* outer = f.code;
        if (auto parentBlock = as<IRBlock>(f.code->getParent()))
        {
            if (auto parentGeneric = as<IRGeneric>(parentBlock->getParent()))
                outer = parentGeneric;
        }

        bool reachable = false;
        if (!as<IRGeneric>(outer))
        {
            // Non-generic function: reachable if it has any remaining uses.
            reachable = outer->hasUses();
        }
        else
        {
            // Generic function: reachable if any IRSpecialize of it is still called.
            for (auto use = outer->firstUse; use; use = use->nextUse)
            {
                auto specInst = as<IRSpecialize>(use->getUser());
                if (!specInst)
                    continue;
                if (specInst->hasUses())
                {
                    reachable = true;
                    break;
                }
            }
        }

        if (reachable)
        {
            sink->diagnose(Diagnostics::ProfileIncompatibleWithTargetSwitch{
                .profile = f.profile,
                .location = f.loc,
            });
        }
    }
}

} // namespace Slang
