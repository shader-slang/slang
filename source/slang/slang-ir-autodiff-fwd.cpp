// slang-ir-autodiff-fwd.cpp
#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-fwd.h"

#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-util.h"
#include "slang-ir-inst-pass-base.h"

namespace Slang
{

static IRInst* _unwrapAttributedType(IRInst* type)
{
    while (auto attrType = as<IRAttributedType>(type))
        type = attrType->getBaseType();
    return type;
}

DiagnosticSink* ForwardDerivativeTranscriber::getSink()
{
    SLANG_ASSERT(sink);
    return sink;
}

void ForwardDerivativeTranscriber::mapDifferentialInst(IRInst* origInst, IRInst* diffInst)
{
    if (hasDifferentialInst(origInst))
    {
        if (lookupDiffInst(origInst) != diffInst)
        {
            SLANG_UNEXPECTED("Inconsistent differential mappings");
        }
    }
    else
    {
        instMapD.Add(origInst, diffInst);
    }
}

void ForwardDerivativeTranscriber::mapPrimalInst(IRInst* origInst, IRInst* primalInst)
{
    if (cloneEnv.mapOldValToNew.ContainsKey(origInst) && cloneEnv.mapOldValToNew[origInst] != primalInst)
    {
        getSink()->diagnose(origInst->sourceLoc,
            Diagnostics::internalCompilerError,
            "inconsistent primal instruction for original");
    }
    else
    {
        cloneEnv.mapOldValToNew[origInst] = primalInst;
    }
}

IRInst* ForwardDerivativeTranscriber::lookupDiffInst(IRInst* origInst)
{
    return instMapD[origInst];
}

IRInst* ForwardDerivativeTranscriber::lookupDiffInst(IRInst* origInst, IRInst* defaultInst)
{
    return (hasDifferentialInst(origInst)) ? instMapD[origInst] : defaultInst;
}

bool ForwardDerivativeTranscriber::hasDifferentialInst(IRInst* origInst)
{
    return instMapD.ContainsKey(origInst);
}

bool ForwardDerivativeTranscriber::shouldUseOriginalAsPrimal(IRInst* origInst)
{
    if (as<IRGlobalValueWithCode>(origInst))
        return true;
    if (origInst->parent && origInst->parent->getOp() == kIROp_Module)
        return true;
    return false;
}

IRInst* ForwardDerivativeTranscriber::lookupPrimalInst(IRInst* origInst)
{
    if (!origInst)
        return nullptr;
    if (shouldUseOriginalAsPrimal(origInst))
        return origInst;
    return cloneEnv.mapOldValToNew[origInst];
}

IRInst* ForwardDerivativeTranscriber::lookupPrimalInst(IRInst* origInst, IRInst* defaultInst)
{
    if (!origInst)
        return nullptr;
    return (hasPrimalInst(origInst)) ? lookupPrimalInst(origInst) : defaultInst;
}

bool ForwardDerivativeTranscriber::hasPrimalInst(IRInst* origInst)
{
    if (!origInst)
        return true;
    if (shouldUseOriginalAsPrimal(origInst))
        return true;
    return cloneEnv.mapOldValToNew.ContainsKey(origInst);
}

IRInst* ForwardDerivativeTranscriber::findOrTranscribeDiffInst(IRBuilder* builder, IRInst* origInst)
{
    if (!hasDifferentialInst(origInst))
    {
        transcribe(builder, origInst);
        SLANG_ASSERT(hasDifferentialInst(origInst));
    }

    return lookupDiffInst(origInst);
}

IRInst* ForwardDerivativeTranscriber::findOrTranscribePrimalInst(IRBuilder* builder, IRInst* origInst)
{
    if (shouldUseOriginalAsPrimal(origInst))
        return origInst;

    if (!hasPrimalInst(origInst))
    {
        transcribe(builder, origInst);
        SLANG_ASSERT(hasPrimalInst(origInst));
    }

    return lookupPrimalInst(origInst);
}

IRFuncType* ForwardDerivativeTranscriber::differentiateFunctionType(IRBuilder* builder, IRFuncType* funcType)
{
    List<IRType*> newParameterTypes;
    IRType* diffReturnType;

    for (UIndex i = 0; i < funcType->getParamCount(); i++)
    {
        auto origType = funcType->getParamType(i);
        origType = (IRType*) lookupPrimalInst(origType, origType);
        if (auto diffPairType = tryGetDiffPairType(builder, origType))
            newParameterTypes.add(diffPairType);
        else
            newParameterTypes.add(origType);
    }

    // Transcribe return type to a pair.
    // This will be void if the primal return type is non-differentiable.
    //
    auto origResultType = (IRType*) lookupPrimalInst(funcType->getResultType(), funcType->getResultType());
    if (auto returnPairType = tryGetDiffPairType(builder, origResultType))
        diffReturnType = returnPairType;
    else
        diffReturnType = origResultType;

    return builder->getFuncType(newParameterTypes, diffReturnType);
}

// Get or construct `:IDifferentiable` conformance for a DifferentiablePair.
IRWitnessTable* ForwardDerivativeTranscriber::getDifferentialPairWitness(IRInst* inDiffPairType)
{
    IRBuilder builder(sharedBuilder);
    builder.setInsertInto(inDiffPairType->parent);
    auto diffPairType = as<IRDifferentialPairType>(inDiffPairType);
    SLANG_ASSERT(diffPairType);

    auto table = builder.createWitnessTable(autoDiffSharedContext->differentiableInterfaceType, diffPairType);

    // Differentiate the pair type to get it's differential (which is itself a pair)
    auto diffDiffPairType = differentiateType(&builder, diffPairType);

    // And place it in the synthesized witness table.
    builder.createWitnessTableEntry(table, autoDiffSharedContext->differentialAssocTypeStructKey, diffDiffPairType);
    // Omit the method synthesis here, since we can just intercept those directly at `getXXMethodForType`.

    // Record this in the context for future lookups
    differentiableTypeConformanceContext.differentiableWitnessDictionary[diffPairType] = table;

    return table;
}

IRType* ForwardDerivativeTranscriber::getOrCreateDiffPairType(IRInst* primalType, IRInst* witness)
{
    IRBuilder builder(sharedBuilder);
    builder.setInsertInto(primalType->parent);
    return builder.getDifferentialPairType(
        (IRType*)primalType,
        witness);
}

IRType* ForwardDerivativeTranscriber::getOrCreateDiffPairType(IRInst* primalType)
{
    IRBuilder builder(sharedBuilder);
    if (!primalType->next)
        builder.setInsertInto(primalType->parent);
    else
        builder.setInsertBefore(primalType->next);

    IRInst* witness = as<IRWitnessTable>(
        differentiableTypeConformanceContext.lookUpConformanceForType((IRType*)primalType));

    if (!witness)
    {
        if (auto primalPairType = as<IRDifferentialPairType>(primalType))
        {
            witness = getDifferentialPairWitness(primalPairType);
        }
        else if (auto extractExistential = as<IRExtractExistentialType>(primalType))
        {
            differentiateExtractExistentialType(&builder, extractExistential, witness);
        }
    }

    return builder.getDifferentialPairType(
        (IRType*)primalType,
        witness);
}

IRType* ForwardDerivativeTranscriber::differentiateType(IRBuilder* builder, IRType* origType)
{
    IRInst* diffType = nullptr;
    if (!instMapD.TryGetValue(origType, diffType))
    {
        diffType = _differentiateTypeImpl(builder, origType);
        instMapD[origType] = diffType;
    }
    return (IRType*)diffType;
}

IRType* ForwardDerivativeTranscriber::_differentiateTypeImpl(IRBuilder* builder, IRType* origType)
{
    if (auto ptrType = as<IRPtrTypeBase>(origType))
        return builder->getPtrType(
            origType->getOp(),
            differentiateType(builder, ptrType->getValueType()));

    // If there is an explicit primal version of this type in the local scope, load that
    // otherwise use the original type. 
    //
    IRInst* primalType = lookupPrimalInst(origType, origType);
    
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

    case kIROp_ExtractExistentialType:
        {
            IRInst* wt = nullptr;
            return differentiateExtractExistentialType(builder, as<IRExtractExistentialType>(primalType), wt);
        }

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

    // Given an interface type, return the lookup path from a witness table of `type` to a witness table of `IDifferentiable`.
bool _findDifferentiableInterfaceLookupPathImpl(
    HashSet<IRInst*>& processedTypes,
    IRInterfaceType* idiffType,
    IRInterfaceType* type,
    List<IRInterfaceRequirementEntry*>& currentPath)
{
    if (processedTypes.Contains(type))
        return false;
    processedTypes.Add(type);

    List<IRInterfaceRequirementEntry*> lookupKeyPath;
    for (UInt i = 0; i < type->getOperandCount(); i++)
    {
        auto entry = as<IRInterfaceRequirementEntry>(type->getOperand(i));
        if (!entry) continue;
        if (auto wt = as<IRWitnessTableTypeBase>(entry->getRequirementVal()))
        {
            currentPath.add(entry);
            if (wt->getConformanceType() == idiffType)
            {
                return true;
            }
            else if (auto subInterfaceType = as<IRInterfaceType>(wt->getConformanceType()))
            {
                if (_findDifferentiableInterfaceLookupPathImpl(processedTypes, idiffType, subInterfaceType, currentPath))
                    return true;
            }
            currentPath.removeLast();
        }
    }
    return false;
}

List<IRInterfaceRequirementEntry*> _findDifferentiableInterfaceLookupPath(
    IRInterfaceType* idiffType,
    IRInterfaceType* type)
{
    List<IRInterfaceRequirementEntry*> currentPath;
    HashSet<IRInst*> processedTypes;
    _findDifferentiableInterfaceLookupPathImpl(processedTypes, idiffType, type, currentPath);
    return currentPath;
}

IRType* ForwardDerivativeTranscriber::differentiateExtractExistentialType(IRBuilder* builder, IRExtractExistentialType* origType, IRInst*& witnessTable)
{
    witnessTable = nullptr;

    // Search for IDifferentiable conformance.
    auto interfaceType = as<IRInterfaceType>(_unwrapAttributedType(origType->getOperand(0)->getDataType()));
    if (!interfaceType)
        return nullptr;
    List<IRInterfaceRequirementEntry*> lookupKeyPath = _findDifferentiableInterfaceLookupPath(
        autoDiffSharedContext->differentiableInterfaceType, interfaceType);

    if (lookupKeyPath.getCount())
    {
        // `interfaceType` does conform to `IDifferentiable`.
        witnessTable = builder->emitExtractExistentialWitnessTable(origType->getOperand(0));
        for (auto node : lookupKeyPath)
        {
            witnessTable = builder->emitLookupInterfaceMethodInst((IRType*)node->getRequirementVal(), witnessTable, node->getRequirementKey());
        }
        auto diffType = builder->emitLookupInterfaceMethodInst(builder->getTypeType(), witnessTable, autoDiffSharedContext->differentialAssocTypeStructKey);
        return (IRType*)diffType;
    }
    return nullptr;
}

IRType* ForwardDerivativeTranscriber::tryGetDiffPairType(IRBuilder* builder, IRType* primalType)
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

InstPair ForwardDerivativeTranscriber::transcribeParam(IRBuilder* builder, IRParam* origParam)
{
    auto primalDataType = lookupPrimalInst(origParam->getDataType(), origParam->getDataType());
    // Do not differentiate generic type (and witness table) parameters
    if (as<IRTypeType>(primalDataType) || as<IRWitnessTableType>(primalDataType))
    {
        return InstPair(
            cloneInst(&cloneEnv, builder, origParam),
            nullptr);    
    }

    // Is this param a phi node or a function parameter?
    auto func = as<IRGlobalValueWithCode>(origParam->getParent()->getParent());
    bool isFuncParam = (func && origParam->getParent() == func->getFirstBlock());
    if (isFuncParam)
    {
        if (auto diffPairType = tryGetDiffPairType(builder, (IRType*)primalDataType))
        {
            IRInst* diffPairParam = builder->emitParam(diffPairType);

            auto diffPairVarName = makeDiffPairName(origParam);
            if (diffPairVarName.getLength() > 0)
                builder->addNameHintDecoration(diffPairParam, diffPairVarName.getUnownedSlice());

            SLANG_ASSERT(diffPairParam);
        
            if (auto pairType = as<IRDifferentialPairType>(diffPairType))
            {
                return InstPair(
                    builder->emitDifferentialPairGetPrimal(diffPairParam),
                    builder->emitDifferentialPairGetDifferential(
                        (IRType*)pairBuilder->getDiffTypeFromPairType(builder, pairType),
                        diffPairParam));
            }
            else if (auto pairPtrType = as<IRPtrTypeBase>(diffPairType))
            {
                auto ptrInnerPairType = as<IRDifferentialPairType>(pairPtrType->getValueType());

                return InstPair(
                    builder->emitDifferentialPairAddressPrimal(diffPairParam),
                    builder->emitDifferentialPairAddressDifferential(
                        builder->getPtrType(
                            kIROp_PtrType,
                            (IRType*)pairBuilder->getDiffTypeFromPairType(builder, ptrInnerPairType)),
                        diffPairParam));
            }
        }

        auto primalInst = cloneInst(&cloneEnv, builder, origParam);
        if (auto primalParam = as<IRParam>(primalInst))
        {
            SLANG_RELEASE_ASSERT(builder->getInsertLoc().getBlock());
            primalParam->removeFromParent();
            builder->getInsertLoc().getBlock()->addParam(primalParam);
        }
        return InstPair(primalInst, nullptr);
    }
    else
    {
        auto primal = cloneInst(&cloneEnv, builder, origParam);
        IRInst* diff = nullptr;
        if (IRType* diffType = differentiateType(builder, (IRType*)primalDataType))
        {
            diff = builder->emitParam(diffType);
        }
        return InstPair(primal, diff);
    }
}

// Returns "d<var-name>" to use as a name hint for variables and parameters.
// If no primal name is available, returns a blank string.
// 
String ForwardDerivativeTranscriber::getJVPVarName(IRInst* origVar)
{
    if (auto namehintDecoration = origVar->findDecoration<IRNameHintDecoration>())
    {
        return ("d" + String(namehintDecoration->getName()));
    }

    return String("");
}

// Returns "dp<var-name>" to use as a name hint for parameters.
// If no primal name is available, returns a blank string.
// 
String ForwardDerivativeTranscriber::makeDiffPairName(IRInst* origVar)
{
    if (auto namehintDecoration = origVar->findDecoration<IRNameHintDecoration>())
    {
        return ("dp" + String(namehintDecoration->getName()));
    }

    return String("");
}

InstPair ForwardDerivativeTranscriber::transcribeVar(IRBuilder* builder, IRVar* origVar)
{
    if (IRType* diffType = differentiateType(builder, origVar->getDataType()->getValueType()))
    {
        IRVar* diffVar = builder->emitVar(diffType);
        SLANG_ASSERT(diffVar);

        auto diffNameHint = getJVPVarName(origVar);
        if (diffNameHint.getLength() > 0)
            builder->addNameHintDecoration(diffVar, diffNameHint.getUnownedSlice());

        return InstPair(cloneInst(&cloneEnv, builder, origVar), diffVar);
    }

    return InstPair(cloneInst(&cloneEnv, builder, origVar), nullptr);
}

InstPair ForwardDerivativeTranscriber::transcribeBinaryArith(IRBuilder* builder, IRInst* origArith)
{
    SLANG_ASSERT(origArith->getOperandCount() == 2);

    IRInst* primalArith = cloneInst(&cloneEnv, builder, origArith);
    
    auto origLeft = origArith->getOperand(0);
    auto origRight = origArith->getOperand(1);

    auto primalLeft = findOrTranscribePrimalInst(builder, origLeft);
    auto primalRight = findOrTranscribePrimalInst(builder, origRight);

    auto diffLeft = findOrTranscribeDiffInst(builder, origLeft);
    auto diffRight = findOrTranscribeDiffInst(builder, origRight);


    if (diffLeft || diffRight)
    {
        diffLeft = diffLeft ? diffLeft : getDifferentialZeroOfType(builder, primalLeft->getDataType());
        diffRight = diffRight ? diffRight : getDifferentialZeroOfType(builder, primalRight->getDataType());

        auto resultType = primalArith->getDataType();
        switch(origArith->getOp())
        {
        case kIROp_Add:
            return InstPair(primalArith, builder->emitAdd(resultType, diffLeft, diffRight));
        case kIROp_Mul:
            return InstPair(primalArith, builder->emitAdd(resultType,
                builder->emitMul(resultType, diffLeft, primalRight),
                builder->emitMul(resultType, primalLeft, diffRight)));
        case kIROp_Sub:
            return InstPair(primalArith, builder->emitSub(resultType, diffLeft, diffRight));
        case kIROp_Div:
            return InstPair(primalArith, builder->emitDiv(resultType, 
                builder->emitSub(
                    resultType,
                    builder->emitMul(resultType, diffLeft, primalRight),
                    builder->emitMul(resultType, primalLeft, diffRight)),
                builder->emitMul(
                    primalRight->getDataType(), primalRight, primalRight
                )));
        default:
            getSink()->diagnose(origArith->sourceLoc,
                Diagnostics::unimplemented,
                "this arithmetic instruction cannot be differentiated");
        }
    }

    return InstPair(primalArith, nullptr);
}


InstPair ForwardDerivativeTranscriber::transcribeBinaryLogic(IRBuilder* builder, IRInst* origLogic)
{
    SLANG_ASSERT(origLogic->getOperandCount() == 2);

    // TODO: Check other boolean cases.
    if (as<IRBoolType>(origLogic->getDataType()))
    {
        // Boolean operations are not differentiable. For the linearization
        // pass, we do not need to do anything but copy them over to the ne
        // function.
        auto primalLogic = cloneInst(&cloneEnv, builder, origLogic);
        return InstPair(primalLogic, nullptr);
    }
    
    SLANG_UNEXPECTED("Logical operation with non-boolean result");
}

InstPair ForwardDerivativeTranscriber::transcribeLoad(IRBuilder* builder, IRLoad* origLoad)
{
    auto origPtr = origLoad->getPtr();
    auto primalPtr = lookupPrimalInst(origPtr, nullptr);
    auto primalPtrValueType = as<IRPtrTypeBase>(primalPtr->getFullType())->getValueType();

    if (auto diffPairType = as<IRDifferentialPairType>(primalPtrValueType))
    {
        // Special case load from an `out` param, which will not have corresponding `diff` and
        // `primal` insts yet.
        
        auto load = builder->emitLoad(primalPtr);
        auto primalElement = builder->emitDifferentialPairGetPrimal(load);
        auto diffElement = builder->emitDifferentialPairGetDifferential(
            (IRType*)pairBuilder->getDiffTypeFromPairType(builder, diffPairType), load);
        return InstPair(primalElement, diffElement);
    }

    auto primalLoad = cloneInst(&cloneEnv, builder, origLoad);
    IRInst* diffLoad = nullptr;
    if (auto diffPtr = lookupDiffInst(origPtr, nullptr))
    {
        // Default case, we're loading from a known differential inst.
        diffLoad = as<IRLoad>(builder->emitLoad(diffPtr));
    } 
    return InstPair(primalLoad, diffLoad);
}

InstPair ForwardDerivativeTranscriber::transcribeStore(IRBuilder* builder, IRStore* origStore)
{
    IRInst* origStoreLocation = origStore->getPtr();
    IRInst* origStoreVal = origStore->getVal();
    auto primalStoreLocation = lookupPrimalInst(origStoreLocation, nullptr);
    auto diffStoreLocation = lookupDiffInst(origStoreLocation, nullptr);
    auto primalStoreVal = lookupPrimalInst(origStoreVal, nullptr);
    auto diffStoreVal = lookupDiffInst(origStoreVal, nullptr);

    if (!diffStoreLocation)
    {
        auto primalLocationPtrType = as<IRPtrTypeBase>(primalStoreLocation->getDataType());
        if (auto diffPairType = as<IRDifferentialPairType>(primalLocationPtrType->getValueType()))
        {
            auto valToStore = builder->emitMakeDifferentialPair(diffPairType, primalStoreVal, diffStoreVal);
            auto store = builder->emitStore(primalStoreLocation, valToStore);
            return InstPair(store, nullptr);
        }
    }

    auto primalStore = cloneInst(&cloneEnv, builder, origStore);

    IRInst* diffStore = nullptr;

    // If the stored value has a differential version, 
    // emit a store instruction for the differential parameter.
    // Otherwise, emit nothing since there's nothing to load.
    // 
    if (diffStoreLocation && diffStoreVal)
    {
        // Default case, storing the entire type (and not a member)
        diffStore = as<IRStore>(
            builder->emitStore(diffStoreLocation, diffStoreVal));
        
        return InstPair(primalStore, diffStore);
    }

    return InstPair(primalStore, nullptr);
}

InstPair ForwardDerivativeTranscriber::transcribeReturn(IRBuilder* builder, IRReturn* origReturn)
{
    IRInst* origReturnVal = origReturn->getVal();

    auto returnDataType = (IRType*) lookupPrimalInst(origReturnVal->getDataType(), origReturnVal->getDataType());
    if (as<IRFunc>(origReturnVal) || as<IRGeneric>(origReturnVal) || as<IRStructType>(origReturnVal) || as<IRFuncType>(origReturnVal))
    {
        // If the return value is itself a function, generic or a struct then this
        // is likely to be a generic scope. In this case, we lookup the differential
        // and return that.
        IRInst* primalReturnVal = findOrTranscribePrimalInst(builder, origReturnVal);
        IRInst* diffReturnVal = findOrTranscribeDiffInst(builder, origReturnVal);
        
        // Neither of these should be nullptr.
        SLANG_RELEASE_ASSERT(primalReturnVal && diffReturnVal);
        IRReturn* diffReturn = as<IRReturn>(builder->emitReturn(diffReturnVal));

        return InstPair(diffReturn, diffReturn);
    }
    else if (auto pairType = tryGetDiffPairType(builder, returnDataType))
    {   
        IRInst* primalReturnVal = findOrTranscribePrimalInst(builder, origReturnVal);
        IRInst* diffReturnVal = findOrTranscribeDiffInst(builder, origReturnVal);
        if(!diffReturnVal)
            diffReturnVal = getDifferentialZeroOfType(builder, returnDataType);

        // If the pair type can be formed, this must be non-null.
        SLANG_RELEASE_ASSERT(diffReturnVal);

        auto diffPair = builder->emitMakeDifferentialPair(pairType, primalReturnVal, diffReturnVal);
        IRReturn* pairReturn = as<IRReturn>(builder->emitReturn(diffPair));
        return InstPair(pairReturn, pairReturn);
    }
    else
    {
        // If the return type is not differentiable, emit the primal value only.
        IRInst* primalReturnVal = findOrTranscribePrimalInst(builder, origReturnVal);

        IRInst* primalReturn = builder->emitReturn(primalReturnVal);
        return InstPair(primalReturn, nullptr);
        
    }
}

// Since int/float literals are sometimes nested inside an IRConstructor
// instruction, we check to make sure that the nested instr is a constant
// and then return nullptr. Literals do not need to be differentiated.
//
InstPair ForwardDerivativeTranscriber::transcribeConstruct(IRBuilder* builder, IRInst* origConstruct)
{   
    IRInst* primalConstruct = cloneInst(&cloneEnv, builder, origConstruct);
    
    // Check if the output type can be differentiated. If it cannot be 
    // differentiated, don't differentiate the inst
    // 
    auto primalConstructType = (IRType*) lookupPrimalInst(origConstruct->getDataType(), origConstruct->getDataType());
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
                operandDataType = (IRType*) lookupPrimalInst(operandDataType, operandDataType);
                diffOperands.add(getDifferentialZeroOfType(builder, operandDataType));
            }
        }
        
