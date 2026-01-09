// slang-ir-autodiff-fwd.cpp
#include "slang-ir-autodiff-fwd.h"

#include "slang-ir-addr-inst-elimination.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-init-local-var.h"
#include "slang-ir-inline.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-propagate-func-properties.h"
#include "slang-ir-single-return.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"
#include "slang-ir-validate.h"

namespace Slang
{

IRFuncType* ForwardDiffTranscriber::differentiateFunctionType(
    IRBuilder* builder,
    IRInst* func,
    IRFuncType* funcType)
{
    SLANG_UNUSED(func);

    List<IRType*> newParameterTypes;
    IRType* diffReturnType;

    for (UIndex i = 0; i < funcType->getParamCount(); i++)
    {
        auto origType = funcType->getParamType(i);
        origType = (IRType*)findOrTranscribePrimalInst(builder, origType);
        if (auto diffPairType = tryGetDiffPairType(builder, origType))
            newParameterTypes.add(diffPairType);
        else
            newParameterTypes.add(origType);
    }

    // Transcribe return type to a pair.
    // This will be void if the primal return type is non-differentiable.
    //
    auto origResultType = (IRType*)findOrTranscribePrimalInst(builder, funcType->getResultType());
    if (auto returnPairType = tryGetDiffPairType(builder, origResultType))
        diffReturnType = returnPairType;
    else
        diffReturnType = origResultType;

    return builder->getFuncType(newParameterTypes, diffReturnType);
}

void ForwardDiffTranscriber::generateTrivialFwdDiffFunc(IRFunc* primalFunc, IRFunc* diffFunc)
{
    IRBuilder builder(diffFunc);
    builder.setInsertInto(diffFunc);
    auto block = builder.emitBlock();
    builder.markInstAsMixedDifferential(block);

    for (auto param : primalFunc->getParams())
    {
        transcribeFuncParam(&builder, param, param->getFullType());
    }
    List<IRParam*> diffParams;
    for (auto param : diffFunc->getParams())
    {
        diffParams.add(param);
    }
    auto emitDiffPairVal = [&](IRDifferentialPairTypeBase* pairType)
    {
        auto primal = builder.emitDefaultConstruct(pairType->getValueType());
        builder.markInstAsPrimal(primal);
        auto diff = getDifferentialZeroOfType(&builder, pairType->getValueType());
        builder.markInstAsDifferential(diff, primal->getDataType());

        auto val = builder.emitMakeDifferentialPair(pairType, primal, diff);
        builder.markInstAsMixedDifferential(val);

        return val;
    };
    for (auto param : diffParams)
    {
        if (auto outType = as<IROutParamTypeBase>(param->getFullType()))
        {
            if (isRelevantDifferentialPair(outType))
            {
                auto pairType = as<IRDifferentialPairTypeBase>(outType->getValueType());
                auto val = emitDiffPairVal(pairType);
                auto store = builder.emitStore(param, val);
                builder.markInstAsMixedDifferential(store);
            }
            else
            {
                auto val = builder.emitDefaultConstruct(outType->getValueType());
                builder.markInstAsPrimal(val);

                auto store = builder.emitStore(param, val);
                builder.markInstAsPrimal(store);
            }
        }
    }
    if (isRelevantDifferentialPair(diffFunc->getResultType()))
    {
        auto pairType = as<IRDifferentialPairTypeBase>(diffFunc->getResultType());
        auto val = emitDiffPairVal(pairType);
        auto returnInst = builder.emitReturn(val);
        builder.markInstAsMixedDifferential(val);
        builder.markInstAsMixedDifferential(returnInst);
    }
    else
    {
        auto retVal = builder.emitDefaultConstruct(diffFunc->getResultType());
        auto returnInst = builder.emitReturn(retVal);
        builder.markInstAsPrimal(retVal);
        builder.markInstAsPrimal(returnInst);
    }
}

// Returns "d<var-name>" to use as a name hint for variables and parameters.
// If no primal name is available, returns a blank string.
//
String ForwardDiffTranscriber::getDiffVarName(IRInst* origVar)
{
    if (auto namehintDecoration = origVar->findDecoration<IRNameHintDecoration>())
    {
        return ("d" + String(namehintDecoration->getName()));
    }

    return String("");
}

InstPair ForwardDiffTranscriber::transcribeUndefined(IRBuilder* builder, IRInst* origInst)
{
    auto primalVal = maybeCloneForPrimalInst(builder, origInst);

    if (IRType* const diffType = differentiateType(builder, origInst->getFullType()))
    {
        auto dzero = getDifferentialZeroOfType(builder, origInst->getFullType());
        if (dzero)
        {
            return InstPair(primalVal, dzero);
        }
    }
    return InstPair(primalVal, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeReinterpret(IRBuilder* builder, IRInst* origInst)
{
    auto primalVal = maybeCloneForPrimalInst(builder, origInst);

    IRInst* diffVal = nullptr;

    if (IRType* const diffType = differentiateType(builder, origInst->getFullType()))
    {
        if (auto diffOperand = findOrTranscribeDiffInst(builder, origInst->getOperand(0)))
        {
            diffVal = builder->emitReinterpret(diffType, diffOperand);
        }
    }

    return InstPair(primalVal, diffVal);
}


InstPair ForwardDiffTranscriber::transcribeAssociatedInstAnnotation(
    IRBuilder* builder,
    IRInst* origInst)
{
    auto primalAnnotation =
        as<IRAssociatedInstAnnotation>(maybeCloneForPrimalInst(builder, origInst));

    IRAssociatedInstAnnotation* annotation = as<IRAssociatedInstAnnotation>(origInst);

    builder->markInstAsPrimal(primalAnnotation);

    return InstPair(primalAnnotation, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeVar(IRBuilder* builder, IRVar* origVar)
{
    if (IRType* diffType = differentiateType(builder, origVar->getDataType()->getValueType()))
    {
        IRVar* diffVar = builder->emitVar(diffType);
        SLANG_ASSERT(diffVar);

        auto diffNameHint = getDiffVarName(origVar);
        if (diffNameHint.getLength() > 0)
            builder->addNameHintDecoration(diffVar, diffNameHint.getUnownedSlice());

        return InstPair(maybeCloneForPrimalInst(builder, origVar), diffVar);
    }
    return InstPair(maybeCloneForPrimalInst(builder, origVar), nullptr);
}

InstPair ForwardDiffTranscriber::transcribeBinaryArith(IRBuilder* builder, IRInst* origArith)
{
    SLANG_ASSERT(origArith->getOperandCount() == 2);

    IRInst* primalArith = maybeCloneForPrimalInst(builder, origArith);

    auto origLeft = origArith->getOperand(0);
    auto origRight = origArith->getOperand(1);

    auto primalLeft = findOrTranscribePrimalInst(builder, origLeft);
    auto primalRight = findOrTranscribePrimalInst(builder, origRight);

    auto diffLeft = findOrTranscribeDiffInst(builder, origLeft);
    auto diffRight = findOrTranscribeDiffInst(builder, origRight);


    if (diffLeft || diffRight)
    {
        diffLeft =
            diffLeft ? diffLeft : getDifferentialZeroOfType(builder, primalLeft->getDataType());

        bool diffRightIsZero = (diffRight == nullptr);
        diffRight =
            diffRight ? diffRight : getDifferentialZeroOfType(builder, primalRight->getDataType());
        diffRightIsZero = diffRightIsZero || isZero(diffRight);

        auto resultType = primalArith->getDataType();
        auto origResultType = origArith->getDataType();
        auto diffType = (IRType*)differentiateType(builder, origResultType);

        switch (origArith->getOp())
        {
        case kIROp_Add:
            {
                auto diffAdd = builder->emitAdd(diffType, diffLeft, diffRight);
                builder->markInstAsDifferential(diffAdd, resultType);

                return InstPair(primalArith, diffAdd);
            }

        case kIROp_Mul:
            {
                auto diffLeftTimesRight = builder->emitMul(diffType, diffLeft, primalRight);
                auto diffRightTimesLeft = builder->emitMul(diffType, diffRight, primalLeft);
                builder->markInstAsDifferential(diffLeftTimesRight, resultType);
                builder->markInstAsDifferential(diffRightTimesLeft, resultType);

                auto diffAdd = builder->emitAdd(diffType, diffLeftTimesRight, diffRightTimesLeft);
                builder->markInstAsDifferential(diffAdd, resultType);

                return InstPair(primalArith, diffAdd);
            }

        case kIROp_Sub:
            {
                auto diffSub = builder->emitSub(diffType, diffLeft, diffRight);
                builder->markInstAsDifferential(diffSub, resultType);

                return InstPair(primalArith, diffSub);
            }
        case kIROp_Div:
            {
                if (diffRightIsZero)
                {
                    // Special case the dRight = 0 case here since it would be difficult
                    // to optimize out in the future.
                    IRInst* diff = nullptr;
                    if (auto constant = as<IRFloatLit>(primalRight))
                    {
                        diff = builder->emitMul(
                            diffType,
                            diffLeft,
                            builder->getFloatValue(
                                constant->getDataType(),
                                1.0 / constant->getValue()));
                        builder->markInstAsDifferential(diff, resultType);
                    }
                    else
                    {
                        diff = builder->emitDiv(diffType, diffLeft, primalRight);
                        builder->markInstAsDifferential(diff, resultType);
                    }
                    return InstPair(primalArith, diff);
                }
                else
                {
                    auto diffLeftTimesRight = builder->emitMul(diffType, diffLeft, primalRight);
                    builder->markInstAsDifferential(diffLeftTimesRight, resultType);

                    auto diffRightTimesLeft = builder->emitMul(diffType, primalLeft, diffRight);
                    builder->markInstAsDifferential(diffRightTimesLeft, resultType);

                    auto diffSub =
                        builder->emitSub(diffType, diffLeftTimesRight, diffRightTimesLeft);
                    builder->markInstAsDifferential(diffSub, resultType);

                    auto diffMul =
                        builder->emitMul(primalRight->getFullType(), primalRight, primalRight);
                    builder->markInstAsPrimal(diffMul);

                    auto diffDiv = builder->emitDiv(diffType, diffSub, diffMul);
                    builder->markInstAsDifferential(diffDiv, resultType);

                    return InstPair(primalArith, diffDiv);
                }
            }
        default:
            getSink()->diagnose(
                origArith->sourceLoc,
                Diagnostics::unimplemented,
                "this arithmetic instruction cannot be differentiated");
        }
    }

    return InstPair(primalArith, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeBinaryLogic(IRBuilder* builder, IRInst* origLogic)
{
    SLANG_ASSERT(origLogic->getOperandCount() == 2);

    // Boolean operations are not differentiable. For the linearization
    // pass, we do not need to do anything but copy them over to the ne
    // function.
    auto primalLogic = maybeCloneForPrimalInst(builder, origLogic);
    return InstPair(primalLogic, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeSelect(IRBuilder* builder, IRInst* origSelect)
{
    auto primalCondition = lookupPrimalInst(builder, origSelect->getOperand(0));

    auto origLeft = origSelect->getOperand(1);
    auto origRight = origSelect->getOperand(2);

    auto primalLeft = findOrTranscribePrimalInst(builder, origLeft);
    auto primalRight = findOrTranscribePrimalInst(builder, origRight);

    auto diffLeft = findOrTranscribeDiffInst(builder, origLeft);
    auto diffRight = findOrTranscribeDiffInst(builder, origRight);

    auto primalSelect = maybeCloneForPrimalInst(builder, origSelect);

    // If both sides have no differential, skip
    if (diffLeft || diffRight)
    {
        diffLeft =
            diffLeft ? diffLeft : getDifferentialZeroOfType(builder, primalLeft->getDataType());
        diffRight =
            diffRight ? diffRight : getDifferentialZeroOfType(builder, primalRight->getDataType());

        auto diffType = differentiateType(builder, origSelect->getDataType());

        return InstPair(
            primalSelect,
            builder->emitIntrinsicInst(
                diffType,
                kIROp_Select,
                3,
                List<IRInst*>(primalCondition, diffLeft, diffRight).getBuffer()));
    }

    return InstPair(primalSelect, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeLoad(IRBuilder* builder, IRLoad* origLoad)
{
    auto origPtr = origLoad->getPtr();
    auto primalPtr = lookupPrimalInst(builder, origPtr, nullptr);

    if (auto primalPtrType = as<IRPtrTypeBase>(primalPtr->getFullType()))
    {
        if (auto diffPairType = as<IRDifferentialPairType>(primalPtrType->getValueType()))
        {
            // Special case load from an `out` param, which will not have corresponding `diff` and
            // `primal` insts yet.

            // TODO: Could we move this load to _after_ DifferentialPairGetPrimal,
            // and DifferentialPairGetDifferential?
            //
            auto load = builder->emitLoad(primalPtr);
            builder->markInstAsMixedDifferential(load, diffPairType);

            auto primalElement = builder->emitDifferentialPairGetPrimal(load);
            auto diffElement = builder->emitDifferentialPairGetDifferential(
                (IRType*)differentiableTypeConformanceContext.getDiffTypeFromPairType(
                    builder,
                    diffPairType),
                load);
            return InstPair(primalElement, diffElement);
        }
        else if (
            auto diffPtrPairType = as<IRDifferentialPtrPairType>(primalPtrType->getValueType()))
        {
            auto load = builder->emitLoad(primalPtr);
            builder->markInstAsPrimal(load);

            auto primalElement = builder->emitDifferentialPtrPairGetPrimal(load);
            auto diffElement = builder->emitDifferentialPtrPairGetDifferential(
                (IRType*)differentiableTypeConformanceContext.getDiffTypeFromPairType(
                    builder,
                    diffPtrPairType),
                load);
            builder->markInstAsPrimal(primalElement);
            builder->markInstAsPrimal(diffElement);
            return InstPair(primalElement, diffElement);
        }
    }

    auto primalLoad = maybeCloneForPrimalInst(builder, origLoad);
    IRInst* diffLoad = nullptr;
    if (auto diffPtr = lookupDiffInst(origPtr, nullptr))
    {
        // Default case, we're loading from a known differential inst.
        diffLoad = as<IRLoad>(builder->emitLoad(diffPtr));
    }
    return InstPair(primalLoad, diffLoad);
}

InstPair ForwardDiffTranscriber::transcribeStore(IRBuilder* builder, IRStore* origStore)
{
    IRInst* origStoreLocation = origStore->getPtr();
    IRInst* origStoreVal = origStore->getVal();
    auto primalStoreLocation = lookupPrimalInst(builder, origStoreLocation, nullptr);
    auto diffStoreLocation = lookupDiffInst(origStoreLocation, nullptr);
    auto primalStoreVal = lookupPrimalInst(builder, origStoreVal, nullptr);
    auto diffStoreVal = lookupDiffInst(origStoreVal, nullptr);

    if (!diffStoreLocation)
    {
        auto primalLocationPtrType = as<IRPtrTypeBase>(primalStoreLocation->getDataType());
        if (auto diffPairType = as<IRDifferentialPairType>(primalLocationPtrType->getValueType()))
        {
            auto valToStore =
                builder->emitMakeDifferentialPair(diffPairType, primalStoreVal, diffStoreVal);
            builder->markInstAsMixedDifferential(diffStoreVal, diffPairType);

            auto store = builder->emitStore(primalStoreLocation, valToStore);
            builder->markInstAsMixedDifferential(store, diffPairType);

            return InstPair(store, nullptr);
        }
        else if (
            auto diffRefPairType =
                as<IRDifferentialPtrPairType>(primalLocationPtrType->getValueType()))
        {
            auto valToStore =
                builder->emitMakeDifferentialPtrPair(diffRefPairType, primalStoreVal, diffStoreVal);
            builder->markInstAsPrimal(valToStore);

            auto store = builder->emitStore(primalStoreLocation, valToStore);
            builder->markInstAsPrimal(store);

            return InstPair(store, nullptr);
        }
    }

    auto primalStore = maybeCloneForPrimalInst(builder, origStore);

    IRInst* diffStore = nullptr;

    // If the stored value has a differential version,
    // emit a store instruction for the differential parameter.
    // Otherwise, emit nothing since there's nothing to load.
    //
    if (diffStoreLocation && diffStoreVal)
    {
        // Default case, storing the entire type (and not a member)
        diffStore = as<IRStore>(builder->emitStore(diffStoreLocation, diffStoreVal));
        markDiffTypeInst(builder, diffStore, primalStoreVal->getDataType());
        return InstPair(primalStore, diffStore);
    }

    return InstPair(primalStore, nullptr);
}

// Since int/float literals are sometimes nested inside an IRConstructor
// instruction, we check to make sure that the nested instr is a constant
// and then return nullptr. Literals do not need to be differentiated.
//
InstPair ForwardDiffTranscriber::transcribeConstruct(IRBuilder* builder, IRInst* origConstruct)
{
    IRInst* primalConstruct = maybeCloneForPrimalInst(builder, origConstruct);

    // Check if the output type can be differentiated. If it cannot be
    // differentiated, don't differentiate the inst
    //
    auto primalConstructType =
        (IRType*)findOrTranscribePrimalInst(builder, origConstruct->getDataType());
    // TODO: Need to update this to generate derivatives on a per-key basis
    if (auto diffConstructType = differentiateType(builder, primalConstructType))
    {
        UCount operandCount = origConstruct->getOperandCount();

        List<IRInst*> diffOperands;
        for (UIndex ii = 0; ii < operandCount; ii++)
        {
            // If the operand has a differential version, replace the original with
            // the differential. Otherwise, use a zero.
            //
            if (auto diffInst = lookupDiffInst(origConstruct->getOperand(ii), nullptr))
                diffOperands.add(diffInst);
            else
            {
                auto operandDataType = origConstruct->getOperand(ii)->getDataType();
                if (const auto diffOperandType = differentiateType(builder, operandDataType))
                {
                    operandDataType = (IRType*)findOrTranscribePrimalInst(builder, operandDataType);
                    diffOperands.add(getDifferentialZeroOfType(builder, operandDataType));
                }
                else
                {
                    diffOperands.add(builder->getVoidValue());
                }
            }
        }

        return InstPair(
            primalConstruct,
            builder->emitIntrinsicInst(
                diffConstructType,
                origConstruct->getOp(),
                diffOperands.getCount(),
                diffOperands.getBuffer()));
    }
    else
    {
        return InstPair(primalConstruct, nullptr);
    }
}

InstPair ForwardDiffTranscriber::transcribeMakeStruct(IRBuilder* builder, IRInst* origMakeStruct)
{
    IRInst* primalMakeStruct = maybeCloneForPrimalInst(builder, origMakeStruct);

    // Check if the output type can be differentiated. If it cannot be
    // differentiated, don't differentiate the inst
    //
    auto primalStructType =
        (IRType*)findOrTranscribePrimalInst(builder, origMakeStruct->getDataType());
    if (auto diffStructType = differentiateType(builder, primalStructType))
    {
        auto primalStruct = as<IRStructType>(getResolvedInstForDecorations(primalStructType));
        SLANG_RELEASE_ASSERT(primalStruct);

        List<IRInst*> diffOperands;
        UIndex ii = 0;
        for (auto field : primalStruct->getFields())
        {
            SLANG_RELEASE_ASSERT(ii < origMakeStruct->getOperandCount());

            // If this field is not differentiable, skip the operand.
            if (!field->getKey()->findDecoration<IRDerivativeMemberDecoration>())
            {
                ii++;
                continue;
            }

            // If the operand has a differential version, replace the original with
            // the differential. Otherwise, use a zero.
            //
            if (auto diffInst = lookupDiffInst(origMakeStruct->getOperand(ii), nullptr))
            {
                diffOperands.add(diffInst);
            }
            else
            {
                auto operandDataType = origMakeStruct->getOperand(ii)->getDataType();
                auto diffOperandType = differentiateType(builder, operandDataType);

                if (diffOperandType)
                {
                    operandDataType = (IRType*)findOrTranscribePrimalInst(builder, operandDataType);
                    diffOperands.add(getDifferentialZeroOfType(builder, operandDataType));
                }
                else
                {
                    // This case is only hit if the field is of a differentiable type but the
                    // operand is of a non-differentiable type. This can happen if the operand is
                    // wrapped in no_diff. In this case, we use the derivative of the field type to
                    // synthesize the 0.
                    //
                    auto diffFieldOperandType = differentiateType(builder, field->getFieldType());
                    SLANG_RELEASE_ASSERT(diffFieldOperandType);
                    diffOperands.add(
                        getDifferentialZeroOfType(builder, (IRType*)diffFieldOperandType));
                }
            }
            ii++;
        }

        return InstPair(
            primalMakeStruct,
            builder->emitIntrinsicInst(
                diffStructType,
                kIROp_MakeStruct,
                diffOperands.getCount(),
                diffOperands.getBuffer()));
    }
    else
    {
        return InstPair(primalMakeStruct, nullptr);
    }
}

static bool _isDifferentiableFunc(IRInst* func)
{
    func = getResolvedInstForDecorations(func);
    for (auto decor = func->getFirstDecoration(); decor; decor = decor->getNextDecoration())
    {
        switch (decor->getOp())
        {
        case kIROp_ForwardDerivativeDecoration:
        case kIROp_ForwardDifferentiableDecoration:
        case kIROp_BackwardDerivativeDecoration:
        case kIROp_BackwardDifferentiableDecoration:
        case kIROp_UserDefinedBackwardDerivativeDecoration:
            return true;
        }
    }
    return false;
}

static IRFuncType* _getCalleeActualFuncType(
    DifferentiableTypeConformanceContext* context,
    IRInst* callee)
{
    IRBuilder builder(callee);
    auto type = context->resolveType(&builder, callee->getFullType());
    if (auto funcType = as<IRFuncType>(type))
        return funcType;
    if (auto specialize = as<IRSpecialize>(callee))
        return as<IRFuncType>(
            findGenericReturnVal(as<IRGeneric>(specialize->getBase()))->getFullType());
    return nullptr;
}

IRInst* tryFindPrimalSubstitute(IRBuilder* builder, IRInst* callee)
{
    if (auto func = as<IRFunc>(callee))
    {
        if (auto decor = func->findDecoration<IRPrimalSubstituteDecoration>())
            return decor->getPrimalSubstituteFunc();
    }
    else if (auto specialize = as<IRSpecialize>(callee))
    {
        auto innerGen = as<IRGeneric>(specialize->getBase());
        if (!innerGen)
            return callee;
        auto innerFunc = findGenericReturnVal(innerGen);
        if (auto decor = innerFunc->findDecoration<IRPrimalSubstituteDecoration>())
        {
            auto substSpecialize = as<IRSpecialize>(decor->getPrimalSubstituteFunc());
            SLANG_RELEASE_ASSERT(substSpecialize);
            SLANG_RELEASE_ASSERT(substSpecialize->getArgCount() == specialize->getArgCount());
            List<IRInst*> args;
            for (UInt i = 0; i < specialize->getArgCount(); i++)
                args.add(specialize->getArg(i));
            return builder->emitSpecializeInst(
                callee->getFullType(),
                substSpecialize->getBase(),
                (UInt)args.getCount(),
                args.getBuffer());
        }
    }
    return callee;
}

static void emitCalleeAnnotationsForHigherOrderDiff(
    IRBuilder* builder,
    DifferentiableTypeConformanceContext* context,
    IRInst* primalCallee)
{
    auto fwdDiffCallee =
        context->tryGetAssociationOfKind(primalCallee, ValAssociationKind::ForwardDerivative);

    // Pull up `primal : IForwardDifferentiable`
    if (auto fwdDiffTable = context->tryGetAssociationOfKind(
            primalCallee,
            ValAssociationKind::ForwardDerivativeWitnessTable))
    {
        IRInterfaceType* fwdDiffInterfaceType = cast<IRInterfaceType>(
            getGenericReturnVal(context->sharedContext->forwardDifferentiableInterfaceType));

        IRInterfaceType* bwdDiffInterfaceType = cast<IRInterfaceType>(
            getGenericReturnVal(context->sharedContext->backwardDifferentiableInterfaceType));

        IRInterfaceType* bwdCallableInterfaceType = cast<IRInterfaceType>(
            getGenericReturnVal(context->sharedContext->backwardCallableInterfaceType));

        // Key for `primal.fwd_diff`
        auto fwdDiffReqKey = cast<IRInterfaceRequirementEntry>(fwdDiffInterfaceType->getOperand(0))
                                 ->getRequirementKey();
        // Key for `primal.fwd_diff : IForwardDifferentiable`
        auto fwdDiffTableReqKey =
            cast<IRInterfaceRequirementEntry>(fwdDiffInterfaceType->getOperand(1))
                ->getRequirementKey();

        // Key for `primal.fwd_diff : IBackwardDifferentiable`
        auto bwdDiffTableReqKey =
            cast<IRInterfaceRequirementEntry>(fwdDiffInterfaceType->getOperand(2))
                ->getRequirementKey();

        // Key for contextType in IBackwardDifferentiable table
        auto bwdDiffContextTypeReqKey =
            cast<IRInterfaceRequirementEntry>(bwdDiffInterfaceType->getOperand(0))
                ->getRequirementKey();

        // Key for contextType : IBackwardCallable in IBackwardDifferentiable table
        auto bwdDiffContextTypeCallableConformanceReqKey =
            cast<IRInterfaceRequirementEntry>(bwdDiffInterfaceType->getOperand(1))
                ->getRequirementKey();

        // Key for `apply` in IBackwardDifferentiable table
        auto bwdDiffApplyReqKey =
            cast<IRInterfaceRequirementEntry>(bwdDiffInterfaceType->getOperand(2))
                ->getRequirementKey();

        // Key for 'operator()' (back-prop) in IBackwardCallable table
        auto bwdCallableOpReqKey =
            cast<IRInterfaceRequirementEntry>(bwdCallableInterfaceType->getOperand(0))
                ->getRequirementKey();

        // Key for 'getVal()' in IBackwardCallable table
        auto bwdCallableGetValReqKey =
            cast<IRInterfaceRequirementEntry>(bwdCallableInterfaceType->getOperand(1))
                ->getRequirementKey();

        IRInst* callee = primalCallee;

        // Lookup primal.fwd_diff : IForwardDifferentiable
        auto higherOrderFwdDiffTable = _lookupWitness(
            builder,
            fwdDiffTable,
            fwdDiffTableReqKey,
            builder->getWitnessTableType((IRType*)builder->emitSpecializeInst(
                builder->getTypeKind(),
                context->sharedContext->forwardDifferentiableInterfaceType,
                1,
                &fwdDiffCallee)));

        // Lookup `primal.fwd_diff.fwd_diff`
        IRInst* operand = fwdDiffCallee->getFullType();
        auto higherOrderFwdDiffFunc = _lookupWitness(
            builder,
            higherOrderFwdDiffTable,
            fwdDiffReqKey,
            (IRType*)builder->emitIntrinsicInst(
                builder->getTypeKind(),
                kIROp_ForwardDiffFuncType,
                1,
                &operand));

        {
            IRInst* args[] = {
                fwdDiffCallee,
                builder->getIntValue((int)ValAssociationKind::ForwardDerivative),
                higherOrderFwdDiffFunc};
            builder->emitIntrinsicInst(
                builder->getVoidType(),
                kIROp_AssociatedInstAnnotation,
                3,
                args);
            context->associationCache.remove(
                {fwdDiffCallee, ValAssociationKind::ForwardDerivative});
        }

        {
            IRInst* args[] = {
                fwdDiffCallee,
                builder->getIntValue((int)ValAssociationKind::ForwardDerivativeWitnessTable),
                higherOrderFwdDiffTable};
            builder->emitIntrinsicInst(
                builder->getVoidType(),
                kIROp_AssociatedInstAnnotation,
                3,
                args);
            context->associationCache.remove(
                {fwdDiffCallee, ValAssociationKind::ForwardDerivativeWitnessTable});
        }

        // Lookup `primal.fwd_diff : IBackwardDifferentiable`
        auto higherOrderBwdDiffTable = _lookupWitness(
            builder,
            fwdDiffTable,
            bwdDiffTableReqKey,
            builder->getWitnessTableType((IRType*)builder->emitSpecializeInst(
                builder->getTypeKind(),
                context->sharedContext->backwardDifferentiableInterfaceType,
                1,
                &fwdDiffCallee)));

        {
            IRInst* args[] = {
                fwdDiffCallee,
                builder->getIntValue((int)ValAssociationKind::BackwardDerivativeWitnessTable),
                higherOrderBwdDiffTable};
            builder->emitIntrinsicInst(
                builder->getVoidType(),
                kIROp_AssociatedInstAnnotation,
                3,
                args);
            context->associationCache.remove(
                {fwdDiffCallee, ValAssociationKind::BackwardDerivativeWitnessTable});
        }

        // Lookup contextType in IBackwardDifferentiable table
        auto higherOrderContextType = _lookupWitness(
            builder,
            higherOrderBwdDiffTable,
            bwdDiffContextTypeReqKey,
            (IRType*)builder->getTypeKind());

        {
            IRInst* args[] = {
                fwdDiffCallee,
                builder->getIntValue((int)ValAssociationKind::BackwardDerivativeContext),
                higherOrderContextType};
            builder->emitIntrinsicInst(
                builder->getVoidType(),
                kIROp_AssociatedInstAnnotation,
                3,
                args);
            context->associationCache.remove(
                {fwdDiffCallee, ValAssociationKind::BackwardDerivativeContext});
        }

        // Lookup `apply` in IBackwardDifferentiable table
        IRInst* bwdApplyFuncTypeOperands[] = {fwdDiffCallee->getFullType(), higherOrderContextType};
        auto higherOrderBwdDiffFunc = _lookupWitness(
            builder,
            higherOrderBwdDiffTable,
            bwdDiffApplyReqKey,
            (IRType*)builder->emitIntrinsicInst(
                builder->getTypeKind(),
                kIROp_ApplyForBwdFuncType,
                2,
                bwdApplyFuncTypeOperands));

        {
            IRInst* args[] = {
                fwdDiffCallee,
                builder->getIntValue((int)ValAssociationKind::BackwardDerivativeApply),
                higherOrderBwdDiffFunc};
            builder->emitIntrinsicInst(
                builder->getVoidType(),
                kIROp_AssociatedInstAnnotation,
                3,
                args);
            context->associationCache.remove(
                {fwdDiffCallee, ValAssociationKind::BackwardDerivativeApply});
        }

        // Lookup contextType : IBackwardCallable in IBackwardDifferentiable table
        auto higherOrderContextCallableTable = _lookupWitness(
            builder,
            higherOrderBwdDiffTable,
            bwdDiffContextTypeCallableConformanceReqKey,
            builder->getWitnessTableType((IRType*)builder->emitSpecializeInst(
                builder->getTypeKind(),
                context->sharedContext->backwardCallableInterfaceType,
                1,
                &fwdDiffCallee)));

        // Lookup 'operator()' (back-prop) in IBackwardCallable table
        IRInst* bwdCallableFuncTypeOperands[] = {
            fwdDiffCallee->getFullType(),
            higherOrderContextType};
        auto higherOrderBwdPropFunc = _lookupWitness(
            builder,
            higherOrderContextCallableTable,
            bwdCallableOpReqKey,
            (IRType*)builder->emitIntrinsicInst(
                builder->getTypeKind(),
                kIROp_BwdCallableFuncType,
                2,
                bwdCallableFuncTypeOperands));

        {
            IRInst* args[] = {
                fwdDiffCallee,
                builder->getIntValue((int)ValAssociationKind::BackwardDerivativePropagate),
                higherOrderBwdPropFunc};
            builder->emitIntrinsicInst(
                builder->getVoidType(),
                kIROp_AssociatedInstAnnotation,
                3,
                args);
            context->associationCache.remove(
                {fwdDiffCallee, ValAssociationKind::BackwardDerivativePropagate});
        }

        // Lookup 'getVal()' in IBackwardCallable table
        IRInst* funcResultTypeOperands[] = {fwdDiffCallee->getFullType(), higherOrderContextType};
        auto higherOrderGetValFunc = _lookupWitness(
            builder,
            higherOrderContextCallableTable,
            bwdCallableGetValReqKey,
            (IRType*)builder->emitIntrinsicInst(
                builder->getTypeKind(),
                kIROp_FuncResultType,
                2,
                funcResultTypeOperands));

        {
            IRInst* args[] = {
                fwdDiffCallee,
                builder->getIntValue((int)ValAssociationKind::BackwardDerivativeContextGetVal),
                higherOrderGetValFunc};
            builder->emitIntrinsicInst(
                builder->getVoidType(),
                kIROp_AssociatedInstAnnotation,
                3,
                args);
            context->associationCache.remove(
                {fwdDiffCallee, ValAssociationKind::BackwardDerivativeContextGetVal});
        }
    }
}

// Differentiating a call instruction here is primarily about generating
// an appropriate call list based on whichever parameters have differentials
// in the current transcription context.
//
InstPair ForwardDiffTranscriber::transcribeCall(IRBuilder* builder, IRCall* origCall)
{
    IRInst* origCallee = origCall->getCallee();

    if (!origCallee)
    {
        // Note that this can only happen if the callee is a result
        // of a higher-order operation. For now, we assume that we cannot
        // differentiate such calls safely.
        // TODO(sai): Should probably get checked in the front-end.
        //
        getSink()->diagnose(
            origCall->sourceLoc,
            Diagnostics::internalCompilerError,
            "attempting to differentiate unresolved callee");

        return InstPair(nullptr, nullptr);
    }

    auto primalCallee = findOrTranscribePrimalInst(builder, origCallee);
    IRInst* diffCallee = (this->differentiableTypeConformanceContext.tryGetAssociationOfKind(
        primalCallee,
        ValAssociationKind::ForwardDerivative));

    if (!diffCallee || as<IRVoidLit>(diffCallee))
    {
        // TODO: It may be better to lower this as a SynthesizedFuncDecl from the front-end
        // if a function is backward-differentiable, but not forward-differentiable.
        //
        if (this->useTrivialFwdsForBwdDifferentiableFuncs)
        {
            // See if the callee is backward differentiable.
            if (this->differentiableTypeConformanceContext.tryGetAssociationOfKind(
                    primalCallee,
                    ValAssociationKind::BackwardDerivativeApply))
            {
                // If yes, then we want to generate a trivial forward function.
                IRInst* args[] = {primalCallee->getDataType()};
                auto fwdDiffFuncTypeInst = builder->emitIntrinsicInst(
                    builder->getTypeKind(),
                    kIROp_ForwardDiffFuncType,
                    1,
                    args);
                auto resolvedFuncType =
                    differentiableTypeConformanceContext.resolveType(builder, fwdDiffFuncTypeInst);
                // Generate a trivial forward differentiation function
                auto trivialFwdCallee = builder->emitIntrinsicInst(
                    resolvedFuncType,
                    kIROp_TrivialForwardDifferentiate,
                    1,
                    &primalCallee);

                diffCallee = trivialFwdCallee;
            }
        }
    }

    if (as<IRVoidLit>(diffCallee))
    {
        // Diagnose.
        getSink()->diagnose(
            origCall->sourceLoc,
            Diagnostics::encounteredNonDifferentiableFunctionDuringHigherOrderDiff);
        IRInst* primalCall = maybeCloneForPrimalInst(builder, origCall);
        return InstPair(primalCall, nullptr);
    }

    if (!diffCallee)
    {
        // The callee is non differentiable, just return primal value with null diff value.
        IRInst* primalCall = maybeCloneForPrimalInst(builder, origCall);
        return InstPair(primalCall, nullptr);
    }

    auto calleeType = _getCalleeActualFuncType(&differentiableTypeConformanceContext, primalCallee);
    SLANG_ASSERT(calleeType);
    SLANG_RELEASE_ASSERT(calleeType->getParamCount() == origCall->getArgCount());

    auto diffCalleeType =
        _getCalleeActualFuncType(&differentiableTypeConformanceContext, diffCallee);
    SLANG_ASSERT(diffCalleeType);
    SLANG_RELEASE_ASSERT(diffCalleeType->getParamCount() == origCall->getArgCount());

    auto placeholderCallee = builder->emitPoison(builder->getTypeKind());
    auto placeholderCall = builder->emitCallInst(nullptr, placeholderCallee, 0, nullptr);
    builder->setInsertBefore(placeholderCall);
    IRBuilder argBuilder = *builder;
    IRBuilder afterBuilder = argBuilder;
    afterBuilder.setInsertAfter(placeholderCall);

    List<IRInst*> args;
    // Go over the parameter list and create pairs for each input (if required)
    for (UIndex ii = 0; ii < origCall->getArgCount(); ii++)
    {
        auto origArg = origCall->getArg(ii);
        auto primalArg = findOrTranscribePrimalInst(&argBuilder, origArg);
        SLANG_ASSERT(primalArg);

        auto origType = origCall->getArg(ii)->getDataType();
        auto primalType = primalArg->getDataType();
        auto originalParamType = calleeType->getParamType(ii);
        auto diffParamType = diffCalleeType->getParamType(ii);

        auto [originalParamPassingMode, originalParamBaseType] =
            splitParameterDirectionAndType(originalParamType);
        if (differentiableTypeConformanceContext.isDifferentiableType(originalParamBaseType))
        {
            switch (originalParamPassingMode.kind)
            {
            case ParameterDirectionInfo::In:
                {
                    auto diffArg = findOrTranscribeDiffInst(&argBuilder, origArg);
                    auto pairType = tryGetDiffPairType(&argBuilder, primalType);

                    if (!diffArg)
                    {
                        diffArg = getDifferentialZeroOfType(&argBuilder, originalParamBaseType);

                        // If there is no differential arg, it's still possible that we have a
                        // argument pair type (i.e. type is differentiable, but there was no value
                        // in this particular instance).
                        //
                        // If not, then we'll construct the pair type from the parameter base type,
                        // which may be a super-type of the argument type. Note: It's important to
                        // not default to the parameter type because of the potential sub-type
                        // relationship.
                        //
                        if (!pairType)
                            pairType = tryGetDiffPairType(&argBuilder, originalParamBaseType);
                    }

                    auto diffPair =
                        argBuilder.emitMakeDifferentialPair(pairType, primalArg, diffArg);
                    markDiffPairTypeInst(&argBuilder, diffPair, pairType);

                    args.add(diffPair);
                    break;
                }
            case ParameterDirectionInfo::BorrowIn:
            case ParameterDirectionInfo::BorrowInOut:
            case ParameterDirectionInfo::Out:
            case ParameterDirectionInfo::Ref:
                {
                    // The input must be a pointer.
                    SLANG_ASSERT(as<IRPtrTypeBase>(primalType));
                    auto primalValType = as<IRPtrTypeBase>(primalType)->getValueType();
                    auto basePairType = tryGetDiffPairType(&argBuilder, primalValType);
                    auto ptrPairType = builder->getPtrTypeWithAddressSpace(
                        primalValType,
                        as<IRPtrTypeBase>(primalType));

                    auto srcVar = argBuilder.emitVar(basePairType);
                    markDiffPairTypeInst(
                        &argBuilder,
                        srcVar,
                        basePairType); // TODO: Why the val type?

                    auto diffArg = findOrTranscribeDiffInst(&argBuilder, origArg);
                    if (originalParamPassingMode.kind == ParameterDirectionInfo::BorrowInOut)
                    {
                        // Set initial value.
                        auto primalVal = argBuilder.emitLoad(primalArg);
                        auto diffArgVal = diffArg;
                        if (!diffArg)
                            diffArgVal = getDifferentialZeroOfType(builder, (IRType*)primalValType);
                        else
                        {
                            diffArgVal = argBuilder.emitLoad(diffArg);
                            markDiffTypeInst(&argBuilder, diffArgVal, primalValType);
                        }
                        auto initVal = argBuilder.emitMakeDifferentialPair(
                            basePairType,
                            primalVal,
                            diffArgVal);
                        markDiffPairTypeInst(&argBuilder, initVal, basePairType);

                        auto store = argBuilder.emitStore(srcVar, initVal);
                        markDiffPairTypeInst(&argBuilder, store, basePairType);
                    }

                    if (originalParamPassingMode.kind == ParameterDirectionInfo::BorrowInOut ||
                        originalParamPassingMode.kind == ParameterDirectionInfo::Out)
                    {
                        // Read back new value.
                        auto newVal = afterBuilder.emitLoad(srcVar);
                        markDiffPairTypeInst(&afterBuilder, newVal, basePairType);
                        auto newPrimalVal =
                            afterBuilder.emitDifferentialPairGetPrimal(primalValType, newVal);
                        afterBuilder.emitStore(primalArg, newPrimalVal);

                        if (diffArg)
                        {
                            auto newDiffVal = afterBuilder.emitDifferentialPairGetDifferential(
                                (IRType*)differentiableTypeConformanceContext
                                    .getDifferentialForType(&afterBuilder, primalValType),
                                newVal);
                            markDiffTypeInst(&afterBuilder, newDiffVal, primalValType);

                            auto storeInst = afterBuilder.emitStore(diffArg, newDiffVal);
                            markDiffTypeInst(&afterBuilder, storeInst, primalValType);
                        }
                    }

                    args.add(srcVar);
                    break;
                }
            default:
                SLANG_UNREACHABLE("unknown parameter passing mode");
            }

            continue;
        }

        /*
        TODO: Test this case properly
        {
            // --WORKAROUND--
            // This is a temporary workaround for a very specific case..
            //
            // If all the following are true:
            // 1. the parameter type expects a differential pair,
            // 2. the argument is derived from a no_diff type, and
            // 3. the argument type is a run-time type (i.e. extract_existential_type),
            // then we need to generate a differential 0, but the IR has no
            // information on the diff witness.
            //
            // We will bypass the conformance system & brute-force the lookup for the interface
            // keys, but the proper fix is to lower this key mapping during `no_diff` lowering.
            //

            // Condition 1
            if (differentiableTypeConformanceContext.isDifferentiableType((originalParamType)))
            {
                // Condition 3
                if (auto extractExistentialType = as<IRExtractExistentialType>(primalType))
                {
                    // Condition 2
                    if (isNoDiffType(extractExistentialType->getOperand(0)->getDataType()))
                    {
                        // Force-differentiate the type (this will perform a search for the witness
                        // without going through the diff-type annotation list)
                        //
                        IRInst* witnessTable = nullptr;
                        SLANG_UNEXPECTED("Fail");
                        auto diffType = differentiateExtractExistentialType(
                            &argBuilder,
                            extractExistentialType,
                            witnessTable);

                        auto pairType =
                            getOrCreateDiffPairType(&argBuilder, primalType, witnessTable);
                        auto zeroMethod = argBuilder.emitLookupInterfaceMethodInst(
                            differentiableTypeConformanceContext.sharedContext->zeroMethodType,
                            witnessTable,
                            differentiableTypeConformanceContext.sharedContext
                                ->zeroMethodStructKey);
                        auto diffZero = argBuilder.emitCallInst(diffType, zeroMethod, 0, nullptr);
                        auto diffPair =
                            argBuilder.emitMakeDifferentialPair(pairType, primalArg, diffZero);

                        args.add(diffPair);
                        continue;
                    }
                }
            }
        }
        */

        // Argument is not differentiable.
        // Add original/primal argument.
        args.add(primalArg);
    }

    IRType* diffReturnType = nullptr;
    auto primalReturnType =
        (IRType*)findOrTranscribePrimalInst(&argBuilder, origCall->getFullType());

    diffReturnType = tryGetDiffPairType(&argBuilder, primalReturnType);

    if (!diffReturnType)
    {
        diffReturnType = primalReturnType;
    }

    auto callInst = argBuilder.emitCallInst(diffReturnType, diffCallee, args);
    placeholderCall->removeAndDeallocate();
    placeholderCallee->removeAndDeallocate();

    argBuilder.markInstAsMixedDifferential(callInst, diffReturnType);

    IRBuilder annotationBuilder(argBuilder.getModule());
    annotationBuilder.setInsertInto(argBuilder.getModule());
    emitCalleeAnnotationsForHigherOrderDiff(
        &annotationBuilder,
        &differentiableTypeConformanceContext,
        primalCallee);

    // Stick a link back the primal callee so that any future passes can
    // use this info (e.g. backward AD pass would want to retreive the primalCallee
    // so that it can look up the associated backward derivative).
    //
    argBuilder.addAutoDiffOriginalValueDecoration(callInst, primalCallee);

    *builder = afterBuilder;

    if (as<IRDifferentialPairType>(diffReturnType) || as<IRDifferentialPtrPairType>(diffReturnType))
    {
        IRInst* primalResultValue = afterBuilder.emitDifferentialPairGetPrimal(callInst);
        auto diffType = differentiateType(&afterBuilder, primalReturnType);
        IRInst* diffResultValue =
            afterBuilder.emitDifferentialPairGetDifferential(diffType, callInst);
        return InstPair(primalResultValue, diffResultValue);
    }
    else
    {
        // Return the inst itself if the return value is non-differentiable.
        // This is fine since these values should only be used by non-differentiable code.
        //
        return InstPair(callInst, callInst);
    }
}

InstPair ForwardDiffTranscriber::transcribeSwizzle(IRBuilder* builder, IRSwizzle* origSwizzle)
{
    IRInst* primalSwizzle = maybeCloneForPrimalInst(builder, origSwizzle);
    if (auto diffBase = lookupDiffInst(origSwizzle->getBase(), nullptr))
    {
        // `diffBase` may exist even if the type is non-differentiable (e.g. IRCall inst that
        // creates other differentiable outputs).
        //
        // We'll check to see if we can get a differential for the type in order to determine
        // whether to generate a differential swizzle inst.
        //
        if (auto diffType = differentiateType(builder, primalSwizzle->getDataType()))
        {
            List<IRInst*> swizzleIndices;
            for (UIndex ii = 0; ii < origSwizzle->getElementCount(); ii++)
                swizzleIndices.add(origSwizzle->getElementIndex(ii));

            return InstPair(
                primalSwizzle,
                builder->emitSwizzle(
                    diffType,
                    diffBase,
                    origSwizzle->getElementCount(),
                    swizzleIndices.getBuffer()));
        }
    }

    return InstPair(primalSwizzle, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeByPassthrough(IRBuilder* builder, IRInst* origInst)
{
    IRInst* primalInst = maybeCloneForPrimalInst(builder, origInst);

    UCount operandCount = origInst->getOperandCount();

    List<IRInst*> diffOperands;
    for (UIndex ii = 0; ii < operandCount; ii++)
    {
        // If the operand has a differential version, replace the original with the
        // differential.
        // Otherwise, abandon the differentiation attempt and assume that origInst
        // cannot (or does not need to) be differentiated.
        //
        if (auto diffInst = lookupDiffInst(origInst->getOperand(ii), nullptr))
            diffOperands.add(diffInst);
        else
            return InstPair(primalInst, nullptr);
    }

    return InstPair(
        primalInst,
        builder->emitIntrinsicInst(
            differentiateType(builder, origInst->getDataType()),
            origInst->getOp(),
            operandCount,
            diffOperands.getBuffer()));
}

InstPair ForwardDiffTranscriber::transcribeControlFlow(IRBuilder* builder, IRInst* origInst)
{
    switch (origInst->getOp())
    {
    case kIROp_UnconditionalBranch:
    case kIROp_Loop:
        auto origBranch = as<IRUnconditionalBranch>(origInst);
        auto targetBlock = origBranch->getTargetBlock();

        // Grab the differentials for any phi nodes.
        List<IRInst*> newArgs;
        for (UIndex ii = 0; ii < origBranch->getArgCount(); ii++)
        {
            auto origParam = getParamAt(targetBlock, ii);
            auto origArg = origBranch->getArg(ii);
            auto primalArg = lookupPrimalInst(builder, origArg);
            newArgs.add(primalArg);

            if (differentiateType(builder, origParam->getDataType()))
            {
                auto diffArg = lookupDiffInst(origArg, nullptr);
                if (diffArg)
                    newArgs.add(diffArg);
                else
                    newArgs.add(getDifferentialZeroOfType(builder, origArg->getDataType()));
            }
        }

        IRInst* diffBranch = nullptr;
        if (auto diffBlock = findOrTranscribeDiffInst(builder, origBranch->getTargetBlock()))
        {
            if (auto origLoop = as<IRLoop>(origInst))
            {
                auto breakBlock = findOrTranscribeDiffInst(builder, origLoop->getBreakBlock());
                auto continueBlock =
                    findOrTranscribeDiffInst(builder, origLoop->getContinueBlock());
                List<IRInst*> operands;
                operands.add(diffBlock);
                operands.add(breakBlock);
                operands.add(continueBlock);
                operands.addRange(newArgs);
                diffBranch = builder->emitIntrinsicInst(
                    nullptr,
                    kIROp_Loop,
                    operands.getCount(),
                    operands.getBuffer());
                if (auto maxItersDecoration = origLoop->findDecoration<IRLoopMaxItersDecoration>())
                    builder->addLoopMaxItersDecoration(
                        diffBranch,
                        maxItersDecoration->getMaxIters());
            }
            else
            {
                diffBranch = builder->emitBranch(
                    as<IRBlock>(diffBlock),
                    newArgs.getCount(),
                    newArgs.getBuffer());
            }
        }

        // For now, every block in the original fn must have a corresponding
        // block to compute *both* primals and derivatives (i.e linearized block)
        SLANG_ASSERT(diffBranch);

        // Since blocks always compute both primals and differentials, the branch
        // instructions are also always mixed.
        //
        builder->markInstAsMixedDifferential(diffBranch);

        return InstPair(diffBranch, diffBranch);
    }

    getSink()->diagnose(
        origInst->sourceLoc,
        Diagnostics::unimplemented,
        "attempting to differentiate unhandled control flow");

    return InstPair(nullptr, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeConst(IRBuilder*, IRInst* origInst)
{
    switch (origInst->getOp())
    {
    case kIROp_FloatLit:
    case kIROp_IntLit:
        return InstPair(origInst, nullptr);
    case kIROp_VoidLit:
        return InstPair(origInst, origInst);
    }

    getSink()->diagnose(
        origInst->sourceLoc,
        Diagnostics::unimplemented,
        "attempting to differentiate unhandled const type");

    return InstPair(nullptr, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeFieldExtract(IRBuilder* builder, IRInst* originalInst)
{
    SLANG_ASSERT(as<IRFieldExtract>(originalInst) || as<IRFieldAddress>(originalInst));

    IRInst* origBase = originalInst->getOperand(0);
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    auto field = originalInst->getOperand(1);
    auto derivativeRefDecor = field->findDecoration<IRDerivativeMemberDecoration>();
    auto primalType = (IRType*)findOrTranscribePrimalInst(builder, originalInst->getDataType());

    IRInst* primalOperands[] = {primalBase, field};
    IRInst* primalFieldExtract =
        builder->emitIntrinsicInst(primalType, originalInst->getOp(), 2, primalOperands);

    if (!derivativeRefDecor)
    {
        return InstPair(primalFieldExtract, nullptr);
    }

    IRInst* diffFieldExtract = nullptr;

    if (auto diffType = differentiateType(builder, originalInst->getDataType()))
    {
        if (auto diffBase = findOrTranscribeDiffInst(builder, origBase))
        {
            IRInst* diffOperands[] = {diffBase, derivativeRefDecor->getDerivativeMemberStructKey()};
            diffFieldExtract =
                builder->emitIntrinsicInst(diffType, originalInst->getOp(), 2, diffOperands);
        }
    }
    return InstPair(primalFieldExtract, diffFieldExtract);
}

InstPair ForwardDiffTranscriber::transcribeGetElement(IRBuilder* builder, IRInst* origGetElementPtr)
{
    SLANG_ASSERT(as<IRGetElement>(origGetElementPtr) || as<IRGetElementPtr>(origGetElementPtr));

    IRInst* origBase = origGetElementPtr->getOperand(0);
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    auto primalIndex = findOrTranscribePrimalInst(builder, origGetElementPtr->getOperand(1));

    auto primalType =
        (IRType*)findOrTranscribePrimalInst(builder, origGetElementPtr->getDataType());

    IRInst* primalOperands[] = {primalBase, primalIndex};
    IRInst* primalGetElementPtr =
        builder->emitIntrinsicInst(primalType, origGetElementPtr->getOp(), 2, primalOperands);

    IRInst* diffGetElementPtr = nullptr;

    if (auto diffType = differentiateType(builder, origGetElementPtr->getDataType()))
    {
        if (auto diffBase = findOrTranscribeDiffInst(builder, origBase))
        {
            IRInst* diffOperands[] = {diffBase, primalIndex};
            diffGetElementPtr =
                builder->emitIntrinsicInst(diffType, origGetElementPtr->getOp(), 2, diffOperands);
        }
    }

    return InstPair(primalGetElementPtr, diffGetElementPtr);
}

InstPair ForwardDiffTranscriber::transcribeGetTupleElement(IRBuilder* builder, IRInst* originalInst)
{
    IRInst* origBase = originalInst->getOperand(0);
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    auto primalIndex = originalInst->getOperand(1);

    auto primalType = (IRType*)findOrTranscribePrimalInst(builder, originalInst->getDataType());

    IRInst* primalOperands[] = {primalBase, primalIndex};
    IRInst* primalGetElement =
        builder->emitIntrinsicInst(primalType, originalInst->getOp(), 2, primalOperands);

    IRInst* diffGetElement = nullptr;

    if (auto diffType = differentiateType(builder, primalGetElement->getDataType()))
    {
        if (auto diffBase = findOrTranscribeDiffInst(builder, origBase))
        {
            IRInst* diffOperands[] = {diffBase, primalIndex};
            diffGetElement =
                builder->emitIntrinsicInst(diffType, originalInst->getOp(), 2, diffOperands);
        }
    }

    return InstPair(primalGetElement, diffGetElement);
}

InstPair ForwardDiffTranscriber::transcribeGetOptionalValue(
    IRBuilder* builder,
    IRInst* originalInst)
{
    IRInst* origBase = originalInst->getOperand(0);
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);

    auto primalType = (IRType*)findOrTranscribePrimalInst(builder, originalInst->getDataType());

    IRInst* primalGetOptionalVal =
        builder->emitIntrinsicInst(primalType, originalInst->getOp(), 1, &primalBase);

    IRInst* diffGetOptionalVal = nullptr;

    if (auto diffType = differentiateType(builder, primalGetOptionalVal->getDataType()))
    {
        if (auto diffBase = findOrTranscribeDiffInst(builder, origBase))
        {
            diffGetOptionalVal =
                builder->emitIntrinsicInst(diffType, originalInst->getOp(), 1, &diffBase);
        }
    }

    return InstPair(primalGetOptionalVal, diffGetOptionalVal);
}

InstPair ForwardDiffTranscriber::transcribeUpdateElement(IRBuilder* builder, IRInst* originalInst)
{
    auto updateInst = as<IRUpdateElement>(originalInst);

    IRInst* origBase = updateInst->getOldValue();
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    List<IRInst*> primalAccessChain;
    for (UInt i = 0; i < updateInst->getAccessKeyCount(); i++)
    {
        auto originalKey = updateInst->getAccessKey(i);
        auto primalKey = findOrTranscribePrimalInst(builder, originalKey);
        primalAccessChain.add(primalKey);
    }
    auto origVal = updateInst->getElementValue();
    auto primalVal = findOrTranscribePrimalInst(builder, origVal);

    IRInst* primalUpdateField =
        builder->emitUpdateElement(primalBase, primalAccessChain.getArrayView(), primalVal);

    IRInst* diffUpdateElement = nullptr;
    List<IRInst*> diffAccessChain;
    for (auto key : primalAccessChain)
    {
        if (as<IRStructKey>(key))
        {
            auto decor = key->findDecoration<IRDerivativeMemberDecoration>();
            if (decor)
                diffAccessChain.add(decor->getDerivativeMemberStructKey());
            else
            {
                auto diffBase = findOrTranscribeDiffInst(builder, origBase);
                return InstPair(primalUpdateField, diffBase);
            }
        }
        else
        {
            diffAccessChain.add(key);
        }
    }
    if (const auto diffType = differentiateType(builder, originalInst->getDataType()))
    {
        auto diffBase = findOrTranscribeDiffInst(builder, origBase);
        if (!diffBase)
        {
            diffBase = getDifferentialZeroOfType(builder, origBase->getDataType());
        }
        if (auto diffVal = findOrTranscribeDiffInst(builder, origVal))
        {
            auto primalElementType = primalVal->getDataType();

            diffUpdateElement =
                builder->emitUpdateElement(diffBase, diffAccessChain.getArrayView(), diffVal);
            builder->addPrimalElementTypeDecoration(diffUpdateElement, primalElementType);
        }
        else
        {
            auto primalElementType = primalVal->getDataType();
            auto zeroElementDiff = getDifferentialZeroOfType(builder, primalElementType);
            diffUpdateElement = builder->emitUpdateElement(
                diffBase,
                diffAccessChain.getArrayView(),
                zeroElementDiff);
            builder->addPrimalElementTypeDecoration(diffUpdateElement, primalElementType);
        }
    }
    return InstPair(primalUpdateField, diffUpdateElement);
}

InstPair ForwardDiffTranscriber::transcribeSwitch(IRBuilder* builder, IRSwitch* origSwitch)
{
    // Transcribe condition (primal only, conditions do not produce differentials)
    auto primalCondition = findOrTranscribePrimalInst(builder, origSwitch->getCondition());
    SLANG_ASSERT(primalCondition);

    // Transcribe 'default' block
    IRBlock* diffDefaultBlock =
        as<IRBlock>(findOrTranscribeDiffInst(builder, origSwitch->getDefaultLabel()));
    SLANG_ASSERT(diffDefaultBlock);

    // Transcribe 'default' block
    IRBlock* diffBreakBlock =
        as<IRBlock>(findOrTranscribeDiffInst(builder, origSwitch->getBreakLabel()));
    SLANG_ASSERT(diffBreakBlock);

    // Transcribe all other operands
    List<IRInst*> diffCaseValuesAndLabels;
    for (UIndex ii = 0; ii < origSwitch->getCaseCount(); ii++)
    {
        auto primalCaseValue = findOrTranscribePrimalInst(builder, origSwitch->getCaseValue(ii));
        SLANG_ASSERT(primalCaseValue);

        auto diffCaseBlock = findOrTranscribeDiffInst(builder, origSwitch->getCaseLabel(ii));
        SLANG_ASSERT(diffCaseBlock);

        diffCaseValuesAndLabels.add(primalCaseValue);
        diffCaseValuesAndLabels.add(diffCaseBlock);
    }

    auto diffSwitchInst = builder->emitSwitch(
        primalCondition,
        diffBreakBlock,
        diffDefaultBlock,
        diffCaseValuesAndLabels.getCount(),
        diffCaseValuesAndLabels.getBuffer());
    builder->markInstAsMixedDifferential(diffSwitchInst);

    return InstPair(diffSwitchInst, diffSwitchInst);
}

InstPair ForwardDiffTranscriber::transcribeIfElse(IRBuilder* builder, IRIfElse* origIfElse)
{
    // IfElse Statements come with 4 blocks. We transcribe each block into it's
    // linear form, and then wire them up in the same way as the original if-else

    // Transcribe condition block
    auto primalConditionBlock = findOrTranscribePrimalInst(builder, origIfElse->getCondition());
    SLANG_ASSERT(primalConditionBlock);

    // Transcribe 'true' block (condition block branches into this if true)
    auto diffTrueBlock = findOrTranscribeDiffInst(builder, origIfElse->getTrueBlock());
    SLANG_ASSERT(diffTrueBlock);

    // Transcribe 'false' block (condition block branches into this if true)
    auto diffFalseBlock = findOrTranscribeDiffInst(builder, origIfElse->getFalseBlock());
    SLANG_ASSERT(diffFalseBlock);

    // Transcribe 'after' block (true and false blocks branch into this)
    auto diffAfterBlock = findOrTranscribeDiffInst(builder, origIfElse->getAfterBlock());
    SLANG_ASSERT(diffAfterBlock);

    List<IRInst*> diffIfElseArgs;
    diffIfElseArgs.add(primalConditionBlock);
    diffIfElseArgs.add(diffTrueBlock);
    diffIfElseArgs.add(diffFalseBlock);
    diffIfElseArgs.add(diffAfterBlock);

    // If there are any other operands, use their primal versions.
    for (UIndex ii = diffIfElseArgs.getCount(); ii < origIfElse->getOperandCount(); ii++)
    {
        auto primalOperand = findOrTranscribePrimalInst(builder, origIfElse->getOperand(ii));
        diffIfElseArgs.add(primalOperand);
    }

    IRInst* diffIfElse = builder->emitIntrinsicInst(
        nullptr,
        kIROp_IfElse,
        diffIfElseArgs.getCount(),
        diffIfElseArgs.getBuffer());
    builder->markInstAsMixedDifferential(diffIfElse);

    return InstPair(diffIfElse, diffIfElse);
}

InstPair ForwardDiffTranscriber::transcribeMakeDifferentialPair(
    IRBuilder* builder,
    IRInst* origInst)
{
    auto origPrimalVal = origInst->getOperand(0);
    auto origDiffVal = origInst->getOperand(1);

    auto primalVal = findOrTranscribePrimalInst(builder, origPrimalVal);
    SLANG_ASSERT(primalVal);
    auto diffPrimalVal = findOrTranscribePrimalInst(builder, origDiffVal);
    SLANG_ASSERT(diffPrimalVal);

    auto primalDiffVal = findOrTranscribeDiffInst(builder, origPrimalVal);
    if (!primalDiffVal)
        primalDiffVal = getDifferentialZeroOfType(builder, origPrimalVal->getDataType());
    SLANG_ASSERT(primalDiffVal);

    auto diffDiffVal = findOrTranscribeDiffInst(builder, origDiffVal);
    if (!diffDiffVal)
        diffDiffVal = getDifferentialZeroOfType(builder, origDiffVal->getDataType());
    SLANG_ASSERT(diffDiffVal);

    auto primalPairType = getOrCreateDiffPairType(
        builder,
        primalVal->getDataType()); // findOrTranscribePrimalInst(builder, origInst->getFullType());
    auto diffPairType = getOrCreateDiffPairType(
        builder,
        diffPrimalVal
            ->getDataType()); // findOrTranscribeDiffInst(builder, origInst->getFullType());
    if (origInst->getOp() == kIROp_MakeDifferentialPair)
    {
        auto primalPair = builder->emitMakeDifferentialValuePair(
            (IRType*)primalPairType,
            primalVal,
            diffPrimalVal);
        auto diffPair = builder->emitMakeDifferentialValuePair(
            (IRType*)diffPairType,
            primalDiffVal,
            diffDiffVal);
        return InstPair(primalPair, diffPair);
    }
    else
    {
        SLANG_UNEXPECTED("unknown make differential pair op");
    }
}

InstPair ForwardDiffTranscriber::transcribeDifferentialPairGetElement(
    IRBuilder* builder,
    IRInst* origInst)
{
    auto primalVal = findOrTranscribePrimalInst(builder, origInst->getOperand(0));
    SLANG_ASSERT(primalVal);

    auto diffVal = findOrTranscribeDiffInst(builder, origInst->getOperand(0));
    SLANG_ASSERT(diffVal);

    auto primalType = findOrTranscribePrimalInst(builder, origInst->getFullType());
    auto diffResultType = differentiateType(builder, origInst->getFullType());

    auto primalResult =
        builder->emitIntrinsicInst((IRType*)primalType, origInst->getOp(), 1, &primalVal);

    auto diffResult =
        builder->emitIntrinsicInst((IRType*)diffResultType, origInst->getOp(), 1, &diffVal);
    return InstPair(primalResult, diffResult);
}

InstPair ForwardDiffTranscriber::transcribeSingleOperandInst(IRBuilder* builder, IRInst* origInst)
{
    IRInst* origBase = origInst->getOperand(0);
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    auto primalType = (IRType*)findOrTranscribePrimalInst(builder, origInst->getDataType());

    IRInst* primalResult =
        builder->emitIntrinsicInst(primalType, origInst->getOp(), 1, &primalBase);

    IRInst* diffResult = nullptr;

    if (auto diffType = differentiateType(builder, primalType))
    {
        if (auto diffBase = findOrTranscribeDiffInst(builder, origBase))
        {
            diffResult = builder->emitIntrinsicInst(diffType, origInst->getOp(), 1, &diffBase);
        }
    }
    return InstPair(primalResult, diffResult);
}

InstPair ForwardDiffTranscriber::transcribeMakeExistential(
    IRBuilder* builder,
    IRMakeExistential* origMakeExistential)
{
    auto origBase = origMakeExistential->getWrappedValue();
    auto origWitnessTable = origMakeExistential->getWitnessTable();

    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    auto primalWitnessTable = findOrTranscribePrimalInst(builder, origWitnessTable);
    auto primalType =
        (IRType*)findOrTranscribePrimalInst(builder, origMakeExistential->getDataType());

    auto primalWrappedType = (IRType*)findOrTranscribePrimalInst(builder, origBase->getDataType());

    auto diffBase = findOrTranscribeDiffInst(builder, origBase);

    IRInst* primalResult = builder->emitMakeExistential(primalType, primalBase, primalWitnessTable);

    if (diffBase)
    {
        if (auto diffWrappedType = differentiableTypeConformanceContext
                                       .tryGetDifferentiableValueType(builder, primalWrappedType))
        {
            // DiffType : IDifferentiable
            auto diffTypeDiffWitness =
                as<IRDifferentialPairType>(
                    differentiableTypeConformanceContext.tryGetAssociationOfKind(
                        diffWrappedType,
                        ValAssociationKind::DifferentialPairType))
                    ->getWitness();

            return InstPair(
                primalResult,
                builder->emitMakeExistential(
                    this->autoDiffSharedContext->differentiableInterfaceType,
                    diffBase,
                    diffTypeDiffWitness));
        }
    }

    // If there's no diffBase, should we be creating a zero differential existential?

    return InstPair(primalResult, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeDefaultConstruct(IRBuilder* builder, IRInst* origInst)
{
    IRInst* primalConstruct = maybeCloneForPrimalInst(builder, origInst);

    IRInst* diffConstruct = nullptr;

    if (auto diffType = differentiateType(builder, origInst->getDataType()))
    {
        diffConstruct = builder->emitDefaultConstructRaw(diffType);
    }
    return InstPair(primalConstruct, diffConstruct);
}

InstPair ForwardDiffTranscriber::transcribeWrapExistential(IRBuilder* builder, IRInst* origInst)
{
    auto primalType = (IRType*)findOrTranscribePrimalInst(builder, origInst->getDataType());

    List<IRInst*> primalArgs;
    for (UInt i = 0; i < origInst->getOperandCount(); i++)
    {
        auto primalArg = findOrTranscribePrimalInst(builder, origInst->getOperand(i));
        primalArgs.add(primalArg);
    }

    IRInst* primalResult = builder->emitIntrinsicInst(
        primalType,
        origInst->getOp(),
        primalArgs.getCount(),
        primalArgs.getBuffer());

    IRInst* diffResult = nullptr;

    if (auto diffType = differentiateType(builder, origInst->getDataType()))
    {
        List<IRInst*> diffArgs;
        for (UInt i = 0; i < origInst->getOperandCount(); i++)
        {
            auto arg = findOrTranscribeDiffInst(builder, origInst->getOperand(i));
            if (arg)
            {
                diffArgs.add(arg);
            }
            else if (i == 0)
            {
                // If we can't diff the first operand (base), abort now.
                break;
            }
        }
        if (diffArgs.getCount())
        {
            diffResult = builder->emitIntrinsicInst(
                diffType,
                origInst->getOp(),
                diffArgs.getCount(),
                diffArgs.getBuffer());
        }
    }
    return InstPair(primalResult, diffResult);
}

// Create an empty func to represent the transcribed func of `origFunc`.
InstPair ForwardDiffTranscriber::transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc)
{
    IRFunc* diffFunc = nullptr;
    diffFunc = transcribeFuncHeaderImpl(inBuilder, origFunc);

    inBuilder->addFloatingModeOverrideDecoration(diffFunc, FloatingPointMode::Fast);

    copyOriginalDecorations(origFunc, diffFunc);

    return InstPair(origFunc, diffFunc);
}

IRFunc* ForwardDiffTranscriber::transcribeFuncHeaderImpl(IRBuilder* inBuilder, IRFunc* origFunc)
{
    IRBuilder builder = *inBuilder;

    auto diffFunc = builder.createFunc();

    SLANG_ASSERT(as<IRFuncType>(origFunc->getFullType()));
    IRType* diffFuncType = this->differentiateFunctionType(
        &builder,
        origFunc,
        as<IRFuncType>(origFunc->getFullType()));
    diffFunc->setFullType(diffFuncType);

    if (auto nameHint = origFunc->findDecoration<IRNameHintDecoration>())
    {
        auto originalName = nameHint->getName();
        StringBuilder newNameSb;
        newNameSb << "s_fwd_" << originalName;
        builder.addNameHintDecoration(diffFunc, newNameSb.getUnownedSlice());
    }

    // Mark the generated derivative function itself as differentiable.
    builder.addForwardDifferentiableDecoration(diffFunc);
    if (isBackwardDifferentiableFunc(origFunc))
        builder.addBackwardDifferentiableDecoration(diffFunc);

    // Transfer checkpoint hint decorations
    copyCheckpointHints(&builder, origFunc, diffFunc);
    return diffFunc;
}

void ForwardDiffTranscriber::checkAutodiffInstDecorations(IRFunc* fwdFunc)
{
    for (auto block = fwdFunc->getFirstBlock(); block; block = block->getNextBlock())
    {
        for (auto inst = block->getFirstOrdinaryInst(); inst; inst = inst->getNextInst())
        {
            // TODO: Special case, not sure why these insts show up
            if (as<IRUndefined>(inst))
                continue;

            List<IRDecoration*> decorations;
            for (auto decoration : inst->getDecorations())
            {
                if (as<IRAutodiffInstDecoration>(decoration))
                    decorations.add(decoration);
            }

            // TODO: reenable this assert, it's been nonfunctional since about
            // 2023 since as<IRUndefined> always returned true until now

            // Must have _exactly_ one autodiff tag.
            // SLANG_ASSERT(decorations.getCount() == 1);
        }
    }
}

void insertTempVarForMutableParams(IRModule* module, IRFunc* func)
{
    IRBuilder builder(module);
    auto firstBlock = func->getFirstBlock();
    builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());

    OrderedDictionary<IRParam*, IRVar*> mapParamToTempVar;
    List<IRParam*> params;
    for (auto param : firstBlock->getParams())
    {
        if (const auto ptrType = as<IROutParamTypeBase>(param->getDataType()))
        {
            params.add(param);
        }
    }

    for (auto param : params)
    {
        auto ptrType = asRelevantPtrType(param->getDataType());
        auto tempVar = builder.emitVar(ptrType->getValueType());
        param->replaceUsesWith(tempVar);
        mapParamToTempVar[param] = tempVar;
        if (ptrType->getOp() != kIROp_OutParamType)
        {
            builder.emitStore(tempVar, builder.emitLoad(param));
        }
        else
        {
            builder.emitStore(tempVar, builder.emitDefaultConstruct(ptrType->getValueType()));
        }
    }

    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_Return)
            {
                builder.setInsertBefore(inst);
                for (const auto& [param, var] : mapParamToTempVar)
                    builder.emitStore(param, builder.emitLoad(var));
            }
        }
    }
}

bool isLocalPointer(IRInst* ptrInst)
{
    // If it's not a local var or a function parameter, then it's probably
    // referencing something outside the function scope.
    //
    auto addr = getRootAddr(ptrInst);
    return as<IRVar>(addr) || as<IRParam, IRDynamicCastBehavior::NoUnwrap>(addr);
}

void lowerSwizzledStores(IRModule* module, IRFunc* func)
{
    List<IRInst*> instsToRemove;

    IRBuilder builder(module);
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (auto swizzledStore = as<IRSwizzledStore>(inst))
            {
                if (!isLocalPointer(swizzledStore->getDest()))
                    continue;

                builder.setInsertBefore(inst);
                for (UIndex ii = 0; ii < swizzledStore->getElementCount(); ii++)
                {
                    auto indexVal = swizzledStore->getElementIndex(ii);
                    auto indexedPtr =
                        builder.emitElementAddress(swizzledStore->getDest(), indexVal);
                    builder.emitStore(
                        indexedPtr,
                        builder.emitElementExtract(
                            swizzledStore->getSource(),
                            builder.getIntValue(builder.getIntType(), ii)));
                }
                instsToRemove.add(inst);
            }
        }
    }

    for (auto inst : instsToRemove)
    {
        inst->removeAndDeallocate();
    }
}

SlangResult ForwardDiffTranscriber::prepareFuncForForwardDiff(IRFunc* func)
{
    insertTempVarForMutableParams(autoDiffSharedContext->moduleInst->getModule(), func);
    removeLinkageDecorations(func);

    performPreAutoDiffForceInlining(func);

    initializeLocalVariables(autoDiffSharedContext->moduleInst->getModule(), func);

    lowerSwizzledStores(autoDiffSharedContext->moduleInst->getModule(), func);

    auto result = eliminateAddressInsts(func, sink);

    if (SLANG_SUCCEEDED(result))
    {
        auto validationScope = disableIRValidationScope();
        auto simplifyOptions = IRSimplificationOptions::getDefault(nullptr);
        simplifyOptions.removeRedundancy = true;
        simplifyOptions.hoistLoopInvariantInsts = true;
        simplifyFunc(nullptr, func, simplifyOptions);
    }
    return result;
}

static IRInst* maybeHoist(IRBuilder& builder, IRInst* inst)
{
    IRInst* specializedVal = nullptr;
    auto hoistResult = hoistValueFromGeneric(builder, inst, specializedVal, true);
    return hoistResult; //(as<IRGeneric>(hoistResult)) ? getGenericReturnVal(hoistResult) :
                        // hoistResult;
}

void ForwardDiffTranscriber::_transcribeFuncImpl(
    IRBuilder* inBuilder,
    IRInst* origInst,
    IRInst*& fwdDiffFunc)
{
    auto origFunc = as<IRFunc>(origInst);
    SLANG_ASSERT(origFunc);
    InstPair pair = this->transcribeFuncHeader(inBuilder, origFunc);
    fwdDiffFunc = pair.differential;

    this->transcribeFunc(inBuilder, origFunc, cast<IRFunc>(fwdDiffFunc));

    fwdDiffFunc = maybeHoist(*inBuilder, fwdDiffFunc);
    stripTempDecorations(fwdDiffFunc);
    copyDebugInfo(origFunc, fwdDiffFunc);
    propagatePropertiesForSingleFunc(inBuilder->getModule(), cast<IRFunc>(fwdDiffFunc));
}

// Transcribe a function definition.
InstPair ForwardDiffTranscriber::transcribeFunc(
    IRBuilder* inBuilder,
    IRFunc* primalFunc,
    IRFunc* diffFunc)
{
    if (primalFunc->findDecoration<IRTreatAsDifferentiableDecoration>())
    {
        // Generate a trivial implementation for [TreatAsDifferentiable] functions.
        generateTrivialFwdDiffFunc(primalFunc, diffFunc);
        return InstPair(primalFunc, diffFunc);
    }

    IRBuilder builder = *inBuilder;
    builder.setInsertBefore(primalFunc);

    // Create a clone for original func and run additional transformations on the clone.
    IRCloneEnv env;
    auto primalFuncClone = as<IRFunc>(cloneInst(&env, &builder, primalFunc));
    prepareFuncForForwardDiff(primalFuncClone);

    builder.setInsertInto(diffFunc);

    mapInOutParamToWriteBackValue.clear();

    // Create and map blocks in diff func.
    for (auto block = primalFuncClone->getFirstBlock(); block; block = block->getNextBlock())
    {
        auto diffBlock = builder.emitBlock();
        mapPrimalInst(block, diffBlock);
        mapDifferentialInst(block, diffBlock);
    }

    // Now actually transcribe the content of each block.
    for (auto block = primalFuncClone->getFirstBlock(); block; block = block->getNextBlock())
        this->transcribeBlock(&builder, block);

    for (auto block : diffFunc->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_Return)
            {
                // Insert write backs to mutable parameters before returning.
                builder.setInsertBefore(inst);
                for (auto& writeBack : mapInOutParamToWriteBackValue)
                {
                    auto param = writeBack.key;
                    auto primalVal = builder.emitLoad(writeBack.value.primal);
                    IRInst* valToStore = nullptr;
                    if (writeBack.value.differential)
                    {
                        auto pairValType =
                            cast<IRPtrTypeBase>(param->getFullType())->getValueType();
                        auto diffVal = builder.emitLoad(writeBack.value.differential);
                        markDiffTypeInst(&builder, diffVal, primalVal->getFullType());

                        valToStore =
                            builder.emitMakeDifferentialPair(pairValType, primalVal, diffVal);

                        markDiffPairTypeInst(&builder, valToStore, pairValType);
                    }
                    else
                    {
                        valToStore = builder.emitLoad(writeBack.value.primal);
                    }

                    auto storeInst = builder.emitStore(param, valToStore);

                    if (writeBack.value.differential)
                    {
                        markDiffPairTypeInst(&builder, storeInst, valToStore->getFullType());
                    }
                }
            }
        }
    }

#if _DEBUG
    checkAutodiffInstDecorations(diffFunc);
#endif

    return InstPair(primalFunc, diffFunc);
}

InstPair ForwardDiffTranscriber::transcribeInstImpl(IRBuilder* builder, IRInst* origInst)
{
    // Handle common SSA-style operations
    switch (origInst->getOp())
    {
    case kIROp_Param:
        return transcribeParam(builder, as<IRParam>(origInst));

    case kIROp_Var:
        return transcribeVar(builder, as<IRVar>(origInst));

    case kIROp_Load:
        return transcribeLoad(builder, as<IRLoad>(origInst));

    case kIROp_Store:
        return transcribeStore(builder, as<IRStore>(origInst));

    case kIROp_Return:
        return transcribeReturn(builder, as<IRReturn>(origInst));

    case kIROp_Add:
    case kIROp_Mul:
    case kIROp_Sub:
    case kIROp_Div:
        return transcribeBinaryArith(builder, origInst);

    case kIROp_Less:
    case kIROp_Greater:
    case kIROp_And:
    case kIROp_Or:
    case kIROp_Geq:
    case kIROp_Leq:
    case kIROp_Eql:
    case kIROp_Neq:
        return transcribeBinaryLogic(builder, origInst);

    case kIROp_Select:
        return transcribeSelect(builder, origInst);

    case kIROp_MakeVector:
    case kIROp_MakeMatrix:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_MatrixReshape:
    case kIROp_IntCast:
    case kIROp_FloatCast:
    case kIROp_MakeVectorFromScalar:
    case kIROp_MakeArray:
    case kIROp_MakeArrayFromElement:
    case kIROp_MakeTuple:
    case kIROp_MakeOptionalValue:
    case kIROp_MakeResultValue:
    case kIROp_MakeValuePack:
    case kIROp_BuiltinCast:
        return transcribeConstruct(builder, origInst);
    case kIROp_MakeStruct:
        return transcribeMakeStruct(builder, origInst);

    case kIROp_Call:
        return transcribeCall(builder, as<IRCall>(origInst));

    case kIROp_Swizzle:
        return transcribeSwizzle(builder, as<IRSwizzle>(origInst));

    case kIROp_Neg:
        return transcribeByPassthrough(builder, origInst);

    case kIROp_UpdateElement:
        return transcribeUpdateElement(builder, origInst);

    case kIROp_UnconditionalBranch:
    case kIROp_Loop:
        return transcribeControlFlow(builder, origInst);

    case kIROp_FloatLit:
    case kIROp_IntLit:
    case kIROp_VoidLit:
        return transcribeConst(builder, origInst);

    case kIROp_FieldExtract:
    case kIROp_FieldAddress:
        return transcribeFieldExtract(builder, origInst);

    case kIROp_GetElement:
    case kIROp_GetElementPtr:
        return transcribeGetElement(builder, origInst);

    case kIROp_GetTupleElement:
        return transcribeGetTupleElement(builder, origInst);
    case kIROp_GetOptionalValue:
        return transcribeGetOptionalValue(builder, origInst);

    case kIROp_IfElse:
        return transcribeIfElse(builder, as<IRIfElse>(origInst));

    case kIROp_Switch:
        return transcribeSwitch(builder, as<IRSwitch>(origInst));

    case kIROp_MakeDifferentialPair:
        return transcribeMakeDifferentialPair(builder, origInst);

    case kIROp_DifferentialPairGetPrimal:
    case kIROp_DifferentialPairGetDifferential:
        return transcribeDifferentialPairGetElement(builder, origInst);

    case kIROp_ExtractExistentialValue:
        return transcribeSingleOperandInst(builder, origInst);

    case kIROp_PackAnyValue:
        return transcribeSingleOperandInst(builder, origInst);

    case kIROp_MakeExistential:
        return transcribeMakeExistential(builder, as<IRMakeExistential>(origInst));

    case kIROp_WrapExistential:
        return transcribeWrapExistential(builder, origInst);

    case kIROp_DefaultConstruct:
        return transcribeDefaultConstruct(builder, origInst);

    case kIROp_Poison:
    case kIROp_LoadFromUninitializedMemory:
        return transcribeUndefined(builder, origInst);

    case kIROp_Reinterpret:
        return transcribeReinterpret(builder, origInst);

    case kIROp_AssociatedInstAnnotation:
        return transcribeAssociatedInstAnnotation(builder, origInst);

        // Differentiable insts that should have been lowered in a previous pass.
    case kIROp_SwizzledStore:
        {
            // If we have a non-null dest ptr, then we error out because something went wrong
            // when lowering swizzle-stores to regular stores
            //
            auto swizzledStore = as<IRSwizzledStore>(origInst);
            SLANG_RELEASE_ASSERT(lookupDiffInst(swizzledStore->getDest(), nullptr) == nullptr);
            return transcribeNonDiffInst(builder, swizzledStore);
        }
        // Known non-differentiable insts.
    case kIROp_Not:
    case kIROp_BitAnd:
    case kIROp_BitNot:
    case kIROp_BitXor:
    case kIROp_BitOr:
    case kIROp_BitCast:
    case kIROp_Lsh:
    case kIROp_Rsh:
    case kIROp_IRem:
    case kIROp_ByteAddressBufferLoad:
    case kIROp_ByteAddressBufferStore:
    case kIROp_StructuredBufferLoad:
    case kIROp_RWStructuredBufferLoad:
    case kIROp_RWStructuredBufferLoadStatus:
    case kIROp_RWStructuredBufferStore:
    case kIROp_RWStructuredBufferGetElementPtr:
    case kIROp_NonUniformResourceIndex:
    case kIROp_IsType:
    case kIROp_StaticAssert:
    case kIROp_ImageSubscript:
    case kIROp_ImageLoad:
    case kIROp_ImageStore:
    case kIROp_UnpackAnyValue:
    case kIROp_GetNativePtr:
    case kIROp_CastIntToFloat:
    case kIROp_CastFloatToInt:
    case kIROp_CastIntToEnum:
    case kIROp_CastEnumToInt:
    case kIROp_EnumCast:
    case kIROp_DetachDerivative:
    case kIROp_GetSequentialID:
    case kIROp_GetStringHash:
    case kIROp_SPIRVAsm:
    case kIROp_SPIRVAsmOperandLiteral:
    case kIROp_SPIRVAsmOperandInst:
    case kIROp_SPIRVAsmOperandRayPayloadFromLocation:
    case kIROp_SPIRVAsmOperandRayAttributeFromLocation:
    case kIROp_SPIRVAsmOperandRayCallableFromLocation:
    case kIROp_SPIRVAsmOperandEnum:
    case kIROp_SPIRVAsmOperandBuiltinVar:
    case kIROp_SPIRVAsmOperandGLSL450Set:
    case kIROp_SPIRVAsmOperandDebugPrintfSet:
    case kIROp_SPIRVAsmOperandConvertTexel:
    case kIROp_SPIRVAsmOperandId:
    case kIROp_SPIRVAsmOperandResult:
    case kIROp_SPIRVAsmOperandTruncate:
    case kIROp_SPIRVAsmOperandEntryPoint:
    case kIROp_SPIRVAsmOperandSampledType:
    case kIROp_SPIRVAsmOperandImageType:
    case kIROp_SPIRVAsmOperandSampledImageType:
    case kIROp_Specialize:
    case kIROp_DebugLine:
    case kIROp_DebugVar:
    case kIROp_DebugValue:
    case kIROp_DebugInlinedAt:
    case kIROp_DebugScope:
    case kIROp_DebugNoScope:
    case kIROp_DebugInlinedVariable:
    case kIROp_DebugFunction:
    case kIROp_GetArrayLength:
    case kIROp_SizeOf:
    case kIROp_AlignOf:
    case kIROp_Printf:
    case kIROp_MakeCoopVector:
    case kIROp_MakeCoopVectorFromValuePack:
    case kIROp_GetCurrentStage:
    case kIROp_GetOffsetPtr:
    case kIROp_IsNullExistential:
    case kIROp_MakeResultError:
    case kIROp_IsResultError:
    case kIROp_GetResultError:
    case kIROp_MakeOptionalNone:
    case kIROp_OptionalHasValue:
    case kIROp_LookupWitnessMethod:
    case kIROp_ExtractExistentialType:
    case kIROp_ExtractExistentialWitnessTable:
        return transcribeNonDiffInst(builder, origInst);

        // A call to createDynamicObject<T>(arbitraryData) cannot provide a diff value,
        // so we treat this inst as non differentiable.
        // We can extend the frontend and IR with a separate op-code that can provide an
        // explicit diff value.
        //
        // However, we can't skip this instruction since it also produces a _type_ which may be
        // used by other differentiable instructions. Therefore, we'll create another
        // existential object but with a dzero() for it's value.
        //
    case kIROp_CreateExistentialObject:
        return transcribeNonDiffInst(builder, origInst);

    case kIROp_StructKey:
        return InstPair(origInst, nullptr);

    case kIROp_Unreachable:
        {
            auto unreachInst = builder->emitUnreachable();
            builder->markInstAsMixedDifferential(unreachInst);
            return InstPair(unreachInst, nullptr);
        }

    case kIROp_MakeExistentialWithRTTI:
        SLANG_UNEXPECTED("MakeExistentialWithRTTI inst is not expected in autodiff pass.");
        break;
    }

    return InstPair(nullptr, nullptr);
}

String ForwardDiffTranscriber::makeDiffPairName(IRInst* origVar)
{
    if (auto namehintDecoration = origVar->findDecoration<IRNameHintDecoration>())
    {
        return ("dp" + String(namehintDecoration->getName()));
    }

    return String("");
}

InstPair ForwardDiffTranscriber::transcribeFuncParam(
    IRBuilder* builder,
    IRParam* origParam,
    IRInst* primalType)
{
    SLANG_UNUSED(primalType);

    auto [origParamPassingMode, origParamBaseType] =
        splitParameterDirectionAndType(origParam->getFullType());

    auto diffPairBaseType =
        (IRType*)this->differentiableTypeConformanceContext.tryGetAssociationOfKind(
            origParamBaseType,
            ValAssociationKind::DifferentialPairType);
    bool isMixedDifferential = true;

    if (!diffPairBaseType)
    {
        diffPairBaseType =
            (IRType*)this->differentiableTypeConformanceContext.tryGetAssociationOfKind(
                origParamBaseType,
                ValAssociationKind::DifferentialPtrPairType);
        if (diffPairBaseType)
            isMixedDifferential = false;
    }

    if (diffPairBaseType)
    {
        IRInst* diffPairParam = builder->emitParam(
            fromDirectionAndType(builder, origParamPassingMode, diffPairBaseType));

        if (isMixedDifferential)
            builder->markInstAsMixedDifferential(diffPairParam);
        else
            builder->markInstAsPrimal(diffPairParam);

        auto diffPairVarName = makeDiffPairName(origParam);
        if (diffPairVarName.getLength() > 0)
            builder->addNameHintDecoration(diffPairParam, diffPairVarName.getUnownedSlice());

        SLANG_ASSERT(diffPairParam);

        if (origParamPassingMode.kind == ParameterDirectionInfo::Kind::In)
        {
            auto diffType = differentiateType(builder, (IRType*)origParam->getFullType());

            // TODO: Should we be marking these as primal??
            return InstPair(
                builder->emitDifferentialPairGetPrimal(diffPairParam),
                builder->emitDifferentialPairGetDifferential(diffType, diffPairParam));
        }
        else if (
            origParamPassingMode.kind == ParameterDirectionInfo::Kind::Out ||
            origParamPassingMode.kind == ParameterDirectionInfo::Kind::BorrowInOut)
        {
            auto ptrInnerPairType = as<IRDifferentialPairTypeBase>(diffPairBaseType);
            // Make a local copy of the parameter for primal and diff parts.
            auto primal = builder->emitVar(ptrInnerPairType->getValueType());

            auto diffType = differentiateType(
                builder,
                cast<IRPtrTypeBase>(origParam->getDataType())->getValueType());
            auto diff = builder->emitVar(diffType);

            markDiffTypeInst(builder, diff, builder->getPtrType(ptrInnerPairType->getValueType()));

            IRInst* primalInitVal = nullptr;
            IRInst* diffInitVal = nullptr;
            if (origParamPassingMode.kind == ParameterDirectionInfo::Kind::Out)
            {
                // Out
                primalInitVal = builder->emitDefaultConstruct(ptrInnerPairType->getValueType());
                diffInitVal = builder->emitDefaultConstructRaw(diffType);
            }
            else
            {
                // BorrowInOut
                auto initVal = builder->emitLoad(diffPairParam);
                markDiffPairTypeInst(builder, initVal, ptrInnerPairType);

                primalInitVal = builder->emitDifferentialPairGetPrimal(initVal);
                diffInitVal = builder->emitDifferentialPairGetDifferential(diffType, initVal);
            }

            markDiffTypeInst(builder, diffInitVal, ptrInnerPairType->getValueType());

            builder->emitStore(primal, primalInitVal);

            auto diffStore = builder->emitStore(diff, diffInitVal);
            markDiffTypeInst(builder, diffStore, ptrInnerPairType->getValueType());

            mapInOutParamToWriteBackValue[diffPairParam] = InstPair(primal, diff);
            return InstPair(primal, diff);
        }
    }

    auto primalInst = cloneInst(&cloneEnv, builder, origParam);
    if (auto primalParam = as<IRParam, IRDynamicCastBehavior::NoUnwrap>(primalInst))
    {
        SLANG_RELEASE_ASSERT(builder->getInsertLoc().getBlock());
        primalParam->removeFromParent();
        builder->getInsertLoc().getBlock()->addParam(primalParam);
    }
    return InstPair(primalInst, nullptr);
}


void ForwardDiffTranscriber::mapDifferentialInst(IRInst* origInst, IRInst* diffInst)
{
    if (hasDifferentialInst(origInst))
    {
        auto existingDiffInst = lookupDiffInst(origInst);
        if (existingDiffInst != diffInst)
        {
            SLANG_UNEXPECTED("Inconsistent differential mappings");
        }
    }

    instMapD[origInst] = diffInst;
}

void ForwardDiffTranscriber::mapPrimalInst(IRInst* origInst, IRInst* primalInst)
{
    if (cloneEnv.mapOldValToNew.containsKey(origInst) &&
        cloneEnv.mapOldValToNew[origInst] != primalInst)
    {
        getSink()->diagnose(
            origInst->sourceLoc,
            Diagnostics::internalCompilerError,
            "inconsistent primal instruction for original");
    }
    else
    {
        cloneEnv.mapOldValToNew[origInst] = primalInst;
    }
}

IRInst* ForwardDiffTranscriber::lookupDiffInst(IRInst* origInst)
{
    return instMapD[origInst];
}

IRInst* ForwardDiffTranscriber::lookupDiffInst(IRInst* origInst, IRInst* defaultInst)
{
    if (auto lookupResult = instMapD.tryGetValue(origInst))
        return *lookupResult;
    return defaultInst;
}

bool ForwardDiffTranscriber::hasDifferentialInst(IRInst* origInst)
{
    if (!origInst)
        return false;
    return instMapD.containsKey(origInst);
}

bool ForwardDiffTranscriber::shouldUseOriginalAsPrimal(IRInst* currentParent, IRInst* origInst)
{
    if (as<IRGlobalValueWithCode>(origInst))
        return true;
    if (origInst->parent && origInst->parent->getOp() == kIROp_ModuleInst)
        return true;
    if (isChildInstOf(currentParent, origInst->getParent()))
        return true;

    // If origInst is defined in the first block of the same function as current inst (e.g. a
    // param), we can use it as primal. More generally, we should test if origInst dominates
    // currentParent, but that requires calculating a dom tree on the fly. Right now just testing if
    // it is first block for parameters seems sufficient.
    auto parentFunc = getParentFunc(currentParent);
    if (parentFunc && origInst->parent == parentFunc->getFirstBlock())
        return true;
    return false;
}

IRInst* ForwardDiffTranscriber::lookupPrimalInstImpl(IRInst* currentParent, IRInst* origInst)
{
    if (!origInst)
        return nullptr;
    if (shouldUseOriginalAsPrimal(currentParent, origInst))
        return origInst;
    return cloneEnv.mapOldValToNew[origInst];
}

IRInst* ForwardDiffTranscriber::lookupPrimalInst(
    IRInst* currentParent,
    IRInst* origInst,
    IRInst* defaultInst)
{
    if (!origInst)
        return nullptr;
    return (hasPrimalInst(currentParent, origInst)) ? lookupPrimalInstImpl(currentParent, origInst)
                                                    : defaultInst;
}

bool ForwardDiffTranscriber::hasPrimalInst(IRInst* currentParent, IRInst* origInst)
{
    if (!origInst)
        return false;
    if (shouldUseOriginalAsPrimal(currentParent, origInst))
        return true;
    return cloneEnv.mapOldValToNew.containsKey(origInst);
}

IRInst* ForwardDiffTranscriber::findOrTranscribeDiffInst(IRBuilder* builder, IRInst* origInst)
{
    if (!hasDifferentialInst(origInst))
    {
        transcribe(builder, origInst);
        SLANG_ASSERT(hasDifferentialInst(origInst));
    }

    return lookupDiffInst(origInst);
}


// to differentiate type (origType -> primalType -> Lookup on primalType)
IRInst* ForwardDiffTranscriber::findOrTranscribePrimalInst(IRBuilder* builder, IRInst* origInst)
{
    if (!origInst)
        return origInst;

    auto currentParent = builder->getInsertLoc().getParent();

    if (shouldUseOriginalAsPrimal(currentParent, origInst))
        return origInst;

    if (!hasPrimalInst(currentParent, origInst))
    {
        transcribe(builder, origInst);
        SLANG_ASSERT(hasPrimalInst(currentParent, origInst));
    }

    return lookupPrimalInstImpl(currentParent, origInst);
}

IRInst* ForwardDiffTranscriber::maybeCloneForPrimalInst(IRBuilder* builder, IRInst* inst)
{
    if (!inst)
        return nullptr;

    IRInst* primal = lookupPrimalInst(builder, inst, nullptr);
    if (!primal)
    {
        IRInst* type = inst->getFullType();
        if (type)
        {
            type = maybeCloneForPrimalInst(builder, type);
        }
        List<IRInst*> operands;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto operand = maybeCloneForPrimalInst(builder, inst->getOperand(i));
            operands.add(operand);
        }
        auto cloneResult = builder->emitIntrinsicInst(
            (IRType*)type,
            inst->getOp(),
            operands.getCount(),
            operands.getBuffer());
        IRBuilder subBuilder = *builder;
        subBuilder.setInsertInto(cloneResult);
        for (auto child : inst->getDecorationsAndChildren())
        {
            maybeCloneForPrimalInst(&subBuilder, child);
        }
        cloneEnv.mapOldValToNew[inst] = cloneResult;

        // Also clone any inst annotations immediately.
        traverseUsers(
            inst,
            [&](IRInst* user)
            {
                if (auto diffAnnotation = as<IRAssociatedInstAnnotation>(user))
                {
                    if (diffAnnotation->getTarget() == inst)
                        maybeCloneForPrimalInst(builder, user);
                }
            });

        return cloneResult;
    }

    return primal;
}

IRType* ForwardDiffTranscriber::getOrCreateDiffPairType(IRBuilder* builder, IRInst* originalType)
{
    auto primalType = lookupPrimalInst(builder, originalType, originalType);
    SLANG_RELEASE_ASSERT(primalType);

    auto diffValuePairType = (IRType*)differentiableTypeConformanceContext.tryGetAssociationOfKind(
        primalType,
        ValAssociationKind::DifferentialPairType);

    auto diffPtrPairType = (IRType*)differentiableTypeConformanceContext.tryGetAssociationOfKind(
        primalType,
        ValAssociationKind::DifferentialPtrPairType);

    SLANG_ASSERT(diffValuePairType || diffPtrPairType);

    return diffValuePairType ? diffValuePairType : diffPtrPairType;
}

IRInst* _lookupWitness(IRBuilder* builder, IRInst* witness, IRInst* requirementKey);

InstPair ForwardDiffTranscriber::transcribeParam(IRBuilder* builder, IRParam* origParam)
{
    auto primalDataType = findOrTranscribePrimalInst(builder, origParam->getDataType());
    // Do not differentiate generic type (and witness table) parameters
    if (isGenericParam(origParam))
    {
        return InstPair(cloneInst(&cloneEnv, builder, origParam), nullptr);
    }

    // Is this param a phi node or a function parameter?
    auto func = as<IRGlobalValueWithCode>(origParam->getParent()->getParent());
    bool isFuncParam = (func && origParam->getParent() == func->getFirstBlock());
    if (isFuncParam)
    {
        return transcribeFuncParam(builder, origParam, primalDataType);
    }
    else
    {
        auto primal = cloneInst(&cloneEnv, builder, origParam);
        IRInst* diff = nullptr;
        if (IRType* diffType = differentiateType(builder, (IRType*)origParam->getDataType()))
        {
            diff = builder->emitParam(diffType);
        }
        return InstPair(primal, diff);
    }
}


// In differential computation, the 'default' differential value is always zero.
// This is a consequence of differential computing being inherently linear. As a
// result, it's useful to have a method to generate zero literals of any (arithmetic) type.
// The current implementation requires that types are defined linearly.
//
IRInst* ForwardDiffTranscriber::getDifferentialZeroOfType(IRBuilder* builder, IRType* originalType)
{
    originalType = (IRType*)unwrapAttributedType(originalType);
    auto primalType = (IRType*)lookupPrimalInst(builder, originalType);

    // Can't generate zero for differentiable ptr types. Should never hit this case.
    SLANG_ASSERT(!differentiableTypeConformanceContext.isDifferentiablePtrType(originalType));

    if (auto diffType = differentiateType(builder, originalType))
    {
        if (isExistentialType(diffType) && !as<IRLookupWitnessMethod>(diffType))
        {
            // Emit null differential & pack it into an IDifferentiable existential.

            auto nullDiffValue = differentiableTypeConformanceContext.emitNullDifferential(builder);
            builder->markInstAsDifferential(
                nullDiffValue,
                autoDiffSharedContext->nullDifferentialStructType);

            auto nullDiffExistential = builder->emitMakeExistential(
                diffType,
                nullDiffValue,
                autoDiffSharedContext->nullDifferentialWitness);
            builder->markInstAsDifferential(nullDiffExistential, primalType);

            return nullDiffExistential;
        }

        switch (diffType->getOp())
        {
        case kIROp_DifferentialPairType:
            {
                auto makeDiffPair = builder->emitMakeDifferentialPair(
                    diffType,
                    getDifferentialZeroOfType(
                        builder,
                        as<IRDifferentialPairType>(diffType)->getValueType()),
                    getDifferentialZeroOfType(
                        builder,
                        as<IRDifferentialPairType>(diffType)->getValueType()));
                builder->markInstAsDifferential(
                    makeDiffPair,
                    as<IRDifferentialPairType>(diffType)->getValueType());
                return makeDiffPair;
            }
        }

        if (auto arrayType = as<IRArrayType>(originalType))
        {
            auto diffElementType =
                (IRType*)differentiableTypeConformanceContext.getDifferentialForType(
                    builder,
                    arrayType->getElementType());
            SLANG_RELEASE_ASSERT(diffElementType);
            auto diffArrayType =
                builder->getArrayType(diffElementType, arrayType->getElementCount());
            auto diffElementZero = getDifferentialZeroOfType(builder, arrayType->getElementType());
            auto result = builder->emitMakeArrayFromElement(diffArrayType, diffElementZero);
            builder->markInstAsDifferential(result, primalType);
            return result;
        }

        auto zeroMethod =
            differentiableTypeConformanceContext.getZeroMethodForType(builder, originalType);
        SLANG_RELEASE_ASSERT(zeroMethod);

        auto emptyArgList = List<IRInst*>();

        auto callInst = builder->emitCallInst((IRType*)diffType, zeroMethod, emptyArgList);
        builder->markInstAsDifferential(callInst, primalType);

        return callInst;
    }
    else
    {
        if (isScalarIntegerType(primalType))
        {
            return builder->getIntValue(primalType, 0);
        }

        getSink()->diagnose(
            primalType->sourceLoc,
            Diagnostics::internalCompilerError,
            "could not generate zero value for given type");
        return nullptr;
    }
}


InstPair ForwardDiffTranscriber::transcribeBlockImpl(
    IRBuilder* builder,
    IRBlock* origBlock,
    HashSet<IRInst*>& instsToSkip)
{
    IRBuilder subBuilder = *builder;
    subBuilder.setInsertLoc(builder->getInsertLoc());

    IRInst* diffBlock = lookupDiffInst(origBlock);
    SLANG_RELEASE_ASSERT(diffBlock);
    subBuilder.markInstAsMixedDifferential(diffBlock);

    subBuilder.setInsertInto(diffBlock);

    // First transcribe every parameter in the block.
    for (auto param = origBlock->getFirstParam(); param; param = param->getNextParam())
        this->transcribe(&subBuilder, param);

    // Then, run through every instruction and use the transcriber to generate the appropriate
    // derivative code.
    //
    for (auto child = origBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
    {
        if (instsToSkip.contains(child))
        {
            continue;
        }

        this->transcribe(&subBuilder, child);
    }

    return InstPair(diffBlock, diffBlock);
}

InstPair ForwardDiffTranscriber::transcribeNonDiffInst(IRBuilder* builder, IRInst* origInst)
{
    auto primal = maybeCloneForPrimalInst(builder, origInst);
    return InstPair(primal, nullptr);
}

InstPair ForwardDiffTranscriber::transcribeReturn(IRBuilder* builder, IRReturn* origReturn)
{
    IRInst* origReturnVal = origReturn->getVal();

    auto returnDataType =
        (IRType*)findOrTranscribePrimalInst(builder, origReturnVal->getDataType());
    if (as<IRFunc>(origReturnVal) || as<IRGeneric>(origReturnVal) ||
        as<IRStructType>(origReturnVal) || as<IRFuncType>(origReturnVal))
    {
        // If the return value is itself a function, generic or a struct then this
        // is likely to be a generic scope. In this case, we lookup the differential
        // and return that.
        IRInst* primalReturnVal = findOrTranscribePrimalInst(builder, origReturnVal);
        IRInst* diffReturnVal = findOrTranscribeDiffInst(builder, origReturnVal);

        // Neither of these should be nullptr.
        SLANG_RELEASE_ASSERT(primalReturnVal && diffReturnVal);
        IRReturn* diffReturn = as<IRReturn>(builder->emitReturn(diffReturnVal));
        builder->markInstAsMixedDifferential(diffReturn, nullptr);

        return InstPair(diffReturn, diffReturn);
    }
    else if (auto pairType = tryGetDiffPairType(builder, returnDataType))
    {
        IRInst* primalReturnVal = findOrTranscribePrimalInst(builder, origReturnVal);
        IRInst* diffReturnVal = findOrTranscribeDiffInst(builder, origReturnVal);
        if (!diffReturnVal)
            diffReturnVal = getDifferentialZeroOfType(builder, returnDataType);

        // If the pair type can be formed, this must be non-null.
        SLANG_RELEASE_ASSERT(diffReturnVal);

        auto diffPair = builder->emitMakeDifferentialPair(pairType, primalReturnVal, diffReturnVal);
        builder->markInstAsMixedDifferential(diffPair, pairType);

        IRReturn* pairReturn = as<IRReturn>(builder->emitReturn(diffPair));
        builder->markInstAsMixedDifferential(pairReturn, pairType);

        return InstPair(pairReturn, pairReturn);
    }
    else
    {
        // If the return type is not differentiable, emit the primal value only.
        IRInst* primalReturnVal = findOrTranscribePrimalInst(builder, origReturnVal);

        IRInst* primalReturn = builder->emitReturn(primalReturnVal);
        builder->markInstAsMixedDifferential(primalReturn, nullptr);
        return InstPair(primalReturn, nullptr);
    }
}

void handleNameHint(IRBuilder* builder, IRInst* primal, IRInst* diff)
{
    // Ignore types that already have a name hint.
    if (as<IRType>(diff) && diff->findDecoration<IRNameHintDecoration>())
        return;

    if (auto nameHint = primal->findDecoration<IRNameHintDecoration>())
    {
        StringBuilder sb;
        sb << "s_diff_" << nameHint->getName();
        builder->addNameHintDecoration(diff, sb.getUnownedSlice());
    }
}

IRInst* getActualInstToTranscribe(IRInst* inst)
{
    if (auto gen = as<IRGeneric>(inst))
    {
        auto retVal = findGenericReturnVal(gen);
        if (retVal->getOp() != kIROp_Func)
            return inst;
        if (auto primalSubst = retVal->findDecoration<IRPrimalSubstituteDecoration>())
        {
            auto spec = as<IRSpecialize>(primalSubst->getPrimalSubstituteFunc());
            SLANG_RELEASE_ASSERT(spec);
            return spec->getBase();
        }
    }
    else if (auto func = as<IRFunc>(inst))
    {
        if (auto primalSubst = func->findDecoration<IRPrimalSubstituteDecoration>())
        {
            auto actualFunc = as<IRFunc>(primalSubst->getPrimalSubstituteFunc());
            SLANG_RELEASE_ASSERT(actualFunc);
            return actualFunc;
        }
    }
    return inst;
}

IRInst* ForwardDiffTranscriber::transcribe(IRBuilder* builder, IRInst* origInst)
{
    // If a differential instruction is already mapped for
    // this original inst, return that.
    //
    if (auto diffInst = lookupDiffInst(origInst, nullptr))
    {
        SLANG_ASSERT(lookupPrimalInst(builder, origInst)); // Consistency check.
        return diffInst;
    }

    // Otherwise, dispatch to the appropriate method
    // depending on the op-code.
    //
    instsInProgress.add(origInst);
    auto actualInstToTranscribe = getActualInstToTranscribe(origInst);
    InstPair pair = transcribeInst(builder, actualInstToTranscribe);

    instsInProgress.remove(origInst);

    if (pair.primal)
    {
        mapPrimalInst(origInst, pair.primal);
        mapDifferentialInst(origInst, pair.differential);

        if (pair.primal != pair.differential &&
            !pair.primal->findDecoration<IRAutodiffInstDecoration>() &&
            !as<IRConstant>(pair.primal))
        {
            builder->markInstAsPrimal(pair.primal);
        }

        if (pair.differential)
        {
            switch (pair.differential->getOp())
            {
            case kIROp_Func:
            case kIROp_Generic:
            case kIROp_Block:
                // Don't generate again for these.
                // Functions already have their names generated in `transcribeFuncHeader`.
                break;
            default:
                // Generate name hint for the inst.
                handleNameHint(builder, pair.primal, pair.differential);

                // Automatically tag the primal and differential results
                // if they haven't already been handled by the
                // code.
                //
                if (pair.primal != pair.differential)
                {
                    if (!pair.differential->findDecoration<IRAutodiffInstDecoration>() &&
                        !as<IRConstant>(pair.differential))
                    {
                        auto primalType = (IRType*)(pair.primal->getDataType());
                        markDiffTypeInst(builder, pair.differential, primalType);
                    }
                }
                else
                {
                    if (!pair.primal->findDecoration<IRAutodiffInstDecoration>())
                    {
                        if (as<IRConstant>(pair.differential))
                            break;
                        if (as<IRType>(pair.differential))
                            break;
                        auto mixedType = (IRType*)(pair.primal->getDataType());
                        markDiffPairTypeInst(builder, pair.primal, mixedType);
                    }
                }

                break;
            }
        }

        /*
        if (pair.primal != origInst)
        {
            // If we generated a non-trivial primal inst,
            // also handle all inst annotations.
            //
            traverseUsers(
                origInst,
                [&](IRInst* user)
                {
                    if (auto diffAnnotation = as<IRDifferentiableTypeAnnotation>(user))
                    {
                        if (diffAnnotation->getBaseType() == origInst)
                            this->transcribe(builder, user);
                    }
                });
        }
        */

        return pair.differential;
    }

    getSink()->diagnose(
        origInst->sourceLoc,
        Diagnostics::internalCompilerError,
        "failed to transcibe instruction");
    return nullptr;
}

InstPair ForwardDiffTranscriber::transcribeInst(IRBuilder* builder, IRInst* origInst)
{
    // Handle instructions with children
    switch (origInst->getOp())
    {
    case kIROp_Block:
        return transcribeBlock(builder, as<IRBlock>(origInst));
    }

    // At this point we should not see any global insts that are differentiable.
    // If the inst's parent is IRModule, return (inst, null).
    //
    if (as<IRModuleInst>(origInst->getParent()) && !as<IRType>(origInst))
        return InstPair(origInst, nullptr);

    IRBuilderSourceLocRAII sourceLocationScope(builder, origInst->sourceLoc);

    auto result = transcribeInstImpl(builder, origInst);
    if (result.primal == nullptr && result.differential == nullptr)
    {
        if (auto origType = as<IRType>(origInst))
        {
            IRInst* primal = maybeCloneForPrimalInst(builder, origType);
            result = InstPair(primal, nullptr);
        }
    }

    if (result.primal == nullptr && result.differential == nullptr)
    {
        // If we reach this statement, the instruction type is likely unhandled.
        getSink()->diagnose(
            origInst->sourceLoc,
            Diagnostics::unimplemented,
            "this instruction cannot be differentiated");
    }

    return result;
}


IRInst* maybeTranslateForwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRForwardDifferentiate* inst)
{
    ForwardDiffTranscriber transcriber(sharedContext, sink);
    auto base = inst->getOperand(0);

    if (as<IRGeneric>(base))
        base = getGenericReturnVal(base);

    if (!as<IRFunc>(base))
        return inst;

    IRFunc* targetFunc = cast<IRFunc>(base);

    IRInst* fwdDiffFunc;
    IRBuilder builder(sharedContext->moduleInst);

    builder.setInsertAfter(targetFunc);
    transcriber._transcribeFuncImpl(&builder, targetFunc, fwdDiffFunc);

    // TODO: Should hoist outside..
    return fwdDiffFunc;
}

IRInst* maybeTranslateTrivialForwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRTrivialForwardDifferentiate* inst)
{
    ForwardDiffTranscriber transcriber(sharedContext, sink);
    auto base = inst->getOperand(0);

    if (as<IRGeneric>(base))
        base = getGenericReturnVal(base);

    if (!as<IRFunc>(base))
        return inst;

    IRFunc* targetFunc = cast<IRFunc>(base);

    IRBuilder builder(sharedContext->moduleInst);

    builder.setInsertAfter(targetFunc);

    IRInst* fwdDiffFunc = builder.createFunc();
    IRInst* typeOperand = targetFunc->getFullType();
    fwdDiffFunc->setFullType(transcriber.differentiableTypeConformanceContext.resolveType(
        &builder,
        builder
            .emitIntrinsicInst(builder.getTypeKind(), kIROp_ForwardDiffFuncType, 1, &typeOperand)));
    transcriber.generateTrivialFwdDiffFunc(targetFunc, cast<IRFunc>(fwdDiffFunc));

    return fwdDiffFunc;
}

IRInst* maybeTranslateForwardDerivativeWitness(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRSynthesizedForwardDerivativeWitnessTable* translateInst)
{
    auto fwdDiffWitnessSynInst = as<IRSynthesizedForwardDerivativeWitnessTable>(translateInst);
    auto fwdDiffFunc = fwdDiffWitnessSynInst->getOperand(0);

    auto baseConformanceType = cast<IRInterfaceType>(
        getGenericReturnVal(sharedContext->forwardDifferentiableInterfaceType));

    IRBuilder builder(sharedContext->moduleInst);
    builder.setInsertAfter(translateInst);
    auto newWitnessTable = builder.createWitnessTable(
        (IRType*)cast<IRWitnessTableType>(fwdDiffWitnessSynInst->getDataType())
            ->getConformanceType(),
        (IRType*)fwdDiffFunc);

    // For now, we're going to hardcode the fact that the IForwardDifferentiable interface
    // always contains exactly 3 requirements, the fwd_diff and its two derivatives.
    //
    SLANG_ASSERT(baseConformanceType->getRequirementCount() == 3);
    IRInst* fwdDiffFuncTypeOperand = fwdDiffFunc->getFullType();
    auto higherOrderfwdDiffFn = builder.emitForwardDifferentiateInst(
        (IRType*)builder.emitIntrinsicInst(
            builder.getTypeKind(),
            kIROp_ForwardDiffFuncType,
            1,
            &fwdDiffFuncTypeOperand),
        fwdDiffFunc);
    builder.createWitnessTableEntry(
        newWitnessTable,
        as<IRInterfaceRequirementEntry>(baseConformanceType->getOperand(0))->getRequirementKey(),
        higherOrderfwdDiffFn);

    // Add synthetic witness inst to further generate higher order derivatives if necessary.
    auto higherOrderFwdDiffFwdWitness = builder.emitIntrinsicInst(
        (IRType*)builder.getWitnessTableType((IRType*)builder.emitSpecializeInst(
            builder.getTypeKind(),
            sharedContext->forwardDifferentiableInterfaceType,
            List<IRInst*>(higherOrderfwdDiffFn))),
        kIROp_SynthesizedForwardDerivativeWitnessTable,
        1,
        &higherOrderfwdDiffFn);
    builder.createWitnessTableEntry(
        newWitnessTable,
        as<IRInterfaceRequirementEntry>(baseConformanceType->getOperand(1))->getRequirementKey(),
        higherOrderFwdDiffFwdWitness);

    auto higherOrderFwdDiffBwdWitness = builder.emitIntrinsicInst(
        (IRType*)builder.emitSpecializeInst(
            builder.getTypeKind(),
            sharedContext->backwardDifferentiableInterfaceType,
            List<IRInst*>(higherOrderfwdDiffFn)),
        kIROp_SynthesizedBackwardDerivativeWitnessTable,
        1,
        &higherOrderfwdDiffFn);
    builder.createWitnessTableEntry(
        newWitnessTable,
        as<IRInterfaceRequirementEntry>(baseConformanceType->getOperand(2))->getRequirementKey(),
        higherOrderFwdDiffBwdWitness);

    List<IRInst*> args = {
        fwdDiffFunc,
        builder.getIntValue((int)ValAssociationKind::ForwardDerivative),
        higherOrderfwdDiffFn};

    // Also emit annotations.
    builder.emitIntrinsicInst(
        (IRType*)builder.getVoidType(),
        kIROp_AssociatedInstAnnotation,
        args.getCount(),
        args.getBuffer());

    return newWitnessTable;
}

IRInst* maybeTranslateBackwardDerivativeWitness(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRSynthesizedBackwardDerivativeWitnessTable* translateInst)
{
    auto bwdDiffWitnessSynInst = as<IRSynthesizedBackwardDerivativeWitnessTable>(translateInst);
    auto baseFunc = bwdDiffWitnessSynInst->getOperand(0);
    auto baseConformanceType = cast<IRInterfaceType>(
        getGenericReturnVal(sharedContext->backwardDifferentiableInterfaceType));

    IRBuilder builder(sharedContext->moduleInst);
    builder.setInsertAfter(translateInst);
    auto newWitnessTable = builder.createWitnessTable(
        (IRType*)cast<IRWitnessTableType>(bwdDiffWitnessSynInst->getDataType())
            ->getConformanceType(),
        (IRType*)baseFunc);

    // For now, we're going to hardcode the fact that the IBackwardDifferentiable interface
    // always contains exactly 3 requirements, the context-type, context-type : IBackwardCallable,
    // apply and bwd_diff (legacy)
    //
    // This will help us catch errors if the interface definition changes.
    //
    SLANG_ASSERT(baseConformanceType->getRequirementCount() == 4);
    IRInst* typeOperand = baseFunc->getFullType();

    auto contextType = builder.emitIntrinsicInst(
        builder.getTypeKind(),
        kIROp_BackwardDiffIntermediateContextType,
        1,
        &baseFunc);
    builder.createWitnessTableEntry(
        newWitnessTable,
        as<IRInterfaceRequirementEntry>(baseConformanceType->getOperand(0))->getRequirementKey(),
        contextType);
    {
        auto callableConformanceBaseType = cast<IRInterfaceType>(
            getGenericReturnVal(sharedContext->backwardCallableInterfaceType));
        SLANG_ASSERT(callableConformanceBaseType->getRequirementCount() == 2);

        auto callableConformanceType = builder.emitSpecializeInst(
            builder.getTypeKind(),
            sharedContext->backwardCallableInterfaceType,
            List<IRInst*>(baseFunc));
        auto callableWitnessTable =
            builder.createWitnessTable((IRType*)callableConformanceType, (IRType*)contextType);

        IRInst* propFuncOperands[] = {typeOperand, contextType};
        auto propFunc = builder.emitIntrinsicInst(
            (IRType*)builder.emitIntrinsicInst(
                builder.getTypeKind(),
                kIROp_BwdCallableFuncType,
                2,
                propFuncOperands),
            kIROp_BackwardDifferentiatePropagate,
            1,
            &baseFunc);
        builder.createWitnessTableEntry(
            callableWitnessTable,
            as<IRInterfaceRequirementEntry>(callableConformanceBaseType->getOperand(0))
                ->getRequirementKey(),
            propFunc);

        IRInst* getValFuncOperands[] = {typeOperand};
        auto getValFunc = builder.emitIntrinsicInst(
            (IRType*)builder.emitIntrinsicInst(
                builder.getTypeKind(),
                kIROp_FuncResultType,
                1,
                getValFuncOperands),
            kIROp_BackwardContextGetPrimalVal,
            1,
            &baseFunc);
        builder.createWitnessTableEntry(
            callableWitnessTable,
            as<IRInterfaceRequirementEntry>(callableConformanceBaseType->getOperand(1))
                ->getRequirementKey(),
            getValFunc);

        builder.createWitnessTableEntry(
            newWitnessTable,
            as<IRInterfaceRequirementEntry>(baseConformanceType->getOperand(1))
                ->getRequirementKey(),
            callableWitnessTable);
    }

    {
        IRInst* applyFuncTypeOperands[] = {typeOperand, contextType};
        auto applyFunc = builder.emitBackwardDifferentiatePrimalInst(
            (IRType*)builder.emitIntrinsicInst(
                builder.getTypeKind(),
                kIROp_ApplyForBwdFuncType,
                2,
                applyFuncTypeOperands),
            baseFunc);
        builder.createWitnessTableEntry(
            newWitnessTable,
            as<IRInterfaceRequirementEntry>(baseConformanceType->getOperand(2))
                ->getRequirementKey(),
            applyFunc);
    }

    {
        // This should never be required, so we just emit a poison value.
        builder.createWitnessTableEntry(
            newWitnessTable,
            as<IRInterfaceRequirementEntry>(baseConformanceType->getOperand(3))
                ->getRequirementKey(),
            builder.emitPoison(builder.getVoidType()));
    }

    return newWitnessTable;
}

} // namespace Slang
