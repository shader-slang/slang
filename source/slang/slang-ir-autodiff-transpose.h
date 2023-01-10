// slang-ir-autodiff-transpose.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-fwd.h"

namespace Slang
{

struct DiffTransposePass
{
    
    struct RevGradient
    {
        enum Flavor 
        {
            Simple,
            Swizzle,
            GetElement,
            GetDifferential,
            FieldExtract,

            Invalid
        };

        RevGradient() :
            flavor(Flavor::Invalid), targetInst(nullptr), revGradInst(nullptr), fwdGradInst(nullptr)
        { }
        
        RevGradient(Flavor flavor, IRInst* targetInst, IRInst* revGradInst, IRInst* fwdGradInst) : 
            flavor(flavor), targetInst(targetInst), revGradInst(revGradInst), fwdGradInst(fwdGradInst)
        { }

        RevGradient(IRInst* targetInst, IRInst* revGradInst, IRInst* fwdGradInst) : 
            flavor(Flavor::Simple), targetInst(targetInst), revGradInst(revGradInst), fwdGradInst(fwdGradInst)
        { }

        bool operator==(const RevGradient& other) const
        {
            return (other.targetInst == targetInst) && 
                (other.revGradInst == revGradInst) && 
                (other.fwdGradInst == fwdGradInst) &&
                (other.flavor == flavor);
        }
        
        IRInst* targetInst;
        IRInst* revGradInst;
        IRInst* fwdGradInst;

        Flavor flavor;
    };

    struct BlockPredecessorEntry
    {
        // Previous block.
        IRBlock*        prevBlock;

        // Integer value corresponding to this predecessor.
        IRIntegerValue* indexVal;
    };

    struct ControlFlowTranspositionInfo
    {
        // Variable used for recording control flow.
        IRVar*                      controlVar;

        // Info about all possible predecessor blocks.
        Dictionary<IRBlock*, IRInst*> predEntries;
    };

    DiffTransposePass(AutoDiffSharedContext* autodiffContext) : 
        autodiffContext(autodiffContext), pairBuilder(autodiffContext), diffTypeContext(autodiffContext)
    { }

    struct TranspositionResult
    {
        // Holds a set of pairs of 
        // (original-inst, inst-to-accumulate-for-orig-inst)
        List<RevGradient> revPairs;

        TranspositionResult()
        { }

        TranspositionResult(List<RevGradient> revPairs) : revPairs(revPairs)
        { }
    };

    struct FuncTranspositionInfo
    {
        // Inst that represents the reverse-mode derivative
        // of the *output* of the function.
        // 
        IRInst* dOutInst;

        // Mapping between *primal* insts in the forward-mode function, and the 
        // reverse-mode function
        //
        Dictionary<IRInst*, IRInst*>* primalsMap;
    };

    void transposeDiffBlocksInFunc(
        IRFunc* revDiffFunc,
        FuncTranspositionInfo transposeInfo)
    {
        // Grab all differentiable type information.
        diffTypeContext.setFunc(revDiffFunc);
        
        // Note down terminal primal and terminal differential blocks
        // since we need to link them up at the end.
        auto terminalPrimalBlocks = getTerminalPrimalBlocks(revDiffFunc);
        auto terminalDiffBlocks = getTerminalDiffBlocks(revDiffFunc);

        // Traverse all instructions/blocks in reverse (starting from the terminator inst)
        // look for insts/blocks marked with IRDifferentialInstDecoration,
        // and transpose them in the revDiffFunc.
        //
        IRBuilder builder;
        builder.init(autodiffContext->sharedBuilder);

        // Insert after the last block.
        builder.setInsertInto(revDiffFunc);

        List<IRBlock*> workList;

        // Build initial list of blocks to process by checking if they're differential blocks.
        for (IRBlock* block = revDiffFunc->getFirstBlock(); block; block = block->getNextBlock())
        {
            if (!isDifferentialInst(block))
            {
                // Skip blocks that aren't computing differentials.
                // At this stage we should have 'unzipped' the function
                // into blocks that either entirely deal with primal insts,
                // or entirely with differential insts.
                continue;
            }
            workList.add(block);
        }

        // Reverse the order of the blocks.
        workList.reverse();
        
        // Emit empty rev-mode blocks for every fwd-mode block.
        for (auto block : workList)
        {
            revBlockMap[block] = builder.emitBlock();
            builder.markInstAsDifferential(revBlockMap[block]);
        }

        // Keep track of first diff block, since this is where 
        // we'll emit temporary vars to hold per-block derivatives.
        // 
        firstRevDiffBlockMap[revDiffFunc] = revBlockMap[workList[0]];

        for (auto block : workList)
        {
            // Set dOutParameter as the transpose gradient for the return inst, if any.
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                this->addRevGradientForFwdInst(returnInst, RevGradient(returnInst, transposeInfo.dOutInst, nullptr));
            }

            IRBlock* revBlock = revBlockMap[block];
            this->transposeBlock(block, revBlock);
        }

        // Link the last differential fwd-mode block (which will be the first
        // rev-mode block) as the successor to the last primal block.
        // We assume that the original function is in single-return form
        // So, there should be exactly 1 'last' block of each type.
        // 
        {
            SLANG_ASSERT(terminalPrimalBlocks.getCount() == 1);
            SLANG_ASSERT(terminalDiffBlocks.getCount() == 1);

            auto terminalPrimalBlock = terminalPrimalBlocks[0];
            auto terminalRevBlock = as<IRBlock>(revBlockMap[terminalDiffBlocks[0]]);

            terminalPrimalBlock->getTerminator()->removeAndDeallocate();
            
            IRBuilder subBuilder(builder.getSharedBuilder());
            subBuilder.setInsertInto(terminalPrimalBlock);

            // There should be no parameters in the first reverse-mode block.
            SLANG_ASSERT(terminalRevBlock->getFirstParam() == nullptr);

            subBuilder.emitBranch(terminalRevBlock);
        }

        // Remove fwd-mode blocks.
        for (auto block : workList)
        {
            block->removeAndDeallocate();
        }
    }

