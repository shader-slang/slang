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

    Dictionary<IRInst*, List<IRInst*>>      assignmentsMap;

    DiffTransposePass(AutoDiffSharedContext* autodiffContext) : autodiffContext(autodiffContext)
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

    void transposeFunc(
        IRFunc* fwdDiffFunc,
        IRFunc* revDiffFunc,
        // TODO: Maybe there's a more elegant way to pass this information.
        IRParam* dOutParameter)
    {
        // Traverse all instructions/blocks in reverse (starting from the terminator inst)
        // look for insts/blocks marked with IRDifferentialInstDecoration,
        // and transpose them in the revDiffFunc.
        //
        IRBuilder* builder;
        builder->init(autodiffContext->sharedBuilder);

        // Insert after the last block.
        builder->setInsertInto(revDiffFunc);

        for (IRBlock* block = fwdDiffFunc->getFirstBlock(); block; block = block->getNextBlock())
        {
            if (!isDifferentialInst(block))
            {
                // Skip blocks that aren't computing differentials.
                // At this stage we should have 'unzipped' the function
                // into blocks that either entirely deal with primal insts,
                // or entirely with differential insts.
                continue;
            }

            IRBlock* revBlock = builder->emitBlock();
            this->transposeBlock(block, revBlock);
        }
    }

    void transposeBlock(IRBlock* fwdBlock, IRBlock* revBlock)
    {
        IRBuilder* builder;
        builder->init(autodiffContext->sharedBuilder);

        // Insert after the last block.
        builder->setInsertInto(revBlock);

        // Note the 'reverse' traversal here.
        for (IRInst* child = fwdBlock->getLastChild(); child; child = child->getPrevInst())
        {
            transposeInst(builder, child);
        }
    }

    void transposeInst(IRBuilder* builder, IRInst* inst)
    {
        // Look for assignment entry for this inst.
        IRInst* revValue = builder->getFloatValue(builder->getType(kIROp_FloatType), 0.0);
        if (hasRevAssignments(inst))
        {
            // Emit the aggregate of all the assignments here. This will form the derivative
            revValue = emitAggregateValue(builder, getRevAssignments(inst));
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
                return transposeArithmetic(builder, fwdInst, revValue);

            default:
                SLANG_ASSERT_FAILURE("Unhandled instruction");
        }

        return TranspositionResult();
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
                                fwdInst->getOperand(0),
                                revValue)));
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

    bool hasRevAssignments(IRInst* fwdInst)
    {
        return assignmentsMap.ContainsKey(fwdInst);
    }
};


}