        return InstPair(
            primalConstruct, 
            builder->emitIntrinsicInst(
                diffConstructType,
                origConstruct->getOp(),
                operandCount,
                diffOperands.getBuffer()));
    }
    else
    {
        return InstPair(primalConstruct, nullptr);
    }
}

// Differentiating a call instruction here is primarily about generating
// an appropriate call list based on whichever parameters have differentials 
// in the current transcription context.
// 
InstPair ForwardDerivativeTranscriber::transcribeCall(IRBuilder* builder, IRCall* origCall)
{   
    
    IRInst* origCallee = origCall->getCallee();

    if (!origCallee)
    {
        // Note that this can only happen if the callee is a result
        // of a higher-order operation. For now, we assume that we cannot
        // differentiate such calls safely.
        // TODO(sai): Should probably get checked in the front-end.
        //
        getSink()->diagnose(origCall->sourceLoc,
            Diagnostics::internalCompilerError,
            "attempting to differentiate unresolved callee");
        
        return InstPair(nullptr, nullptr);
    }

    // Since concrete functions are globals, the primal callee is the same
    // as the original callee.
    //
    auto primalCallee = origCallee;

    IRInst* diffCallee = nullptr;

    if (instMapD.TryGetValue(origCallee, diffCallee))
    {
    }
    else if (auto derivativeReferenceDecor = primalCallee->findDecoration<IRForwardDerivativeDecoration>())
    {
        // If the user has already provided an differentiated implementation, use that.
        diffCallee = derivativeReferenceDecor->getForwardDerivativeFunc();
    }
    else if (primalCallee->findDecoration<IRForwardDifferentiableDecoration>())
    {
        // If the function is marked for auto-diff, push a `differentiate` inst for a follow up pass
        // to generate the implementation.
        diffCallee = builder->emitForwardDifferentiateInst(
            differentiateFunctionType(builder, as<IRFuncType>(primalCallee->getFullType())),
            primalCallee);
    }

    if (!diffCallee)
    {
        // The callee is non differentiable, just return primal value with null diff value.
        IRInst* primalCall = cloneInst(&cloneEnv, builder, origCall);
        return InstPair(primalCall, nullptr);
    }

    auto calleeType = as<IRFuncType>(diffCallee->getDataType());
    SLANG_ASSERT(calleeType);
    SLANG_RELEASE_ASSERT(calleeType->getParamCount() == origCall->getArgCount());

    List<IRInst*> args;
    // Go over the parameter list and create pairs for each input (if required)
    for (UIndex ii = 0; ii < origCall->getArgCount(); ii++)
    {
        auto origArg = origCall->getArg(ii);
        auto primalArg = findOrTranscribePrimalInst(builder, origArg);
        SLANG_ASSERT(primalArg);

        auto primalType = primalArg->getDataType();
        auto paramType = calleeType->getParamType(ii);
        if (!isNoDiffType(paramType))
        {
            if (isNoDiffType(primalType))
            {
                while (auto attrType = as<IRAttributedType>(primalType))
                    primalType = attrType->getBaseType();
            }
            if (auto pairType = tryGetDiffPairType(builder, primalType))
            {
                auto diffArg = findOrTranscribeDiffInst(builder, origArg);
                if (!diffArg)
                    diffArg = getDifferentialZeroOfType(builder, primalType);

                // If a pair type can be formed, this must be non-null.
                SLANG_RELEASE_ASSERT(diffArg);
                auto diffPair = builder->emitMakeDifferentialPair(pairType, primalArg, diffArg);
                args.add(diffPair);
                continue;
            }
        }
        // Argument is not differentiable.
        // Add original/primal argument.
        args.add(primalArg);
    }
        
    IRType* diffReturnType = nullptr;
    diffReturnType = tryGetDiffPairType(builder, origCall->getFullType());

    if (!diffReturnType)
    {
        SLANG_RELEASE_ASSERT(origCall->getFullType()->getOp() == kIROp_VoidType);
        diffReturnType = builder->getVoidType();
    }

    auto callInst = builder->emitCallInst(
        diffReturnType,
        diffCallee,
        args);

    if (diffReturnType->getOp() != kIROp_VoidType)
    {
        IRInst* primalResultValue = builder->emitDifferentialPairGetPrimal(callInst);
        auto diffType = differentiateType(builder, origCall->getFullType());
        IRInst* diffResultValue = builder->emitDifferentialPairGetDifferential(diffType, callInst);
        return InstPair(primalResultValue, diffResultValue);
    }
    else
    {
        // Return the inst itself if the return value is void.
        // This is fine since these values should never actually be used anywhere.
        // 
        return InstPair(callInst, callInst);
    }
}