    // Fetch or create a gradient accumulator var
    // corresponding to a inst. These are used to
    // accumulate gradients across blocks.
    //
    IRVar* getOrCreateAccumulatorVar(IRInst* fwdInst)
    {
        // Check if we have a var already.
        if (revAccumulatorVarMap.ContainsKey(fwdInst))
            return revAccumulatorVarMap[fwdInst];
        
        IRBuilder tempVarBuilder(autodiffContext->sharedBuilder);
        
        IRBlock* firstDiffBlock = firstRevDiffBlockMap[as<IRFunc>(fwdInst->getParent()->getParent())];
        tempVarBuilder.setInsertBefore(firstDiffBlock->getTerminator());
        
        auto primalType = tryGetPrimalTypeFromDiffInst(fwdInst);
        auto diffType = fwdInst->getDataType();

        auto zeroMethod = diffTypeContext.getZeroMethodForType(
                &tempVarBuilder,
                primalType);

        SLANG_ASSERT(zeroMethod);

        // Emit a var in the top-level differential block to hold the gradient, 
        // and initialize it.
        auto tempRevVar = tempVarBuilder.emitVar(diffType);
        auto diffZero = tempVarBuilder.emitCallInst(
            diffType,
            zeroMethod,
            List<IRInst*>());
        tempVarBuilder.emitStore(tempRevVar, diffZero);

        revAccumulatorVarMap[fwdInst] = tempRevVar;

        return tempRevVar;
    }

    bool isInstUsedOutsideParentBlock(IRInst* inst)
    {
        auto currBlock = inst->getParent();

        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            if (use->getUser()->getParent() != currBlock)
                return true;
        }

