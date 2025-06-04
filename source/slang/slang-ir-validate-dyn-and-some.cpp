#include "slang-ir-validate-dyn-and-some.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct ValidateDynAndSomeUsage
{
    DiagnosticSink* m_sink;
    
    IRInst* isVarAnOutParam(IRInst* var, IRCall* call)
    {
        IRFunc* calleeFunc = as<IRFunc>(call->getCallee());
        if (!calleeFunc)
            return nullptr;

        auto count = -1;
        for (auto arg : call->getArgsList())
        {
            count++;
            if (var != arg)
                continue;
            break;
        }
        if (as<IROutType>(calleeFunc->getParamType(count)))
        {
            return calleeFunc->getParamType(count);
        }
        return nullptr;
    }

    struct ValidateSingleWrite
    {
        ValidateDynAndSomeUsage* m_context;

        // Passes on the fail-point if any.
        // Recursively counts the number of assignments to a var.
        //
        Dictionary<IRInst*, int> blockTraversalCache;

        // fail point is the "first block which when followed causes a failiure",
        // not the write which causes a failiure (that may be multiple writes).
        IRInst* failPoint = nullptr;

        List<IRInst*> potentialIssues = List<IRInst*>();

        HashSet<IRBlock*> blocksToSkip = HashSet<IRBlock*>();
        void resetChecks()
        {
            blockTraversalCache = Dictionary<IRInst*, int>();
            failPoint = nullptr;
            potentialIssues = {};
            blocksToSkip = {};
        }

        int totalAssignmentsToVarGivenInst(IRInst* var, IRInst* instToCheck)
        {
            switch (instToCheck->getOp())
            {
            case kIROp_Store:
                {
                    // always considered 1 assignment
                    IRStore* storeOp = as<IRStore>(instToCheck);
                    if (storeOp->getPtr() != var)
                        return 0;
                    potentialIssues.add(instToCheck);
                    return 1;
                }
            case kIROp_Call:
                {
                    // If `var` is an `out` param, we need to add 1 to the total assignments
                    IRCall* callOp = as<IRCall>(instToCheck);
                    if (m_context->isVarAnOutParam(var, callOp))
                    {
                        potentialIssues.add(callOp);
                        return 1;
                    }
                    return 0;
                }
            case kIROp_ifElse:
                {
                    // Store the max total assignments in the if-else block.
                    // We assume worst case, so if 1 block has 1 assignment
                    // and rest have 0, we assume the entire block to have 1 assignment.

                    /*
                    we need to step through a mini cfg since `uses` does not store which
                    if-else-chain a inst comes from; this is important because 1 write inside an if
                    and 1 write inside an else block is considered 1 write total
                    */

                    IRIfElse* ifElseOp = as<IRIfElse>(instToCheck);
                    int total = 0;

                    List<IRInst*> blocksToIterOn = List<IRInst*>();
                    blocksToIterOn.reserve(2);
                    blocksToIterOn.add(ifElseOp->getTrueBlock());
                    blocksToIterOn.add(ifElseOp->getFalseBlock());

                    for (auto maybeBlock : blocksToIterOn)
                    {
                        if (!maybeBlock)
                            continue;

                        int totalAssignments =
                            collectAssignmentsInBlock(var, as<IRBlock>(maybeBlock));
                        if (failPoint)
                            return 0;
                        total = (total > totalAssignments) ? total : totalAssignments;
                    }
                    return total;
                }
            case kIROp_loop:
                {
                    IRLoop* loopOp = as<IRLoop>(instToCheck);
                    // We can assume iterators can always be changed if loops
                    // are not-unrollable, therefore if this loop runs with an
                    // assignment we must fail.
                    //
                    // part of loop logic is TargetBlock->BreakBlock. This is an
                    // issue since the BreakBlock is outside the loop, so we need to
                    // visit the BreakBlock first to ensure we can remove the total
                    // assignments from the BreakBlock from the for-loop assignment count

                    int totalAssignmentsPostLoop =
                        collectAssignmentsInBlock(var, loopOp->getBreakBlock());
                    if (failPoint)
                        return 0;

                    int totalAssignmentsInLoop =
                        collectAssignmentsInBlock(var, loopOp->getTargetBlock()) -
                        totalAssignmentsPostLoop;
                    if (totalAssignmentsInLoop > 0)
                        return 2;

                    return 0;
                }
            case kIROp_Switch:
                {
                    IRSwitch* switchOp = as<IRSwitch>(instToCheck);
                    int total = 0;
                    for (auto i = 0; i < switchOp->getCaseCount(); i++)
                    {
                        auto caseBlock = as<IRBlock>(switchOp->getCaseLabel(i));
                        int totalAssignments = collectAssignmentsInBlock(var, caseBlock);
                        if (failPoint)
                            return 0;
                        total = (total > totalAssignments) ? total : totalAssignments;
                    }
                    return total;
                }
            case kIROp_unconditionalBranch:
                {
                    // just run through all elements of the branch
                    IRUnconditionalBranch* branchOp = as<IRUnconditionalBranch>(instToCheck);
                    int totalAssignments =
                        collectAssignmentsInBlock(var, branchOp->getTargetBlock());
                    if (failPoint)
                        return 0;
                    return totalAssignments;
                }
            default:
                return 0;
            }
        }

        int* getFromCache(IRBlock* inst) { return blockTraversalCache.tryGetValue(inst); }

        void addBlockToCache(IRBlock* inst, int value) { blockTraversalCache.add(inst, value); }

        bool maybeSkipBlock(IRBlock* inst)
        {
            if (blocksToSkip.contains(inst))
                return true;
            blocksToSkip.add(inst);
            return false;
        }

        int collectAssignmentsInBlock(IRInst* var, IRBlock* block)
        {
            if (!block)
                return 0;


            // any repeat blocks found need to be fetched from cache
            if (auto cachedTotal = getFromCache(block))
                return *cachedTotal;

            // If we get here, its due to a loop; we are in-process of checking and will populate
            // the cache after the recursion ends
            if (maybeSkipBlock(block))
                return 0;

            int total = 0;
            for (auto inst : block->getChildren())
            {
                total += totalAssignmentsToVarGivenInst(var, inst);

                if (failPoint)
                    return 0;
                if (total > 1)
                {
                    failPoint = inst;
                    return 0;
                }
            }

            addBlockToCache(block, total);
            return total;
        }

        // returns nullptr if the var was not written more than once,
        // otherwise returns the inst that wrote it more than once.
        void ensureLocalVarIsWrittenOnce(IRInst* var)
        {
            // Idea here is we need to walk every IRBlock to figure out if we have 2 assignments to
            // our variable in question. We cannot look at IRUse's since these would show 2
            // `IRStore` in cases with branches without a clear connection between the 2:
            /*
                some Type myVar;
                if(thing[0])
                    myVar = Thing();
                else
                    myVar = Thing();
            */
            auto parentToTraverse = as<IRBlock>(var->getParent());
            collectAssignmentsInBlock(var, parentToTraverse);
        }

        ValidateSingleWrite(ValidateDynAndSomeUsage* context, IRInst* var)
        { 
            m_context = context;
            ensureLocalVarIsWrittenOnce(var);

            if (failPoint)
            {
                m_context->m_sink->diagnose(var, Diagnostics::localSomeVarWrittenMoreThanOnce, var);
                for (auto i : potentialIssues)
                    m_context->m_sink->diagnose(i, Diagnostics::seeUseSite);
            }

        }
    };
    
    struct ValidateWritingSameValueTypeToOut
    {
        ValidateDynAndSomeUsage* m_context = nullptr;
        IRInst* promisedValueType = nullptr;
        IRInst* originalPromisedInst = nullptr;
        // Essentially, we are breaking down every IR op into such a way that
        // either we are exactly the same (type or value) or not the same at all
        IRInst* getValueToCompareWith(IRInst* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_MakeExistential:
                {
                    IRMakeExistential* op = as<IRMakeExistential>(inst);
                    return op->getWrappedValue()->getFullType();
                }
            default:
                return inst;
            }
        }

        void validateAgainstPromisedOutValueType(IRInst* other)
        { 
            auto valToCompareWith = getValueToCompareWith(other);
            if (!promisedValueType)
            {
                promisedValueType = valToCompareWith;
                return;
            }

            if (valToCompareWith == promisedValueType)
                return;

            // We will be conservative and error by default
            m_context->m_sink->diagnose(
                other,
                Diagnostics::assigningDifferentTypesToOutSomeType);
            m_context->m_sink->diagnose(originalPromisedInst, Diagnostics::seeUseSite);
        }
        void ensureOutVarIsWrittenByTheSameValues(IRInst* var)
        {
            traverseUsers(
                var,
                [&](IRInst* user)
                {
                    switch (user->getOp())
                    {
                    case kIROp_Store:
                        {
                            // always considered 1 assignment
                            IRStore* storeOp = as<IRStore>(user);
                            if (storeOp->getPtr() != var)
                                return;

                            validateAgainstPromisedOutValueType(storeOp->getVal());
                            return;
                        }
                    case kIROp_Call:
                        {
                            // If `var` is an `out` param, we need to add 1 to the total assignments
                            IRCall* callOp = as<IRCall>(user);
                            auto outParamType = m_context->isVarAnOutParam(var, callOp);
                            if (!outParamType)
                                return;

                            validateAgainstPromisedOutValueType(callOp);
                            return;
                        }
                    }
                });
        }

        ValidateWritingSameValueTypeToOut(ValidateDynAndSomeUsage* context, IRInst* var)
        { 
            m_context = context;
            ensureOutVarIsWrittenByTheSameValues(var);
        }
    };

    ValidateDynAndSomeUsage(IRModule* module, DiagnosticSink* sink)
    {
        m_sink = sink;
        for (auto globalInst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(globalInst);
            if (!func)
                continue;

            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    auto op = inst->getOp();
                    if (op != kIROp_Var && op != kIROp_Param)
                        continue;

                    if (!inst->findDecoration<IRSomeTypeDecoration>())
                        continue;

                    // Ensure written once
                    ValidateSingleWrite(this, inst);
                    
                    // Ensure `out` var is only written by the same value-type
                    if (op != kIROp_Param || !as<IROutType>(inst->getFullType()))
                        continue;

                    ValidateWritingSameValueTypeToOut(this, inst);
                }
            }
        }
    }
};

void validateDynAndSomeUsage(IRModule* module, DiagnosticSink* sink)
{
    
    //Validation happening here:
    // * ensure `some` var is assigned once
    ValidateDynAndSomeUsage(module, sink);
}

} // namespace Slang