InstPair ForwardDerivativeTranscriber::transcribeSwizzle(IRBuilder* builder, IRSwizzle* origSwizzle)
{
    IRInst* primalSwizzle = cloneInst(&cloneEnv, builder, origSwizzle);

    if (auto diffBase = lookupDiffInst(origSwizzle->getBase(), nullptr))
    {
        List<IRInst*> swizzleIndices;
        for (UIndex ii = 0; ii < origSwizzle->getElementCount(); ii++)
            swizzleIndices.add(origSwizzle->getElementIndex(ii));
        
        return InstPair(
            primalSwizzle,
            builder->emitSwizzle(
                differentiateType(builder, primalSwizzle->getDataType()),
                diffBase,
                origSwizzle->getElementCount(),
                swizzleIndices.getBuffer()));
    }

    return InstPair(primalSwizzle, nullptr);
}

InstPair ForwardDerivativeTranscriber::transcribeByPassthrough(IRBuilder* builder, IRInst* origInst)
{
    IRInst* primalInst = cloneInst(&cloneEnv, builder, origInst);

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
            differentiateType(builder, primalInst->getDataType()),
            origInst->getOp(),
            operandCount,
            diffOperands.getBuffer()));
}

InstPair ForwardDerivativeTranscriber::transcribeControlFlow(IRBuilder* builder, IRInst* origInst)
{
    switch(origInst->getOp())
    {
        case kIROp_unconditionalBranch:
        case kIROp_loop:
            auto origBranch = as<IRUnconditionalBranch>(origInst);

            // Grab the differentials for any phi nodes.
            List<IRInst*> newArgs;
            for (UIndex ii = 0; ii < origBranch->getArgCount(); ii++)
            {
                auto origArg = origBranch->getArg(ii);
                auto primalArg = lookupPrimalInst(origArg);
                newArgs.add(primalArg);

                if (differentiateType(builder, primalArg->getDataType()))
                {
                    auto diffArg = lookupDiffInst(origArg, nullptr);
                    if (diffArg)
                        newArgs.add(diffArg);
                }
            }

            IRInst* diffBranch = nullptr;
            if (auto diffBlock = findOrTranscribeDiffInst(builder, origBranch->getTargetBlock()))
            {
                if (auto origLoop = as<IRLoop>(origInst))
                {
                    auto breakBlock = findOrTranscribeDiffInst(builder, origLoop->getBreakBlock());
                    auto continueBlock = findOrTranscribeDiffInst(builder, origLoop->getContinueBlock());
                    List<IRInst*> operands;
                    operands.add(breakBlock);
                    operands.add(continueBlock);
                    operands.addRange(newArgs);
                    diffBranch = builder->emitIntrinsicInst(
                        nullptr,
                        kIROp_loop,
                        operands.getCount(),
                        operands.getBuffer());
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

            return InstPair(diffBranch, diffBranch);
    }

    getSink()->diagnose(
        origInst->sourceLoc,
        Diagnostics::unimplemented,
        "attempting to differentiate unhandled control flow");

    return InstPair(nullptr, nullptr);
}

InstPair ForwardDerivativeTranscriber::transcribeConst(IRBuilder* builder, IRInst* origInst)
{
    switch(origInst->getOp())
    {
        case kIROp_FloatLit:
            return InstPair(origInst, builder->getFloatValue(origInst->getDataType(), 0.0f));
        case kIROp_VoidLit:
            return InstPair(origInst, origInst);
        case kIROp_IntLit:
            return InstPair(origInst, builder->getIntValue(origInst->getDataType(), 0));
    }

    getSink()->diagnose(
        origInst->sourceLoc,
        Diagnostics::unimplemented,
        "attempting to differentiate unhandled const type");

    return InstPair(nullptr, nullptr);
}

IRInst* ForwardDerivativeTranscriber::findInterfaceRequirement(IRInterfaceType* type, IRInst* key)
{
    for (UInt i = 0; i < type->getOperandCount(); i++)
    {
        if (auto req = as<IRInterfaceRequirementEntry>(type->getOperand(i)))
        {
            if (req->getRequirementKey() == key)
                return req->getRequirementVal();
        }
    }
    return nullptr;
}

InstPair ForwardDerivativeTranscriber::transcribeSpecialize(IRBuilder* builder, IRSpecialize* origSpecialize)
{
    auto primalBase = findOrTranscribePrimalInst(builder, origSpecialize->getBase());
    List<IRInst*> primalArgs;
    for (UInt i = 0; i < origSpecialize->getArgCount(); i++)
    {
        primalArgs.add(findOrTranscribePrimalInst(builder, origSpecialize->getArg(i)));
    }
    auto primalType = findOrTranscribePrimalInst(builder, origSpecialize->getFullType());
    auto primalSpecialize = (IRSpecialize*)builder->emitSpecializeInst(
        (IRType*)primalType, primalBase, primalArgs.getCount(), primalArgs.getBuffer());

    IRInst* diffBase = nullptr;
    if (instMapD.TryGetValue(origSpecialize->getBase(), diffBase))
    {
        List<IRInst*> args;
        for (UInt i = 0; i < primalSpecialize->getArgCount(); i++)
        {
            args.add(primalSpecialize->getArg(i));
        }
        auto diffSpecialize = builder->emitSpecializeInst(
            builder->getTypeKind(), diffBase, args.getCount(), args.getBuffer());
        return InstPair(primalSpecialize, diffSpecialize);
    }

    auto genericInnerVal = findInnerMostGenericReturnVal(as<IRGeneric>(origSpecialize->getBase()));
    // Look for an IRForwardDerivativeDecoration on the specialize inst.
    // (Normally, this would be on the inner IRFunc, but in this case only the JVP func
    // can be specialized, so we put a decoration on the IRSpecialize)
    //
    if (auto jvpFuncDecoration = origSpecialize->findDecoration<IRForwardDerivativeDecoration>())
    {
        auto jvpFunc = jvpFuncDecoration->getForwardDerivativeFunc();

        // Make sure this isn't itself a specialize .
        SLANG_RELEASE_ASSERT(!as<IRSpecialize>(jvpFunc));

        return InstPair(primalSpecialize, jvpFunc);
    }
    else if (auto derivativeDecoration = genericInnerVal->findDecoration<IRForwardDerivativeDecoration>())
    {
        diffBase = derivativeDecoration->getForwardDerivativeFunc();
        List<IRInst*> args;
        for (UInt i = 0; i < primalSpecialize->getArgCount(); i++)
        {
            args.add(primalSpecialize->getArg(i));
        }
        auto diffSpecialize = builder->emitSpecializeInst(
            builder->getTypeKind(), diffBase, args.getCount(), args.getBuffer());
        return InstPair(primalSpecialize, diffSpecialize);
    }
    else if (auto diffDecor = genericInnerVal->findDecoration<IRForwardDifferentiableDecoration>())
    {
        List<IRInst*> args;
        for (UInt i = 0; i < primalSpecialize->getArgCount(); i++)
        {
            args.add(primalSpecialize->getArg(i));
        }
        diffBase = findOrTranscribeDiffInst(builder, origSpecialize->getBase());
        auto diffSpecialize = builder->emitSpecializeInst(
            builder->getTypeKind(), diffBase, args.getCount(), args.getBuffer());
        return InstPair(primalSpecialize, diffSpecialize);
    }
    else
    {
        return InstPair(primalSpecialize, nullptr);
    }
}

InstPair ForwardDerivativeTranscriber::transcribeLookupInterfaceMethod(IRBuilder* builder, IRLookupWitnessMethod* lookupInst)
{
    auto primalWt = findOrTranscribePrimalInst(builder, lookupInst->getWitnessTable());
    auto primalKey = findOrTranscribePrimalInst(builder, lookupInst->getRequirementKey());
    auto primalType = findOrTranscribePrimalInst(builder, lookupInst->getFullType());
    auto primal = (IRSpecialize*)builder->emitLookupInterfaceMethodInst((IRType*)primalType, primalWt, primalKey);

    auto interfaceType = as<IRInterfaceType>(_unwrapAttributedType(as<IRWitnessTableTypeBase>(lookupInst->getWitnessTable()->getDataType())->getConformanceType()));
    if (!interfaceType)
    {
        return InstPair(primal, nullptr);
    }
    auto dict = interfaceType->findDecoration<IRDifferentiableMethodRequirementDictionaryDecoration>();
    if (!dict)
    {
        return InstPair(primal, nullptr);
    }

    for (auto child : dict->getChildren())
    {
        if (auto item = as<IRForwardDifferentiableMethodRequirementDictionaryItem>(child))
        {
            if (item->getOperand(0) == lookupInst->getRequirementKey())
            {
                auto diffKey = item->getOperand(1);
                if (auto diffType = findInterfaceRequirement(interfaceType, diffKey))
                {
                    auto diff = builder->emitLookupInterfaceMethodInst((IRType*)diffType, primalWt, diffKey);
                    return InstPair(primal, diff);
                }
                break;
            }
        }
    }
    return InstPair(primal, nullptr);
}

// In differential computation, the 'default' differential value is always zero.
// This is a consequence of differential computing being inherently linear. As a 
// result, it's useful to have a method to generate zero literals of any (arithmetic) type.
// The current implementation requires that types are defined linearly.
// 
IRInst* ForwardDerivativeTranscriber::getDifferentialZeroOfType(IRBuilder* builder, IRType* primalType)
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
        if (!zeroMethod)
        {
            // if the differential type itself comes from a witness lookup, we can just lookup the
            // zero method from the same witness table.
            if (auto lookupInterface = as<IRLookupWitnessMethod>(diffType))
            {
                auto wt = lookupInterface->getWitnessTable();
                zeroMethod = builder->emitLookupInterfaceMethodInst(builder->getFuncType(List<IRType*>(), diffType), wt, autoDiffSharedContext->zeroMethodStructKey);
            }
        }
        SLANG_RELEASE_ASSERT(zeroMethod);

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

InstPair ForwardDerivativeTranscriber::transcribeBlock(IRBuilder* builder, IRBlock* origBlock)
{
    IRBuilder subBuilder(builder->getSharedBuilder());
    subBuilder.setInsertLoc(builder->getInsertLoc());
    
    IRInst* diffBlock = subBuilder.emitBlock();
    
    // Note: for blocks, we setup the mapping _before_
    // processing the children since we could encounter
    // a lookup while processing the children.
    // 
    mapPrimalInst(origBlock, diffBlock);
    mapDifferentialInst(origBlock, diffBlock);

    subBuilder.setInsertInto(diffBlock);

    // First transcribe every parameter in the block.
    for (auto param = origBlock->getFirstParam(); param; param = param->getNextParam())
        this->transcribe(&subBuilder, param);

    // Then, run through every instruction and use the transcriber to generate the appropriate
    // derivative code.
    //
    for (auto child = origBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
        this->transcribe(&subBuilder, child);

    return InstPair(diffBlock, diffBlock);
}

InstPair ForwardDerivativeTranscriber::transcribeFieldExtract(IRBuilder* builder, IRInst* originalInst)
{
    SLANG_ASSERT(as<IRFieldExtract>(originalInst) || as<IRFieldAddress>(originalInst));

    IRInst* origBase = originalInst->getOperand(0);
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    auto field = originalInst->getOperand(1);
    auto derivativeRefDecor = field->findDecoration<IRDerivativeMemberDecoration>();
    auto primalType = (IRType*)lookupPrimalInst(originalInst->getDataType(), originalInst->getDataType());

    IRInst* primalOperands[] = { primalBase, field };
    IRInst* primalFieldExtract = builder->emitIntrinsicInst(
        primalType,
        originalInst->getOp(),
        2,
        primalOperands);

    if (!derivativeRefDecor)
    {
        return InstPair(primalFieldExtract, nullptr);
    }

    IRInst* diffFieldExtract = nullptr;

    if (auto diffType = differentiateType(builder, primalType))
    {
        if (auto diffBase = findOrTranscribeDiffInst(builder, origBase))
        {
            IRInst* diffOperands[] = { diffBase, derivativeRefDecor->getDerivativeMemberStructKey() };
            diffFieldExtract = builder->emitIntrinsicInst(
                diffType,
                originalInst->getOp(),
                2,
                diffOperands);
        }
    }
    return InstPair(primalFieldExtract, diffFieldExtract);
}

InstPair ForwardDerivativeTranscriber::transcribeGetElement(IRBuilder* builder, IRInst* origGetElementPtr)
{
    SLANG_ASSERT(as<IRGetElement>(origGetElementPtr) || as<IRGetElementPtr>(origGetElementPtr));

    IRInst* origBase = origGetElementPtr->getOperand(0);
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    auto primalIndex = findOrTranscribePrimalInst(builder, origGetElementPtr->getOperand(1));

    auto primalType = (IRType*)lookupPrimalInst(origGetElementPtr->getDataType(), origGetElementPtr->getDataType());

    IRInst* primalOperands[] = {primalBase, primalIndex};
    IRInst* primalGetElementPtr = builder->emitIntrinsicInst(
        primalType,
        origGetElementPtr->getOp(),
        2,
        primalOperands);

    IRInst* diffGetElementPtr = nullptr;

    if (auto diffType = differentiateType(builder, primalType))
    {
        if (auto diffBase = findOrTranscribeDiffInst(builder, origBase))
        {
            IRInst* diffOperands[] = {diffBase, primalIndex};
            diffGetElementPtr = builder->emitIntrinsicInst(
                diffType,
                origGetElementPtr->getOp(),
                2,
                diffOperands);
        }
    }

    return InstPair(primalGetElementPtr, diffGetElementPtr);
}

InstPair ForwardDerivativeTranscriber::transcribeLoop(IRBuilder* builder, IRLoop* origLoop)
{
    // The loop comes with three blocks.. we just need to transcribe each one
    // and assemble the new loop instruction.
    
    // Transcribe the target block (this is the 'condition' part of the loop, which
    // will branch into the loop body)
    auto diffTargetBlock = findOrTranscribeDiffInst(builder, origLoop->getTargetBlock());

    // Transcribe the break block (this is the block after the exiting the loop)
    auto diffBreakBlock = findOrTranscribeDiffInst(builder, origLoop->getBreakBlock());

    // Transcribe the continue block (this is the 'update' part of the loop, which will
    // branch into the condition block)
    auto diffContinueBlock = findOrTranscribeDiffInst(builder, origLoop->getContinueBlock());

    
    List<IRInst*> diffLoopOperands;
    diffLoopOperands.add(diffTargetBlock);
    diffLoopOperands.add(diffBreakBlock);
    diffLoopOperands.add(diffContinueBlock);

    // If there are any other operands, use their primal versions.
    for (UIndex ii = diffLoopOperands.getCount(); ii < origLoop->getOperandCount(); ii++)
    {
        auto primalOperand = findOrTranscribePrimalInst(builder, origLoop->getOperand(ii));
        diffLoopOperands.add(primalOperand);
    }

    IRInst* diffLoop = builder->emitIntrinsicInst(
        nullptr,
        kIROp_loop,
        diffLoopOperands.getCount(),
        diffLoopOperands.getBuffer());

    return InstPair(diffLoop, diffLoop);
}

InstPair ForwardDerivativeTranscriber::transcribeIfElse(IRBuilder* builder, IRIfElse* origIfElse)
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
    // TODO (sai): What happens if there's no false block?
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

    IRInst* diffLoop = builder->emitIntrinsicInst(
        nullptr,
        kIROp_ifElse,
        diffIfElseArgs.getCount(),
        diffIfElseArgs.getBuffer());

    return InstPair(diffLoop, diffLoop);
}

InstPair ForwardDerivativeTranscriber::transcribeMakeDifferentialPair(IRBuilder* builder, IRMakeDifferentialPair* origInst)
{
    auto primalVal = findOrTranscribePrimalInst(builder, origInst->getPrimalValue());
    SLANG_ASSERT(primalVal);
    auto diffPrimalVal = findOrTranscribePrimalInst(builder, origInst->getDifferentialValue());
    SLANG_ASSERT(diffPrimalVal);
    auto primalDiffVal = findOrTranscribeDiffInst(builder, origInst->getPrimalValue());
    SLANG_ASSERT(primalDiffVal);
    auto diffDiffVal = findOrTranscribeDiffInst(builder, origInst->getDifferentialValue());
    SLANG_ASSERT(diffDiffVal);

    auto primalPair = builder->emitMakeDifferentialPair(origInst->getDataType(), primalVal, diffPrimalVal);
    auto diffPair = builder->emitMakeDifferentialPair(
        differentiateType(builder, origInst->getDataType()),
        primalDiffVal,
        diffDiffVal);
    return InstPair(primalPair, diffPair);
}

InstPair ForwardDerivativeTranscriber::trascribeNonDiffInst(IRBuilder* builder, IRInst* origInst)
{
    auto primal = cloneInst(&cloneEnv, builder, origInst);
    return InstPair(primal, nullptr);
}

InstPair ForwardDerivativeTranscriber::transcribeDifferentialPairGetElement(IRBuilder* builder, IRInst* origInst)
{
    SLANG_ASSERT(
        origInst->getOp() == kIROp_DifferentialPairGetDifferential ||
        origInst->getOp() == kIROp_DifferentialPairGetPrimal);

    auto primalVal = findOrTranscribePrimalInst(builder, origInst->getOperand(0));
    SLANG_ASSERT(primalVal);

    auto diffVal = findOrTranscribeDiffInst(builder, origInst->getOperand(0));
    SLANG_ASSERT(diffVal);

    auto primalResult = builder->emitIntrinsicInst(origInst->getFullType(), origInst->getOp(), 1, &primalVal);

    auto diffValPairType = as<IRDifferentialPairType>(diffVal->getDataType());
    IRInst* diffResultType = nullptr;
    if (origInst->getOp() == kIROp_DifferentialPairGetDifferential)
        diffResultType = pairBuilder->getDiffTypeFromPairType(builder, diffValPairType);
    else
        diffResultType = diffValPairType->getValueType();
    auto diffResult = builder->emitIntrinsicInst((IRType*)diffResultType, origInst->getOp(), 1, &diffVal);
    return InstPair(primalResult, diffResult);
}

// Create an empty func to represent the transcribed func of `origFunc`.
InstPair ForwardDerivativeTranscriber::transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc)
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
        newNameSb << "s_fwd_" << originalName;
        builder.addNameHintDecoration(diffFunc, newNameSb.getUnownedSlice());
    }
    builder.addForwardDerivativeDecoration(origFunc, diffFunc);

    // Mark the generated derivative function itself as differentiable.
    builder.addForwardDifferentiableDecoration(diffFunc);

    // Find and clone `DifferentiableTypeDictionaryDecoration` to the new diffFunc.
    if (auto dictDecor = origFunc->findDecoration<IRDifferentiableTypeDictionaryDecoration>())
    {
        cloneDecoration(dictDecor, diffFunc);
    }

    auto result = InstPair(primalFunc, diffFunc);
    followUpFunctionsToTranscribe.add(result);
    return result;
}

