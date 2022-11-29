#include "slang-ir-autodiff-rev.h"

#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-util.h"
#include "slang-ir-inst-pass-base.h"

#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-propagate.h"
#include "slang-ir-autodiff-unzip.h"
#include "slang-ir-autodiff-transpose.h"


namespace Slang
{
struct BackwardDiffTranscriber
{
    // Stores the mapping of arbitrary 'R-value' instructions to instructions that represent
    // their differential values.
    Dictionary<IRInst*, IRInst*> orginalToTranscribed;

    // Set of insts currently being transcribed. Used to avoid infinite loops.
    HashSet<IRInst*>                        instsInProgress;

    // Cloning environment to hold mapping from old to new copies for the primal
    // instructions.
    IRCloneEnv                              cloneEnv;

    // Diagnostic sink for error messages.
    DiagnosticSink* sink;

    // Type conformance information.
    AutoDiffSharedContext* autoDiffSharedContext;

    // Builder to help with creating and accessing the 'DifferentiablePair<T>' struct
    DifferentialPairTypeBuilder* pairBuilder;

    DifferentiableTypeConformanceContext    differentiableTypeConformanceContext;

    List<InstPair>                          followUpFunctionsToTranscribe;

    // Map that stores the upper gradient given an IRInst*
    Dictionary<IRInst*, List<IRInst*>> upperGradients;
    Dictionary<IRInst*, IRInst*> primalToDiffPair;

    SharedIRBuilder* sharedBuilder;
    // Witness table that `DifferentialBottom:IDifferential`.
    IRWitnessTable* differentialBottomWitness = nullptr;
    Dictionary<InstPair, IRInst*> differentialPairTypes;

    // References to other passes that for reverse-mode transcription.
    ForwardDerivativeTranscriber    *fwdDiffTranscriber;
    DiffTransposePass               *diffTransposePass;
    DiffPropagationPass             *diffPropagationPass;
    DiffUnzipPass                   *diffUnzipPass;

    // Allocate space for the passes.
    ForwardDerivativeTranscriber    fwdDiffTranscriberStorage;
    DiffTransposePass               diffTransposePassStorage;
    DiffPropagationPass             diffPropagationPassStorage;
    DiffUnzipPass                   diffUnzipPassStorage;


    BackwardDiffTranscriber(AutoDiffSharedContext* shared, SharedIRBuilder* inSharedBuilder, DiagnosticSink* inSink)
        : autoDiffSharedContext(shared)
        , sink(inSink)
        , differentiableTypeConformanceContext(shared)
        , sharedBuilder(inSharedBuilder)
        , fwdDiffTranscriberStorage(shared, inSharedBuilder)
        , diffTransposePassStorage(shared)
        , diffPropagationPassStorage(shared)
        , diffUnzipPassStorage(shared)
        , fwdDiffTranscriber(&fwdDiffTranscriberStorage)
        , diffTransposePass(&diffTransposePassStorage)
        , diffPropagationPass(&diffPropagationPassStorage)
        , diffUnzipPass(&diffUnzipPassStorage)
    { }

    DiagnosticSink* getSink()
    {
        SLANG_ASSERT(sink);
        return sink;
    }

    IRFuncType* differentiateFunctionType(IRBuilder* builder, IRFuncType* funcType)
    {
        List<IRType*> newParameterTypes;
        IRType* diffReturnType;

        for (UIndex i = 0; i < funcType->getParamCount(); i++)
        {
            auto origType = funcType->getParamType(i);
            if (auto diffPairType = tryGetDiffPairType(builder, origType))
            {
                auto inoutDiffPairType = builder->getPtrType(kIROp_InOutType, diffPairType);
                newParameterTypes.add(inoutDiffPairType);
            }
            else
                newParameterTypes.add(origType);
        }

        newParameterTypes.add(funcType->getResultType());

        diffReturnType = builder->getVoidType();

        return builder->getFuncType(newParameterTypes, diffReturnType);
    }

