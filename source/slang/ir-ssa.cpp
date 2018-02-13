// ir-ssa.cpp
#include "ir-ssa.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang {

// Track information on a phi node we are in
// the process of constructing.
struct PhiInfo : RefObject
{
    // The phi node will be represented as a parameter
    // to a (non-entry) basic block.
    IRParam*    phi;

    // The original variable that this phi will be replacing.
    IRVar* var;

    // The operands to the phi will be stored as uses here,
    // because our IR parameters don't have operands.
    //
    // Once we've collected all the values we plan to use,
    // we will turn this into argument in predecessor blocks
    // that branch to this one.
    //
    // The order of elements in this list must match the
    // order in which the predecessor blocks get enumerated.
    List<IRUse> operands;

    // If this phi ended up being removed as trivial, then
    // this will be the value that we replaced it with.
    IRValue* replacement = nullptr;
};

// Information about a basic block that we generate/use
// during SSA construction.
struct SSABlockInfo : RefObject
{
    // Map a promotable variable to the value to
    // use for that variable
    Dictionary<IRVar*, IRValue*> valueForVar;

    // The underlying basic block.
    IRBlock* block;

    // Have we processed all the instructions in the
    // body of this block (so that we would have
    // found any stores to SSA variables)?
    bool isFilled = false;

    // Have we filled all the predecessors of
    // this block, so that we can actually perform
    // look up in them?
    bool isSealed = false;

    // An IR builder to use when we want to construct
    // stuff in the context of this block
    IRBuilder builder;

    // Phi nodes we are creating for this block.
    List<PhiInfo*> phis;

    // Arguments that this block needs to pass along
    // to the phi nodes defined by is sucessor
    List<IRValue*> successorArgs;
};

// State for constructing SSA form for a global value
// with code (usually a function).
struct ConstructSSAContext
{
    // The value that we want to rewrite into SSA form
    // (usually an IR function)
    IRGlobalValueWithCode* globalVal;

    // Variables that we've identified for promotion
    // to SSA values.
    List<IRVar*> promotableVars;

    // Information about each basic block
    Dictionary<IRBlock*, RefPtr<SSABlockInfo>> blockInfos;

    // IR building state to use during the operation
    SharedIRBuilder sharedBuilder;


    Dictionary<IRParam*, RefPtr<PhiInfo>> phiInfos;

    PhiInfo* getPhiInfo(IRParam* phi)
    {
        if(auto found = phiInfos.TryGetValue(phi))
            return *found;
        return nullptr;
    }
};

// Is the given variable one that we can promote to SSA form?
bool isPromotableVar(
    ConstructSSAContext*    /*context*/,
    IRVar*                  var)
{
    // If the variable is only used directly as the pointer
    // operand of load and store instructions, then it should
    // be promote-able.
    //
    // For now, we won't deal with cases where the variable
    // is an aggregate an we sometimes pull out individual
    // fields or elements. This is an important extension,
    // but we probably also need to think about scalarizing
    // any aggregates when we promote them to registers.

    for (auto u = var->firstUse; u; u = u->nextUse)
    {
        auto user = u->getUser();
        switch (user->op)
        {
        default:
            // If the variable gets used by any operation
            // we can't account for directly, then it isn't
            // promotable.
            return false;

        case kIROp_Load:
            {
                // A load has only a single argument, so
                // it had better be our pointer.
                assert(u == &((IRLoad*) user)->ptr);
            }
            break;

        case kIROp_Store:
            {
                auto storeInst = (IRStore*)user;

                // We don't want to promote a variable if
                // its address gets stored into another
                // variable, so check for that case.
                if (u == &storeInst->val)
                    return false;

                // Otherwise our variable is being used
                // as the destination for the store, and
                // that is okay by us.
                assert(u == &storeInst->ptr);
            }
            break;
        }
    }

    // If all of the uses passed our checking, then
    // we are good to go.
    return true;
}

