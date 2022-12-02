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
                this->addRevGradientForFwdInst(returnInst, RevGradient(returnInst, transposeInfo.dOutInst, nullptr));
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

    // A[cond_inst] -> (B or C) -> D => D[cond_inst] -> (B_T -> C_T) -> A_T

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
        for (auto pair : gradientsMap)
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
        // Look for gradient entries for this inst.
        List<RevGradient> gradients;
        if (hasRevGradients(inst))
            gradients = popRevGradients(inst);

        // Are we dealing with DifferentialPairType? 
        if (as<IRDifferentialPairType>(inst->getDataType()))
        {
            // This will be a 'hybrid' primal-differential inst, 
            // so we add a pair (primal_value, 0) as an additional
            // gradient to represent the primal part of the computation.
            // 
            // Now, if the unzip pass has done it's job, the _only_ 
            // case should be that inst is IRMakeDifferentialPair
            // 
            SLANG_ASSERT(as<IRMakeDifferentialPair>(inst));
            auto primalType = as<IRDifferentialPairType>(inst->getDataType())->getValueType();
            auto diffType = (IRType*)pairBuilder.getDiffTypeFromPairType(builder, as<IRDifferentialPairType>(inst->getDataType()));

            auto primalInst = as<IRMakeDifferentialPair>(inst)->getPrimalValue();
            auto zeroMethod = diffTypeContext.getZeroMethodForType(builder, primalType);

            // Must exist. 
            SLANG_ASSERT(zeroMethod);
            auto diffInst = builder->emitCallInst(diffType, zeroMethod, List<IRInst*>());
            
            gradients.add(
                RevGradient(
                    inst,
                    builder->emitMakeDifferentialPair(inst->getDataType(), primalInst, diffInst),
                    nullptr));
        }
        

        // Emit the aggregate of all the gradients here. This will form the total derivative for this inst.
        auto revValue = emitAggregateValue(builder, inst->getDataType(), gradients);

        // If revValue is null, gradients are not applicable to this inst 
        // (store, pointer manipulation etc). Ignore the inst entirely.
        //
        if (!revValue)
            return;

        auto transposeResult = transposeInst(builder, inst, revValue);
        
        // Add the new results to the gradients map.
        for (auto gradient : transposeResult.revPairs)
        {
            addRevGradientForFwdInst(gradient.targetInst, gradient);
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
            
            case kIROp_swizzle:
                return transposeSwizzle(builder, as<IRSwizzle>(fwdInst), revValue);

            case kIROp_Return:
                return transposeReturn(builder, as<IRReturn>(fwdInst), revValue);

            case kIROp_MakeDifferentialPair:
                return transposeMakePair(builder, as<IRMakeDifferentialPair>(fwdInst), revValue);

            case kIROp_DifferentialPairGetDifferential:
                return transposeGetDifferential(builder, as<IRDifferentialPairGetDifferential>(fwdInst), revValue);
            
            case kIROp_Construct:
                return transposeConstruct(builder, fwdInst, revValue);

            default:
                SLANG_ASSERT_FAILURE("Unhandled instruction");
        }
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

    TranspositionResult transposeMakePair(IRBuilder* builder, IRMakeDifferentialPair* fwdMakePair, IRInst* revValue)
    {
        // (P = (A, dA)) -> (dA += dP)
        return TranspositionResult(
                    List<RevGradient>(
                        RevGradient(
                            RevGradient::Flavor::Simple,
                            fwdMakePair->getDifferentialValue(), 
                            builder->emitDifferentialPairGetDifferential(
                                fwdMakePair->getDifferentialValue()->getDataType(),
                                revValue),
                            fwdMakePair)));
    }

    TranspositionResult transposeGetDifferential(IRBuilder*, IRDifferentialPairGetDifferential* fwdGetDiff, IRInst* revValue)
    {
        // (A = GetDiff(P)) -> (dP.d += dA)
        return TranspositionResult(
                    List<RevGradient>(
                        RevGradient(
                            RevGradient::Flavor::GetDifferential,
                            fwdGetDiff->getBase(),
                            revValue,
                            fwdGetDiff)));
    }

    TranspositionResult transposeConstruct(IRBuilder* builder, IRInst* fwdConstruct, IRInst* revValue)
    {
        // For now, we support only vector types. Extend this to other built-in types if necessary.
        SLANG_ASSERT(as<IRVectorType>(fwdConstruct->getDataType()));

        List<RevGradient> gradients;
        for (UIndex ii = 0; ii < fwdConstruct->getOperandCount(); ii++)
        {
            auto gradAtIndex = builder->emitElementExtract(
                fwdConstruct->getOperand(ii)->getDataType(),
                revValue,
                builder->getIntValue(builder->getIntType(), ii));

            gradients.add(RevGradient(
                            RevGradient::Flavor::Simple,
                            fwdConstruct->getOperand(ii),
                            gradAtIndex,
                            fwdConstruct));
        }

        // (A = float3(X, Y, Z)) -> [(dX += dA), (dY += dA), (dZ += dA)]
        return TranspositionResult(gradients);
    }

    // Gather all reverse-mode gradients for a Load inst, aggregate them and store them in the ptr.
    // 
    void accumulateGradientsForLoad(IRBuilder* builder, IRLoad* revLoad)
    {
        auto revPtr = revLoad->getPtr();
 
        // Assert that ptr type is of the form IRPtrTypeBase<IRDifferentialPairType<T>>
        SLANG_ASSERT(as<IRPtrTypeBase>(revPtr->getDataType()));
        SLANG_ASSERT(as<IRPtrTypeBase>(revPtr->getDataType())->getValueType()->getOp() == kIROp_DifferentialPairType);

        auto paramPairType = as<IRDifferentialPairType>(as<IRPtrTypeBase>(revPtr->getDataType())->getValueType());

        // Gather gradients.
        auto gradients = popRevGradients(revLoad);
        if (gradients.getCount() == 0)
        {
            // Ignore.
            return;
        }
        else
        {
            // Re-emit a load to get the _current_ value of revPtr.
            auto revCurrGrad = builder->emitLoad(revPtr);

            // Add the current value to the aggregation list.
            gradients.add(
                RevGradient(
                    revLoad,
                    revCurrGrad,
                    nullptr));
            
            // Get the _total_ value.
            auto aggregateGradient = emitAggregateValue(builder, paramPairType, gradients);

            // Store this back into the pointer.
            builder->emitStore(revPtr, aggregateGradient);
        }
    }

    TranspositionResult transposeReturn(IRBuilder*, IRReturn* fwdReturn, IRInst* revValue)
    {
        // TODO: This check needs to be changed to something like: isRelevantDifferentialPair()
        if (as<IRDifferentialPairType>(fwdReturn->getVal()->getDataType()))
        {
            // This is a subtle case, even though the returned value is returning
            // a pair, we need to pretend that the primal value is not being returned
            // since we only care about transposing differential computation.
            // So we're going to assume there is an implicit GetDifferential()
            // around the return value before returning.
            // 
            return TranspositionResult(
                        List<RevGradient>(
                            RevGradient(
                                RevGradient::Flavor::GetDifferential,
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
            SLANG_ASSERT(!as<IRVectorType>(currentType->getDataType()));

            auto targetVectorType = as<IRVectorType>(targetType);
            
            List<IRInst*> operands;
            for (Index ii = 0; ii < as<IRIntLit>(targetVectorType->getElementCount())->getValue(); ii++)
            {
                operands.add(inst);
            }

            IRInst* newInst = builder->emitConstructorInst(targetType, operands.getCount(), operands.getBuffer());
            
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

    RevGradient materializeGradientOfSwizzle(IRBuilder* builder, RevGradient gradient)
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
        
        for (Index ii = 0; ii < elementCount; ii++)
        {
            constructArgs.add(zeroValueInst);
        }

        // Replace swizzled elements with their gradients.
        for (UIndex ii = 0; ii < fwdSwizzleInst->getElementCount(); ii++)
        {
            auto sourceIndex = ii;
            auto targetIndexInst = fwdSwizzleInst->getElementIndex(ii);
            SLANG_ASSERT(as<IRIntLit>(targetIndexInst));
            auto targetIndex = as<IRIntLit>(targetIndexInst)->getValue();

            // Special-case for when the swizzled output is a single element.
            if (fwdSwizzleInst->getElementCount() == 1)
            {
                constructArgs[targetIndex] = gradient.revGradInst;
            }
            else
            {
                auto gradAtIndex = builder->emitElementExtract(elementType, gradient.revGradInst, builder->getIntValue(builder->getIntType(), sourceIndex));
                constructArgs[targetIndex] = gradAtIndex;
            }
        }

        return RevGradient(
            gradient.targetInst,
            builder->emitConstructorInst(baseType, elementCount, constructArgs.getBuffer()),
            gradient.fwdGradInst);
    }

    RevGradient materializeGradient(IRBuilder* builder, RevGradient gradient)
    {
        switch (gradient.flavor)
        {
            case RevGradient::Flavor::Simple:
                return gradient;
            
            case RevGradient::Flavor::Swizzle:
                return materializeGradientOfSwizzle(builder, gradient);

            default:
                SLANG_ASSERT_FAILURE("Unhandled gradient flavor for materialization");
        }
    }

    IRInst* emitAggregateDifferentialPair(IRBuilder* builder, IRType* aggregateType, List<RevGradient> pairGradients)
    {
        SLANG_ASSERT(as<IRDifferentialPairType>(aggregateType));

        IRType* diffType = (IRType*)pairBuilder.getDiffTypeFromPairType(builder, as<IRDifferentialPairType>(aggregateType));

        IRInst* primalInst = nullptr;
        IRInst* diffInst = nullptr;

        List<RevGradient> gradients;
        for (auto gradient : pairGradients)
        {
            switch (gradient.flavor)
            {
            case RevGradient::Flavor::Simple:
            {
                // In this case, the gradient is a 'pair' already, but we need to treat the primal element 
                // as if it didn't exist (we simply copy it over)
                // If we already saw a pair, throw an error since we don't know how to combine to primals.
                // (i.e. something went wrong prior to this step.)
                // 
                if (primalInst)
                {
                    SLANG_UNEXPECTED("Encountered multiple pair types in emitAggregateDifferentialPair");
                }
                
                primalInst = builder->emitDifferentialPairGetPrimal(gradient.revGradInst);
                gradients.add(
                    RevGradient(
                        RevGradient::Flavor::Simple,
                        gradient.targetInst,
                        builder->emitDifferentialPairGetDifferential(
                            diffType,
                            gradient.revGradInst),
                        gradient.fwdGradInst));
                break;
            }

            case RevGradient::Flavor::GetDifferential:
            {
                // In this case, the gradient is the result of transposing a GetDifferential
                // so we have only the gradient part. Just add it to the list of gradients to aggregate
                gradients.add(
                    RevGradient(
                        RevGradient::Flavor::Simple,
                        gradient.targetInst,
                        gradient.revGradInst,
                        gradient.fwdGradInst));
                break;
            }
            default:
                SLANG_UNEXPECTED("Unexpected gradient flavor in emitAggregateDifferentialPair");
            }
        }

        // Aggregate only the differentials
        diffInst = emitAggregateValue(builder, diffType, gradients);

        // Pack them back together.
        return builder->emitMakeDifferentialPair(aggregateType, primalInst, diffInst);
    }

    IRInst* emitAggregateValue(IRBuilder* builder, IRType* aggregateType, List<RevGradient> gradients)
    {
        // If we're dealing with the differential-pair types, we need to use a different aggregation method, since
        // a differential pair is really a 'hybrid' primal-differential type.
        //
        if (as<IRDifferentialPairType>(aggregateType))
            return emitAggregateDifferentialPair(builder, aggregateType, gradients);

        // Process non-simple gradients into simple gradients.
        // TODO: This is where we can improve efficiency later.
        // For instance if we have one gradient each for var.x, var.y and var.z
        // we can construct one single gradient vector out of the three vectors (i.e. float3(x_grad, y_grad, z_grad))
        // instead of creating one vector for each gradient and accumulating them 
        // (i.e. float3(x_grad, 0, 0) + float3(0, y_grad, 0) + float3(0, 0, z_grad))
        // The same concept can be extended for struct and array types (and for any combination of the three)
        // 
        List<RevGradient> simpleGradients;
        for (auto gradient : gradients)
        {
            simpleGradients.add(materializeGradient(builder, gradient));
        }

        if (simpleGradients.getCount() == 0)
        {
            // If there are no gradients to add up, check the type and emit a 0/null value.
            if (aggregateType != nullptr && !as<IRVoidType>(aggregateType))
            {
                // If type is non-null/non-void, call T.dzero() to produce a 0 gradient.
                auto zeroMethod = diffTypeContext.getZeroMethodForType(builder, aggregateType);
                return builder->emitCallInst(
                    (IRType*)diffTypeContext.getDifferentialForType(builder, aggregateType),
                    zeroMethod,
                    List<IRInst*>());
            }
            else
            {
                // Otherwise, gradients may not be applicable for this inst. return null
                return nullptr;
            }
        }
        else if (simpleGradients.getCount() == 1)
        {
            // If there's only one value to add up, just return it in order
            // to avoid a stack of 0 + 0 + 0 + ...
            return simpleGradients[0].revGradInst;
        }

        // If there's more than one gradient, aggregate them by adding them up.
        IRInst* currentValue = nullptr;
        for (auto gradient : simpleGradients)
        {
            if (!currentValue)
            {
                currentValue = gradient.revGradInst;
                continue;
            }

            auto addMethod = diffTypeContext.getAddMethodForType(builder, aggregateType);
            currentValue = builder->emitCallInst(aggregateType, addMethod, List<IRInst*>(currentValue, gradient.revGradInst));
        }

        return currentValue;
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

    AutoDiffSharedContext*                  autodiffContext;

    DifferentiableTypeConformanceContext    diffTypeContext;

    DifferentialPairTypeBuilder             pairBuilder;

    Dictionary<IRInst*, List<RevGradient>>      gradientsMap;

    Dictionary<IRInst*, IRInst*>*           primalsMap;
};


}