    // Get or construct `:IDifferentiable` conformance for a DifferentiablePair.
    IRWitnessTable* getDifferentialPairWitness(IRInst* inDiffPairType)
    {
        IRBuilder builder(sharedBuilder);
        builder.setInsertInto(inDiffPairType->parent);
        auto diffPairType = as<IRDifferentialPairType>(inDiffPairType);
        SLANG_ASSERT(diffPairType);

        auto table = builder.createWitnessTable(autoDiffSharedContext->differentiableInterfaceType, diffPairType);
        auto diffType = differentiateType(&builder, diffPairType->getValueType());
        auto differentialType = builder.getDifferentialPairType(diffType, nullptr);
        builder.createWitnessTableEntry(table, autoDiffSharedContext->differentialAssocTypeStructKey, differentialType);
        // Omit the method synthesis here, since we can just intercept those directly at `getXXMethodForType`.

        differentiableTypeConformanceContext.differentiableWitnessDictionary[diffPairType] = table;
        return table;
    }

    IRType* getOrCreateDiffPairType(IRInst* primalType, IRInst* witness)
    {
        IRBuilder builder(sharedBuilder);
        builder.setInsertInto(primalType->parent);
        return builder.getDifferentialPairType(
            (IRType*)primalType,
            witness);
    }

    IRType* getOrCreateDiffPairType(IRInst* primalType)
    {
        IRBuilder builder(sharedBuilder);
        builder.setInsertInto(primalType->parent);
        auto witness = as<IRWitnessTable>(
            differentiableTypeConformanceContext.lookUpConformanceForType((IRType*)primalType));

        return builder.getDifferentialPairType(
            (IRType*)primalType,
            witness);
    }

    IRType* differentiateType(IRBuilder* builder, IRType* origType)
    {
        IRInst* diffType = nullptr;
        if (!orginalToTranscribed.TryGetValue(origType, diffType))
        {
            diffType = _differentiateTypeImpl(builder, origType);
            orginalToTranscribed[origType] = diffType;
        }
        return (IRType*)diffType;
    }

    IRType* _differentiateTypeImpl(IRBuilder* builder, IRType* origType)
    {
        if (auto ptrType = as<IRPtrTypeBase>(origType))
            return builder->getPtrType(
                origType->getOp(),
                differentiateType(builder, ptrType->getValueType()));

        // If there is an explicit primal version of this type in the local scope, load that
        // otherwise use the original type. 
        //
        IRInst* primalType = origType;

        // Special case certain compound types (PtrType, FuncType, etc..)
        // otherwise try to lookup a differential definition for the given type.
        // If one does not exist, then we assume it's not differentiable.
        // 
        switch (primalType->getOp())
        {
        case kIROp_Param:
            if (as<IRTypeType>(primalType->getDataType()))
                return (IRType*)(differentiableTypeConformanceContext.getDifferentialForType(
                    builder,
                    (IRType*)primalType));
            else if (as<IRWitnessTableType>(primalType->getDataType()))
                return (IRType*)primalType;

        case kIROp_ArrayType:
        {
            auto primalArrayType = as<IRArrayType>(primalType);
            if (auto diffElementType = differentiateType(builder, primalArrayType->getElementType()))
                return builder->getArrayType(
                    diffElementType,
                    primalArrayType->getElementCount());
            else
                return nullptr;
        }

        case kIROp_DifferentialPairType:
        {
            auto primalPairType = as<IRDifferentialPairType>(primalType);
            return getOrCreateDiffPairType(
                pairBuilder->getDiffTypeFromPairType(builder, primalPairType),
                pairBuilder->getDiffTypeWitnessFromPairType(builder, primalPairType));
        }

        case kIROp_FuncType:
            return differentiateFunctionType(builder, as<IRFuncType>(primalType));

        case kIROp_OutType:
            if (auto diffValueType = differentiateType(builder, as<IROutType>(primalType)->getValueType()))
                return builder->getOutType(diffValueType);
            else
                return nullptr;

        case kIROp_InOutType:
            if (auto diffValueType = differentiateType(builder, as<IRInOutType>(primalType)->getValueType()))
                return builder->getInOutType(diffValueType);
            else
                return nullptr;

        case kIROp_TupleType:
        {
            auto tupleType = as<IRTupleType>(primalType);
            List<IRType*> diffTypeList;
            // TODO: what if we have type parameters here?
            for (UIndex ii = 0; ii < tupleType->getOperandCount(); ii++)
                diffTypeList.add(
                    differentiateType(builder, (IRType*)tupleType->getOperand(ii)));

            return builder->getTupleType(diffTypeList);
        }

        default:
            return (IRType*)(differentiableTypeConformanceContext.getDifferentialForType(builder, (IRType*)primalType));
        }
    }

