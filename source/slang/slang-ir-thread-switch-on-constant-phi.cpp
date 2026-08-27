// slang-ir-thread-switch-on-constant-phi.cpp
#include "slang-ir-thread-switch-on-constant-phi.h"

#include "slang-ir-dce.h"
#include "slang-ir-insts.h"
#include "slang-ir-simplify-cfg.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

#include <initializer_list>

namespace Slang
{

// Bound the analysis so a pathological function cannot cause an unbounded walk.
static const UInt kMaxThreadableCases = 64;
static const Index kMaxChainBlocks = 128;

// Return `block`'s terminator only if it is a plain unconditional branch.
// `IRLoop` derives from `IRUnconditionalBranch`, so an `as<IRUnconditionalBranch>`
// check would also accept a loop header; matching the op exactly excludes loops
// (and any other terminator whose break/continue structure we must not discard).
static IRUnconditionalBranch* getPlainBranch(IRBlock* block)
{
    auto term = block->getTerminator();
    if (term && term->getOp() == kIROp_UnconditionalBranch)
        return as<IRUnconditionalBranch>(term);
    return nullptr;
}

// True if `block` is referenced by a control-flow inst other than `ownSwitch`
// whose op is in `rejectOps`. Such structural-label uses do not appear in
// `getPredecessors()`, so a single-predecessor block can still be a region
// label; re-routing an edge into a block that labels a region we do not own
// would corrupt that region. `ownSwitch` is always excluded (a case block is
// legitimately referenced by the switch we are threading).
static bool isUsedAsStructuralLabel(
    IRBlock* block,
    IRSwitch* ownSwitch,
    std::initializer_list<IROp> rejectOps)
{
    for (auto use = block->firstUse; use; use = use->nextUse)
    {
        if (use->getUser() == ownSwitch)
            continue;
        auto op = use->getUser()->getOp();
        for (auto rejectOp : rejectOps)
        {
            if (op == rejectOp)
                return true;
        }
    }
    return false;
}

// The single parameter of a "forwarding" phi merge, together with the branch
// that forwards it. A forwarding merge has one parameter whose only role is to
// pass one hop toward the switch header: its terminator is a plain branch that
// passes exactly that parameter, and the parameter has no other consumer. This
// is the shape an `if/else-if` ladder selecting a constant tag lowers to in SSA
// -- each nested `if`'s merge block forwards the running tag value outward.
struct ForwardingMerge
{
    IRParam* param = nullptr;
    IRUnconditionalBranch* forwardBranch = nullptr;
};

// One selecting arm proven to reach exactly one case: the branch `armBranch`
// supplies a constant into the phi chain that the switch matches to `caseBlock`.
struct ThreadedArm
{
    IRUnconditionalBranch* armBranch = nullptr;
    IRBlock* caseBlock = nullptr;
};

// The full rewrite plan for one threadable switch, assembled during the
// prove phase and only committed if every gate passes (all-or-nothing).
struct SwitchThreadingPlan
{
    IRSwitch* switchInst = nullptr;
    IRParam* selectorParam = nullptr; // the switch selector (a phi in header)
    IRBlock* mergeBlock = nullptr;    // switch break label = common continuation
    IRType* resultType = nullptr;     // type carried on the merge phi
    List<IRBlock*> chainBlocks;       // every phi-chain block, including header
    List<ThreadedArm> arms;           // one per selecting constant arm
};

// Return the block parameter that a switch selector reads, but only when that
// parameter is a genuine phi (its block is not the function entry and every
// predecessor supplies it via an unconditional branch). Otherwise null.
static IRParam* getSelectorPhi(IRSwitch* switchInst, IRBlock* headerBlock)
{
    auto param = as<IRParam>(switchInst->getCondition());
    if (!param)
        return nullptr;
    if (param->getParent() != headerBlock)
        return nullptr;
    if (headerBlock == headerBlock->getParent()->getFirstBlock())
        return nullptr;
    for (auto pred : headerBlock->getPredecessors())
    {
        if (!getPlainBranch(pred))
            return nullptr;
    }
    return param;
}

// Find the case block a `switch` routes a given integer constant to, or the
// default block when no case matches. Returns null if any case value is not an
// integer literal (so we cannot reason about coverage).
static IRBlock* findCaseBlockForValue(
    IRSwitch* switchInst,
    IRIntegerValue value,
    bool& outIsDefault)
{
    outIsDefault = false;
    UInt caseCount = switchInst->getCaseCount();
    for (UInt i = 0; i < caseCount; ++i)
    {
        auto caseVal = as<IRIntLit>(switchInst->getCaseValue(i));
        if (!caseVal)
            return nullptr;
        if (caseVal->getValue() == value)
            return switchInst->getCaseLabel(i);
    }
    outIsDefault = true;
    return switchInst->getDefaultLabel();
}

// A case block is threadable when its only entry is the switch itself and it
// hands a single value to the common merge block. That single-entry, single-arg
// shape is what lets us re-route a selecting arm into it and forward the value
// up the (soon-to-be-retyped) phi chain without cloning anything.
static bool isThreadableCaseBlock(IRBlock* caseBlock, IRBlock* mergeBlock, IRSwitch* ownSwitch)
{
    // Sole predecessor must be the switch itself, and the case block must not
    // double as any structured-region label (which `getPredecessors()` would not
    // reveal), so re-routing a selecting arm into it cannot corrupt a region.
    if (caseBlock->getPredecessors().getCount() != 1)
        return false;
    if (isUsedAsStructuralLabel(caseBlock, ownSwitch, {kIROp_Loop, kIROp_IfElse, kIROp_Switch}))
        return false;

    auto term = getPlainBranch(caseBlock);
    if (!term)
        return false;
    if (term->getTargetBlock() != mergeBlock)
        return false;
    if (term->getArgCount() != 1)
        return false;

    return true;
}

// If `block` is a forwarding phi merge (see ForwardingMerge), populate `out` and
// return true. `param`'s only permitted uses are its own forwarding branch arg
// (and, for the switch header, the switch condition, handled by the caller).
static bool asForwardingMerge(IRBlock* block, ForwardingMerge& out)
{
    auto firstParam = block->getFirstParam();
    if (!firstParam)
        return false;
    if (firstParam->getNextParam())
        return false;

    auto term = getPlainBranch(block);
    if (!term)
        return false;

    // The block must be a pure forwarder: nothing between the param and the
    // terminator (no side-effecting or value-producing insts to preserve).
    for (auto inst = block->getFirstOrdinaryInst(); inst; inst = inst->getNextInst())
    {
        if (inst == term)
            break;
        if (inst->getOp() == kIROp_DebugLine)
            continue;
        return false;
    }

    // The terminator must forward exactly the single parameter.
    bool found = false;
    for (UInt i = 0; i < term->getArgCount(); ++i)
    {
        if (term->getArg(i) == firstParam)
        {
            found = true;
            break;
        }
    }
    if (!found)
        return false;

    out.param = firstParam;
    out.forwardBranch = term;
    return true;
}

// Verify that `param`'s only use is `allowedUse` (the branch arg or switch
// condition that legitimately consumes the running tag). Any other consumer
// means the tag escapes the chain and threading would drop a live value.
static bool paramHasOnlyUse(IRParam* param, IRInst* allowedUser)
{
    for (auto use = param->firstUse; use; use = use->nextUse)
    {
        if (use->getUser() != allowedUser)
            return false;
    }
    return true;
}

// Try to prove that `switchInst` is threadable and, if so, assemble its rewrite
// plan. Returns false (leaving `plan` unspecified) on any gate failure -- a
// conservative bail is always a missed optimization, never a miscompile.
static bool tryPlanSwitchThreading(IRSwitch* switchInst, SwitchThreadingPlan& plan)
{
    auto headerBlock = as<IRBlock>(switchInst->getParent());
    if (!headerBlock)
        return false;

    UInt caseCount = switchInst->getCaseCount();
    if (caseCount == 0 || caseCount > kMaxThreadableCases)
        return false;

    auto selectorParam = getSelectorPhi(switchInst, headerBlock);
    if (!selectorParam)
        return false;

    // If the switch header itself labels a loop or another switch, the switch
    // sits at a region boundary (e.g. a loop break block); threading an arm into
    // a case would move case work across that boundary. Chain blocks are checked
    // the same way as they are discovered below.
    if (isUsedAsStructuralLabel(headerBlock, switchInst, {kIROp_Loop, kIROp_Switch}))
        return false;

    auto mergeBlock = switchInst->getBreakLabel();
    if (!mergeBlock)
        return false;

    // The merge block carries the per-case result as a single phi param; that is
    // the type the retargeted chain will forward. (A parameterless merge is the
    // `eliminatePhis` form, which this pass runs before -- bail.)
    auto mergeParam = mergeBlock->getFirstParam();
    if (!mergeParam || mergeParam->getNextParam())
        return false;
    plan.resultType = mergeParam->getFullType();

    // Walk the phi cascade backward from the switch header. Each block in the
    // chain has one param; each incoming edge either supplies an integer literal
    // (a selecting arm -> a proven case) or forwards another chain block's param
    // (one hop deeper). We enqueue forwarded blocks and require every leaf to be
    // a distinct constant that the switch matches to a real case.
    HashSet<IRBlock*> visited;
    List<IRBlock*> workList;
    HashSet<IRIntegerValue> seenConstants;

    workList.add(headerBlock);
    visited.add(headerBlock);
    plan.chainBlocks.add(headerBlock);

    // The header's selector param may only be consumed by the switch condition.
    if (!paramHasOnlyUse(selectorParam, switchInst))
        return false;

    for (Index wi = 0; wi < workList.getCount(); ++wi)
    {
        auto block = workList[wi];

        auto param = block->getFirstParam();
        if (!param || param->getNextParam())
            return false;
        int paramIndex = getParamIndexInBlock(param);
        if (paramIndex < 0)
            return false;

        for (auto pred : block->getPredecessors())
        {
            auto predBranch = getPlainBranch(pred);
            if (!predBranch)
                return false;
            if ((UInt)paramIndex >= predBranch->getArgCount())
                return false;
            auto arg = predBranch->getArg((UInt)paramIndex);

            if (auto constArg = as<IRIntLit>(arg))
            {
                // A selecting arm: this edge proves one case.
                bool isDefault = false;
                auto caseBlock = findCaseBlockForValue(switchInst, constArg->getValue(), isDefault);
                if (!caseBlock)
                    return false;
                // Bail if any edge selects the default: threading requires every
                // reaching value to hit a real case so the switch becomes fully
                // dead (a surviving default edge would keep the switch live).
                if (isDefault)
                    return false;
                // A repeated constant would map two arms to one case block, which
                // then could not stay single-entry after threading.
                if (seenConstants.contains(constArg->getValue()))
                    return false;
                seenConstants.add(constArg->getValue());

                if (!isThreadableCaseBlock(caseBlock, mergeBlock, switchInst))
                    return false;

                ThreadedArm arm;
                arm.armBranch = predBranch;
                arm.caseBlock = caseBlock;
                plan.arms.add(arm);
            }
            else if (auto forwardedParam = as<IRParam>(arg))
            {
                // A forwarding hop: the arg is another chain block's param.
                auto forwardedBlock = as<IRBlock>(forwardedParam->getParent());
                if (!forwardedBlock)
                    return false;
                // Chain blocks are legitimately the after-blocks of the `if/else`
                // cascade (an `IRIfElse` merge is their normal role, so we must
                // not reject that). But a chain block that also labels a loop or
                // another switch would mean the selector crosses a loop/other
                // region boundary -- redirecting an arm there could change
                // convergence or derivative behavior, so bail.
                if (isUsedAsStructuralLabel(forwardedBlock, switchInst, {kIROp_Loop, kIROp_Switch}))
                    return false;
                ForwardingMerge fm;
                if (!asForwardingMerge(forwardedBlock, fm))
                    return false;
                if (fm.param != forwardedParam)
                    return false;
                if (!paramHasOnlyUse(forwardedParam, fm.forwardBranch))
                    return false;
                if (!visited.contains(forwardedBlock))
                {
                    if (plan.chainBlocks.getCount() >= kMaxChainBlocks)
                        return false;
                    visited.add(forwardedBlock);
                    workList.add(forwardedBlock);
                    plan.chainBlocks.add(forwardedBlock);
                }
            }
            else
            {
                // Neither a proving constant nor a forwarding phi -> not the
                // shape we can thread safely.
                return false;
            }
        }
    }

    // Require that every case is covered exactly once, so the switch is fully
    // dead after threading (no residual live edge into a case block, which would
    // be unstructurable).
    if (plan.arms.getCount() != (Index)caseCount)
        return false;

    // Guard the header block's own instructions. After threading, each arm
    // branches straight to its case body, so the header runs only via the
    // forwarding chain -- i.e. after the cases. A header instruction is therefore
    // safe to leave in place only if (a) it has no side effect whose order
    // relative to the case bodies could become observable, and (b) every value it
    // defines is consumed only by the header, the switch, or a genuinely dead
    // default block. Anything a case block or the live continuation reads would
    // lose dominance -- including a load with no side effect, whose value would
    // change if it moved after a case-body store.
    //
    // The default block is exempt only when it is a distinct, switch-only block
    // that becomes unreachable after threading. When the default label instead
    // aliases the break label, the "default" is the live continuation, so
    // exempting uses there would wrongly permit a header value the continuation
    // consumes; require the default to differ from the merge.
    auto defaultBlock = switchInst->getDefaultLabel();
    IRBlock* deadDefaultBlock = nullptr;
    if (defaultBlock && defaultBlock != mergeBlock &&
        defaultBlock->getPredecessors().getCount() == 1)
    {
        deadDefaultBlock = defaultBlock;
    }
    for (auto headerInst = headerBlock->getFirstOrdinaryInst(); headerInst;
         headerInst = headerInst->getNextInst())
    {
        if (headerInst == switchInst)
            break;
        if (headerInst->mightHaveSideEffects())
            return false;
        for (auto use = headerInst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (user == switchInst)
                continue;
            auto userBlock = as<IRBlock>(getInstInBlock(user)->getParent());
            if (userBlock == headerBlock || userBlock == deadDefaultBlock)
                continue;
            return false;
        }
    }

    plan.switchInst = switchInst;
    plan.selectorParam = selectorParam;
    plan.mergeBlock = mergeBlock;
    return true;
}

// Commit a proven plan: re-route each selecting arm into its case block, retype
// the phi chain to carry the case result instead of the tag, and replace the
// dead switch with a branch to the common merge. The result is the same control
// shape as dispatching concretely in each arm.
static void applySwitchThreading(SwitchThreadingPlan& plan)
{
    IRBuilder builder(plan.switchInst->getModule());

    for (auto& arm : plan.arms)
    {
        auto armBranch = arm.armBranch;
        auto chainBlock = armBranch->getTargetBlock();
        auto caseTerm = as<IRUnconditionalBranch>(arm.caseBlock->getTerminator());
        auto resultVal = caseTerm->getArg(0);

        // The case body now feeds the chain block the arm used to enter, so its
        // result rides the same phi chain (retyped below) up to the merge.
        builder.setInsertBefore(caseTerm);
        builder.emitBranch(chainBlock, 1, &resultVal);
        caseTerm->removeAndDeallocate();

        // The arm enters the case body directly; case blocks take no arguments.
        builder.setInsertBefore(armBranch);
        builder.emitBranch(arm.caseBlock);
        armBranch->removeAndDeallocate();
    }

    // The chain now forwards results, not tags. Retyping every chain parameter in
    // lockstep keeps each forwarding branch's argument and its target parameter
    // in agreement across the whole chain.
    for (auto chainBlock : plan.chainBlocks)
    {
        auto param = chainBlock->getFirstParam();
        param->setFullType(plan.resultType);
    }

    // The switch is now dead: every path into the header carries a case result in
    // the header parameter, so branching it to the merge preserves the merge phi.
    builder.setInsertBefore(plan.switchInst);
    IRInst* headerResult = plan.selectorParam;
    builder.emitBranch(plan.mergeBlock, 1, &headerResult);
    plan.switchInst->removeAndDeallocate();
}

bool threadSwitchOnConstantPhi(IRModule* module)
{
    bool changedAny = false;

    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRGlobalValueWithCode>(inst);
        if (!func || !func->getFirstBlock())
            continue;

        // Collect candidate switches first; the rewrite mutates the CFG.
        List<IRSwitch*> switches;
        for (auto block : func->getBlocks())
        {
            if (auto switchInst = as<IRSwitch>(block->getTerminator()))
                switches.add(switchInst);
        }

        bool changedThisFunc = false;
        for (auto switchInst : switches)
        {
            SwitchThreadingPlan plan;
            if (tryPlanSwitchThreading(switchInst, plan))
            {
                applySwitchThreading(plan);
                changedThisFunc = true;
            }
        }

        if (changedThisFunc)
        {
            // After threading, each selecting arm reaches its case body directly
            // and the switch is a branch to the common merge. A distinct,
            // switch-only default block becomes unreachable; removing it (via CFG
            // cleanup) can leave the merge phi with a single predecessor, which
            // then collapses to a plain value. DCE drops the resulting dead insts.
            // Both run in the later pipeline too, but cleaning up here keeps the IR
            // well formed for any validation between passes.
            simplifyCFG(func, CFGSimplificationOptions::getFast());
            eliminateDeadCode(func);
            changedAny = true;
        }
    }

    return changedAny;
}

} // namespace Slang