// Transcribe a function definition.
InstPair ForwardDerivativeTranscriber::transcribeFunc(IRBuilder* inBuilder, IRFunc* primalFunc, IRFunc* diffFunc)
{
    IRBuilder builder(inBuilder->getSharedBuilder());
    builder.setInsertInto(diffFunc);

    differentiableTypeConformanceContext.setFunc(primalFunc);
    // Transcribe children from origFunc into diffFunc
    for (auto block = primalFunc->getFirstBlock(); block; block = block->getNextBlock())
        this->transcribe(&builder, block);

    return InstPair(primalFunc, diffFunc);
}

// Transcribe a generic definition
InstPair ForwardDerivativeTranscriber::transcribeGeneric(IRBuilder* inBuilder, IRGeneric* origGeneric)
{
    auto innerVal = findInnerMostGenericReturnVal(origGeneric);
    if (auto innerFunc = as<IRFunc>(innerVal))
    {
        differentiableTypeConformanceContext.setFunc(innerFunc);
    }
    else if (auto funcType = as<IRFuncType>(innerVal))
    {
    }
    else
    {
        return InstPair(origGeneric, nullptr);
    }

    IRGeneric* primalGeneric = origGeneric;

    IRBuilder builder(inBuilder->getSharedBuilder());
    builder.setInsertBefore(origGeneric);

    auto diffGeneric = builder.emitGeneric();

    // Process type of generic. If the generic is a function, then it's type will also be a 
    // generic and this logic will transcribe that generic first before continuing with the 
    // function itself.
    // 
    auto primalType =  primalGeneric->getFullType();

    IRType* diffType = nullptr;
    if (primalType)
    {
        diffType = (IRType*) findOrTranscribeDiffInst(&builder, primalType);
    }

    diffGeneric->setFullType(diffType);

        // Transcribe children from origFunc into diffFunc.
        builder.setInsertInto(diffGeneric);
        for (auto block = origGeneric->getFirstBlock(); block; block = block->getNextBlock())
            this->transcribe(&builder, block);

    return InstPair(primalGeneric, diffGeneric);
}