    IRType* tryGetDiffPairType(IRBuilder* builder, IRType* primalType)
    {
        // If this is a PtrType (out, inout, etc..), then create diff pair from
        // value type and re-apply the appropropriate PtrType wrapper.
        // 
        if (auto origPtrType = as<IRPtrTypeBase>(primalType))
        {
            if (auto diffPairValueType = tryGetDiffPairType(builder, origPtrType->getValueType()))
                return builder->getPtrType(primalType->getOp(), diffPairValueType);
            else
                return nullptr;
        }
        auto diffType = differentiateType(builder, primalType);
        if (diffType)
            return (IRType*)getOrCreateDiffPairType(primalType);
        return nullptr;
    }

    InstPair transcribeParam(IRBuilder* builder, IRParam* origParam)
    {
        auto primalDataType = origParam->getDataType();
        // Do not differentiate generic type (and witness table) parameters
        if (as<IRTypeType>(primalDataType) || as<IRWitnessTableType>(primalDataType))
        {
            return InstPair(
                cloneInst(&cloneEnv, builder, origParam),
                nullptr);
        }

        if (auto diffPairType = tryGetDiffPairType(builder, (IRType*)primalDataType))
        {
            IRInst* diffPairParam = builder->emitParam(diffPairType);

            auto diffPairVarName = makeDiffPairName(origParam);
            if (diffPairVarName.getLength() > 0)
                builder->addNameHintDecoration(diffPairParam, diffPairVarName.getUnownedSlice());

            SLANG_ASSERT(diffPairParam);

            if (auto pairType = as<IRDifferentialPairType>(diffPairParam->getDataType()))
            {
                return InstPair(
                    builder->emitDifferentialPairGetPrimal(diffPairParam),
                    builder->emitDifferentialPairGetDifferential(
                        (IRType*)pairBuilder->getDiffTypeFromPairType(builder, pairType),
                        diffPairParam));
            }
            // If this is an `in/inout DifferentialPair<>` parameter, we can't produce
            // its primal and diff parts right now because they would represent a reference
            // to a pair field, which doesn't make sense since pair types are considered mutable.
            // We encode the result as if the param is non-differentiable, and handle it
            // with special care at load/store.
            return InstPair(diffPairParam, nullptr);
        }


        return InstPair(
            cloneInst(&cloneEnv, builder, origParam),
            nullptr);
    }

    // Returns "dp<var-name>" to use as a name hint for parameters.
    // If no primal name is available, returns a blank string.
    // 
    String makeDiffPairName(IRInst* origVar)
    {
        if (auto namehintDecoration = origVar->findDecoration<IRNameHintDecoration>())
        {
            return ("dp" + String(namehintDecoration->getName()));
        }

        return String("");
    }


    // In differential computation, the 'default' differential value is always zero.
    // This is a consequence of differential computing being inherently linear. As a 
    // result, it's useful to have a method to generate zero literals of any (arithmetic) type.
    // The current implementation requires that types are defined linearly.
    // 
    IRInst* getDifferentialZeroOfType(IRBuilder* builder, IRType* primalType)
    {
        if (auto diffType = differentiateType(builder, primalType))
        {
            switch (diffType->getOp())
            {
            case kIROp_DifferentialPairType:
                return builder->emitMakeDifferentialPair(
                    diffType,
                    getDifferentialZeroOfType(builder, as<IRDifferentialPairType>(diffType)->getValueType()),
                    getDifferentialZeroOfType(builder, as<IRDifferentialPairType>(diffType)->getValueType()));
            }
            // Since primalType has a corresponding differential type, we can lookup the 
            // definition for zero().
            auto zeroMethod = differentiableTypeConformanceContext.getZeroMethodForType(builder, primalType);
            SLANG_ASSERT(zeroMethod);

            auto emptyArgList = List<IRInst*>();
            return builder->emitCallInst((IRType*)diffType, zeroMethod, emptyArgList);
        }
        else
        {
            if (isScalarIntegerType(primalType))
            {
                return builder->getIntValue(primalType, 0);
            }

            getSink()->diagnose(primalType->sourceLoc,
                Diagnostics::internalCompilerError,
                "could not generate zero value for given type");
            return nullptr;
        }
    }

