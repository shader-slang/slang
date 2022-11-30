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
    AutoDiffSharedContext*                  autodiffContext;

    DifferentialPairTypeBuilder             pairBuilder;

    Dictionary<IRInst*, List<IRInst*>>      assignmentsMap;

    Dictionary<IRInst*, IRInst*>*           primalsMap;

    DiffTransposePass(AutoDiffSharedContext* autodiffContext) : 
        autodiffContext(autodiffContext), pairBuilder(autodiffContext)
    { }

    struct RevAssignment
    {
        IRInst* lvalue;
        IRInst* rvalue;

        RevAssignment(IRInst* lvalue, IRInst* rvalue) : lvalue(lvalue), rvalue(rvalue)
        { }
        RevAssignment() : lvalue(nullptr), rvalue(nullptr)
        { }
    };

    struct TranspositionResult
    {
        // Holds a set of pairs of 
        // (original-inst, inst-to-accumulate-for-orig-inst)
        List<RevAssignment> revPairs;

        TranspositionResult()
        { }

        TranspositionResult(List<RevAssignment> revPairs) : revPairs(revPairs)
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
        // TODO: Maybe there's a more elegant way to pass this information.
        FuncTranspositionInfo transposeInfo)
    {

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

        // TODO: We *might* need a step here that 'sorts' the work list in reverse order starting with 'leaf'
        // differential blocks, and following the branches backwards.
        // The alternative is to make phi nodes and treat all intermediaries & their gradients as arguments.

        for (auto block : workList)
        {
            // Set dOutParameter as the transpose gradient for the return inst, if any.
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                this->addRevAssignmentForFwdInst(returnInst, transposeInfo.dOutInst);
            }

            IRBlock* revBlock = builder.emitBlock();
            this->transposeBlock(block, revBlock);

            // TODO: This should only really be used for the transition from
            // the 'last' primal block(s) to the first differential block.
            // Transitions from differential blocks to 
            block->replaceUsesWith(revBlock);
            block->removeAndDeallocate();
        }
    }

    void transposeBlock(IRBlock* fwdBlock, IRBlock* revBlock)
    {
        IRBuilder builder;
        builder.init(autodiffContext->sharedBuilder);
 
        // Insert after the last block.
        builder.setInsertInto(revBlock);

        // Note the 'reverse' traversal here.
        for (IRInst* child = fwdBlock->getLastChild(); child; child = child->getPrevInst())
        {
            if (as<IRDecoration>(child))
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
        for (auto pair : assignmentsMap)
        {
            if (auto param = as<IRLoad>(pair.Key))
                accumulateGradientsForLoad(&builder, param);
        }

        // Emit a terminator inst.
        // TODO: need a be a lot smarter here. For now, we assume a single differential
        // block, so it should end in a return statement.
        if (as<IRReturn>(fwdBlock->getTerminator()))
        {
            // Emit a void return.
            builder.emitReturn();
        }
        else
        {
            SLANG_UNEXPECTED("Unhandled block terminator");
        }
    }

    void transposeInst(IRBuilder* builder, IRInst* inst)
    {
        // Look for assignment entry for this inst.
        IRInst* revValue = builder->getFloatValue(builder->getType(kIROp_FloatType), 0.0);
        if (hasRevAssignments(inst))
        {
            // Emit the aggregate of all the assignments here. This will form the derivative
            revValue = emitAggregateValue(builder, popRevAssignments(inst));
        }

        auto transposeResult = transposeInst(builder, inst, revValue);
        
        // Add the new results to the assignments map.
        for (auto pair : transposeResult.revPairs)
        {
            addRevAssignmentForFwdInst(pair.lvalue, pair.rvalue);
        }
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

            case kIROp_Return:
                return transposeReturn(builder, as<IRReturn>(fwdInst), revValue);

            case kIROp_MakeDifferentialPair:
                return transposeMakePair(builder, as<IRMakeDifferentialPair>(fwdInst), revValue);

            case kIROp_DifferentialPairGetDifferential:
                return transposeGetDifferential(builder, as<IRDifferentialPairGetDifferential>(fwdInst), revValue);

            default:
                SLANG_ASSERT_FAILURE("Unhandled instruction");
        }
    }

    TranspositionResult transposeMakePair(IRBuilder*, IRMakeDifferentialPair* fwdMakePair, IRInst* revValue)
    {
        // (P = (A, dA)) -> (dA += dP)
        return TranspositionResult(
                    List<RevAssignment>(
                        RevAssignment(
                            fwdMakePair->getDifferentialValue(), 
                            revValue)));
    }

    TranspositionResult transposeGetDifferential(IRBuilder*, IRDifferentialPairGetDifferential* fwdGetDiff, IRInst* revValue)
    {
        // (A = GetDiff(P)) -> (dP.d += dA)
        return TranspositionResult(
                    List<RevAssignment>(
                        RevAssignment(
                            fwdGetDiff->getBase(),
                            revValue)));
    }

    // Gather all reverse-mode gradients for parameters, and store to the differential
    // 
    void accumulateGradientsForLoad(IRBuilder* builder, IRLoad* revLoad)
    {
        auto revPtr = revLoad->getPtr();

        // Assert that param type is of the form IRPtrTypeBase<IRDifferentialPairType<T>>
        SLANG_ASSERT(as<IRPtrTypeBase>(revPtr->getDataType()));
        SLANG_ASSERT(as<IRPtrTypeBase>(revPtr->getDataType())->getValueType()->getOp() == kIROp_DifferentialPairType);

        auto paramPairType = as<IRDifferentialPairType>(as<IRPtrTypeBase>(revPtr->getDataType())->getValueType());
        auto diffType = (IRType*) pairBuilder.getDiffTypeFromPairType(builder, paramPairType);

        // Gather gradients.
        auto gradients = popRevAssignments(revLoad);
        if (gradients.getCount() == 0)
        {
            // Ignore.
            return;
        }
        else
        {
            // Re-emit a load to get the _current_ value of revPtr.
            auto revCurrLoad = builder->emitLoad(revPtr);

            // Grab the current gradient value.
            auto revCurrGrad = builder->emitDifferentialPairGetDifferential(diffType, revCurrLoad);

            // Add the current value to the aggregation list.
            gradients.add(revCurrGrad);
            
            // Get the _total_ value.
            auto aggregateGradient = emitAggregateValue(builder, gradients);

            // Grab the current primal value.
            auto revCurrPrimal = builder->emitDifferentialPairGetPrimal(revCurrLoad);

            // Make the pair with the new gradient.
            auto newDiffPair = builder->emitMakeDifferentialPair(paramPairType, revCurrPrimal, aggregateGradient);

            // Store this back into the parameter.
            builder->emitStore(revPtr, newDiffPair);
        }
    }

    TranspositionResult transposeReturn(IRBuilder*, IRReturn* fwdReturn, IRInst* revValue)
    {
        
        if (as<IRDifferentialPairType>(fwdReturn->getVal()->getDataType()))
        {
            // If the type is a differential pair, we add the reverse-value for the *pair* 
            // itself. TODO: Signal this through flags in the 'RevAssignment' struct.
            // (return (A, dA)) -> (dA += dOut)
            return TranspositionResult(
                        List<RevAssignment>(
                            RevAssignment(
                                fwdReturn->getVal(), 
                                revValue)));
        }
        else
        {
            // (return A) -> (empty)
            return TranspositionResult();
        }
    }

    TranspositionResult transposeArithmetic(IRBuilder* builder, IRInst* fwdInst, IRInst* revValue)
    {
        IRType* floatType = builder->getType(kIROp_FloatType);
        switch(fwdInst->getOp())
        {
            case kIROp_Add:
            {
                // (Out = dA + dB) -> [(dA += dOut), (dB += dOut)]
                return TranspositionResult(
                        List<RevAssignment>(
                            RevAssignment(
                                fwdInst->getOperand(0),
                                revValue),
                            RevAssignment(
                                fwdInst->getOperand(1),
                                revValue)));
            }
            case kIROp_Sub:
            {
                // (Out = dA - dB) -> [(dA += dOut), (dB -= dOut)]
                return TranspositionResult(
                        List<RevAssignment>(
                            RevAssignment(
                                fwdInst->getOperand(0),
                                revValue),
                            RevAssignment(
                                fwdInst->getOperand(1),
                                builder->emitNeg(
                                    revValue->getDataType(), revValue))));
            }
            case kIROp_Mul: 
            {
                if (isDifferentialInst(fwdInst->getOperand(0)))
                {
                    // (Out = dA * B) -> (dA += B * dOut)
                    return TranspositionResult(
                        List<RevAssignment>(
                            RevAssignment(
                                fwdInst->getOperand(0),
                                builder->emitMul(floatType, fwdInst->getOperand(1), revValue))));
                }
                else if (isDifferentialInst(fwdInst->getOperand(1)))
                {
                    // (Out = A * dB) -> (dB += A * dOut)
                    return TranspositionResult(
                        List<RevAssignment>(
                            RevAssignment(
                                fwdInst->getOperand(1),
                                builder->emitMul(floatType, fwdInst->getOperand(0), revValue))));
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

    IRInst* emitAggregateValue(IRBuilder* builder, List<IRInst*> values)
    {
        // We're handling the case where the types are all float,
        // so we can use a bunch of kIROp_Add insts to add them up.
        // If this is an arbitrary type T, we need to lookup and 
        // call T.dadd()

        IRInst* initialValue = builder->getFloatValue(builder->getType(kIROp_FloatType), 0.0);
        if (values.getCount() == 0)
        {
            // If there's not values to add up, emit a 0 value.
            return initialValue;
        }
        else if (values.getCount() == 1)
        {
            // If there's only one value to add up, just return it in order
            // to avoid a stack of 0 + 0 + 0 + ...
            return values[0];
        }

        // If there's more than one value, aggregate them by adding them up.

        SLANG_ASSERT(values[0]->getDataType()->getOp() == kIROp_FloatType);

        IRInst* currentValue = initialValue;
        for (auto value : values)
        {
            currentValue = builder->emitAdd(
                builder->getType(kIROp_FloatType), currentValue, value);
        }

        return currentValue;
    }

    void addRevAssignmentForFwdInst(IRInst* fwdInst, IRInst* assignment)
    {
        if (!hasRevAssignments(fwdInst))
        {
            assignmentsMap[fwdInst] = List<IRInst*>();
        }

        assignmentsMap[fwdInst].GetValue().add(assignment);
    }

    List<IRInst*> getRevAssignments(IRInst* fwdInst)
    {
        return assignmentsMap[fwdInst];
    }

    List<IRInst*> popRevAssignments(IRInst* fwdInst)
    {
        List<IRInst*> val = assignmentsMap[fwdInst].GetValue();
        assignmentsMap.Remove(fwdInst);
        return val;
    }

    bool hasRevAssignments(IRInst* fwdInst)
    {
        return assignmentsMap.ContainsKey(fwdInst);
    }
};


}