IRInst* ForwardDerivativeTranscriber::transcribe(IRBuilder* builder, IRInst* origInst)
{
    // If a differential intstruction is already mapped for 
    // this original inst, return that.
    //
    if (auto diffInst = lookupDiffInst(origInst, nullptr))
    {
        SLANG_ASSERT(lookupPrimalInst(origInst)); // Consistency check.
        return diffInst;
    }

    // Otherwise, dispatch to the appropriate method 
    // depending on the op-code.
    // 
    instsInProgress.Add(origInst);
    InstPair pair = transcribeInst(builder, origInst);
    instsInProgress.Remove(origInst);

    if (auto primalInst = pair.primal)
    {
        mapPrimalInst(origInst, pair.primal);
        mapDifferentialInst(origInst, pair.differential);
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
                if (auto primalNameHint = primalInst->findDecoration<IRNameHintDecoration>())
                {
                    StringBuilder sb;
                    sb << "s_diff_" << primalNameHint->getName();
                    builder->addNameHintDecoration(pair.differential, sb.getUnownedSlice());
                }
                break;
            }
        }
        return pair.differential;
    }
    getSink()->diagnose(origInst->sourceLoc,
        Diagnostics::internalCompilerError,
        "failed to transcibe instruction");
    return nullptr;
}