    InstPair transcribeBlock(IRBuilder* builder, IRBlock* origBlock)
    {
        IRBuilder subBuilder(builder->getSharedBuilder());
        subBuilder.setInsertLoc(builder->getInsertLoc());

        IRBlock* diffBlock = subBuilder.emitBlock();

        subBuilder.setInsertInto(diffBlock);

        // First transcribe every parameter in the block.
        for (auto param = origBlock->getFirstParam(); param; param = param->getNextParam())
            this->copyParam(&subBuilder, param);

        // The extra param for input gradient
        auto gradParam = subBuilder.emitParam(as<IRFuncType>(origBlock->getParent()->getFullType())->getResultType());

        // Then, run through every instruction and use the transcriber to generate the appropriate
        // derivative code.
        //
        for (auto child = origBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
            this->copyInst(&subBuilder, child);

        auto lastInst = diffBlock->getLastOrdinaryInst();
        List<IRInst*> grads = { gradParam };
        upperGradients.Add(lastInst, grads);
        for (auto child = diffBlock->getLastOrdinaryInst(); child; child = child->getPrevInst())
        {
            auto upperGrads = upperGradients.TryGetValue(child);
            if (!upperGrads)
                continue;
            if (upperGrads->getCount() > 1)
            {
                auto sumGrad = upperGrads->getFirst();
                for (auto i = 1; i < upperGrads->getCount(); i++)
                {
                    sumGrad = subBuilder.emitAdd(sumGrad->getDataType(), sumGrad, (*upperGrads)[i]);
                }
                this->transcribeInstBackward(&subBuilder, child, sumGrad);
            }
            else
                this->transcribeInstBackward(&subBuilder, child, upperGrads->getFirst());
        }

        subBuilder.emitReturn();

        return InstPair(diffBlock, diffBlock);
    }

    // Create an empty func to represent the transcribed func of `origFunc`.
    InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc)
    {
        IRBuilder builder(inBuilder->getSharedBuilder());
        builder.setInsertBefore(origFunc);

        IRFunc* primalFunc = origFunc;

        differentiableTypeConformanceContext.setFunc(origFunc);

        primalFunc = origFunc;

        auto diffFunc = builder.createFunc();

        SLANG_ASSERT(as<IRFuncType>(origFunc->getFullType()));
        IRType* diffFuncType = this->differentiateFunctionType(
            &builder,
            as<IRFuncType>(origFunc->getFullType()));
        diffFunc->setFullType(diffFuncType);

        if (auto nameHint = origFunc->findDecoration<IRNameHintDecoration>())
        {
            auto originalName = nameHint->getName();
            StringBuilder newNameSb;
            newNameSb << "s_bwd_" << originalName;
            builder.addNameHintDecoration(diffFunc, newNameSb.getUnownedSlice());
        }
        builder.addBackwardDerivativeDecoration(origFunc, diffFunc);

        // Mark the generated derivative function itself as differentiable.
        builder.addBackwardDifferentiableDecoration(diffFunc);

        // Find and clone `DifferentiableTypeDictionaryDecoration` to the new diffFunc.
        if (auto dictDecor = origFunc->findDecoration<IRDifferentiableTypeDictionaryDecoration>())
        {
            cloneDecoration(dictDecor, diffFunc);
        }

        auto result = InstPair(primalFunc, diffFunc);
        followUpFunctionsToTranscribe.add(result);
        return result;
    }

    // Puts parameters into their own block.
    void makeParameterBlock(IRBuilder* inBuilder, IRFunc* func)
    {
        IRBuilder builder(inBuilder->getSharedBuilder());

        auto firstBlock = func->getFirstBlock();
        IRParam* param = func->getFirstParam();

        builder.setInsertBefore(firstBlock);
        
        // Note: It looks like emitBlock() doesn't use the current 
        // builder position, so we're going to manually move the new block
        // to before the existing block.
        auto paramBlock = builder.emitBlock();
        paramBlock->insertBefore(firstBlock);
        builder.setInsertInto(paramBlock);

        while(param)
        {
            IRParam* nextParam = param->getNextParam();

            // Copy inst into the new parameter block.
            auto clonedParam = cloneInst(&cloneEnv, &builder, param);
            param->replaceUsesWith(clonedParam);
            param->removeAndDeallocate();

            param = nextParam;
        }
        
        // Replace this block as the first block.
        firstBlock->replaceUsesWith(paramBlock);

        // Add terminator inst.
        builder.emitBranch(firstBlock);
    }