// Identify local variables that can be promoted to SSA form
void identifyPromotableVars(
    ConstructSSAContext* context)
{
    for (auto bb = context->globalVal->getFirstBlock(); bb; bb = bb->getNextBlock())
    {
        for (auto ii = bb->getFirstInst(); ii; ii = ii->getNextInst())
        {
            if (ii->op != kIROp_Var)
                continue;

            IRVar* var = (IRVar*)ii;

            if (isPromotableVar(context, var))
            {
                context->promotableVars.Add(var);
            }
        }
    }
}

IRVar* asPromotableVar(
    ConstructSSAContext*    context,
    IRValue*                value)
{
    if (value->op != kIROp_Var)
        return nullptr;

    IRVar* var = (IRVar*)value;
    if (!context->promotableVars.Contains(var))
        return nullptr;

    return var;
}

// Try to read the value of an SSA variable
// in the context of the given block. If
// the variable is defined in the block, then
// that value will be used. If not, this all
// may recursively work its way up through
// the predecessors of the block.
IRValue* readVar(
    ConstructSSAContext*    context,
    SSABlockInfo*           blockInfo,
    IRVar*                  var);


// Add a phi node to represent the given variable
PhiInfo* addPhi(
    ConstructSSAContext*    context,
    SSABlockInfo*           blockInfo,
    IRVar*                  var)
{
    auto builder = &blockInfo->builder;
    IRParam* phi = builder->createParam(var->getType()->getValueType());

    RefPtr<PhiInfo> phiInfo = new PhiInfo();
    context->phiInfos.Add(phi, phiInfo);

    phiInfo->phi = phi;
    phiInfo->var = var;

    blockInfo->phis.Add(phiInfo);

    return phiInfo;
}

IRValue* tryRemoveTrivialPhi(
    ConstructSSAContext*    context,
    PhiInfo*                phiInfo)
{
    auto phi = phiInfo->phi;

    // We are going to check if all of the operands
    // to the phi are either the same, or are equal
    // to the phi itself.

    IRValue* same = nullptr;
    for (auto u : phiInfo->operands)
    {
        auto usedVal = u.get();
        assert(usedVal);

        if (usedVal == same || usedVal == phi)
        {
            // Either this is a self-reference, or it refers
            // to the same value we've seen already.
            continue;
        }
        if (same != nullptr)
        {
            // We've found at least two distinct values
            // other than the phi itself, so this phi
            // indeed appears to be non-trivial.
            //
            // We will keep the phi around.
            return phi;
        }
        else
        {
            // This value is distinct from the phi itself,
            // so we need to track its value.
            same = usedVal;
        }
    }

    if (!same)
    {
        // There were no operands other than the phi itself.
        // This implies that the value at the use sites should
        // actually be undefined.

        assert(!"unimplemented");
    }

    // Removing this phi as trivial may make other phi nodes
    // become trivial. We will recognize such candidates
    // by looking for phi nodes that use this node.
    List<PhiInfo*> otherPhis;
    for( auto u = phi->firstUse; u; u = u->nextUse )
    {
        auto user = u->user;
        if(!user) continue;
        if(user == phi) continue;

        if( user->op == kIROp_Param )
        {
            auto maybeOtherPhi = (IRParam*) user;
            if( auto otherPhiInfo = context->getPhiInfo(maybeOtherPhi) )
            {
                otherPhis.Add(otherPhiInfo);
            }
        }
    }

    // replace uses of the phi (including its possible uses
    // of itself) with the unique non-phi value.
    phi->replaceUsesWith(same);

    // Clear out the operands to the phi, since they won't
    // actually get used in the program any more.
    for( auto& u : phiInfo->operands )
    {
        u.clear();
    }

    // We will record the value that was used to replace this
    // phi, so that we can easily look it up later.
    phiInfo->replacement = same;

    // Now that we've cleaned up this phi, we need to consider
    // other phis that might have become  trivial.
    for( auto otherPhi : otherPhis )
    {
        tryRemoveTrivialPhi(context, otherPhi);
    }

    return same;
}