InstPair ForwardDerivativeTranscriber::transcribeInst(IRBuilder* builder, IRInst* origInst)
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
        return transcribeBinaryLogic(builder, origInst);

    case kIROp_Construct:
        return transcribeConstruct(builder, origInst);

    case kIROp_lookup_interface_method:
        return transcribeLookupInterfaceMethod(builder, as<IRLookupWitnessMethod>(origInst));

    case kIROp_Call:
        return transcribeCall(builder, as<IRCall>(origInst));
    
    case kIROp_swizzle:
        return transcribeSwizzle(builder, as<IRSwizzle>(origInst));
    
    case kIROp_constructVectorFromScalar:
    case kIROp_MakeTuple:
        return transcribeByPassthrough(builder, origInst);

    case kIROp_unconditionalBranch:
        return transcribeControlFlow(builder, origInst);

    case kIROp_FloatLit:
    case kIROp_IntLit:
    case kIROp_VoidLit:
        return transcribeConst(builder, origInst);

    case kIROp_Specialize:
        return transcribeSpecialize(builder, as<IRSpecialize>(origInst));

    case kIROp_FieldExtract:
    case kIROp_FieldAddress:
        return transcribeFieldExtract(builder, origInst);
    case kIROp_getElement:
    case kIROp_getElementPtr:
        return transcribeGetElement(builder, origInst);
    
    case kIROp_loop:
        return transcribeLoop(builder, as<IRLoop>(origInst));

    case kIROp_ifElse:
        return transcribeIfElse(builder, as<IRIfElse>(origInst));

    case kIROp_MakeDifferentialPair:
        return transcribeMakeDifferentialPair(builder, as<IRMakeDifferentialPair>(origInst));
    case kIROp_DifferentialPairGetPrimal:
    case kIROp_DifferentialPairGetDifferential:
        return transcribeDifferentialPairGetElement(builder, origInst);
    case kIROp_ExtractExistentialWitnessTable:
    case kIROp_ExtractExistentialType:
    case kIROp_ExtractExistentialValue:
    case kIROp_WrapExistential:
    case kIROp_MakeExistential:
    case kIROp_MakeExistentialWithRTTI:
        return trascribeNonDiffInst(builder, origInst);
    case kIROp_StructKey:
        return InstPair(origInst, nullptr);
    }

    // If none of the cases have been hit, check if the instruction is a
    // type. Only need to explicitly differentiate types if they appear inside a block.
    // 
    if (auto origType = as<IRType>(origInst))
    {   
        // If this is a generic type, transcibe the parent 
        // generic and derive the type from the transcribed generic's
        // return value.
        // 
        if (as<IRGeneric>(origType->getParent()->getParent()) && 
            findInnerMostGenericReturnVal(as<IRGeneric>(origType->getParent()->getParent())) == origType && 
            !instsInProgress.Contains(origType->getParent()->getParent()))
        {
            auto origGenericType = origType->getParent()->getParent();
            auto diffGenericType = findOrTranscribeDiffInst(builder, origGenericType);
            auto innerDiffGenericType = findInnerMostGenericReturnVal(as<IRGeneric>(diffGenericType));
            return InstPair(
                origGenericType,
                innerDiffGenericType
            );
        }
        else if (as<IRBlock>(origType->getParent()))
            return InstPair(
                cloneInst(&cloneEnv, builder, origType),
                differentiateType(builder, origType));
        else
            return InstPair(
                cloneInst(&cloneEnv, builder, origType),
                nullptr);
    }

    // Handle instructions with children
    switch (origInst->getOp())
    {
    case kIROp_Func:
        return transcribeFuncHeader(builder, as<IRFunc>(origInst)); 

    case kIROp_Block:
        return transcribeBlock(builder, as<IRBlock>(origInst));
    
    case kIROp_Generic:
        return transcribeGeneric(builder, as<IRGeneric>(origInst)); 
    }

    // If we reach this statement, the instruction type is likely unhandled.
    getSink()->diagnose(origInst->sourceLoc,
                Diagnostics::unimplemented,
                "this instruction cannot be differentiated");

    return InstPair(nullptr, nullptr);
}