    // Transcribe a function definition.
    InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc)
    {
        SLANG_ASSERT(primalFunc);
        SLANG_ASSERT(diffFunc);
        // Reverse-mode transcription uses 4 separate steps:
        // TODO(sai): Fill in documentation.

        // Generate a temporary forward derivative function as an intermediate step.
        IRFunc* fwdDiffFunc = as<IRFunc>(fwdDiffTranscriber->transcribeFuncHeader(builder, (IRFunc*)primalFunc).differential);
        SLANG_ASSERT(fwdDiffFunc);

        // Transcribe the body of the primal function into it's linear (fwd-diff) form.
        // TODO(sai): Handle the case when we already have a user-defined fwd-derivative function.
        fwdDiffTranscriber->transcribeFunc(builder, primalFunc, as<IRFunc>(fwdDiffFunc));
        
        // Split first block into a paramter block.
        this->makeParameterBlock(builder, as<IRFunc>(fwdDiffFunc));
        
        // This steps adds a decoration to instructions that are computing the differential.
        diffPropagationPass->propagateDiffInstDecoration(builder, fwdDiffFunc);

        // Copy primal insts to the first block of the unzipped function, copy diff insts to the
        // second block of the unzipped function.
        // 
        IRFunc* unzippedFwdDiffFunc = diffUnzipPass->unzipDiffInsts(fwdDiffFunc);
        
        // Clone the primal blocks from unzippedFwdDiffFunc
        // to the reverse-mode function.
        // TODO: This is the spot where we can make a decision to split
        // the primal and differential into two different funcitons
        // instead of two blocks in the same function.
        // 
        // Special care needs to be taken for the first block since it holds the parameters
        
        // Clone all blocks into a temporary diff func.
        // We're using a temporary sice we don't want to clone decorations, 
        // only blocks, and right now there's no provision in slang-ir-clone.h
        // for that.
        // 
        builder->setInsertInto(diffFunc->getParent());
        auto tempDiffFunc = as<IRFunc>(cloneInst(&cloneEnv, builder, unzippedFwdDiffFunc));

        // Move blocks to the diffFunc shell.
        {
            List<IRBlock*> workList;
            for (auto block = tempDiffFunc->getFirstBlock(); block; block = block->getNextBlock())
                workList.add(block);
            
            for (auto block : workList)
                block->insertAtEnd(diffFunc);
        }

        // Transpose the first block (parameter block)
        transcribeParameterBlock(builder, diffFunc);

        builder->setInsertInto(diffFunc);

        auto dOutParameter = diffFunc->getLastParam();

        // Transpose differential blocks from unzippedFwdDiffFunc into diffFunc (with dOutParameter) representing the 
        DiffTransposePass::FuncTranspositionInfo info = {dOutParameter, nullptr};
        diffTransposePass->transposeDiffBlocksInFunc(diffFunc, info);

        // Clean up by deallocating intermediate steps.
        tempDiffFunc->removeAndDeallocate();
        unzippedFwdDiffFunc->removeAndDeallocate();
        fwdDiffFunc->removeAndDeallocate();

        return InstPair(primalFunc, diffFunc);
    }

    void transcribeParameterBlock(IRBuilder* builder, IRFunc* diffFunc)
    {
        IRBlock* fwdDiffParameterBlock = diffFunc->getFirstBlock();

        // Find the 'next' block using the terminator inst of the parameter block.
        auto fwdParamBlockBranch = as<IRUnconditionalBranch>(fwdDiffParameterBlock->getTerminator());
        auto nextBlock = fwdParamBlockBranch->getTargetBlock();

        builder->setInsertInto(fwdDiffParameterBlock);

        // 1. Turn fwd-diff versions of the parameters into reverse-diff versions by wrapping them as InOutType<>
        for (auto child = fwdDiffParameterBlock->getFirstParam(); child;)
        {
            IRParam* nextChild = child->getNextParam();

            auto fwdParam = as<IRParam>(child);
            SLANG_ASSERT(fwdParam);
            
            // TODO: Handle ptr<pair> types.
            if (auto diffPairType = as<IRDifferentialPairType>(fwdParam->getDataType()))
            {
                // Create inout version. 
                auto inoutDiffPairType = builder->getInOutType(diffPairType);
                auto newParam = builder->emitParam(inoutDiffPairType);

                // Map the _load_ of the new parameter as the clone of the old one.
                auto newParamLoad = builder->emitLoad(newParam);
                newParamLoad->insertAtStart(nextBlock); // Move to first block _after_ the parameter block.
                fwdParam->replaceUsesWith(newParamLoad);
                fwdParam->removeAndDeallocate();
            }
            else
            {
                // Default case (parameter has nothing to do with differentiation)
                // Do nothing.
            }

            child = nextChild;
        }

        auto paramCount = as<IRFuncType>(diffFunc->getDataType())->getParamCount();

        // 2. Add a parameter for 'derivative of the output' (d_out). 
        // The type is the last parameter type of the function.
        // 
        auto dOutParamType = as<IRFuncType>(diffFunc->getDataType())->getParamType(paramCount - 1);

        SLANG_ASSERT(dOutParamType);

        builder->emitParam(dOutParamType);
    }

