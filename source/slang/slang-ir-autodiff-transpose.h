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
            if(!as<IRStore>(inst))
            {
                SLANG_UNEXPECTED("Could not resolve primal type for diff inst");
            }
        }

        // Emit the aggregate of all the gradients here. This will form the total derivative for this inst.
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

    IRInst* emitAggregateDifferentialPair(IRBuilder* builder, IRType* aggPrimalType, List<RevGradient> pairGradients)
    {
        SLANG_UNEXPECTED("Should not run.");

        auto aggPairType = as<IRDifferentialPairType>(aggPrimalType);
        SLANG_ASSERT(aggPairType);

        IRType* diffType = (IRType*)pairBuilder.getDiffTypeFromPairType(builder, aggPairType);

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
        diffInst = emitAggregateValue(builder, aggPairType->getValueType(), gradients);

        // Pack them back together.
        return builder->emitMakeDifferentialPair(aggPrimalType, primalInst, diffInst);
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

    AutoDiffSharedContext*                  autodiffContext;

    DifferentiableTypeConformanceContext    diffTypeContext;

    DifferentialPairTypeBuilder             pairBuilder;

    Dictionary<IRInst*, List<RevGradient>>      gradientsMap;

    Dictionary<IRInst*, IRInst*>*           primalsMap;

    List<IRInst*>                           usedPtrs;
};


}