        return false;
    }
    
    void transposeBlock(IRBlock* fwdBlock, IRBlock* revBlock)
    {
        IRBuilder builder;
        builder.init(autodiffContext->sharedBuilder);
 
        // Insert into our reverse block.
        builder.setInsertInto(revBlock);

        // Check if this block has any 'outputs' (in the form of phi args
        // sent to the successor bvock)
        // 
        if (auto branchInst = as<IRUnconditionalBranch>(fwdBlock->getTerminator()))
        {
            for (UIndex ii = 0; ii < branchInst->getArgCount(); ii++)
            {
                auto arg = branchInst->getArg(ii);
                if (isDifferentialInst(arg))
                {
                    auto diffType = arg->getDataType();
                    auto revParam = builder.emitParam(diffType);

                    addRevGradientForFwdInst(
                        arg, 
                        RevGradient(
                            RevGradient::Flavor::Simple,
                            arg,
                            revParam,
                            nullptr));
                }
            }
        }

        // Move pointer & reference insts to the top of the reverse-mode block.
        List<IRInst*> nonValueInsts;
        for (IRInst* child = fwdBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
        {
            // If the instruction is pointer typed, it's not actually computing a value.
            // 
            if (as<IRPtrTypeBase>(child->getDataType()))
                nonValueInsts.add(child);
            
            // Slang doesn't support function values. So if we see a func-typed inst
            // it's proabably a reference to a function.
            // 
            if (as<IRFuncType>(child->getDataType()))
                nonValueInsts.add(child);
        }

        for (auto inst : nonValueInsts)
        {
            inst->insertAtEnd(revBlock);
        }


        // Then, go backwards through the regular instructions, and transpose them into the new
        // rev block.
        // Note the 'reverse' traversal here.
        // 
        for (IRInst* child = fwdBlock->getLastChild(); child; child = child->getPrevInst())
        {
            if (as<IRDecoration>(child) || as<IRParam>(child))
                continue;

            transposeInst(&builder, child);
        }

        // After processing the block's instructions, we 'flush' any remaining gradients 
        // in the assignments map.
        // For now, these are only function parameter gradients (or of the form IRLoad(IRParam))
        // TODO: We should be flushing *all* gradients accumulated in this block to some 
        // function scope variable, since control flow can affect what blocks contribute to
        // for a specific inst.
        // 
        for (auto pair : gradientsMap)
        {
            if (auto loadInst = as<IRLoad>(pair.Key))
                accumulateGradientsForLoad(&builder, loadInst);
        }

        // Do the same thing with the phi parameters if the block.
        List<IRInst*> phiParamRevGradInsts;
        for (IRParam* param = fwdBlock->getFirstParam(); param; param = param->getNextParam())
        {
            if (hasRevGradients(param))
            {
                auto gradients = popRevGradients(param);

                auto gradInst = emitAggregateValue(
                    &builder,
                    tryGetPrimalTypeFromDiffInst(param),
                    gradients);
                
                phiParamRevGradInsts.add(gradInst);
            }
        }

        // Also handle any remaining gradients for insts that appear in prior blocks.
        List<IRInst*> externInsts; // Holds insts in a different block, same function.
        List<IRInst*> globalInsts; // Holds insts in the global scope.
        for (auto pair : gradientsMap)
        {
            auto instParent = pair.Key->getParent();
            if (instParent != fwdBlock)
            {
                if (instParent->getParent() == fwdBlock->getParent())
                    externInsts.add(pair.Key);
                
                if (as<IRModuleInst>(instParent))
                    globalInsts.add(pair.Key);
            }
        }

        for (auto externInst : externInsts)
        {
            auto primalType = tryGetPrimalTypeFromDiffInst(externInst);
            SLANG_ASSERT(primalType);

            if (auto accVar = getOrCreateAccumulatorVar(externInst))
            {
                // Accumulate all gradients, including our accumulator variable,
                // into one inst.
                //
                auto gradients = popRevGradients(externInst);
                gradients.add(RevGradient(externInst, builder.emitLoad(accVar), nullptr));

                auto gradInst = emitAggregateValue(
                    &builder,
                    primalType,
                    gradients);
                
                builder.emitStore(accVar, gradInst);
            }
        }

        // For now, we're not going to handle global insts, and simply ignore them
        // Eventually, we want to turn these into global writes.
        // 
        for (auto globalInst : globalInsts)
        {
            if (hasRevGradients(globalInst))
                popRevGradients(globalInst);
        }

        // We _should_ be completely out of gradients to process at this point.
        SLANG_ASSERT(gradientsMap.Count() == 0);

        emitTerminator(&builder, fwdBlock, phiParamRevGradInsts);
    }

    void transposeInst(IRBuilder* builder, IRInst* inst)
    {
        // Look for gradient entries for this inst.
        List<RevGradient> gradients;
        if (hasRevGradients(inst))
            gradients = popRevGradients(inst);

        IRType* primalType = tryGetPrimalTypeFromDiffInst(inst);

        if (!primalType)
        {
            // Special-case instructions.
            if (auto returnInst = as<IRReturn>(inst))
            {
                auto returnPairType = as<IRDifferentialPairType>(
                    tryGetPrimalTypeFromDiffInst(returnInst->getVal()));
                primalType = returnPairType->getValueType();
            }
            else if (auto loadInst = as<IRLoad>(inst))
            {
                // TODO: Unzip loads properly to avoid having to side-step this check for IRLoad
                if (auto pairType = as<IRDifferentialPairType>(loadInst->getDataType()))
                {
                    primalType = pairType->getValueType();
                }   
            }
        }

        if (!primalType)
        {
            // Check for special insts for which a reverse-mode gradient doesn't apply.
            if(!as<IRStore>(inst) && !as<IRTerminatorInst>(inst))
            {
                SLANG_UNEXPECTED("Could not resolve primal type for diff inst");
            }

            // If we still can't resolve a differential type, there shouldn't 
            // be any gradients to aggregate.
            // 
            SLANG_ASSERT(gradients.getCount() == 0);
        }

        // Is this inst used in another differential block?
        // Emit a function-scope accumulator variable, and include it's value.
        // Also, we ignore this if it's a load since those are turned into stores
        // on a per-block basis. (We should change this behaviour to treat loads like
        // any other inst)
        // 
        if (isInstUsedOutsideParentBlock(inst) && !as<IRLoad>(inst))
        {
            auto accVar = getOrCreateAccumulatorVar(inst);
            gradients.add(
                RevGradient(inst, builder->emitLoad(accVar), nullptr));
        }
        
        // Emit the aggregate of all the gradients here. 
        // This will form the total derivative for this inst.
        auto revValue = emitAggregateValue(builder, primalType, gradients);

        auto transposeResult = transposeInst(builder, inst, revValue);
        
        if (auto fwdNameHint = inst->findDecoration<IRNameHintDecoration>())
        {
            StringBuilder sb;
            sb << fwdNameHint->getName() << "_T";
            builder->addNameHintDecoration(revValue, sb.getUnownedSlice());
        }
        
        // Add the new results to the gradients map.
        for (auto gradient : transposeResult.revPairs)
        {
            addRevGradientForFwdInst(gradient.targetInst, gradient);
        }
    }

    TranspositionResult transposeCall(IRBuilder* builder, IRCall* fwdCall, IRInst* revValue)
    {
        auto fwdDiffCallee = as<IRForwardDifferentiate>(fwdCall->getCallee());

        // If the callee is not a fwd-differentiate(fn), then there's only two
        // cases. This is a call to something that doesn't need to be transposed
        // or this is a user-written function calling something that isn't marked
        // with IRForwardDifferentiate, but is handling differentials. 
        // We currently do not handle the latter.
        // However, if we see a callee with no parameters, we can just skip over.
        // since there's nothing to backpropagate to.
        // 
        if (!fwdDiffCallee)
        {
            if (fwdCall->getArgCount() == 0)
            {
                return TranspositionResult(List<RevGradient>());
            }
            else
            {
                SLANG_UNIMPLEMENTED_X(
                    "This case should only trigger on a user-defined fwd-mode function"
                    " calling another user-defined function not marked with __fwd_diff()");
            }
        }

        auto baseFn = fwdDiffCallee->getBaseFn();

        List<IRInst*> args;
        List<IRType*> argTypes;
        List<bool> isArgPtrTyped;

        for (UIndex ii = 0; ii < fwdCall->getArgCount(); ii++)
        {
            auto arg = fwdCall->getArg(ii);
            
            // If this isn't a ptr-type, make a var.
            if (!as<IRPtrTypeBase>(arg->getDataType()) && as<IRDifferentialPairType>(arg->getDataType()))
            {
                auto pairType = as<IRDifferentialPairType>(arg->getDataType());

                auto var = builder->emitVar(arg->getDataType());

                SLANG_ASSERT(as<IRMakeDifferentialPair>(arg));

                // Initialize this var to (arg.primal, 0).
                builder->emitStore(
                    var, 
                    builder->emitMakeDifferentialPair(
                        arg->getDataType(),
                        as<IRMakeDifferentialPair>(arg)->getPrimalValue(),
                        builder->emitCallInst(
                            (IRType*)diffTypeContext.getDifferentialForType(builder, pairType->getValueType()),
                            diffTypeContext.getZeroMethodForType(builder, pairType->getValueType()),
                            List<IRInst*>())));
                
                args.add(var);
                argTypes.add(builder->getInOutType(pairType));
                isArgPtrTyped.add(false);
            }
            else
            {
                args.add(arg);
                argTypes.add(arg->getDataType());
                isArgPtrTyped.add(true);
            }
        }

        args.add(revValue);
        argTypes.add(revValue->getDataType());

        auto revFnType = builder->getFuncType(argTypes, builder->getVoidType());
        auto revCallee = builder->emitBackwardDifferentiateInst(
            revFnType,
            baseFn);

        builder->emitCallInst(revFnType->getResultType(), revCallee, args);

        List<RevGradient> gradients;
        for (UIndex ii = 0; ii < fwdCall->getArgCount(); ii++)
        {
            // Is this arg relevant to auto-diff?
            if (as<IRDifferentialPairType>(as<IRPtrTypeBase>(args[ii]->getDataType())->getValueType()))
            {
                // If this is ptr typed, ignore (the gradient will be accumulated on the pointer)
                // automatically.
                // 
                if (!isArgPtrTyped[ii])
                {
                    auto diffArgType = (IRType*)diffTypeContext.getDifferentialForType(
                        builder, 
                        as<IRDifferentialPairType>(
                            as<IRPtrTypeBase>(argTypes[ii])->getValueType())->getValueType());
                    auto diffArgPtrType = builder->getPtrType(kIROp_PtrType, diffArgType);
                    
                    gradients.add(RevGradient(
                        RevGradient::Flavor::Simple,
                        fwdCall->getArg(ii),
                        builder->emitLoad(
                            builder->emitDifferentialPairAddressDifferential(
                                diffArgPtrType,
                                args[ii])), 
                        nullptr));
                }
            }
        }
        
        return TranspositionResult(gradients);
    }

    IRBlock* getPrimalBlock(IRBlock* fwdBlock)
    {
        if (auto fwdDiffDecoration = fwdBlock->findDecoration<IRDifferentialInstDecoration>())
        {
            return as<IRBlock>(fwdDiffDecoration->getPrimalInst());
        }

        return nullptr;
    }

    IRBlock* getFirstCodeBlock(IRGlobalValueWithCode* func)
    {
        return func->getFirstBlock()->getNextBlock();
    }

    List<IRBlock*> getTerminalPrimalBlocks(IRGlobalValueWithCode* func)
    {
        // 'Terminal' primal blocks are those that branch into a differential block.
        List<IRBlock*> terminalPrimalBlocks;
        for (auto block : func->getBlocks())
            for (auto successor : block->getSuccessors())
                if (!isDifferentialInst(block) && isDifferentialInst(successor))
                    terminalPrimalBlocks.add(block);

        return terminalPrimalBlocks;
    }

    List<IRBlock*> getTerminalDiffBlocks(IRGlobalValueWithCode* func)
    {
        // Terminal differential blocks are those with a return statement.
        // Note that this method is designed to work with Fwd-Mode blocks, 
        // and this logic will be different for Rev-Mode blocks.
        // 
        List<IRBlock*> terminalDiffBlocks;
        for (auto block : func->getBlocks())
            if (as<IRReturn>(block->getTerminator()))
                terminalDiffBlocks.add(block);

        return terminalDiffBlocks;
    }
    
    IRInst* addPredecessorForBlock(IRBlock* block, IRBlock* predBlock)
    {
        if (!this->blockEntries.ContainsKey(block))
        {
            // We haven't encountered this block yet, create a var for this in the 
            // first code block.
            auto firstCodeBlock = getFirstCodeBlock(block->getParent());

            IRBuilder subBuilder(this->autodiffContext->sharedBuilder);
            subBuilder.setInsertBefore(firstCodeBlock->getTerminator());
            auto controlVar = subBuilder.emitVar(subBuilder.getUIntType());

            ControlFlowTranspositionInfo info;
            info.controlVar = controlVar;

            this->blockEntries[block] = info;
        }

        auto info = this->blockEntries[block];

        // Does precessor block already exist?
        if (info.GetValue().predEntries.ContainsKey(predBlock))
        {
            return info.GetValue().predEntries[predBlock];
        }

        // Otherwise, create an entry..
        auto uniqueIndex = info.GetValue().predEntries.Count();

        IRBuilder builder(this->autodiffContext->sharedBuilder);
        auto uniqueIndexLiteral = builder.getIntValue(builder.getUIntType(), uniqueIndex);
        
        info.GetValue().predEntries[predBlock] = uniqueIndexLiteral;

        return uniqueIndexLiteral;
    }

    IRVar* getControlVar(IRBlock* block)
    {
        return this->blockEntries[block].GetValue().controlVar;
    }
    
    // Inserts a block between the branch from fwdPredecessorBlock to fwdBlock, which sets a control
    // variable to a unique index.
    //  
    IRInst* insertPreludeForPredecessor(IRBlock* fwdBlock, IRBlock* fwdPredecessorBlock)
    {
        // Get associated primal blocks for both the differential blocks.
        auto primalPredecessorBlock = getPrimalBlock(fwdPredecessorBlock);
        SLANG_ASSERT(primalPredecessorBlock);

        auto primalBlock = getPrimalBlock(fwdBlock);
        SLANG_ASSERT(primalBlock);

        // Add this block as a predecessor, and get an unique index (as an integer literal)
        auto indexVal = addPredecessorForBlock(fwdBlock, fwdPredecessorBlock);

        IRBuilder subBuilder(this->autodiffContext->sharedBuilder);
        subBuilder.setInsertInto(primalPredecessorBlock->getParent());

        IRInst* preludeBlock = subBuilder.emitBlock();
        preludeBlock->insertAfter(primalPredecessorBlock);

        // Copy over phi parameters.
        List<IRInst*> phiParams;
        for (auto param = primalBlock->getFirstParam(); param; param = param->getNextParam())
        {
            phiParams.add(subBuilder.emitParam(param->getDataType()));
        }

        auto controlVar = getControlVar(fwdBlock);
        subBuilder.emitStore(controlVar, indexVal);

        // Branch into the successor block using all the same phi parameters.
        subBuilder.emitBranch(primalBlock, phiParams.getCount(), phiParams.getBuffer());

        // Scan through uses of primalBlock to find the ones that are in
        // primalPredecessorBlock, and replace them with branches to
        // preludeBlock.
        // 
        List<IRUse*> relevantUses;
        for (auto use = primalBlock->firstUse; use; use = use->nextUse)
        {
            if (use->getUser()->getParent() == primalPredecessorBlock)
                relevantUses.add(use);
        }

        for (auto use : relevantUses)
            use->set(preludeBlock);

        return indexVal;
    }

    bool doesBlockHaveDifferentialPredecessors(IRBlock* fwdBlock)
    {
        for (auto block : fwdBlock->getPredecessors())
        {
            if (isDifferentialInst(block))
            {
                return true;
            }
        }

        return false;
    }

    void emitTerminator(IRBuilder* builder, IRBlock* fwdBlockInst, List<IRInst*> phiParamGrads)
    {
        // If this block has no differential predecessors, add a return statement.
        if (!doesBlockHaveDifferentialPredecessors(fwdBlockInst))
        {
            // Emit a void return.
            builder->emitReturn();
            return;
        }

        for (auto predecessor : fwdBlockInst->getPredecessors())
        {
            // Insert code into the *primal* version of the predecessor block
            // to set the control variable to indexVal before branching.
            // 
            insertPreludeForPredecessor(fwdBlockInst, predecessor);
        }

        List<IRBlock*> revPredecessorBlocks;
        List<IRInst*> indexVals;

        for (auto blockEntry : this->blockEntries[fwdBlockInst].GetValue().predEntries)
        {
            revPredecessorBlocks.add(revBlockMap[blockEntry.Key]);
            indexVals.add(blockEntry.Value);
        }

        auto predCount = revPredecessorBlocks.getCount();

        SLANG_ASSERT(predCount > 0);

        List<IRBlock*> intermediateBranchBlocks;

        IRBuilder branchBlockBuilder(builder->getSharedBuilder());
        
        branchBlockBuilder.setInsertInto(builder->getFunc());

        // Make a block to unconditionally branch into predecessor-0 with the
        // appropriate phi gradients.
        // 
        auto firstBranchBlock = branchBlockBuilder.emitBlock();
        intermediateBranchBlocks.add(firstBranchBlock);

        branchBlockBuilder.markInstAsDifferential(firstBranchBlock);
        branchBlockBuilder.emitBranch(
            revPredecessorBlocks[0],
            phiParamGrads.getCount(),
            phiParamGrads.getBuffer());

        // Create a builder to insert loads and comparison insts to figure
        // out which block to branch into based on the control vars.
        // This builder is set up to emit into the last _primal_ block.
        // 
        IRBuilder booleanIndicatorBuilder(builder->getSharedBuilder());
        auto terminalPrimalBlock = getTerminalPrimalBlocks(builder->getFunc())[0];

        booleanIndicatorBuilder.setInsertBefore(terminalPrimalBlock->getTerminator());
        
        if (predCount == 1)
        {
            builder->emitBranch(firstBranchBlock);
        }
        else
        {
            IRBuilder ladderBlockBuilder(builder->getSharedBuilder());
            ladderBlockBuilder.setInsertInto(builder->getFunc());

            // TODO: For now, we're trivially setting 'afterBlock' to 
            // the first reverse block. This is not really optimal for the 
            // restructuring passes since the 'then' and 'else' regions
            // can have significant overlap.
            // 
            auto firstFwdDiffBlock = (*terminalPrimalBlock->getSuccessors().begin());
            SLANG_ASSERT(firstFwdDiffBlock);

            auto defaultAfterBlock = revBlockMap[firstFwdDiffBlock];

            auto nextLadderBlock = firstBranchBlock;
            for (Index ii = 0; ii < predCount - 1; ii++)
            {
                // Make the 'leaf' block. This just branches into
                // predecessor-i+1 with the appropriate phi args.
                // 
                branchBlockBuilder.setInsertInto(branchBlockBuilder.getFunc());
                
                auto thisIndexBlock = branchBlockBuilder.emitBlock();
                intermediateBranchBlocks.add(thisIndexBlock);

                branchBlockBuilder.markInstAsDifferential(thisIndexBlock);
                branchBlockBuilder.emitBranch(
                    revPredecessorBlocks[ii+1],
                    phiParamGrads.getCount(),
                    phiParamGrads.getBuffer());

                // Emit a boolean inst to represent whether we need to branch into
                // block ii.
                auto blockIndicatorInst = booleanIndicatorBuilder.emitEql(
                        booleanIndicatorBuilder.emitLoad(getControlVar(fwdBlockInst)),
                        indexVals[ii+1]);

                // Create a block to branch between i+1 and the rest of the ladder so far
                // (0 ... i)
                //
                auto upperLadderBlock = ladderBlockBuilder.emitBlock();
                intermediateBranchBlocks.add(upperLadderBlock);

                ladderBlockBuilder.markInstAsDifferential(upperLadderBlock);
                ladderBlockBuilder.emitIfElse(
                    blockIndicatorInst,
                    thisIndexBlock,
                    nextLadderBlock,
                    defaultAfterBlock);
                
                nextLadderBlock = upperLadderBlock;
            }

            // Branch into the last ladder block.
            builder->emitBranch(nextLadderBlock);
        }

        // Insert all intermediate blocks in the order they were created, right after
        // the current reverse block.
        
        auto revBlock = revBlockMap[fwdBlockInst];
        SLANG_ASSERT(revBlock);

        for (auto block : intermediateBranchBlocks)
        {
            block->insertAfter(revBlock);
        }

        return;
    }
    
    TranspositionResult transposeInst(IRBuilder* builder, IRInst* fwdInst, IRInst* revValue)
    {

        // Dispatch logic.
        switch(fwdInst->getOp())
        {
            case kIROp_Add:
            case kIROp_Mul:
            case kIROp_Sub: 
                return transposeArithmetic(builder, fwdInst, revValue);

            case kIROp_Call:
                return transposeCall(builder, as<IRCall>(fwdInst), revValue);
            
            case kIROp_swizzle:
                return transposeSwizzle(builder, as<IRSwizzle>(fwdInst), revValue);
            
            case kIROp_FieldExtract:
                return transposeFieldExtract(builder, as<IRFieldExtract>(fwdInst), revValue);

            case kIROp_Return:
                return transposeReturn(builder, as<IRReturn>(fwdInst), revValue);
            
            case kIROp_Store:
                return transposeStore(builder, as<IRStore>(fwdInst), revValue);
            
            case kIROp_Load:
                return transposeLoad(builder, as<IRLoad>(fwdInst), revValue);

            case kIROp_MakeDifferentialPair:
                return transposeMakePair(builder, as<IRMakeDifferentialPair>(fwdInst), revValue);

            case kIROp_DifferentialPairGetDifferential:
                return transposeGetDifferential(builder, as<IRDifferentialPairGetDifferential>(fwdInst), revValue);
            
            case kIROp_MakeVector:
                return transposeMakeVector(builder, fwdInst, revValue);
            
            case kIROp_unconditionalBranch:
            case kIROp_conditionalBranch:
            case kIROp_ifElse:
            case kIROp_loop:
            {
                // Ignore. transposeBlock() should take care of adding the
                // appropriate branch instruction.
                return TranspositionResult();
            }

            default:
                SLANG_ASSERT_FAILURE("Unhandled instruction");
        }
    }

    TranspositionResult transposeLoad(IRBuilder* builder, IRLoad* fwdLoad, IRInst* revValue)
    {
        auto revPtr = fwdLoad->getPtr();

        auto primalType = tryGetPrimalTypeFromDiffInst(fwdLoad);
        auto loadType = fwdLoad->getDataType();

        List<RevGradient> gradients(RevGradient(
            revPtr,
            revValue,
            nullptr));

        if (usedPtrs.contains(revPtr))
        {
            // Re-emit a load to get the _current_ value of revPtr.
            auto revCurrGrad = builder->emitLoad(revPtr);

            // Add the current value to the aggregation list.
            gradients.add(RevGradient(
                revPtr,
                revCurrGrad,
                nullptr));
        }
        else
        {
            usedPtrs.add(revPtr);
        }
        
        // Get the _total_ value.
        auto aggregateGradient = emitAggregateValue(
            builder,
            primalType,
            gradients);
        
        if (as<IRDifferentialPairType>(loadType))
        {
            auto primalPtr = builder->emitDifferentialPairAddressPrimal(revPtr);
            auto primalVal = builder->emitLoad(primalPtr);

            auto pairVal = builder->emitMakeDifferentialPair(loadType, primalVal, aggregateGradient);

            builder->emitStore(revPtr, pairVal);
        }
        else
        {
            // Store this back into the pointer.
            builder->emitStore(revPtr, aggregateGradient);
        }

        return TranspositionResult(List<RevGradient>());
    }

    TranspositionResult transposeStore(IRBuilder* builder, IRStore* fwdStore, IRInst*)
    {
        return TranspositionResult(
                    List<RevGradient>(
                        RevGradient(
                            RevGradient::Flavor::Simple,
                            fwdStore->getVal(),
                            builder->emitLoad(fwdStore->getPtr()),
                            fwdStore)));
    }

    TranspositionResult transposeSwizzle(IRBuilder*, IRSwizzle* fwdSwizzle, IRInst* revValue)
    {
        // (A = p.x) -> (p = float3(dA, 0, 0))
        return TranspositionResult(
                    List<RevGradient>(
                        RevGradient(
                            RevGradient::Flavor::Swizzle,
                            fwdSwizzle->getBase(),
                            revValue,
                            fwdSwizzle)));
    }

    
    TranspositionResult transposeFieldExtract(IRBuilder*, IRFieldExtract* fwdExtract, IRInst* revValue)
    {
        return TranspositionResult(
                    List<RevGradient>(
                        RevGradient(
                            RevGradient::Flavor::FieldExtract,
                            fwdExtract->getBase(),
                            revValue,
                            fwdExtract)));
    }

    TranspositionResult transposeMakePair(IRBuilder*, IRMakeDifferentialPair* fwdMakePair, IRInst* revValue)
    {
        // Even though makePair returns a pair of (primal, differential)
        // revValue will only contain the reverse-value for 'differential'
        //
        // (P = (A, dA)) -> (dA += dP)
        //
        return TranspositionResult(
                    List<RevGradient>(
                        RevGradient(
                            RevGradient::Flavor::Simple,
                            fwdMakePair->getDifferentialValue(), 
                            revValue,
                            fwdMakePair)));
    }

    TranspositionResult transposeGetDifferential(IRBuilder*, IRDifferentialPairGetDifferential* fwdGetDiff, IRInst* revValue)
    {
        // (A = GetDiff(P)) -> (dP.d += dA)
        return TranspositionResult(
                    List<RevGradient>(
                        RevGradient(
                            RevGradient::Flavor::Simple,
                            fwdGetDiff->getBase(),
                            revValue,
                            fwdGetDiff)));
    }

    TranspositionResult transposeMakeVector(IRBuilder* builder, IRInst* fwdMakeVector, IRInst* revValue)
    {
        // For now, we support only vector types. Extend this to other built-in types if necessary.
        SLANG_ASSERT(fwdMakeVector->getOp() == kIROp_MakeVector);

        List<RevGradient> gradients;
        for (UIndex ii = 0; ii < fwdMakeVector->getOperandCount(); ii++)
        {
            auto gradAtIndex = builder->emitElementExtract(
                fwdMakeVector->getOperand(ii)->getDataType(),
                revValue,
                builder->getIntValue(builder->getIntType(), ii));

            gradients.add(RevGradient(
                            RevGradient::Flavor::Simple,
                            fwdMakeVector->getOperand(ii),
                            gradAtIndex,
                            fwdMakeVector));
        }

        // (A = float3(X, Y, Z)) -> [(dX += dA), (dY += dA), (dZ += dA)]
        return TranspositionResult(gradients);
    }

    // Gather all reverse-mode gradients for a Load inst, aggregate them and store them in the ptr.
    // 
    void accumulateGradientsForLoad(IRBuilder* builder, IRLoad* revLoad)
    {
        return transposeInst(builder, revLoad);
    }

    TranspositionResult transposeReturn(IRBuilder*, IRReturn* fwdReturn, IRInst* revValue)
    {
        // TODO: This check needs to be changed to something like: isRelevantDifferentialPair()
        if (as<IRDifferentialPairType>(fwdReturn->getVal()->getDataType()))
        {
            // Simply pass on the gradient to the previous inst.
            // (Even if the return value is pair typed, we only care about the differential part)
            // So this will remain a 'simple' gradient.
            // 
            return TranspositionResult(
                        List<RevGradient>(
                            RevGradient(
                                RevGradient::Flavor::Simple,
                                fwdReturn->getVal(), 
                                revValue,
                                fwdReturn)));
        }
        else
        {
            // (return A) -> (empty)
            return TranspositionResult();
        }
    }

    IRInst* promoteToType(IRBuilder* builder, IRType* targetType, IRInst* inst)
    {
        auto currentType = inst->getDataType();

        switch (targetType->getOp())
        {

        case kIROp_VectorType:
        {
            // current type should be a scalar.
            SLANG_RELEASE_ASSERT(!as<IRVectorType>(currentType->getDataType()));

            auto targetVectorType = as<IRVectorType>(targetType);
            
            List<IRInst*> operands;
            for (Index ii = 0; ii < as<IRIntLit>(targetVectorType->getElementCount())->getValue(); ii++)
            {
                operands.add(inst);
            }

            IRInst* newInst = builder->emitMakeVector(targetType, operands.getCount(), operands.getBuffer());
            
            if (isDifferentialInst(inst))
                builder->markInstAsDifferential(newInst);
            
            return newInst;
        }
        
        default:
            SLANG_ASSERT_FAILURE("Unhandled target type for promotion");
        }
    }

    IRInst* promoteOperandsToTargetType(IRBuilder* builder, IRInst* fwdInst)
    {
        auto oldLoc = builder->getInsertLoc();
        // If operands are not of the same type, cast them to the target type.
        IRType* targetType = fwdInst->getDataType();

        bool needNewInst = false;
        
        List<IRInst*> newOperands;
        for (UIndex ii = 0; ii < fwdInst->getOperandCount(); ii++)
        {
            auto operand = fwdInst->getOperand(ii);
            if (operand->getDataType() != targetType)
            {
                // Insert new operand just after the old operand, so we have the old
                // operands available.
                // 
                builder->setInsertAfter(operand);

                IRInst* newOperand = promoteToType(builder, targetType, operand);
                newOperands.add(newOperand);

                needNewInst = true;
            }
            else
            {
                newOperands.add(operand);
            }
        }

        if(needNewInst)
        {
            builder->setInsertAfter(fwdInst);
            IRInst* newInst = builder->emitIntrinsicInst(
                fwdInst->getDataType(),
                fwdInst->getOp(),
                newOperands.getCount(),
                newOperands.getBuffer());
            
            builder->setInsertLoc(oldLoc);

            if (isDifferentialInst(fwdInst))
                builder->markInstAsDifferential(newInst);

            return newInst;
        }
        else
        {
            builder->setInsertLoc(oldLoc);
            return fwdInst;
        }
    }

    TranspositionResult transposeArithmetic(IRBuilder* builder, IRInst* fwdInst, IRInst* revValue)
    {
        
        // Only handle arithmetic on uniform types. If the types aren't uniform, we need some
        // promotion/demotion logic. Note that this can create a new inst in place of the old, but since we're
        // at the transposition step for the old inst, and already have it's aggregate gradient, there's
        // no need to worry about the 'gradientsMap' being out-of-date
        // TODO: There are some opportunities for optimization here (otherwise we might be increasing the intermediate
        // data size unnecessarily)
        // 
        fwdInst = promoteOperandsToTargetType(builder, fwdInst);

        auto operandType = fwdInst->getOperand(0)->getDataType();

        switch(fwdInst->getOp())
        {
            case kIROp_Add:
            {
                // (Out = dA + dB) -> [(dA += dOut), (dB += dOut)]
                return TranspositionResult(
                        List<RevGradient>(
                            RevGradient(
                                fwdInst->getOperand(0),
                                revValue,
                                fwdInst),
                            RevGradient(
                                fwdInst->getOperand(1),
                                revValue,
                                fwdInst)));
            }
            case kIROp_Sub:
            {
                // (Out = dA - dB) -> [(dA += dOut), (dB -= dOut)]
                return TranspositionResult(
                        List<RevGradient>(
                            RevGradient(
                                fwdInst->getOperand(0),
                                revValue,
                                fwdInst),
                            RevGradient(
                                fwdInst->getOperand(1),
                                builder->emitNeg(
                                    revValue->getDataType(), revValue),
                                fwdInst)));
            }
            case kIROp_Mul: 
            {
                if (isDifferentialInst(fwdInst->getOperand(0)))
                {
                    // (Out = dA * B) -> (dA += B * dOut)
                    return TranspositionResult(
                        List<RevGradient>(
                            RevGradient(
                                fwdInst->getOperand(0),
                                builder->emitMul(operandType, fwdInst->getOperand(1), revValue),
                                fwdInst)));
                }
                else if (isDifferentialInst(fwdInst->getOperand(1)))
                {
                    // (Out = A * dB) -> (dB += A * dOut)
                    return TranspositionResult(
                        List<RevGradient>(
                            RevGradient(
                                fwdInst->getOperand(1),
                                builder->emitMul(operandType, fwdInst->getOperand(0), revValue),
                                fwdInst)));
                }
                else
                {
                    SLANG_ASSERT_FAILURE("Neither operand of a mul instruction is a differential inst");
                }
            }   

            default:
                SLANG_ASSERT_FAILURE("Unhandled arithmetic");
        }
    }

    RevGradient materializeSwizzleGradients(IRBuilder* builder, IRType* aggPrimalType, List<RevGradient> gradients)
    {
        List<RevGradient> simpleGradients;

        for (auto gradient : gradients)
        {
            // Peek at the fwd-mode swizzle inst to see what type we need to materialize.
            IRSwizzle* fwdSwizzleInst = as<IRSwizzle>(gradient.fwdGradInst);
            SLANG_ASSERT(fwdSwizzleInst);

            auto baseType = fwdSwizzleInst->getBase()->getDataType();

            // Assume for now that this is a vector type.
            SLANG_ASSERT(as<IRVectorType>(baseType));

            IRInst* elementCountInst = as<IRVectorType>(baseType)->getElementCount();
            IRType* elementType = as<IRVectorType>(baseType)->getElementType();

            // Must be a concrete integer (auto-diff must always occur after specialization)
            // For generic code, we would need to generate a for loop.
            // 
            SLANG_ASSERT(as<IRIntLit>(elementCountInst));

            auto elementCount = as<IRIntLit>(elementCountInst)->getValue();

            // Make a list of 0s
            List<IRInst*> constructArgs;
            auto zeroMethod = diffTypeContext.getZeroMethodForType(builder, elementType);

            // Must exist. 
            SLANG_ASSERT(zeroMethod);

            auto zeroValueInst = builder->emitCallInst(elementType, zeroMethod, List<IRInst*>());
            
            for (Index ii = 0; ii < ((Index)elementCount); ii++)
            {
                constructArgs.add(zeroValueInst);
            }

            // Replace swizzled elements with their gradients.
            for (Index ii = 0; ii < ((Index)fwdSwizzleInst->getElementCount()); ii++)
            {
                auto sourceIndex = ii;
                auto targetIndexInst = fwdSwizzleInst->getElementIndex(ii);
                SLANG_ASSERT(as<IRIntLit>(targetIndexInst));
                auto targetIndex = as<IRIntLit>(targetIndexInst)->getValue();

                // Special-case for when the swizzled output is a single element.
                if (fwdSwizzleInst->getElementCount() == 1)
                {
                    constructArgs[(Index)targetIndex] = gradient.revGradInst;
                }
                else
                {
                    auto gradAtIndex = builder->emitElementExtract(elementType, gradient.revGradInst, builder->getIntValue(builder->getIntType(), sourceIndex));
                    constructArgs[(Index)targetIndex] = gradAtIndex;
                }
            }

            simpleGradients.add(
                RevGradient(
                    gradient.targetInst,
                    builder->emitMakeVector(baseType, (UInt)elementCount, constructArgs.getBuffer()),
                    gradient.fwdGradInst));
        }

        return materializeSimpleGradients(builder, aggPrimalType, simpleGradients);
    }

    RevGradient materializeGradientSet(IRBuilder* builder, IRType* aggPrimalType, List<RevGradient> gradients)
    {
        switch (gradients[0].flavor)
        {
            case RevGradient::Flavor::Simple:
                return materializeSimpleGradients(builder, aggPrimalType, gradients);
            
            case RevGradient::Flavor::Swizzle:
                return materializeSwizzleGradients(builder, aggPrimalType, gradients);

            case RevGradient::Flavor::FieldExtract:
                return materializeFieldExtractGradients(builder, aggPrimalType, gradients);

            default:
                SLANG_ASSERT_FAILURE("Unhandled gradient flavor for materialization");
        }
    }

    RevGradient materializeFieldExtractGradients(IRBuilder* builder, IRType* aggPrimalType, List<RevGradient> gradients)
    {
        // Setup a temporary variable to aggregate gradients.
        // TODO: We can extend this later to grab an existing ptr to allow aggregation of
        // gradients across blocks without constructing new variables.
        // Looking up an existing pointer could also allow chained accesses like x.a.b[1] to directly
        // write into the specific sub-field that is affected without constructing intermediate vars.
        // 
        auto revGradVar = builder->emitVar(
            (IRType*)diffTypeContext.getDifferentialForType(builder, aggPrimalType));

        // Initialize with T.dzero()
        auto zeroValueInst = emitDZeroOfDiffInstType(builder, aggPrimalType);

        builder->emitStore(revGradVar, zeroValueInst);

        Dictionary<IRStructKey*, List<RevGradient>> bucketedGradients;
        for (auto gradient : gradients)
        {
            // Grab the field affected by this gradient.
            auto fieldExtractInst = as<IRFieldExtract>(gradient.fwdGradInst);
            SLANG_ASSERT(fieldExtractInst);

            auto structKey = as<IRStructKey>(fieldExtractInst->getField());
            SLANG_ASSERT(structKey);

            if (!bucketedGradients.ContainsKey(structKey))
            {
                bucketedGradients[structKey] = List<RevGradient>();
            }
            
            bucketedGradients[structKey].GetValue().add(RevGradient(
                RevGradient::Flavor::Simple,
                gradient.targetInst,
                gradient.revGradInst,
                gradient.fwdGradInst
            ));

        }

        for (auto pair : bucketedGradients)
        {
            auto subGrads = pair.Value;

            auto primalType = tryGetPrimalTypeFromDiffInst(subGrads[0].fwdGradInst);

            SLANG_ASSERT(primalType);
    
            // Consruct address to this field in revGradVar.
            auto revGradTargetAddress = builder->emitFieldAddress(
                builder->getPtrType(subGrads[0].revGradInst->getDataType()),
                revGradVar,
                pair.Key);

            builder->emitStore(revGradTargetAddress, emitAggregateValue(builder, primalType, subGrads));
        }
            
        // Load the entire var and return it.
        return RevGradient(
            RevGradient::Flavor::Simple,
            gradients[0].targetInst,
            builder->emitLoad(revGradVar),
            nullptr);
    }

    RevGradient materializeSimpleGradients(IRBuilder* builder, IRType* aggPrimalType, List<RevGradient> gradients)
    {
        if (gradients.getCount() == 1)
        {
            // If there's only one value to add up, just return it in order
            // to avoid a stack of 0 + 0 + 0 + ...
            return gradients[0];
        }

        // If there's more than one gradient, aggregate them by adding them up.
        IRInst* currentValue = nullptr;
        for (auto gradient : gradients)
        {
            if (!currentValue)
            {
                currentValue = gradient.revGradInst;
                continue;
            }

            currentValue = emitDAddOfDiffInstType(builder, aggPrimalType, currentValue, gradient.revGradInst);
        }

        return RevGradient(
                    RevGradient::Flavor::Simple,
                    gradients[0].targetInst,
                    currentValue,
                    nullptr);
    }

    IRInst* emitAggregateValue(IRBuilder* builder, IRType* aggPrimalType, List<RevGradient> gradients)
    {
        // If we're dealing with the differential-pair types, we need to use a different aggregation method, since
        // a differential pair is really a 'hybrid' primal-differential type.
        //
        if (as<IRDifferentialPairType>(aggPrimalType))
        {
            SLANG_UNEXPECTED("Should not occur");
        }

        // Process non-simple gradients into simple gradients.
        // TODO: This is where we can improve efficiency later.
        // For instance if we have one gradient each for var.x, var.y and var.z
        // we can construct one single gradient vector out of the three vectors (i.e. float3(x_grad, y_grad, z_grad))
        // instead of creating one vector for each gradient and accumulating them 
        // (i.e. float3(x_grad, 0, 0) + float3(0, y_grad, 0) + float3(0, 0, z_grad))
        // The same concept can be extended for struct and array types (and for any combination of the three)
        // 
        List<RevGradient> simpleGradients;
        {
            // Start by sorting gradients based on flavor.
            gradients.sort([&](const RevGradient& a, const RevGradient& b) -> bool { return a.flavor < b.flavor; });

            Index ii = 0;
            while (ii < gradients.getCount())
            {
                List<RevGradient> gradientsOfFlavor;

                RevGradient::Flavor currentFlavor = (gradients.getCount() > 0) ? gradients[ii].flavor : RevGradient::Flavor::Simple;

                // Pull all the gradients matching the flavor of the top-most gradeint into a temporary list.
                for (; ii < gradients.getCount(); ii++)
                {
                    if (gradients[ii].flavor == currentFlavor)
                    {
                        gradientsOfFlavor.add(gradients[ii]);
                    }
                    else
                    {
                        break;
                    }
                }

                // Turn the set into a simple gradient.
                auto simpleGradient = materializeGradientSet(builder, aggPrimalType, gradientsOfFlavor);
                SLANG_ASSERT(simpleGradient.flavor == RevGradient::Flavor::Simple);

                simpleGradients.add(simpleGradient);
            }
        }

        if (simpleGradients.getCount() == 0)
        {   
            // If there are no gradients to add up, check the type and emit a 0/null value.
            auto aggDiffType = (aggPrimalType) ? diffTypeContext.getDifferentialForType(builder, aggPrimalType) : nullptr;
            if (aggDiffType != nullptr)
            {
                // If type is non-null/non-void, call T.dzero() to produce a 0 gradient.
                return emitDZeroOfDiffInstType(builder, aggPrimalType);
            }
            else
            {
                // Otherwise, gradients may not be applicable for this inst. return N/A
                return nullptr;
            }
        }
        else 
        {
            return materializeSimpleGradients(builder, aggPrimalType, simpleGradients).revGradInst;
        }
    }

    IRType* tryGetPrimalTypeFromDiffInst(IRInst* diffInst)
    {
        // Look for differential inst decoration.
        if (auto diffInstDecoration = diffInst->findDecoration<IRDifferentialInstDecoration>())
        {
            return diffInstDecoration->getPrimalType();
        }
        else
        {
            return nullptr;
        }
    }

    IRInst* emitDZeroOfDiffInstType(IRBuilder* builder, IRType* primalType)
    {
        auto zeroMethod = diffTypeContext.getZeroMethodForType(builder, primalType);

        // Should exist.
        SLANG_ASSERT(zeroMethod);

        return builder->emitCallInst(
            (IRType*)diffTypeContext.getDifferentialForType(builder, primalType),
            zeroMethod,
            List<IRInst*>());
    }

    IRInst* emitDAddOfDiffInstType(IRBuilder* builder, IRType* primalType, IRInst* op1, IRInst* op2)
    {
        auto addMethod = diffTypeContext.getAddMethodForType(builder, primalType);

        // Should exist.
        SLANG_ASSERT(addMethod);

        return builder->emitCallInst(
            (IRType*)diffTypeContext.getDifferentialForType(builder, primalType),
            addMethod,
            List<IRInst*>(op1, op2));
    }

    void addRevGradientForFwdInst(IRInst* fwdInst, RevGradient assignment)
    {
        if (!hasRevGradients(fwdInst))
        {
            gradientsMap[fwdInst] = List<RevGradient>();
        }

        gradientsMap[fwdInst].GetValue().add(assignment);
    }

    List<RevGradient> getRevGradients(IRInst* fwdInst)
    {
        return gradientsMap[fwdInst];
    }

    List<RevGradient> popRevGradients(IRInst* fwdInst)
    {
        List<RevGradient> val = gradientsMap[fwdInst].GetValue();
        gradientsMap.Remove(fwdInst);
        return val;
    }

    bool hasRevGradients(IRInst* fwdInst)
    {
        return gradientsMap.ContainsKey(fwdInst);
    }

    AutoDiffSharedContext*                               autodiffContext;

    DifferentiableTypeConformanceContext                 diffTypeContext;

    DifferentialPairTypeBuilder                          pairBuilder;

    Dictionary<IRInst*, List<RevGradient>>               gradientsMap;

    Dictionary<IRInst*, IRVar*>                          revAccumulatorVarMap;

    Dictionary<IRInst*, IRInst*>*                        primalsMap;

    List<IRInst*>                                        usedPtrs;
    
    Dictionary<IRBlock*, ControlFlowTranspositionInfo>   blockEntries;

    Dictionary<IRBlock*, IRBlock*>                       revBlockMap;

    Dictionary<IRGlobalValueWithCode*, IRBlock*>         firstRevDiffBlockMap;
};


}