    IRInst* copyParam(IRBuilder* builder, IRParam* origParam)
    {
        auto primalDataType = origParam->getDataType();

        if (auto diffPairType = tryGetDiffPairType(builder, (IRType*)primalDataType))
        {
            auto inoutDiffPairType = builder->getPtrType(kIROp_InOutType, diffPairType);
            IRInst* diffParam = builder->emitParam(inoutDiffPairType);

            auto diffPairVarName = makeDiffPairName(origParam);
            if (diffPairVarName.getLength() > 0)
                builder->addNameHintDecoration(diffParam, diffPairVarName.getUnownedSlice());

            SLANG_ASSERT(diffParam);
            auto paramValue = builder->emitLoad(diffParam);
            auto primal = builder->emitDifferentialPairGetPrimal(paramValue);
            orginalToTranscribed.Add(origParam, primal);
            primalToDiffPair.Add(primal, diffParam);

            return diffParam;
        }


        return cloneInst(&cloneEnv, builder, origParam);
    }

    InstPair copyBinaryArith(IRBuilder* builder, IRInst* origArith)
    {
        SLANG_ASSERT(origArith->getOperandCount() == 2);

        auto origLeft = origArith->getOperand(0);
        auto origRight = origArith->getOperand(1);

        IRInst* primalLeft;
        if (!orginalToTranscribed.TryGetValue(origLeft, primalLeft))
        {
            primalLeft = origLeft;
        }
        IRInst* primalRight;
        if (!orginalToTranscribed.TryGetValue(origRight, primalRight))
        {
            primalRight = origRight;
        }

        auto resultType = origArith->getDataType();
        IRInst* newInst;
        switch (origArith->getOp())
        {
        case kIROp_Add:
            newInst = builder->emitAdd(resultType, primalLeft, primalRight);
            break;
        case kIROp_Mul:
            newInst = builder->emitMul(resultType, primalLeft, primalRight);
            break;
        case kIROp_Sub:
            newInst = builder->emitSub(resultType, primalLeft, primalRight);
            break;
        case kIROp_Div:
            newInst = builder->emitDiv(resultType, primalLeft, primalRight);
            break;
        default:
            newInst = nullptr;
            getSink()->diagnose(origArith->sourceLoc,
                Diagnostics::unimplemented,
                "this arithmetic instruction cannot be differentiated");
        }
        orginalToTranscribed.Add(origArith, newInst);
        return InstPair(newInst, nullptr);
    }

    IRInst* transcribeBinaryArithBackward(IRBuilder* builder, IRInst* origArith, IRInst* grad)
    {
        SLANG_ASSERT(origArith->getOperandCount() == 2);

        auto lhs = origArith->getOperand(0);
        auto rhs = origArith->getOperand(1);

        if (as<IRInOutType>(lhs->getDataType()))
        {
            lhs = builder->emitLoad(lhs);
            lhs = builder->emitDifferentialPairGetPrimal(lhs);
        }
        if (as<IRInOutType>(rhs->getDataType()))
        {
            rhs = builder->emitLoad(rhs);
            rhs = builder->emitDifferentialPairGetPrimal(rhs);
        }

        IRInst* leftGrad;
        IRInst* rightGrad;


        switch (origArith->getOp())
        {
        case kIROp_Add:
            leftGrad = grad;
            rightGrad = grad;
            break;
        case kIROp_Mul:
            leftGrad = builder->emitMul(grad->getDataType(), rhs, grad);
            rightGrad = builder->emitMul(grad->getDataType(), lhs, grad);
            break;
        case kIROp_Sub:
            leftGrad = grad;
            rightGrad = builder->emitNeg(grad->getDataType(), grad);
            break;
        case kIROp_Div:
            leftGrad = builder->emitMul(grad->getDataType(), rhs, grad);
            rightGrad = builder->emitMul(grad->getDataType(), lhs, grad); // TODO 1.0 / Grad
            break;
        default:
            getSink()->diagnose(origArith->sourceLoc,
                Diagnostics::unimplemented,
                "this arithmetic instruction cannot be differentiated");
        }

        lhs = origArith->getOperand(0);
        rhs = origArith->getOperand(1);
        if (auto leftGrads = upperGradients.TryGetValue(lhs))
        {
            leftGrads->add(leftGrad);
        }
        else
        {
            upperGradients.Add(lhs, leftGrad);
        }
        if (auto rightGrads = upperGradients.TryGetValue(rhs))
        {
            rightGrads->add(rightGrad);
        }
        else
        {
            upperGradients.Add(rhs, rightGrad);
        }

        return nullptr;
    }