IRValue* addPhiOperands(
    ConstructSSAContext*    context,
    SSABlockInfo*           blockInfo,
    PhiInfo*                phiInfo)
{
    auto var = phiInfo->var;

    auto block = blockInfo->block;

    List<IRValue*> operandValues;
    for (auto predBlock : block->getPredecessors())
    {
        // Precondition: if we have multiple predecessors, then
        // each must have only one successor (no critical edges).
        //
        assert(predBlock->getSuccessors().getCount() == 1);

        auto predInfo = *context->blockInfos.TryGetValue(predBlock);

        auto phiOperand = readVar(context, predInfo, var);

        operandValues.Add(phiOperand);
    }

    // The `IRUse`  type needs to stay at a stable location
    // since they get threaded into lists. We allocate the
    // list with its final size so that we can preserve the
    // required invariant.

    UInt operandCount = operandValues.Count();
    phiInfo->operands.SetSize(operandCount);
    for(UInt ii = 0; ii < operandCount; ++ii)
    {
        phiInfo->operands[ii].init(phiInfo->phi, operandValues[ii]);
    }

    return tryRemoveTrivialPhi(context, phiInfo);
}

void writeVar(
    ConstructSSAContext*    /*context*/,
    SSABlockInfo*           blockInfo,
    IRVar*                  var,
    IRValue*                val)
{
    blockInfo->valueForVar[var] = val;
}

void maybeSealBlock(
    ConstructSSAContext*    context,
    SSABlockInfo*           blockInfo)
{
    // We can't seal a block that has already been sealed.
    if (blockInfo->isSealed)
        return;

    // We can't seal a block until all of its predecessors
    // have been filled.
    for (auto pp : blockInfo->block->getPredecessors())
    {
        auto predInfo = *context->blockInfos.TryGetValue(pp);
        if (!predInfo->isFilled)
            return;
    }

    // All the checks passed, so it seems like we can be sealed.

    // We will loop over any incomplete phis that have been recoreded
    // for this block, and complete them here.
    //
    // Note that we are doing the "inefficient" loop where we compute
    // the count on each iteration to account for the possibility that
    // new incomplete phis will get added while we are working.
    for (UInt ii = 0; ii < blockInfo->phis.Count(); ++ii)
    {
        auto incompletePhi = blockInfo->phis[ii];
        addPhiOperands(context, blockInfo, incompletePhi);
    }

    // After we've completed all our incomplete phis, we can mark this
    // block as sealed and move along.
    blockInfo->isSealed = true;
}

IRValue* readVarRec(
    ConstructSSAContext*    context,
    SSABlockInfo*           blockInfo,
    IRVar*                  var)
{
    IRValue* val = nullptr;
    if (!blockInfo->isSealed)
    {
        // If block isn't sealed, we need to
        // speculatively add a phi to it.
        // This phi may get removed later, once
        // we are able to seal this block.

        PhiInfo* phiInfo = addPhi(context, blockInfo, var);
        val = phiInfo->phi;
    }
    else
    {
        // If the block is sealed, then we are free to look at
        // it predecessor list, and use that to decide what to do.
        auto predecessors = blockInfo->block->getPredecessors();

        // 
        IRBlock* firstPred = nullptr;
        bool multiplePreds = false;
        for (auto pp : predecessors)
        {
            if (!firstPred)
            {
                // A candidate for the sole predecessor
                firstPred = pp;
            }
            else if (pp == firstPred)
            {
                // Same as existing predecessor
            }
            else
            {
                // Multiple unique predecessors
                multiplePreds = true;
            }
        }

        if (!firstPred)
        {
            // The block had *no* predecssors. This will commonly
            // happen for the entry block, but could also conceivably
            // happen for a block that is somehow disconnected
            // from the CFG and thus unreachable.

            // We would only reach this function (`readVarRec`) if
            // a local lookup in the block had already failed, so
            // at this point we are dealing with an undefined value.

            auto type = var->getType()->getValueType();
            val = blockInfo->builder.emitUndefined(type);
        }
        else if (!multiplePreds)
        {
            // There is only a single predecessor for this block,
            // so there is no need to insert a phi. Instead, we
            // just perform the lookup step recursively in
            // the predecessor.
            auto predInfo = *context->blockInfos.TryGetValue(firstPred);
            val = readVar(context, predInfo, var);
        }
        else
        {
            // The default/fallback case requires us to create
            // a phi node in the current block, and then look
            // up the appropriate operands in the predecessor
            // blocks, which will eventually become the operands
            // that drive the phi.

            // Create the phi node for the given variable
            PhiInfo* phiInfo = addPhi(context, blockInfo, var);

            // Mark the phi as the value for the variable inside
            // this block
            writeVar(context, blockInfo, var, phiInfo->phi);

            // Now add operands to the phi and maybe simplify
            // it, based on what gets found.

            val = addPhiOperands(context, blockInfo, phiInfo);
        }
    }

    // Whatever value we find, we need to mark it as the
    // value for the given variable in this block
    writeVar(context, blockInfo, var, val);

    return val;
}