struct ForwardDerivativePass : public InstPassBase
{

    DiagnosticSink* getSink()
    {
        return sink;
    }

    bool processModule()
    {
        // TODO(sai): Move this call.
        transcriberStorage.differentiableTypeConformanceContext.buildGlobalWitnessDictionary();

        IRBuilder builderStorage(this->autodiffContext->sharedBuilder);
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
        bool changed = false;
        List<IRInst*> autoDiffWorkList;
        for (;;)
        {
            // Collect all `ForwardDifferentiate` insts from the module.
            autoDiffWorkList.clear();
            processAllInsts([&](IRInst* inst)
                {
                    switch (inst->getOp())
                    {
                    case kIROp_ForwardDifferentiate:
                        // Only process now if the operand is a materialized function.
                        switch (inst->getOperand(0)->getOp())
                        {
                        case kIROp_Func:
                        case kIROp_Specialize:
                        case kIROp_lookup_interface_method:
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

            transcriberStorage.followUpFunctionsToTranscribe.clear();

            for (auto differentiateInst : autoDiffWorkList)
            {
                IRInst* baseInst = differentiateInst->getOperand(0);
                if (as<IRForwardDifferentiate>(differentiateInst))
                {
                    if (auto existingDiffFunc = lookupJVPReference(baseInst))
                    {
                        differentiateInst->replaceUsesWith(existingDiffFunc);
                        differentiateInst->removeAndDeallocate();
                    }
                    else
                    {
                        IRBuilder subBuilder(*builder);
                        subBuilder.setInsertBefore(differentiateInst);
                        IRInst* diffFunc = transcriberStorage.transcribe(&subBuilder, baseInst);
                        SLANG_ASSERT(diffFunc);
                        differentiateInst->replaceUsesWith(diffFunc);
                        differentiateInst->removeAndDeallocate();
                    }
                    changed = true;
                }
            }
            // Actually synthesize the derivatives.
            List<InstPair> followUpWorkList = _Move(transcriberStorage.followUpFunctionsToTranscribe);
            for (auto task : followUpWorkList)
            {
                auto diffFunc = as<IRFunc>(task.differential);
                SLANG_ASSERT(diffFunc);
                auto primalFunc = as<IRFunc>(task.primal);
                SLANG_ASSERT(primalFunc);

                transcriberStorage.transcribeFunc(builder, primalFunc, diffFunc);
            }

            // Transcribing the function body really shouldn't produce more follow up function body work.
            // However it may produce new `ForwardDifferentiate` instructions, which we collect and process
            // in the next iteration.
            SLANG_RELEASE_ASSERT(transcriberStorage.followUpFunctionsToTranscribe.getCount() == 0);

        }
        return changed;
    }

    // Checks decorators to see if the function should
    // be differentiated (kIROp_ForwardDifferentiableDecoration)
    // 
    bool isMarkedForForwardDifferentiation(IRInst* callable)
    {
        if (auto gen = as<IRGeneric>(callable))
            callable = findGenericReturnVal(gen);
        return callable->findDecoration<IRForwardDifferentiableDecoration>() != nullptr;
    }

    IRStringLit* getForwardDerivativeFuncName(IRInst* func)
    {
        IRBuilder builder(&sharedBuilderStorage);
        builder.setInsertBefore(func);

        IRStringLit* name = nullptr;
        if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
        {
            name = builder.getStringValue((String(linkageDecoration->getMangledName()) + "_fwd_diff").getUnownedSlice());
        }
        else if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
        {
            name = builder.getStringValue((String(namehintDecoration->getName()) + "_fwd_diff").getUnownedSlice());
        }

        return name;
    }

    ForwardDerivativePass(AutoDiffSharedContext* context, DiagnosticSink* sink) :
        InstPassBase(context->moduleInst->getModule()),
        sink(sink),
        transcriberStorage(context, context->sharedBuilder),
        pairBuilderStorage(context),
        autodiffContext(context)
    {
        transcriberStorage.sink = sink;
        transcriberStorage.autoDiffSharedContext = context;
        transcriberStorage.pairBuilder = &(pairBuilderStorage);
    }

protected:
    // A transcriber object that handles the main job of 
    // processing instructions while maintaining state.
    //
    ForwardDerivativeTranscriber                  transcriberStorage;

    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink*                 sink;

    // Shared context.
    AutoDiffSharedContext*          autodiffContext;

    // Builder for dealing with differential pair types.
    DifferentialPairTypeBuilder     pairBuilderStorage;

};

// Set up context and call main process method.
//
bool processForwardDerivativeCalls(
    AutoDiffSharedContext* autodiffContext,
    DiagnosticSink* sink,
    ForwardDerivativePassOptions const&)
{
    ForwardDerivativePass fwdPass(autodiffContext, sink);
    bool changed = fwdPass.processModule();
    return changed;
}

}