    InstPair copyInst(IRBuilder* builder, IRInst* origInst)
    {
        // Handle common SSA-style operations
        switch (origInst->getOp())
        {
        case kIROp_Param:
            return transcribeParam(builder, as<IRParam>(origInst));

        case kIROp_Return:
            return InstPair(nullptr, nullptr);

        case kIROp_Add:
        case kIROp_Mul:
        case kIROp_Sub:
        case kIROp_Div:
            return copyBinaryArith(builder, origInst);

        default:
            // Not yet implemented
            SLANG_ASSERT(0);
        }

        return InstPair(nullptr, nullptr);
    }

    IRInst* transcribeParamBackward(IRBuilder* builder, IRInst* param, IRInst* grad)
    {
        IRInOutType* inoutParam = as<IRInOutType>(param->getDataType());
        auto pairType = as<IRDifferentialPairType>(inoutParam->getValueType());
        auto paramValue = builder->emitLoad(param);
        auto primal = builder->emitDifferentialPairGetPrimal(paramValue);
        auto diff = builder->emitDifferentialPairGetDifferential(
            (IRType*)pairBuilder->getDiffTypeFromPairType(builder, pairType),
            paramValue
        );
        auto newDiff = builder->emitAdd(grad->getDataType(), diff, grad);
        auto updatedParam = builder->emitMakeDifferentialPair(pairType, primal, newDiff);
        auto store = builder->emitStore(param, updatedParam);

        return store;
    }

    IRInst* transcribeInstBackward(IRBuilder* builder, IRInst* origInst, IRInst* grad)
    {
        // Handle common SSA-style operations
        switch (origInst->getOp())
        {
        case kIROp_Param:
            return transcribeParamBackward(builder, as<IRParam>(origInst), grad);

        case kIROp_Add:
        case kIROp_Mul:
        case kIROp_Sub:
        case kIROp_Div:
            return transcribeBinaryArithBackward(builder, origInst, grad);

        case kIROp_DifferentialPairGetPrimal:
        {
            if (auto param = primalToDiffPair.TryGetValue(origInst))
            {
                if (auto leftGrads = upperGradients.TryGetValue(*param))
                {
                    leftGrads->add(grad);
                }
                else
                {
                    upperGradients.Add(*param, grad);
                }
            }
            else
                SLANG_ASSERT(0);
            return nullptr;
        }

        default:
            // Not yet implemented
            SLANG_ASSERT(0);
        }

        return nullptr;
    }


};

struct ReverseDerivativePass : public InstPassBase
{
    DiagnosticSink* getSink()
    {
        return sink;
    }

    bool processModule()
    {

        IRBuilder builderStorage(autodiffContext->sharedBuilder);
        IRBuilder* builder = &builderStorage;

        // Process all ForwardDifferentiate instructions (kIROp_ForwardDifferentiate), by  
        // generating derivative code for the referenced function.
        //
        bool modified = processReferencedFunctions(builder);

        return modified;
    }

    IRInst* lookupJVPReference(IRInst* primalFunction)
    {
        if (auto jvpDefinition = primalFunction->findDecoration<IRForwardDerivativeDecoration>())
            return jvpDefinition->getForwardDerivativeFunc();
        return nullptr;
    }