IRValue* readVar(
    ConstructSSAContext*    context,
    SSABlockInfo*           blockInfo,
    IRVar*                  var)
{
    // In the easy case, there will be a preceeding
    // store in the same block, so we can use
    // that local value.
    IRValue* val = nullptr;
    if (blockInfo->valueForVar.TryGetValue(var, val))
    {
        // Hooray, we found a value to use, and we
        // can proceed without too many complications.

        // Well, let's check for one special case here, which
        // is when the value we intend to use is a `phi`
        // node that we ultimately decided to remove.
        while( val->op == kIROp_Param )
        {
            // The value is a parameter, but is it a phi?
            IRParam* maybePhi = (IRParam*) val;
            RefPtr<PhiInfo> phiInfo = nullptr;
            if(!context->phiInfos.TryGetValue(maybePhi, phiInfo))
                break;

            // Okay, this is indeed a phi we are adding, but
            // is it one that got replaced?
            if(!phiInfo->replacement)
                break;

            // The phi we want to use got replaced, so we
            // had better use the replacement instead.
            val = phiInfo->replacement;
        }


        return val;
    }

    // Otherwise we need to try to non-trivial/recursive
    // case of lookup.
    return readVarRec(context, blockInfo, var);
}

void processBlock(
    ConstructSSAContext*    context,
    IRBlock*                block,
    SSABlockInfo*           blockInfo)
{
    // Before starting, check if this block can be sealed
    maybeSealBlock(context, blockInfo);

    // Walk the instructions in the block, and either
    // leave them as-is, or replace them with a value
    // that we look up with local/global value numbering

    IRInst* next = nullptr;
    for (auto ii = block->getFirstInst(); ii; ii = next)
    {
        next = ii->getNextInst();

        // Any new instructions we create to represent
        // the new value will get inserted before whatever
        // instruction we are working with.
        blockInfo->builder.insertBeforeInst = ii;

        switch (ii->op)
        {
        default:
            // Ordinary instruction -> leave as-is
            break;

        case kIROp_Store:
            {
                auto storeInst = (IRStore*)ii;
                auto ptrArg = storeInst->ptr.get();
                auto valArg = storeInst->val.get();

                if (auto var = asPromotableVar(context, ptrArg))
                {
                    // We are storing to a promotable variable,
                    // so we want to register the value being
                    // stored as the value for the given SSA
                    // variable.
                    writeVar(context, blockInfo, var, valArg);

                    // Also eliminate the store instruction,
                    // since it is no longer needed.
                    storeInst->removeAndDeallocate();
                }
            }
            break;

        case kIROp_Load:
            {
                IRLoad* loadInst = (IRLoad*)ii;
                auto ptrArg = loadInst->ptr.get();

                if (auto var = asPromotableVar(context, ptrArg))
                {
                    // We are loading from a promotable variable.
                    // Look up the value in the context of this
                    // block.
                    auto val = readVar(context, blockInfo, var);

                    // We can just replace all uses of this
                    // load instruction with the given value.
                    loadInst->replaceUsesWith(val);

                    // Also eliminate the load instruction,
                    // since it is no longer needed.
                    loadInst->removeAndDeallocate();
                }
        }
            break;
        }
    }

    blockInfo->builder.insertBeforeInst = block->lastInst;

    // Once we are done with all of the instructions
    // in a block, we can mark it as "filled," which
    // means we can actually consider lookups into
    // it.
    blockInfo->isFilled = true;

    // Having filled this block might allow us to seal some
    // of its successor(s)
    for (auto ss : block->getSuccessors())
    {
        auto successorInfo = *context->blockInfos.TryGetValue(ss);
        maybeSealBlock(context, successorInfo);
    }
}

