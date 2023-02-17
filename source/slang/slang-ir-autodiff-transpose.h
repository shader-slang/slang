// slang-ir-autodiff-transpose.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-cfg-norm.h"

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
    };

    struct PendingBlockTerminatorEntry
    {
        IRBlock* fwdBlock;
        List<IRInst*> phiGrads;

        PendingBlockTerminatorEntry() : fwdBlock(nullptr)
        {}

        PendingBlockTerminatorEntry(IRBlock* fwdBlock, List<IRInst*> phiGrads) : 
            fwdBlock(fwdBlock), phiGrads(phiGrads)
        {}
    };

    bool isBlockLastInRegion(IRBlock* block, List<IRBlock*> endBlocks)
    {
        if (auto branchInst = as<IRUnconditionalBranch>(block->getTerminator()))
        {
            if (endBlocks.contains(branchInst->getTargetBlock()))
                return true;
            else
                return false;
        }
        else if (as<IRReturn>(block->getTerminator()))
        {
            return true;
        }

        return false;
    }

    List<IRInst*> getPhiGrads(IRBlock* block)
    {
        if (!phiGradsMap.ContainsKey(block))
            return List<IRInst*>();
        
        return phiGradsMap[block];
    }

    struct RegionEntryPoint
    {
        IRBlock* revEntry;
        IRBlock* fwdEndPoint;
        bool isTrivial;

        RegionEntryPoint(IRBlock* revEntry, IRBlock* fwdEndPoint) :
            revEntry(revEntry),
            fwdEndPoint(fwdEndPoint),
            isTrivial(false)
        { }

        RegionEntryPoint(IRBlock* revEntry, IRBlock* fwdEndPoint, bool isTrivial) :
            revEntry(revEntry),
            fwdEndPoint(fwdEndPoint),
            isTrivial(isTrivial)
        { }
    };

    IRBlock* getUniquePredecessor(IRBlock* block)
    {
        HashSet<IRBlock*> predecessorSet;
        for (auto predecessor : block->getPredecessors())
            predecessorSet.Add(predecessor);
        
        SLANG_ASSERT(predecessorSet.Count() == 1);

        return (*predecessorSet.begin());
    }

    RegionEntryPoint reverseCFGRegion(IRBlock* block, List<IRBlock*> endBlocks)
    {
        IRBlock* revBlock = revBlockMap[block];

        if (endBlocks.contains(block))
        {
            return RegionEntryPoint(revBlock, block, true);
        }

        // We shouldn't already have a terminator for this block
        SLANG_ASSERT(revBlock->getTerminator() == nullptr);

        IRBuilder builder(autodiffContext->moduleInst->getModule());

        auto currentBlock = block;
        while (!isBlockLastInRegion(currentBlock, endBlocks))
        {
            auto terminator = currentBlock->getTerminator();
            switch(terminator->getOp())
            {
                case kIROp_Return:
                    return RegionEntryPoint(revBlockMap[currentBlock], nullptr);

                case kIROp_unconditionalBranch:
                {
                    auto branchInst = as<IRUnconditionalBranch>(terminator);
                    auto nextBlock = as<IRBlock>(branchInst->getTargetBlock());
                    IRBlock* nextRevBlock = revBlockMap[nextBlock];
                    IRBlock* currRevBlock = revBlockMap[currentBlock];

                    SLANG_ASSERT(nextRevBlock->getTerminator() == nullptr);
                    builder.setInsertInto(nextRevBlock);

                    builder.emitBranch(currRevBlock,
                        getPhiGrads(nextBlock).getCount(),
                        getPhiGrads(nextBlock).getBuffer());
                    

                    currentBlock = nextBlock;
                    break;
                }

                case kIROp_ifElse:
                {
                    auto ifElse = as<IRIfElse>(terminator);
                    
                    auto trueBlock = ifElse->getTrueBlock();
                    auto falseBlock = ifElse->getFalseBlock();
                    auto afterBlock = ifElse->getAfterBlock();

                    auto revTrueRegionInfo = reverseCFGRegion(
                        trueBlock,
                        List<IRBlock*>(afterBlock));
                    auto revFalseRegionInfo = reverseCFGRegion(
                        falseBlock,
                        List<IRBlock*>(afterBlock));
                    //bool isTrueTrivial = (trueBlock == afterBlock);
                    //bool isFalseTrivial = (falseBlock == afterBlock);

                    IRBlock* revCondBlock = revBlockMap[afterBlock];
                    SLANG_ASSERT(revCondBlock->getTerminator() == nullptr);


                    IRBlock* revTrueEntryBlock = revTrueRegionInfo.revEntry;
                    IRBlock* revFalseEntryBlock = revFalseRegionInfo.revEntry;

                    IRBlock* revTrueExitBlock = revBlockMap[trueBlock];
                    IRBlock* revFalseExitBlock = revBlockMap[falseBlock];

                    auto phiGrads = getPhiGrads(afterBlock);
                    if (phiGrads.getCount() > 0)
                    {
                        revTrueEntryBlock = insertPhiBlockBefore(revTrueEntryBlock, phiGrads);
                        revFalseEntryBlock = insertPhiBlockBefore(revFalseEntryBlock, phiGrads);
                    }

                    IRBlock* revAfterBlock = revBlockMap[currentBlock];
                    
                    builder.setInsertInto(revCondBlock);
                    
                    hoistPrimalInst(&builder, ifElse->getCondition());

                    builder.emitIfElse(
                        ifElse->getCondition(),
                        revTrueEntryBlock,
                        revFalseEntryBlock,
                        revAfterBlock);
                    
                    if (!revTrueRegionInfo.isTrivial)
                    {
                        builder.setInsertInto(revTrueExitBlock);
                        SLANG_ASSERT(revTrueExitBlock->getTerminator() == nullptr);
                        builder.emitBranch(
                            revAfterBlock,
                            getPhiGrads(trueBlock).getCount(),
                            getPhiGrads(trueBlock).getBuffer());
                    }

                    if (!revFalseRegionInfo.isTrivial)
                    {
                        builder.setInsertInto(revFalseExitBlock);
                        SLANG_ASSERT(revFalseExitBlock->getTerminator() == nullptr);
                        builder.emitBranch(
                            revAfterBlock,
                            getPhiGrads(falseBlock).getCount(),
                            getPhiGrads(falseBlock).getBuffer());
                    }

                    currentBlock = afterBlock;
                    break;
                }

                case kIROp_loop:
                {
                    auto loop = as<IRLoop>(terminator);
                    
                    auto firstLoopBlock = loop->getTargetBlock();
                    auto breakBlock = loop->getBreakBlock();

                    auto condBlock = getOrCreateTopLevelCondition(loop);

                    auto ifElse = as<IRIfElse>(condBlock->getTerminator());

                    auto trueBlock = ifElse->getTrueBlock();
                    auto falseBlock = ifElse->getFalseBlock();

                    auto trueRegionInfo = reverseCFGRegion(
                        trueBlock,
                        List<IRBlock*>(breakBlock, condBlock));

                    auto falseRegionInfo = reverseCFGRegion(
                        falseBlock,
                        List<IRBlock*>(breakBlock, condBlock));

                    auto preCondRegionInfo = reverseCFGRegion(
                        firstLoopBlock,
                        List<IRBlock*>(condBlock));

                    // assume loop[next] -> cond can be a region and reverse it.
                    // assume cond[false] -> break can be a region and reverse it.
                    // assume cond[true] -> cond can be a region and reverse it.
                    // rev-loop = rev[break]
                    // rev-cond = rev[cond]
                    // rev-cond[true] -> entry of (cond[true] -> cond)
                    // rev-cond[false] -> entry of (loop[next] -> cond)
                    // exit of (cond[false]->break) branches into rev-cond
                    // rev-loop[next] -> entry of (cond[false] -> break)
                    // exit of (cond[true] -> cond) branches into rev-cond
                    // exit of (loop[next] -> cond) branches into rev[loop] (rev-break)

                    // For now, we'll assume the loop is always on the 'true' side
                    // If this assert fails, add in the case where the loop
                    // may be on the 'false' side.
                    // 
                    SLANG_RELEASE_ASSERT(trueRegionInfo.fwdEndPoint == condBlock);

                    auto revTrueBlock = trueRegionInfo.revEntry;
                    auto revFalseBlock = (preCondRegionInfo.isTrivial) ? 
                        revBlockMap[currentBlock] : preCondRegionInfo.revEntry;
                    
                    // The block that will become target of the new loop inst
                    // (the old false-region) This _could_ be the condition itself
                    // 
                    IRBlock* revPreCondBlock = (falseRegionInfo.isTrivial) ? 
                        revBlockMap[condBlock] : falseRegionInfo.revEntry;
                    
                    // Old cond block remains new cond block.
                    IRBlock* revCondBlock = revBlockMap[condBlock];

                    // Old cond block becomes new pre-break block.
                    IRBlock* revBreakBlock = revBlockMap[currentBlock];

                    // Old true-side starting block becomes loop end block.
                    IRBlock* revLoopEndBlock = revBlockMap[trueBlock];
                    builder.setInsertInto(revLoopEndBlock);
                    builder.emitBranch(
                        revCondBlock,
                        getPhiGrads(trueBlock).getCount(),
                        getPhiGrads(trueBlock).getBuffer());
                    
                    // Old false-side starting block becomes end block 
                    // for the new pre-cond region (which could be empty)
                    // 
                    IRBlock* revPreCondEndBlock = revBlockMap[falseBlock];
                    if (!falseRegionInfo.isTrivial)
                    {
                        builder.setInsertInto(revPreCondEndBlock);
                        builder.emitBranch(
                            revCondBlock,
                            getPhiGrads(falseBlock).getCount(),
                            getPhiGrads(falseBlock).getBuffer());
                    }

                    IRBlock* revBreakRegionExitBlock = revBlockMap[firstLoopBlock];
                    if (!preCondRegionInfo.isTrivial)
                    {
                        builder.setInsertInto(revBreakRegionExitBlock);
                        builder.emitBranch(
                            revBreakBlock,
                            getPhiGrads(firstLoopBlock).getCount(),
                            getPhiGrads(firstLoopBlock).getBuffer());
                    }

                    auto phiGrads = getPhiGrads(condBlock);
                    if (phiGrads.getCount() > 0)
                    {
                        revTrueBlock = insertPhiBlockBefore(revTrueBlock, phiGrads);
                        revFalseBlock = insertPhiBlockBefore(revFalseBlock, phiGrads);
                    }

                    // Emit condition into the new cond block.
                    builder.setInsertInto(revCondBlock);
                    hoistPrimalInst(&builder, ifElse->getCondition());

                    builder.emitIfElse(
                        ifElse->getCondition(),
                        revTrueBlock,
                        revFalseBlock,
                        revLoopEndBlock);
                    
                    // Emit loop into rev-version of the break block.
                    auto revLoopBlock = revBlockMap[breakBlock];
                    builder.setInsertInto(revLoopBlock);
                    builder.emitLoop(
                        revPreCondBlock,
                        revBreakBlock,
                        revLoopEndBlock,
                        getPhiGrads(breakBlock).getCount(),
                        getPhiGrads(breakBlock).getBuffer());

                    currentBlock = breakBlock;
                    break;
                }

                case kIROp_Switch:
                {
                    auto switchInst = as<IRSwitch>(terminator);

                    auto breakBlock = switchInst->getBreakLabel();

                    IRBlock* revBreakBlock = revBlockMap[currentBlock];

                    // Reverse each case label
                    List<IRInst*> reverseSwitchArgs;
                    Dictionary<IRBlock*, IRBlock*> reverseLabelEntryBlocks;

                    for (UIndex ii = 0; ii < switchInst->getCaseCount(); ii++)
                    {
                        reverseSwitchArgs.add(switchInst->getCaseValue(ii));

                        auto caseLabel = switchInst->getCaseLabel(ii);
                        if (!reverseLabelEntryBlocks.ContainsKey(caseLabel))
                        {
                            auto labelRegionInfo = reverseCFGRegion(
                                caseLabel,
                                List<IRBlock*>(breakBlock));

                            // Handle this case eventually.
                            SLANG_ASSERT(!labelRegionInfo.isTrivial);

                            // Wire the exit to the break block
                            IRBlock* revLabelExit = revBlockMap[caseLabel];
                            SLANG_ASSERT(revLabelExit->getTerminator() == nullptr);

                            builder.setInsertInto(revLabelExit);
                            builder.emitBranch(revBreakBlock);
                            
                            reverseLabelEntryBlocks[caseLabel] = labelRegionInfo.revEntry;
                            reverseSwitchArgs.add(labelRegionInfo.revEntry);
                        }
                        else
                        {
                            reverseSwitchArgs.add(reverseLabelEntryBlocks[caseLabel]);
                        }
                    }
                    
                    auto defaultRegionInfo = reverseCFGRegion(
                        switchInst->getDefaultLabel(),
                        List<IRBlock*>(breakBlock));
                    SLANG_ASSERT(!defaultRegionInfo.isTrivial);
                    
                    auto revDefaultRegionEntry = defaultRegionInfo.revEntry;

                    builder.setInsertInto(revBlockMap[switchInst->getDefaultLabel()]);
                    builder.emitBranch(revBreakBlock);

                    auto phiGrads = getPhiGrads(breakBlock);
                    if (phiGrads.getCount() > 0)
                    {
                        for (UIndex ii = 0; ii < switchInst->getCaseCount(); ii++)
                        {
                            reverseSwitchArgs[ii * 2 + 1] =
                                insertPhiBlockBefore(as<IRBlock>(reverseSwitchArgs[ii * 2 + 1]), phiGrads);
                        }
                        revDefaultRegionEntry =
                                insertPhiBlockBefore(as<IRBlock>(revDefaultRegionEntry), phiGrads);
                    }

                    auto revSwitchBlock = revBlockMap[breakBlock];

                    builder.setInsertInto(revSwitchBlock);

                    hoistPrimalInst(&builder, switchInst->getCondition());

                    builder.emitSwitch(
                        switchInst->getCondition(),
                        revBreakBlock,
                        revDefaultRegionEntry,
                        reverseSwitchArgs.getCount(),
                        reverseSwitchArgs.getBuffer());

                    currentBlock = breakBlock;
                    break;
                }

            }
        }

        if (auto branchInst = as<IRUnconditionalBranch>(currentBlock->getTerminator()))
        {
            return RegionEntryPoint(
                revBlockMap[currentBlock],
                branchInst->getTargetBlock(),
                false);
        }
        else if (auto returnInst = as<IRReturn>(currentBlock->getTerminator()))
        {
            return RegionEntryPoint(
                revBlockMap[currentBlock],
                nullptr,
                true);
        }
        else
        {
            // Regions should _really_ not end on a conditional branch (I think)
            SLANG_UNEXPECTED("Unexpected: Region ended on a conditional branch");
        }
    }

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
        IRBuilder builder(autodiffContext->moduleInst);

        // Insert after the last block.
        builder.setInsertInto(revDiffFunc);

        List<IRBlock*> workList;

        // Build initial list of blocks to process by checking if they're differential blocks.
        List<IRBlock*> traverseWorkList;
        HashSet<IRBlock*> traverseSet;
        traverseWorkList.add(revDiffFunc->getFirstBlock());

        traverseSet.Add(revDiffFunc->getFirstBlock());
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

        if (!workList.getCount())
            return;

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
        auto firstRevDiffBlock = revBlockMap[terminalDiffBlocks[0]].GetValue();
        firstRevDiffBlockMap[revDiffFunc] = firstRevDiffBlock;

        // Move all diff vars to first block, and initialize them with zero.
        builder.setInsertInto(firstRevDiffBlock);
        for (auto block : workList)
        {
            for (auto inst = block->getFirstInst(); inst;)
            {
                auto nextInst = inst->getNextInst();
                if (auto varInst = as<IRVar>(inst))
                {
                    if (auto diffDecor = varInst->findDecoration<IRDifferentialInstDecoration>())
                    {
                        if (auto ptrPrimalType = as<IRPtrTypeBase>(diffDecor->getPrimalType()))
                        {
                            varInst->insertAtEnd(firstRevDiffBlock);

                            auto dzero = emitDZeroOfDiffInstType(&builder, ptrPrimalType->getValueType());
                            builder.emitStore(varInst, dzero);
                        }
                    }
                }
                inst = nextInst;
            }
        }

        for (auto block : workList)
        {
            // Set dOutParameter as the transpose gradient for the return inst, if any.
            if (transposeInfo.dOutInst)
            {
                if (auto returnInst = as<IRReturn>(block->getTerminator()))
                {
                    this->addRevGradientForFwdInst(returnInst, RevGradient(returnInst, transposeInfo.dOutInst, nullptr));
                }
            }

            IRBlock* revBlock = revBlockMap[block];
            this->transposeBlock(block, revBlock);
        }

        // At this point all insts have been transposed, but the blocks
        // have no control flow.
        // reverseCFG will use fwd-mode blocks as reference, and 
        // wire the corresponding rev-mode blocks in reverse.
        // 
        auto branchInst = as<IRUnconditionalBranch>(terminalPrimalBlocks[0]->getTerminator());
        auto firstFwdDiffBlock = branchInst->getTargetBlock();
        reverseCFGRegion(firstFwdDiffBlock, List<IRBlock*>());

        // Lower any loop-exit-value decorations into initializations for loop intermediate vals, 
        // and convert loop initial values into terminating conditions.
        // 
        // TODO: We need a way to confirm that all required vars have an initial value
        // (is there a built-in dataflow tool for this?)
        // 
        for (auto block : workList)
        {
            if (auto loopInst = as<IRLoop>(block->getTerminator()))
            {
                lowerLoopExitValues(&builder, loopInst);
                invertLoopCondition(&builder, loopInst);
            }
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
            auto firstRevBlock = as<IRBlock>(revBlockMap[terminalDiffBlocks[0]]);

            auto returnDecoration = 
                terminalPrimalBlock->getTerminator()->findDecoration<IRBackwardDerivativePrimalReturnDecoration>();
            SLANG_ASSERT(returnDecoration);
            auto retVal = returnDecoration->getBackwardDerivativePrimalReturnValue();

            terminalPrimalBlock->getTerminator()->removeAndDeallocate();
            
            IRBuilder subBuilder = builder;
            subBuilder.setInsertInto(terminalPrimalBlock);

            // There should be no parameters in the first reverse-mode block.
            SLANG_ASSERT(firstRevBlock->getFirstParam() == nullptr);

            auto branch = subBuilder.emitBranch(firstRevBlock);

            subBuilder.addBackwardDerivativePrimalReturnDecoration(branch, retVal);
        }

        // At this point, the only block left without terminator insts
        // should be the last one. Add a void return to complete it.
        // 
        IRBlock* lastRevBlock = revBlockMap[firstFwdDiffBlock];
        SLANG_ASSERT(lastRevBlock->getTerminator() == nullptr);

        builder.setInsertInto(lastRevBlock);
        builder.emitReturn();

        // Remove fwd-mode blocks.
        for (auto block : workList)
        {
            block->removeAndDeallocate();
        }
    }

    IRInst* extractAccumulatorVarGradient(IRBuilder* builder, IRInst* fwdInst)
    {
        if (auto accVar = getOrCreateAccumulatorVar(fwdInst))
        {
            auto gradValue = builder->emitLoad(accVar);
            builder->emitStore(
                accVar,
                emitDZeroOfDiffInstType(
                    builder,
                    tryGetPrimalTypeFromDiffInst(fwdInst)));
            
            return gradValue;
        }
        else
        {
            return nullptr;
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
        
        IRBuilder tempVarBuilder(autodiffContext->moduleInst->getModule());
        
        IRBlock* firstDiffBlock = firstRevDiffBlockMap[as<IRFunc>(fwdInst->getParent()->getParent())];

        if (auto firstInst = firstDiffBlock->getFirstOrdinaryInst())
            tempVarBuilder.setInsertBefore(firstInst);
        else
            tempVarBuilder.setInsertInto(firstDiffBlock);
        
        auto primalType = tryGetPrimalTypeFromDiffInst(fwdInst);
        auto diffType = fwdInst->getDataType();

        auto zero = emitDZeroOfDiffInstType(&tempVarBuilder, primalType);

        // Emit a var in the top-level differential block to hold the gradient, 
        // and initialize it.
        auto tempRevVar = tempVarBuilder.emitVar(diffType);
        tempVarBuilder.emitStore(tempRevVar, zero);
        revAccumulatorVarMap[fwdInst] = tempRevVar;

        return tempRevVar;
    }

    IRVar* getOrCreateInverseVar(IRInst* primalInst)
    {
        // No need to store inverse values for constants.
        if (as<IRConstant>(primalInst))
            return nullptr;

        // Check if we have a var already.
        if (inverseVarMap.ContainsKey(primalInst))
            return inverseVarMap[primalInst];
        
        IRBuilder tempVarBuilder(autodiffContext->moduleInst);
        
        IRBlock* firstDiffBlock = firstRevDiffBlockMap[as<IRFunc>(primalInst->getParent()->getParent())];

        if (auto firstInst = firstDiffBlock->getFirstOrdinaryInst())
            tempVarBuilder.setInsertBefore(firstInst);
        else
            tempVarBuilder.setInsertInto(firstDiffBlock);
        
        auto primalType = primalInst->getDataType();

        // Emit a var in the top-level differential block to hold the inverse, 
        // and initialize it.
        auto tempInvVar = tempVarBuilder.emitVar(primalType);

        inverseVarMap[primalInst] = tempInvVar;

        return tempInvVar;
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
        IRBuilder builder(autodiffContext->moduleInst);
 
        // Insert into our reverse block.
        builder.setInsertInto(revBlock);

        // Check if this block has any 'outputs' (in the form of phi args
        // sent to the successor block)
        // 
        if (auto branchInst = as<IRUnconditionalBranch>(fwdBlock->getTerminator()))
        {
            for (UIndex ii = 0; ii < branchInst->getArgCount(); ii++)
            {
                auto arg = branchInst->getArg(ii);
                if (isDifferentialInst(arg))
                {
                    // If the arg is a differential, emit a parameter
                    // to accept it's reverse-mode differential as an input
                    // 

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
                else if (isPrimalInst(arg))
                {
                    // If the output arg is a primal, emit a parameter
                    // to accept it as an _input_ for the reverse-mode
                    // 
                    auto primalType = arg->getDataType();
                    auto primalInvParam = builder.emitParam(primalType);

                    setInverse(&builder, arg, primalInvParam);
                }
                else
                {
                    SLANG_UNEXPECTED("Encountered inst not marked as primal or differential");
                }
            }
        }

        // Move pointer & reference insts to the top of the reverse-mode block.
        List<IRInst*> nonValueInsts;
        for (IRInst* child = fwdBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
        {
            // If the instruction is a variable allocation (or reverse-gradient pair reference), 
            // move to top.
            // TODO: This is hacky.. Need a more principled way to handle this 
            // (like primal inst hoisting)
            // 
            if (as<IRVar>(child) || as<IRReverseGradientDiffPairRef>(child))
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
            if (child->findDecoration<IRPrimalValueAccessDecoration>())
                continue;

            if (as<IRDecoration>(child) || as<IRParam>(child))
                continue;

            if (isDifferentialInst(child))
                transposeInst(&builder, child);
            else if (isPrimalInst(child))
                invertInst(&builder, child);
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
            if (isDifferentialInst(param))
            {
                // This param might be used outside this block.
                // If so, add/get an accumulator.
                // 
                if (isInstUsedOutsideParentBlock(param))
                {
                    auto accGradient = extractAccumulatorVarGradient(&builder, param);
                    addRevGradientForFwdInst(
                        param, 
                        RevGradient(param, accGradient, nullptr));
                }
                if (hasRevGradients(param))
                {
                    auto gradients = popRevGradients(param);

                    auto gradInst = emitAggregateValue(
                        &builder,
                        tryGetPrimalTypeFromDiffInst(param),
                        gradients);
                    
                    phiParamRevGradInsts.add(gradInst);
                }
                else
                {
                    phiParamRevGradInsts.add(
                        emitDZeroOfDiffInstType(&builder, tryGetPrimalTypeFromDiffInst(param)));
                }
            }
            else if (isPrimalInst(param))
            {
                if (hasInverse(param))
                    phiParamRevGradInsts.add(getInverse(&builder, param));
                else
                {
                    SLANG_UNEXPECTED("param is a primal inst but has no registered inverse");
                }
            }
            else
            {
                SLANG_UNEXPECTED("param is neither differential nor primal");
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
            if (isNoDiffType(externInst->getDataType()))
            {
                popRevGradients(externInst);
                continue;
            }

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

        // Record any phi gradients for the CFG reversal pass.
        phiGradsMap[fwdBlock] = phiParamRevGradInsts;

    }

    struct InvInstPair
    {
        IRInst* inst;
        IRInst* invInst;

        InvInstPair(IRInst* inst, IRInst* invInst) :
            inst(inst), invInst(invInst)
        { }

        InvInstPair() : inst(nullptr), invInst(nullptr)
        { }
    };

    List<InvInstPair> invertArithmetic(IRBuilder* builder, IRInst* primalInst, IRInst* invOutput)
    {
        switch (primalInst->getOp())
        {
            case kIROp_Add:
            {
                SLANG_RELEASE_ASSERT(as<IRConstant>(primalInst->getOperand(1)));
                return List<InvInstPair>(
                    InvInstPair(
                        primalInst->getOperand(0),
                        builder->emitSub(
                            primalInst->getOperand(0)->getDataType(),
                            invOutput,
                            primalInst->getOperand(1))));
            }
            case kIROp_Sub:
            {
                SLANG_RELEASE_ASSERT(as<IRConstant>(primalInst->getOperand(1)));
                return List<InvInstPair>(
                    InvInstPair(
                        primalInst->getOperand(0),
                        builder->emitAdd(
                            primalInst->getOperand(0)->getDataType(),
                            invOutput,
                            primalInst->getOperand(1))));
            }

            default:
                SLANG_UNEXPECTED("Unhandled arithmetic inst for inversion");
        }
    }

    void lowerLoopExitValues(IRBuilder* builder, IRLoop* fwdLoop)
    {
        for (auto decoration : fwdLoop->getDecorations())
        {
            if (auto loopExitValueDecoration = as<IRLoopExitPrimalValueDecoration>(decoration))
            {
                IRBlock* revLoopInitBlock = revBlockMap[fwdLoop->getBreakBlock()];

                if (auto revLoopInst = revLoopInitBlock->getTerminator())
                    builder->setInsertBefore(revLoopInst);
                else
                    builder->setInsertInto(revLoopInitBlock);

                hoistPrimalInst(builder, loopExitValueDecoration->getLoopExitValInst());

                setInverse(builder, loopExitValueDecoration->getTargetInst(), loopExitValueDecoration->getLoopExitValInst());
            }
        }
    }

    void lowerLoopExitValues(IRBuilder* builder, IRBlock* block)
    {
        if (auto loopInst = as<IRLoop>(block->getTerminator()))
            lowerLoopExitValues(builder, loopInst);
    }

    // Go through loop block phi-args, and look for loop counter
    // arguments, which for a loop means inserting a check into
    // loop condition block.
    // This method also adds logic to skip the first iteration. 
    // (a 'do-while' loop)
    // 
    void invertLoopCondition(IRBuilder* builder, IRLoop* loopInst)
    {   
        auto firstLoopBlock = loopInst->getTargetBlock();

        IRBlock* revLoopCondBlock = revBlockMap[firstLoopBlock];
        builder->setInsertBefore(revLoopCondBlock->getTerminator());

        // Add a terminating condition based on the loop counter's initial primal value

        IRParam* loopCounterParam = nullptr;
        UIndex loopCounterParamIndex = 0;
        for (auto param : firstLoopBlock->getParams())
        {   
            if (param->findDecoration<IRLoopCounterDecoration>())
            {
                // There really should be two (or more) loop counter params.
                SLANG_RELEASE_ASSERT(loopCounterParam == nullptr);
                loopCounterParam = param;
            }
            else
            {
                loopCounterParamIndex++;
            }
        }

        // Should see atleast one loop counter parameter on the first loop block.
        SLANG_RELEASE_ASSERT(loopCounterParam);

        IRInst* loopCounterInitVal = loopInst->getArg(loopCounterParamIndex);

        auto paramBoundsCheck = builder->emitIntrinsicInst(
                builder->getBoolType(),
                kIROp_Neq,
                2,
                List<IRInst*>(
                    hoistPrimalInst(builder, loopCounterParam),
                    hoistPrimalInst(builder, loopCounterInitVal)).getBuffer());

        as<IRIfElse>(revLoopCondBlock->getTerminator())->condition.set(paramBoundsCheck);
    }

    List<InvInstPair> invertInst(IRBuilder* builder, IRInst* primalInst, IRInst* invOutput)
    {
        switch (primalInst->getOp())
        {
            case kIROp_Add:
            case kIROp_Sub:
                return invertArithmetic(builder, primalInst, invOutput);
            
            default:
                SLANG_UNIMPLEMENTED_X("Unhandled inst type for inversion");
        }
    }

    bool hasInverse(IRInst* primalInst)
    {
        if (getOrCreateInverseVar(primalInst))
            return true;
        else
            return false;
    }

    IRInst* getInverse(IRBuilder* builder, IRInst* primalInst)
    {
        // Note: There are other possible cases here, although not important
        // right now. For example, a value is available to load from the primal block.
        // 
        if (auto invVar = getOrCreateInverseVar(primalInst))
            return builder->emitLoad(invVar);
        
        return nullptr;
    }

    void setInverse(IRBuilder* builder, IRInst* inst, IRInst* invInst)
    {
        if (auto invVar = getOrCreateInverseVar(inst))
            builder->emitStore(invVar, invInst);
    }

    IRInst* hoistPrimalInst(IRBuilder* revBuilder, IRInst* inst)
    {
        SLANG_RELEASE_ASSERT(isPrimalInst(inst));

        // Are the operands of this primal inst also available in the reverse-mode context?
        // If not, move/load them.
        // 
        hoistPrimalOperands(revBuilder, inst);

        if (isPrimalInst(inst) && 
            as<IRBlock>(inst->getParent()) && 
            isDifferentialInst(as<IRBlock>(inst->getParent())))
        {
            if (!inst->findDecoration<IRPrimalValueAccessDecoration>())
            {
                return getInverse(revBuilder, inst);
            }
            else
            {
                auto block = as<IRBlock>(inst->getParent());
                SLANG_RELEASE_ASSERT(block);

                if (block == revBuilder->getBlock())
                {
                    // Already in block..
                    return inst;
                }

                // Otherwise, move our inst to the the current builder location.
                inst->removeFromParent();
                revBuilder->addInst(inst);

                return inst;
            }
        }

        return inst;
    }

    void hoistPrimalOperands(IRBuilder* revBuilder, IRInst* fwdInst)
    {
        for (UIndex ii = 0; ii < fwdInst->getOperandCount(); ii++)
        {
            // For now we'll only hoist primal operands that are 
            // generated in differential blocks.
            // Eventually, we also want this method to move primal access
            // insts to the reverse-mode blocks (i.e. this method will
            // make sure all requried primal insts are moved to the right
            // place)
            // 
            if (isPrimalInst(fwdInst->getOperand(ii)))
            {
                auto hoistedPrimalInst = hoistPrimalInst(revBuilder, fwdInst->getOperand(ii));
                fwdInst->setOperand(ii, hoistedPrimalInst);
            }
        }
    }

    void invertInst(IRBuilder* builder, IRInst* primalInst)
    {
        // Look for an available inverse entry for this primalInst's *output*
        if (hasInverse(primalInst))
        {
            auto invOutput = getInverse(builder, primalInst);

            auto invEntries = invertInst(builder, primalInst, invOutput);

            for (auto entry : invEntries)
                setInverse(builder, entry.inst, entry.invInst);
        }
        else
        {
            SLANG_UNEXPECTED("Could not find value for the output of inst. Unable to invert");
        }
    }

    void transposeInst(IRBuilder* builder, IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_ForwardDifferentiate:
            return;
        default:
            break;
        }

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
                if (!returnPairType)
                    return;
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

        // Ensure primal operands are replaced with insts accessible in the 
        // reverse-mode context.
        // 
        hoistPrimalOperands(builder, inst);

        // Is this inst used in another differential block?
        // Emit a function-scope accumulator variable, and include it's value.
        // Also, we ignore this if it's a load since those are turned into stores
        // on a per-block basis. (We should change this behaviour to treat loads like
        // any other inst)
        // 
        if (isInstUsedOutsideParentBlock(inst) && !as<IRLoad>(inst))
        {
            auto accGradient = extractAccumulatorVarGradient(builder, inst);
            gradients.add(
                RevGradient(inst, accGradient, nullptr));
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

        // The call must have been decorated with the continuation context after splitting.
        auto primalContextDecor = fwdCall->findDecoration<IRBackwardDerivativePrimalContextDecoration>();
        SLANG_RELEASE_ASSERT(primalContextDecor);

        auto baseFn = fwdDiffCallee->getBaseFn();

        List<IRInst*> args;
        List<IRType*> argTypes;
        List<bool> argRequiresLoad;

        auto getDiffPairType = [](IRType* type)
        {
            if (auto ptrType = as<IRPtrTypeBase>(type))
                type = ptrType->getValueType();
            return as<IRDifferentialPairType>(type);
        };

        struct DiffValWriteBack
        {
            IRInst* destVar;
            IRInst* srcTempPairVar;
        };
        List<DiffValWriteBack> writebacks;

        auto baseFnType = as<IRFuncType>(baseFn->getDataType());

        SLANG_RELEASE_ASSERT(baseFnType);
        SLANG_RELEASE_ASSERT(fwdCall->getArgCount() == baseFnType->getParamCount());

        for (UIndex ii = 0; ii < fwdCall->getArgCount(); ii++)
        {
            auto arg = fwdCall->getArg(ii);
            auto paramType = baseFnType->getParamType(ii);

            if (as<IRLoadReverseGradient>(arg))
            {
                // Original parameters that are `out DifferentiableType` will turn into
                // a `in Differential` parameter. The split logic will insert LoadReverseGradient insts
                // to inform us this case. Here we just need to generate a load of the derivative variable
                // and use it as the final argument.
                args.add(builder->emitLoad(arg->getOperand(0)));
            }
            else if (auto instPair = as<IRReverseGradientDiffPairRef>(arg))
            {
                // An argument to an inout parameter will come in the form of a ReverseGradientDiffPairRef(primalVar, diffVar) inst
                // after splitting.
                // In order to perform the call, we need a temporary var to store the DiffPair.
                auto pairType = as<IRPtrTypeBase>(arg->getDataType())->getValueType();
                auto tempVar = builder->emitVar(pairType);
                auto primalVal = builder->emitLoad(instPair->getPrimal());
                auto diffVal = builder->emitLoad(instPair->getDiff());
                auto pairVal = builder->emitMakeDifferentialPair(pairType, primalVal, diffVal);
                builder->emitStore(tempVar, pairVal);
                args.add(tempVar);
                writebacks.add(DiffValWriteBack{instPair->getDiff(), tempVar});
            }
            else if (!as<IRPtrTypeBase>(arg->getDataType()) && getDiffPairType(arg->getDataType()))
            {
                // Normal differentiable input parameter will become an inout DiffPair parameter
                // in the propagate func. The split logic has already prepared the initial value
                // to pass in. We need to define a temp variable with this initial value and pass
                // in the temp variable as argument to the inout parameter.

                auto makePairArg = as<IRMakeDifferentialPair>(arg);
                SLANG_RELEASE_ASSERT(makePairArg);

                auto pairType = as<IRDifferentialPairType>(arg->getDataType());
                auto var = builder->emitVar(arg->getDataType());

                // Initialize this var to (arg.primal, 0).
                builder->emitStore(
                    var,
                    builder->emitMakeDifferentialPair(
                        arg->getDataType(),
                        makePairArg->getPrimalValue(),
                        builder->emitCallInst(
                            (IRType*)diffTypeContext.getDifferentialForType(builder, pairType->getValueType()),
                            diffTypeContext.getZeroMethodForType(builder, pairType->getValueType()),
                            List<IRInst*>())));
                
                args.add(var);
                argTypes.add(builder->getInOutType(pairType));
                argRequiresLoad.add(true);
            }
            else
            {
                if (as<IROutType>(paramType))
                {
                    args.add(nullptr);
                    argRequiresLoad.add(false);
                }
                else if (as<IRInOutType>(paramType))
                {
                    arg = builder->emitLoad(arg);
                    args.add(arg);
                    argTypes.add(arg->getDataType());
                    argRequiresLoad.add(false);
                }
                else
                {
                    args.add(arg);
                    argTypes.add(arg->getDataType());
                    argRequiresLoad.add(false);
                }
            }
        }

        if (revValue)
        {
            args.add(revValue);
            argTypes.add(revValue->getDataType());
            argRequiresLoad.add(false);
        }

        args.add(builder->emitLoad(primalContextDecor->getBackwardDerivativePrimalContextVar()));
        argTypes.add(as<IRPtrTypeBase>(
                primalContextDecor->getBackwardDerivativePrimalContextVar()->getDataType())
                ->getValueType());
        argRequiresLoad.add(false);

        auto revFnType = builder->getFuncType(argTypes, builder->getVoidType());
        auto revCallee = builder->emitBackwardDifferentiatePropagateInst(
            revFnType,
            baseFn);

        List<IRInst*> callArgs;
        for (auto arg : args)
            if (arg)
                callArgs.add(arg);
        builder->emitCallInst(revFnType->getResultType(), revCallee, callArgs);

        // Writeback result gradient to their corresponding splitted variable.
        for (auto wb : writebacks)
        {
            auto loadedPair = builder->emitLoad(wb.srcTempPairVar);
            auto diffType = as<IRPtrTypeBase>(wb.destVar->getDataType())->getValueType();
            auto loadedDiff = builder->emitDifferentialPairGetDifferential(diffType, loadedPair);
            builder->emitStore(wb.destVar, loadedDiff);
        }

        List<RevGradient> gradients;
        for (Index ii = 0; ii < args.getCount(); ii++)
        {
            if (!args[ii])
                continue;

            // Is this arg relevant to auto-diff?
            if (auto diffPairType = getDiffPairType(args[ii]->getDataType()))
            {
                // If this is ptr typed, ignore (the gradient will be accumulated on the pointer)
                // automatically.
                // 
                if (argRequiresLoad[ii])
                {
                    auto diffArgType = (IRType*)diffTypeContext.getDifferentialForType(
                        builder, 
                        diffPairType->getValueType());
                    gradients.add(RevGradient(
                        RevGradient::Flavor::Simple,
                        fwdCall->getArg(ii),
                        builder->emitDifferentialPairGetDifferential(
                            diffArgType, builder->emitLoad(args[ii])),
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

    IRBlock* getAfterBlock(IRBlock* block)
    {   
        auto terminatorInst = block->getTerminator();
        switch (terminatorInst->getOp())
        {
            case kIROp_unconditionalBranch:
            case kIROp_Return:
                return nullptr;

            case kIROp_ifElse:
                return as<IRIfElse>(terminatorInst)->getAfterBlock();
            case kIROp_Switch:
                return as<IRSwitch>(terminatorInst)->getBreakLabel();
            case kIROp_loop:
                return as<IRLoop>(terminatorInst)->getBreakBlock();
            
            default:
                SLANG_UNIMPLEMENTED_X("Unhandled terminator inst when building after-block map");
        }
    }

    void buildAfterBlockMap(IRGlobalValueWithCode* fwdFunc)
    {
        // Scan through a fwd-mode function, and build a list of blocks
        // that appear as the 'after' block for any conditional control
        // flow statement.
        //

        for (auto block = fwdFunc->getFirstBlock(); block; block = block->getNextBlock())
        {
            // Only need to process differential blocks.
            if (!isDifferentialInst(block))
                continue;

            IRBlock* afterBlock = getAfterBlock(block);

            if (afterBlock)
            {
                // No block can by the after block for multiple control flow insts.
                //
                SLANG_ASSERT(!(afterBlockMap.ContainsKey(afterBlock) && \
                    afterBlockMap[afterBlock] != block->getTerminator()));

                afterBlockMap[afterBlock] = block->getTerminator();
            }
        }
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

    IRBlock* insertPhiBlockBefore(IRBlock* revBlock, List<IRInst*> phiArgs)
    {
        IRBuilder phiBlockBuilder(autodiffContext->moduleInst->getModule());
        phiBlockBuilder.setInsertBefore(revBlock);

        auto phiBlock = phiBlockBuilder.emitBlock();

        if (isDifferentialInst(revBlock))
            phiBlockBuilder.markInstAsDifferential(phiBlock);
        
        phiBlockBuilder.emitBranch(
            revBlock,
            phiArgs.getCount(),
            phiArgs.getBuffer());
        
        return phiBlock;
    }
    
    TranspositionResult transposeInst(IRBuilder* builder, IRInst* fwdInst, IRInst* revValue)
    {
        // Dispatch logic.
        switch(fwdInst->getOp())
        {
            case kIROp_Add:
            case kIROp_Mul:
            case kIROp_Sub: 
            case kIROp_Div: 
            case kIROp_Neg:
                return transposeArithmetic(builder, fwdInst, revValue);

            case kIROp_Call:
                return transposeCall(builder, as<IRCall>(fwdInst), revValue);
            
            case kIROp_swizzle:
                return transposeSwizzle(builder, as<IRSwizzle>(fwdInst), revValue);
            
            case kIROp_FieldExtract:
                return transposeFieldExtract(builder, as<IRFieldExtract>(fwdInst), revValue);

            case kIROp_GetElement:
                return transposeGetElement(builder, as<IRGetElement>(fwdInst), revValue);

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
            case kIROp_MakeVectorFromScalar:
                return transposeMakeVectorFromScalar(builder, fwdInst, revValue);
            case kIROp_MakeMatrixFromScalar:
                return transposeMakeMatrixFromScalar(builder, fwdInst, revValue);
            case kIROp_MakeMatrix:
                return transposeMakeMatrix(builder, fwdInst, revValue);
            case kIROp_MatrixReshape:
                return transposeMatrixReshape(builder, fwdInst, revValue);
            case kIROp_MakeStruct:
                return transposeMakeStruct(builder, fwdInst, revValue);
            case kIROp_MakeArray:
                return transposeMakeArray(builder, fwdInst, revValue);
            case kIROp_MakeArrayFromElement:
                return transposeMakeArrayFromElement(builder, fwdInst, revValue);

            case kIROp_UpdateElement:
                return transposeUpdateElement(builder, fwdInst, revValue);

            case kIROp_LoadReverseGradient:
            case kIROp_DefaultConstruct:
            case kIROp_Specialize:
            case kIROp_unconditionalBranch:
            case kIROp_conditionalBranch:
            case kIROp_ifElse:
            case kIROp_loop:
            case kIROp_Switch:
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
            auto primalPairVal = builder->emitLoad(revPtr);
            auto primalVal = builder->emitDifferentialPairGetPrimal(primalPairVal);

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
        IRInst* revVal = builder->emitLoad(fwdStore->getPtr());
        if (auto diffPairType = as<IRDifferentialPairType>(revVal->getDataType()))
        {
            revVal = builder->emitDifferentialPairGetDifferential(
                (IRType*)diffTypeContext.getDifferentialTypeFromDiffPairType(
                    builder, diffPairType),
                revVal);
        }
        return TranspositionResult(
                    List<RevGradient>(
                        RevGradient(
                            RevGradient::Flavor::Simple,
                            fwdStore->getVal(),
                            revVal,
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

    TranspositionResult transposeGetElement(IRBuilder*, IRGetElement* fwdGetElement, IRInst* revValue)
    {
        return TranspositionResult(
            List<RevGradient>(
                RevGradient(
                    RevGradient::Flavor::GetElement,
                    fwdGetElement->getBase(),
                    revValue,
                    fwdGetElement)));
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

    TranspositionResult transposeMakeVectorFromScalar(IRBuilder* builder, IRInst* fwdMakeVector, IRInst* revValue)
    {
        auto vectorType = as<IRVectorType>(revValue->getDataType());
        SLANG_RELEASE_ASSERT(vectorType);
        auto vectorSize = as<IRIntLit>(vectorType->getElementCount());
        SLANG_RELEASE_ASSERT(vectorSize);

        List<RevGradient> gradients;
        for (UIndex ii = 0; ii < (UIndex)vectorSize->getValue(); ii++)
        {
            auto revComp = builder->emitElementExtract(revValue, builder->getIntValue(builder->getIntType(), ii));
            gradients.add(RevGradient(
                            RevGradient::Flavor::Simple,
                            fwdMakeVector->getOperand(0),
                            revComp,
                            fwdMakeVector));
        }
        return TranspositionResult(gradients);
    }

    TranspositionResult transposeMakeMatrixFromScalar(IRBuilder* builder, IRInst* fwdMakeMatrix, IRInst* revValue)
    {
        auto matrixType = as<IRMatrixType>(revValue->getDataType());
        SLANG_RELEASE_ASSERT(matrixType);
        auto row = as<IRIntLit>(matrixType->getRowCount());
        auto col = as<IRIntLit>(matrixType->getColumnCount());
        SLANG_RELEASE_ASSERT(row && col);

        List<RevGradient> gradients;
        for (UIndex r = 0; r < (UIndex)row->getValue(); r++)
        {
            for (UIndex c = 0; c < (UIndex)col->getValue(); c++)
            {
                auto revRow = builder->emitElementExtract(revValue, builder->getIntValue(builder->getIntType(), r));
                auto revCol = builder->emitElementExtract(revRow, builder->getIntValue(builder->getIntType(), c));
                gradients.add(RevGradient(
                    RevGradient::Flavor::Simple,
                    fwdMakeMatrix->getOperand(0),
                    revCol,
                    fwdMakeMatrix));
            }
        }
        return TranspositionResult(gradients);
    }

    TranspositionResult transposeMakeMatrix(IRBuilder* builder, IRInst* fwdMakeMatrix, IRInst* revValue)
    {
        List<RevGradient> gradients;
        auto matrixType = as<IRMatrixType>(fwdMakeMatrix->getDataType());
        auto row = as<IRIntLit>(matrixType->getRowCount());
        auto colCount = matrixType->getColumnCount();
        IRType* rowVectorType = nullptr;
        for (UIndex ii = 0; ii < fwdMakeMatrix->getOperandCount(); ii++)
        {
            auto argOperand = fwdMakeMatrix->getOperand(ii);
            IRInst* gradAtIndex = nullptr;
            if (auto vecType = as<IRVectorType>(argOperand->getDataType()))
            {
                gradAtIndex = builder->emitElementExtract(
                    argOperand->getDataType(),
                    revValue,
                    builder->getIntValue(builder->getIntType(), ii));
            }
            else
            {
                SLANG_RELEASE_ASSERT(row);
                UInt rowIndex = ii / (UInt)row->getValue();
                UInt colIndex = ii % (UInt)row->getValue();
                if (!rowVectorType)
                    rowVectorType = builder->getVectorType(matrixType->getElementType(), colCount);
                auto revRow = builder->emitElementExtract(
                    rowVectorType,
                    revValue,
                    builder->getIntValue(builder->getIntType(), rowIndex));
                gradAtIndex = builder->emitElementExtract(
                    matrixType->getElementType(),
                    revRow,
                    builder->getIntValue(builder->getIntType(), colIndex));
            }
            gradients.add(RevGradient(
                RevGradient::Flavor::Simple,
                fwdMakeMatrix->getOperand(ii),
                gradAtIndex,
                fwdMakeMatrix));
        }
        return TranspositionResult(gradients);
    }

    TranspositionResult transposeMatrixReshape(IRBuilder* builder, IRInst* fwdMatrixReshape, IRInst* revValue)
    {
        List<RevGradient> gradients;
        auto operandMatrixType = as<IRMatrixType>(fwdMatrixReshape->getOperand(0)->getDataType());
        SLANG_RELEASE_ASSERT(operandMatrixType);

        auto operandRow = as<IRIntLit>(operandMatrixType->getRowCount());
        auto operandCol = as<IRIntLit>(operandMatrixType->getColumnCount());
        SLANG_RELEASE_ASSERT(operandRow && operandCol);

        auto revMatrixType = as<IRMatrixType>(revValue->getDataType());
        SLANG_RELEASE_ASSERT(revMatrixType);
        auto revRow = as<IRIntLit>(revMatrixType->getRowCount());
        auto revCol = as<IRIntLit>(revMatrixType->getColumnCount());
        SLANG_RELEASE_ASSERT(revRow && revCol);

        IRInst* dzero = nullptr;
        List<IRInst*> elements;
        for (IRIntegerValue r = 0; r < operandRow->getValue(); r++)
        {
            IRInst* dstRow = nullptr;
            if (r < revRow->getValue())
                dstRow = builder->emitElementExtract(revValue, builder->getIntValue(builder->getIntType(), r));
            for (IRIntegerValue c = 0; c < operandCol->getValue(); c++)
            {
                IRInst* element = nullptr;
                if (r < revRow->getValue() && c < revCol->getValue())
                {
                    element = builder->emitElementExtract(dstRow, builder->getIntValue(builder->getIntType(), c));
                }
                else
                {
                    if (!dzero)
                    {
                        dzero = builder->getFloatValue(operandMatrixType->getElementType(), 0.0f);
                    }
                    element = dzero;
                }
                elements.add(element);
            }
        }
        auto gradToProp = builder->emitMakeMatrix(operandMatrixType, (UInt)elements.getCount(), elements.getBuffer());
        gradients.add(RevGradient(
            RevGradient::Flavor::Simple,
            fwdMatrixReshape->getOperand(0),
            gradToProp,
            fwdMatrixReshape));
        return TranspositionResult(gradients);
    }

    TranspositionResult transposeMakeVector(IRBuilder* builder, IRInst* fwdMakeVector, IRInst* revValue)
    {
        List<RevGradient> gradients;
        for (UIndex ii = 0; ii < fwdMakeVector->getOperandCount(); ii++)
        {
            auto argOperand = fwdMakeVector->getOperand(ii);
            UInt componentCount = 1;
            if (auto vecType = as<IRVectorType>(argOperand->getDataType()))
            {
                auto intConstant = as<IRIntLit>(vecType->getElementCount());
                SLANG_RELEASE_ASSERT(intConstant);
                componentCount = (UInt)intConstant->getValue();
            }
            IRInst* gradAtIndex = nullptr;
            if (componentCount == 1)
            {
                gradAtIndex = builder->emitElementExtract(
                    argOperand->getDataType(),
                    revValue,
                    builder->getIntValue(builder->getIntType(), ii));
            }
            else
            {
                ShortList<UInt> componentIndices;
                for (UInt index = ii; index < ii + componentCount; index++)
                    componentIndices.add(index);
                gradAtIndex = builder->emitSwizzle(
                    argOperand->getDataType(),
                    revValue,
                    componentCount,
                    componentIndices.getArrayView().getBuffer());
            }

            gradients.add(RevGradient(
                RevGradient::Flavor::Simple,
                fwdMakeVector->getOperand(ii),
                gradAtIndex,
                fwdMakeVector));
        }

        // (A = float3(X, Y, Z)) -> [(dX += dA), (dY += dA), (dZ += dA)]
        return TranspositionResult(gradients);
    }

    TranspositionResult transposeMakeStruct(IRBuilder* builder, IRInst* fwdMakeStruct, IRInst* revValue)
    {
        List<RevGradient> gradients;
        auto structType = cast<IRStructType>(fwdMakeStruct->getFullType());
        UInt ii = 0;
        for (auto field : structType->getFields())
        {
            auto gradAtField = builder->emitFieldExtract(
                field->getFieldType(),
                revValue,
                field->getKey());
            SLANG_RELEASE_ASSERT(ii < fwdMakeStruct->getOperandCount());
            gradients.add(RevGradient(
                RevGradient::Flavor::Simple,
                fwdMakeStruct->getOperand(ii),
                gradAtField,
                fwdMakeStruct));
            ii++;
        }

        // (A = MakeStruct(F1, F2, F3)) -> [(dF1 += dA.F1), (dF2 += dA.F2), (dF3 += dA.F3)]
        return TranspositionResult(gradients);
    }

    TranspositionResult transposeMakeArray(IRBuilder* builder, IRInst* fwdMakeArray, IRInst* revValue)
    {
        List<RevGradient> gradients;
        auto arrayType = cast<IRArrayType>(fwdMakeArray->getFullType());

        for (UInt ii = 0; ii < fwdMakeArray->getOperandCount(); ii++)
        {
            auto gradAtField = builder->emitElementExtract(
                arrayType->getElementType(),
                revValue,
                builder->getIntValue(builder->getIntType(), ii));
            gradients.add(RevGradient(
                RevGradient::Flavor::Simple,
                fwdMakeArray->getOperand(ii),
                gradAtField,
                fwdMakeArray));
        }

        // (A = MakeArray(F1, F2, F3)) -> [(dF1 += dA.F1), (dF2 += dA.F2), (dF3 += dA.F3)]
        return TranspositionResult(gradients);
    }

    TranspositionResult transposeMakeArrayFromElement(IRBuilder* builder, IRInst* fwdMakeArrayFromElement, IRInst* revValue)
    {
        List<RevGradient> gradients;
        auto arrayType = cast<IRArrayType>(fwdMakeArrayFromElement->getFullType());
        auto arraySize = cast<IRIntLit>(arrayType->getElementCount());
        SLANG_RELEASE_ASSERT(arraySize);
        // TODO: if arraySize is a generic value, we can't statically expand things here.
        // In that case we probably need another opcode e.g. `Sum(arrayValue)` that can be expand
        // later in the pipeline when `arrayValue` becomes a known value.
        for (UInt ii = 0; ii < (UInt)arraySize->getValue(); ii++)
        {
            auto gradAtField = builder->emitElementExtract(
                arrayType->getElementType(),
                revValue,
                builder->getIntValue(builder->getIntType(), ii));
            gradients.add(RevGradient(
                RevGradient::Flavor::Simple,
                fwdMakeArrayFromElement->getOperand(0),
                gradAtField,
                fwdMakeArrayFromElement));
        }

        // (A = MakeArrayFromElement(E)) -> [(dE += dA.F1), (dE += dA.F2), (dE += dA.F3)]
        return TranspositionResult(gradients);
    }

    TranspositionResult transposeUpdateElement(IRBuilder* builder, IRInst* fwdUpdate, IRInst* revValue)
    {
        auto updateInst = as<IRUpdateElement>(fwdUpdate);

        List<RevGradient> gradients;
        auto accessChain = updateInst->getAccessChain();
        auto revElement = builder->emitElementExtract(revValue, accessChain.getArrayView());
        gradients.add(RevGradient(
            RevGradient::Flavor::Simple,
            updateInst->getElementValue(),
            revElement,
            fwdUpdate));

        auto primalElementTypeDecor = updateInst->findDecoration<IRPrimalElementTypeDecoration>();
        SLANG_RELEASE_ASSERT(primalElementTypeDecor);

        auto diffZero = emitDZeroOfDiffInstType(builder, (IRType*)primalElementTypeDecor->getPrimalElementType());
        SLANG_ASSERT(diffZero);
        auto revRest = builder->emitUpdateElement(
            revValue,
            accessChain,
            diffZero);
        gradients.add(RevGradient(
            RevGradient::Flavor::Simple,
            updateInst->getOldValue(),
            revRest,
            fwdUpdate));
        // (A = UpdateElement(arr, index, V)) -> [(dV += dA[index], d_arr += UpdateElement(revValue, index, 0)]
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
            auto operandType = unwrapAttributedType(operand->getDataType());
            if (operandType != targetType)
            {
                // Insert new operand just after the old operand, so we have the old
                // operands available.
                // 
                builder->setInsertAfter(operand);

                IRInst* newOperand = promoteToType(builder, targetType, operand);
                
                if (isDifferentialInst(operand))
                    builder->markInstAsDifferential(
                        newOperand, tryGetPrimalTypeFromDiffInst(fwdInst));

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
                builder->markInstAsDifferential(
                    newInst, tryGetPrimalTypeFromDiffInst(fwdInst));

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
            case kIROp_Div: 
            {
                if (isDifferentialInst(fwdInst->getOperand(0)))
                {
                    SLANG_RELEASE_ASSERT(!isDifferentialInst(fwdInst->getOperand(1)));

                    // (Out = dA / B) -> (dA += dOut / B)
                    return TranspositionResult(
                        List<RevGradient>(
                            RevGradient(
                                fwdInst->getOperand(0),
                                builder->emitDiv(operandType, revValue, fwdInst->getOperand(1)),
                                fwdInst)));
                }
                {
                    SLANG_ASSERT_FAILURE("The first operand of a div inst must be a differential inst");
                }
            }
            case kIROp_Neg: 
            {
                if (isDifferentialInst(fwdInst->getOperand(0)))
                {
                    // (Out = -dA) -> (dA += -dOut)
                    return TranspositionResult(
                        List<RevGradient>(
                            RevGradient(
                                fwdInst->getOperand(0),
                                builder->emitNeg(operandType, revValue),
                                fwdInst)));
                }
                else
                {
                    SLANG_ASSERT_FAILURE("Cannot transpose neg of a non-differentiable inst");
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

            case RevGradient::Flavor::GetElement:
                return materializeGetElementGradients(builder, aggPrimalType, gradients);

            default:
                SLANG_ASSERT_FAILURE("Unhandled gradient flavor for materialization");
        }
    }

    RevGradient materializeGetElementGradients(IRBuilder* builder, IRType* aggPrimalType, List<RevGradient> gradients)
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

        OrderedDictionary<IRInst*, List<RevGradient>> bucketedGradients;
        for (auto gradient : gradients)
        {
            // Grab the element affected by this gradient.
            auto getElementInst = as<IRGetElement>(gradient.fwdGradInst);
            SLANG_ASSERT(getElementInst);

            auto index = getElementInst->getIndex();
            SLANG_ASSERT(index);

            if (!bucketedGradients.ContainsKey(index))
            {
                bucketedGradients[index] = List<RevGradient>();
            }

            bucketedGradients[index].GetValue().add(RevGradient(
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

            // Construct address to this field in revGradVar.
            auto revGradTargetAddress = builder->emitElementAddress(
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

        OrderedDictionary<IRStructKey*, List<RevGradient>> bucketedGradients;
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
    
            // Construct address to this field in revGradVar.
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
        if (auto arrayType = as<IRArrayType>(primalType))
        {
            auto diffElementType = (IRType*)diffTypeContext.getDifferentialForType(builder, arrayType->getElementType());
            SLANG_RELEASE_ASSERT(diffElementType);
            auto diffArrayType = builder->getArrayType(diffElementType, arrayType->getElementCount());
            auto diffElementZero = emitDZeroOfDiffInstType(builder, arrayType->getElementType());
            return builder->emitMakeArrayFromElement(diffArrayType, diffElementZero);
        }
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
        if (auto arrayType = as<IRArrayType>(primalType))
        {
            auto diffElementType = (IRType*)diffTypeContext.getDifferentialForType(builder, arrayType->getElementType());
            SLANG_RELEASE_ASSERT(diffElementType);
            auto arraySize = arrayType->getElementCount();
            if (auto constArraySize = as<IRIntLit>(arraySize))
            {
                List<IRInst*> args;
                for (IRIntegerValue i = 0; i < constArraySize->getValue(); i++)
                {
                    auto index = builder->getIntValue(builder->getIntType(), i);
                    auto op1Val = builder->emitElementExtract(diffElementType, op1, index);
                    auto op2Val = builder->emitElementExtract(diffElementType, op2, index);
                    args.add(emitDAddOfDiffInstType(builder, arrayType->getElementType(), op1Val, op2Val));
                }
                auto diffArrayType = builder->getArrayType(diffElementType, arrayType->getElementCount());
                return builder->emitMakeArray(diffArrayType, (UInt)args.getCount(), args.getBuffer());
            }
            else
            {
                // TODO: insert a runtime loop here.
                SLANG_UNIMPLEMENTED_X("dadd of dynamic array.");
            }
        }
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

    Dictionary<IRInst*, IRVar*>                          inverseVarMap;

    List<IRInst*>                                        usedPtrs;

    Dictionary<IRBlock*, IRBlock*>                       revBlockMap;

    Dictionary<IRGlobalValueWithCode*, IRBlock*>         firstRevDiffBlockMap;

    Dictionary<IRBlock*, IRInst*>                        afterBlockMap;

    List<PendingBlockTerminatorEntry>                    pendingBlocks;

    Dictionary<IRBlock*, List<IRInst*>>                  phiGradsMap;
    
    Dictionary<IRInst*, IRInst*>                         inverseValueMap;
};


}