    // Recursively process instructions looking for JVP calls (kIROp_ForwardDifferentiate),
    // then check that the referenced function is marked correctly for differentiation.
    //
    bool processReferencedFunctions(IRBuilder* builder)
    {
        List<IRInst*> autoDiffWorkList;

        for (;;)
        {
            // Collect all `ForwardDifferentiate` insts from the module.
            autoDiffWorkList.clear();
            processAllInsts([&](IRInst* inst)
                {
                    switch (inst->getOp())
                    {
                    case kIROp_BackwardDifferentiate:
                        // Only process now if the operand is a materialized function.
                        switch (inst->getOperand(0)->getOp())
                        {
                        case kIROp_Func:
                        case kIROp_Specialize:
                            autoDiffWorkList.add(inst);
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                });

            if (autoDiffWorkList.getCount() == 0)
                break;

            // Process collected `ForwardDifferentiate` insts and replace them with placeholders for
            // differentiated functions.

            backwardTranscriberStorage.followUpFunctionsToTranscribe.clear();

            for (auto differentiateInst : autoDiffWorkList)
            {
                IRInst* baseInst = differentiateInst->getOperand(0);
                if (as<IRBackwardDifferentiate>(differentiateInst))
                {
                    if (isMarkedForBackwardDifferentiation(baseInst))
                    {
                        if (as<IRFunc>(baseInst))
                        {
                            IRInst* diffFunc =
                                backwardTranscriberStorage
                                    .transcribeFuncHeader(builder, (IRFunc*)baseInst)
                                    .differential;
                            SLANG_ASSERT(diffFunc);
                            differentiateInst->replaceUsesWith(diffFunc);
                            differentiateInst->removeAndDeallocate();
                        }
                        else
                        {
                            getSink()->diagnose(differentiateInst->sourceLoc,
                                Diagnostics::internalCompilerError,
                                "Unexpected instruction. Expected func or generic");
                        }
                    }
                }
            }
            
            auto followUpWorkList = _Move(backwardTranscriberStorage.followUpFunctionsToTranscribe);
            for (auto task : followUpWorkList)
            {
                auto diffFunc = as<IRFunc>(task.differential);
                SLANG_ASSERT(diffFunc);
                auto primalFunc = as<IRFunc>(task.primal);
                SLANG_ASSERT(primalFunc);

                backwardTranscriberStorage.transcribeFunc(builder, primalFunc, diffFunc);
            }

            // Transcribing the function body really shouldn't produce more follow up function body work.
            // However it may produce new `ForwardDifferentiate` instructions, which we collect and process
            // in the next iteration.
            SLANG_RELEASE_ASSERT(backwardTranscriberStorage.followUpFunctionsToTranscribe.getCount() == 0);

        }
        return true;
    }

    // Checks decorators to see if the function should
    // be differentiated (kIROp_ForwardDifferentiableDecoration)
    // 
    bool isMarkedForBackwardDifferentiation(IRInst* callable)
    {
        return callable->findDecoration<IRBackwardDifferentiableDecoration>() != nullptr;
    }

    IRStringLit* getBackwardDerivativeFuncName(IRInst* func)
    {
        IRBuilder builder(&sharedBuilderStorage);
        builder.setInsertBefore(func);

        IRStringLit* name = nullptr;
        if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
        {
            name = builder.getStringValue((String(linkageDecoration->getMangledName()) + "_bwd_diff").getUnownedSlice());
        }
        else if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
        {
            name = builder.getStringValue((String(namehintDecoration->getName()) + "_bwd_diff").getUnownedSlice());
        }

        return name;
    }

    ReverseDerivativePass(AutoDiffSharedContext* context, DiagnosticSink* sink) :
        InstPassBase(context->moduleInst->getModule()),
        sink(sink),
        backwardTranscriberStorage(context, context->sharedBuilder, sink),
        autodiffContext(context),
        pairBuilderStorage(context)
    {
        backwardTranscriberStorage.pairBuilder = &pairBuilderStorage;
        backwardTranscriberStorage.fwdDiffTranscriberStorage.sink = sink;
        backwardTranscriberStorage.fwdDiffTranscriberStorage.autoDiffSharedContext = context;
        backwardTranscriberStorage.fwdDiffTranscriberStorage.pairBuilder = &(pairBuilderStorage);
    }

protected:
    // A transcriber object that handles the main job of 
    // processing instructions while maintaining state.
    //
    BackwardDiffTranscriber         backwardTranscriberStorage;

    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink*                 sink;

    // Builder for dealing with differential pair types.
    DifferentialPairTypeBuilder     pairBuilderStorage;

    // Autodiff Shared Context
    AutoDiffSharedContext*          autodiffContext;
};

bool processReverseDerivativeCalls(
    AutoDiffSharedContext*                  autodiffContext,
    DiagnosticSink*                         sink,
    IRReverseDerivativePassOptions const&)
{
    ReverseDerivativePass revPass(autodiffContext, sink);
    bool changed = revPass.processModule();
    return changed;
}

}