static void breakCriticalEdges(
    ConstructSSAContext*    context)
{
    // A critical edge is an edge P -> S where
    // P has multiple sucessors, and S has multiple
    // predecessors.
    //
    // In the context of our CFG representation, such an edge
    // will be an `IRUse` in the terminator instruction of block P,
    // which refers to block S.
    //
    // We will make a pass over the CFG to collect all the critical
    // edges, and then we will break them in a follow-up pass.

    List<IRUse*> criticalEdges;

    auto globalVal = context->globalVal;
    for (auto pred = globalVal->getFirstBlock(); pred; pred = pred->getNextBlock())
    {
        auto successors = pred->getSuccessors();
        if (successors.getCount() <= 1)
            continue;

        auto succIter = successors.begin();
        auto succEnd = successors.end();

        for (; succIter != succEnd; ++succIter)
        {
            auto succ = *succIter;

            // For the edge to be critical, the successor must have
            // more than one predecessor.
            // More than that, we require that it has more than one
            // *unique* predecessor, to handle the case where multiple
            // cases of a `switch` might lead to the same block.
            //
            // To implement this, we test if it has any predecessor
            // other than `pred` which we already know about.

            bool multiplePreds = false;
            for (auto pp : succ->getPredecessors())
            {
                if (pp != pred)
                {
                    multiplePreds = true;
                    break;
                }
            }
            if (!multiplePreds)
                continue;

            // We have found a critical edge from `pred` to `succ`.
            //
            // Furthermore, the `IRUse` embedded in `succIter` represents
            // that edge directly.
            auto edgeUse = succIter.use;
            criticalEdges.Add(edgeUse);
        }
    }

    // Now we will iterate over the critical edges and break each
    // one by inserting a new block. Note that we do not try
    // to break the edges while doing the initial walk, because
    // that would change the CFG while we are walking it.

    for (auto edgeUse : criticalEdges)
    {
        auto pred = (IRBlock*) edgeUse->getUser()->parent;
        assert(pred->op == kIROp_Block);

        auto succ = (IRBlock*)edgeUse->get();
        assert(succ->op == kIROp_Block);

        IRBuilder builder;
        builder.sharedBuilder = &context->sharedBuilder;
        builder.curFunc = globalVal;
        builder.curBlock = pred;

        // Create a new block that will sit "along" the edge
        IRBlock* edgeBlock = builder.createBlock();

        edgeUse->debugValidate();

        // The predecessor block should now branch to
        // the edge block.
        edgeUse->set(edgeBlock);

        // The edge block should branch (unconditionally)
        // to the successor block.
        builder.curBlock = edgeBlock;
        builder.emitBranch(succ);

        // Insert the new block into the block list
        // for the function.
        //
        // In principle, the order of this list shouldn't
        // affect the semantics of a program, but we
        // might want to be careful about ordering anyway.
        edgeBlock->insertAfter(pred);
    }
}

// Construct SSA form for a global value with code
void constructSSA(ConstructSSAContext* context)
{
    // First, detect and and break any critical edges in the CFG,
    // because our representation of SSA form doesn't allow for them.
    breakCriticalEdges(context);

    // Figure out what variables we can promote to
    // SSA temporaries.
    identifyPromotableVars(context);

    // If none of the variables are promote-able,
    // then we can exit without making any changes
    if (context->promotableVars.Count() == 0)
        return;

    // We are going to walk the blocks in order,
    // and try to process each, by replacing loads
    // and stores of promotable variables with simple values.

    auto globalVal = context->globalVal;
    for (auto bb = globalVal->firstBlock; bb; bb = bb->nextBlock)
    {
        auto blockInfo = new SSABlockInfo();
        blockInfo->block = bb;

        blockInfo->builder.sharedBuilder = &context->sharedBuilder;
        blockInfo->builder.curBlock = bb;
        blockInfo->builder.curFunc = globalVal;
        blockInfo->builder.insertBeforeInst = bb->lastInst;

        context->blockInfos.Add(bb, blockInfo);
    }
    for (auto bb = globalVal->firstBlock; bb; bb = bb->nextBlock)
    {
        auto blockInfo = * context->blockInfos.TryGetValue(bb);
        processBlock(context, bb, blockInfo);
    }

    // We need to transfer the logical arguments to our phi nodes
    // from the phi nodes back to the predecessor blocks that will
    // pass them in.
    for (auto bb = globalVal->firstBlock; bb; bb = bb->nextBlock)
    {
        auto blockInfo = *context->blockInfos.TryGetValue(bb);

        for (auto phiInfo : blockInfo->phis)
        {
            // If we replaced this phi with another value,
            // then we had better not include it in the result.
            if (phiInfo->replacement)
                continue;

            // We should add the phi as an explicit parameter of
            // the given block.
            bb->addParam(phiInfo->phi);

            UInt predCounter = 0;
            for (auto pp : bb->getPredecessors())
            {
                UInt predIndex = predCounter++;
                auto predInfo = *context->blockInfos.TryGetValue(pp);

                IRValue* operandVal = phiInfo->operands[predIndex].get();

                phiInfo->operands[predIndex].clear();

                predInfo->successorArgs.Add(operandVal);
            }
        }
    }

    // Some blocks may now need to pass along arguments to their sucessor,
    // which have been stored into the `SSABlockInfo::successorArgs` field.
    for (auto bb = globalVal->firstBlock; bb; bb = bb->nextBlock)
    {
        auto blockInfo = * context->blockInfos.TryGetValue(bb);

        // Sanity check: all blocks should be filled and sealed.
        assert(blockInfo->isSealed);
        assert(blockInfo->isFilled);

        // Don't do any work for blocks that don't need to pass along
        // values to the sucessor block.
        auto addedArgCount = blockInfo->successorArgs.Count();
        if (addedArgCount == 0)
            continue;

        // We need to replace the terminator instruction with one that
        // has additional arguments.

        IRTerminatorInst* oldTerminator = (IRTerminatorInst*) bb->getLastInst();
        assert(isTerminatorInst(oldTerminator));

        blockInfo->builder.insertBeforeInst = nullptr;

        auto oldArgCount = oldTerminator->argCount;
        auto newArgCount = oldArgCount + addedArgCount;

        List<IRValue*> newArgs;
        for (UInt aa = 0; aa < oldArgCount; ++aa)
        {
            newArgs.Add(oldTerminator->getArg(aa));
        }
        for (UInt aa = 0; aa < addedArgCount; ++aa)
        {
            newArgs.Add(blockInfo->successorArgs[aa]);
        }

        IRTerminatorInst* newTerminator = (IRTerminatorInst*)blockInfo->builder.emitIntrinsicInst(
            oldTerminator->type,
            oldTerminator->op,
            newArgCount,
            newArgs.Buffer());

        // Swap decorations over to the new instruction
        newTerminator->firstDecoration = oldTerminator->firstDecoration;
        oldTerminator->firstDecoration = nullptr;

        // A terminator better not have uses, so we shouldn't have
        // to replace them.
        assert(!oldTerminator->firstUse);


        // Okay, we should be clear to remove the old terminator
        oldTerminator->removeAndDeallocate();
    }

    // Now we should be able to go through and remove
    // of of the variables
    for (auto var : context->promotableVars)
    {
        var->removeAndDeallocate();
    }
}

// Construct SSA form for a global value with code
void constructSSA(IRModule* module, IRGlobalValueWithCode* globalVal)
{
    ConstructSSAContext context;
    context.globalVal = globalVal;

    context.sharedBuilder.module = module;
    context.sharedBuilder.session = module->session;

    constructSSA(&context);
}

void constructSSA(IRModule* module, IRGlobalValue* globalVal)
{
    switch (globalVal->op)
    {
    case kIROp_Func:
    case kIROp_global_var:
    case kIROp_global_constant:
        constructSSA(module, (IRGlobalValueWithCode*)globalVal);

    default:
        break;
    }
}

void constructSSA(IRModule* module)
{
    for (auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue())
    {
        constructSSA(module, gv);
    }
}

}
