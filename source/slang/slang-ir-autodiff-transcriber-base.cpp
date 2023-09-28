// slang-ir-autodiff-trascriber-base.cpp
#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-transcriber-base.h"

#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-util.h"
#include "slang-ir-inst-pass-base.h"

namespace Slang
{

DiagnosticSink* AutoDiffTranscriberBase::getSink()
{
    SLANG_ASSERT(sink);
    return sink;
}

void AutoDiffTranscriberBase::mapDifferentialInst(IRInst* origInst, IRInst* diffInst)
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

void AutoDiffTranscriberBase::mapPrimalInst(IRInst* origInst, IRInst* primalInst)
{
    if (cloneEnv.mapOldValToNew.containsKey(origInst) && cloneEnv.mapOldValToNew[origInst] != primalInst)
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

IRInst* AutoDiffTranscriberBase::lookupDiffInst(IRInst* origInst)
{
    return instMapD[origInst];
}

IRInst* AutoDiffTranscriberBase::lookupDiffInst(IRInst* origInst, IRInst* defaultInst)
{
    if (auto lookupResult = instMapD.tryGetValue(origInst))
        return *lookupResult;
    return defaultInst;
}

bool AutoDiffTranscriberBase::hasDifferentialInst(IRInst* origInst)
{
    if (!origInst)
        return false;
    return instMapD.containsKey(origInst);
}

bool AutoDiffTranscriberBase::shouldUseOriginalAsPrimal(IRInst* currentParent, IRInst* origInst)
{
    if (as<IRGlobalValueWithCode>(origInst))
        return true;
    if (origInst->parent && origInst->parent->getOp() == kIROp_Module)
        return true;
    if (isChildInstOf(currentParent, origInst->getParent()))
        return true;

    // If origInst is defined in the first block of the same function as current inst (e.g. a param),
    // we can use it as primal.
    // More generally, we should test if origInst dominates currentParent, but that requires calculating
    // a dom tree on the fly. Right now just testing if it is first block for parameters seems sufficient.
    auto parentFunc = getParentFunc(currentParent);
    if (parentFunc && origInst->parent == parentFunc->getFirstBlock())
        return true;
    return false;
}

IRInst* AutoDiffTranscriberBase::lookupPrimalInstImpl(IRInst* currentParent, IRInst* origInst)
{
    if (!origInst)
        return nullptr;
    if (shouldUseOriginalAsPrimal(currentParent, origInst))
        return origInst;
    return cloneEnv.mapOldValToNew[origInst];
}

IRInst* AutoDiffTranscriberBase::lookupPrimalInst(IRInst* currentParent, IRInst* origInst, IRInst* defaultInst)
{
    if (!origInst)
        return nullptr;
    return (hasPrimalInst(currentParent, origInst)) ? lookupPrimalInstImpl(currentParent, origInst) : defaultInst;
}

bool AutoDiffTranscriberBase::hasPrimalInst(IRInst* currentParent, IRInst* origInst)
{
    if (!origInst)
        return false;
    if (shouldUseOriginalAsPrimal(currentParent, origInst))
        return true;
    return cloneEnv.mapOldValToNew.containsKey(origInst);
}

IRInst* AutoDiffTranscriberBase::findOrTranscribeDiffInst(IRBuilder* builder, IRInst* origInst)
{
    if (!hasDifferentialInst(origInst))
    {
        transcribe(builder, origInst);
        SLANG_ASSERT(hasDifferentialInst(origInst));
    }

    return lookupDiffInst(origInst);
}

IRInst* AutoDiffTranscriberBase::findOrTranscribePrimalInst(IRBuilder* builder, IRInst* origInst)
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

IRInst* AutoDiffTranscriberBase::maybeCloneForPrimalInst(IRBuilder* builder, IRInst* inst)
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
            (IRType*)type, inst->getOp(), operands.getCount(), operands.getBuffer());
        IRBuilder subBuilder = *builder;
        subBuilder.setInsertInto(cloneResult);
        for (auto child : inst->getDecorationsAndChildren())
        {
            maybeCloneForPrimalInst(&subBuilder, child);
        }
        cloneEnv.mapOldValToNew[inst] = cloneResult;
        return cloneResult;
    }

    return primal;
}

IRInst* _lookupWitness(IRBuilder* builder, IRInst* witness, IRInst* requirementKey);

// Get or construct `:IDifferentiable` conformance for a DifferentiablePair.
IRWitnessTable* AutoDiffTranscriberBase::getDifferentialPairWitness(IRBuilder* builder, IRInst* inOriginalDiffPairType, IRInst* inPrimalDiffPairType)
{
    // Differentiate the pair type to get it's differential (which is itself a pair)
    auto diffDiffPairType = (IRType*)differentiateType(builder, (IRType*)inOriginalDiffPairType);

    auto addMethod = builder->createFunc();
    auto zeroMethod = builder->createFunc();

    auto table = builder->createWitnessTable(autoDiffSharedContext->differentiableInterfaceType, (IRType*)inPrimalDiffPairType);

    // And place it in the synthesized witness table.
    builder->createWitnessTableEntry(table, autoDiffSharedContext->differentialAssocTypeStructKey, diffDiffPairType);
    builder->createWitnessTableEntry(table, autoDiffSharedContext->differentialAssocTypeWitnessStructKey, table);
    builder->createWitnessTableEntry(table, autoDiffSharedContext->addMethodStructKey, addMethod);
    builder->createWitnessTableEntry(table, autoDiffSharedContext->zeroMethodStructKey, zeroMethod);

    bool isUserCodeType = as<IRDifferentialPairUserCodeType>(inOriginalDiffPairType) ? true : false;

    // Fill in differential method implementations.
    auto elementType = as<IRDifferentialPairTypeBase>(inPrimalDiffPairType)->getValueType();
    auto innerWitness = as<IRDifferentialPairTypeBase>(inPrimalDiffPairType)->getWitness();

    {
        // Add method.
        IRBuilder b = *builder;
        b.setInsertInto(addMethod);
        b.addBackwardDifferentiableDecoration(addMethod);
        IRType* paramTypes[2] = { diffDiffPairType, diffDiffPairType };
        addMethod->setFullType(b.getFuncType(2, paramTypes, diffDiffPairType));
        b.emitBlock();
        auto p0 = b.emitParam(diffDiffPairType);
        auto p1 = b.emitParam(diffDiffPairType);

        // Since we are already dealing with a DiffPair<T>.Differnetial type, we know that value type == diff type.
        auto innerAdd = _lookupWitness(&b, innerWitness, autoDiffSharedContext->addMethodStructKey);
        IRInst* argsPrimal[2] = {
            isUserCodeType ? b.emitDifferentialPairGetPrimalUserCode(p0) : b.emitDifferentialPairGetPrimal(p0),
            isUserCodeType ? b.emitDifferentialPairGetPrimalUserCode(p1) : b.emitDifferentialPairGetPrimal(p1) };
        auto primalPart = b.emitCallInst(elementType, innerAdd, 2, argsPrimal);
        IRInst* argsDiff[2] = {
            isUserCodeType ? b.emitDifferentialPairGetDifferentialUserCode(elementType, p0) : b.emitDifferentialPairGetDifferential(elementType, p0),
            isUserCodeType ? b.emitDifferentialPairGetDifferentialUserCode(elementType, p1) : b.emitDifferentialPairGetDifferential(elementType, p1)};
        auto diffPart = b.emitCallInst(elementType, innerAdd, 2, argsDiff);
        auto retVal =
            isUserCodeType
                ? b.emitMakeDifferentialPairUserCode(diffDiffPairType, primalPart, diffPart)
                : b.emitMakeDifferentialPair(diffDiffPairType, primalPart, diffPart);
        b.emitReturn(retVal);
    }
    {
        // Zero method.
        IRBuilder b = *builder;
        b.setInsertInto(zeroMethod);
        zeroMethod->setFullType(b.getFuncType(0, nullptr, diffDiffPairType));
        b.emitBlock();
        auto innerZero = _lookupWitness(&b, innerWitness, autoDiffSharedContext->zeroMethodStructKey);
        auto zeroVal = b.emitCallInst(elementType, innerZero, 0, nullptr);
        auto retVal =
            isUserCodeType
            ? b.emitMakeDifferentialPairUserCode(diffDiffPairType, zeroVal, zeroVal)
            : b.emitMakeDifferentialPair(diffDiffPairType, zeroVal, zeroVal);
        b.emitReturn(retVal);
    }
    
    // Record this in the context for future lookups
    differentiableTypeConformanceContext.differentiableWitnessDictionary[(IRType*)inOriginalDiffPairType] = table;

    return table;
}

// Get or construct `:IDifferentiable` conformance for an Array.
IRWitnessTable* AutoDiffTranscriberBase::getArrayWitness(IRBuilder* builder, IRInst* inOriginalArrayType, IRInst* inPrimalArrayType)
{
    // Differentiate the pair type to get it's differential (which is itself a pair)
    auto diffArrayType = (IRType*)differentiateType(builder, (IRType*)inOriginalArrayType);

    if (!diffArrayType)
        return nullptr;

    auto innerWitness = tryGetDifferentiableWitness(builder, as<IRArrayTypeBase>(inOriginalArrayType)->getElementType());

    auto addMethod = builder->createFunc();
    auto zeroMethod = builder->createFunc();

    auto table = builder->createWitnessTable(autoDiffSharedContext->differentiableInterfaceType, (IRType*)inPrimalArrayType);

    // And place it in the synthesized witness table.
    builder->createWitnessTableEntry(table, autoDiffSharedContext->differentialAssocTypeStructKey, diffArrayType);
    builder->createWitnessTableEntry(table, autoDiffSharedContext->differentialAssocTypeWitnessStructKey, table);
    builder->createWitnessTableEntry(table, autoDiffSharedContext->addMethodStructKey, addMethod);
    builder->createWitnessTableEntry(table, autoDiffSharedContext->zeroMethodStructKey, zeroMethod);

    auto elementType = as<IRArrayTypeBase>(diffArrayType)->getElementType();

    // Fill in differential method implementations.
    {
        // Add method.
        IRBuilder b = *builder;
        b.setInsertInto(addMethod);
        b.addBackwardDifferentiableDecoration(addMethod);
        IRType* paramTypes[2] = { diffArrayType, diffArrayType };
        addMethod->setFullType(b.getFuncType(2, paramTypes, diffArrayType));
        b.emitBlock();
        auto p0 = b.emitParam(diffArrayType);
        auto p1 = b.emitParam(diffArrayType);

        // Since we are already dealing with a DiffPair<T>.Differnetial type, we know that value type == diff type.
        auto innerAdd = _lookupWitness(&b, innerWitness, autoDiffSharedContext->addMethodStructKey);
        auto resultVar = b.emitVar(diffArrayType);
        IRBlock* loopBodyBlock = nullptr;
        IRBlock* loopBreakBlock = nullptr;
        auto loopCounter = emitLoopBlocks(&b, b.getIntValue(b.getIntType(), 0), as<IRArrayTypeBase>(diffArrayType)->getElementCount(), loopBodyBlock, loopBreakBlock);
        b.setInsertBefore(loopBodyBlock->getTerminator());

        IRInst* args[2] = {
            b.emitElementExtract(p0, loopCounter),
            b.emitElementExtract(p1, loopCounter) };
        auto elementResult = b.emitCallInst(elementType, innerAdd, 2, args);
        auto addr = b.emitElementAddress(resultVar, loopCounter);
        b.emitStore(addr, elementResult);
        b.setInsertInto(loopBreakBlock);
        b.emitReturn(b.emitLoad(resultVar));
    }
    {
        // Zero method.
        IRBuilder b = *builder;
        b.setInsertInto(zeroMethod);
        zeroMethod->setFullType(b.getFuncType(0, nullptr, diffArrayType));
        b.emitBlock();

        auto innerZero = _lookupWitness(&b, innerWitness, autoDiffSharedContext->zeroMethodStructKey);
        auto zeroVal = b.emitCallInst(elementType, innerZero, 0, nullptr);
        auto retVal = b.emitMakeArrayFromElement(diffArrayType, zeroVal);
        b.emitReturn(retVal);
    }

    // Record this in the context for future lookups
    differentiableTypeConformanceContext.differentiableWitnessDictionary[(IRType*)inOriginalArrayType] = table;

    return table;
}

IRInst* AutoDiffTranscriberBase::tryGetDifferentiableWitness(IRBuilder* builder, IRInst* originalType)
{
    if (isNoDiffType((IRType*)originalType))
        return nullptr;

    IRInst* witness =
        differentiableTypeConformanceContext.lookUpConformanceForType((IRType*)originalType);
    if (witness)
    {
        witness = lookupPrimalInst(builder, witness, nullptr);
        SLANG_RELEASE_ASSERT(witness || as<IRArrayType>(originalType));
    }
    if (!witness)
    {
        auto primalType = lookupPrimalInst(builder, originalType, nullptr);
        SLANG_RELEASE_ASSERT(primalType);
        if (auto primalPairType = as<IRDifferentialPairTypeBase>(primalType))
        {
            witness = getDifferentialPairWitness(builder, originalType, primalPairType);
        }
        else if (auto arrayType = as<IRArrayType>(primalType))
        {
            witness = getArrayWitness(builder, originalType, arrayType);
        }
        else if (auto extractExistential = as<IRExtractExistentialType>(originalType))
        {
            differentiateExtractExistentialType(builder, extractExistential, witness);
        }
    }
    return witness;
}

IRType* AutoDiffTranscriberBase::getOrCreateDiffPairType(IRBuilder* builder, IRInst* primalType, IRInst* witness)
{
    return builder->getDifferentialPairType(
        (IRType*)primalType,
        witness);
}

IRType* AutoDiffTranscriberBase::getOrCreateDiffPairType(IRBuilder* builder, IRInst* originalType)
{
    auto primalType = lookupPrimalInst(builder, originalType, nullptr);
    SLANG_RELEASE_ASSERT(primalType);

    IRInst* witness = nullptr;
    if (auto lookup = as<IRLookupWitnessMethod>(primalType))
    {
        if (lookup->getRequirementKey() == autoDiffSharedContext->differentialAssocTypeStructKey)
        {
            witness = builder->emitLookupInterfaceMethodInst(
                lookup->getWitnessTable()->getDataType(),
                lookup->getWitnessTable(),
                autoDiffSharedContext->differentialAssocTypeWitnessStructKey);
        }
    }
    
    // Obtain the witness that primalType conforms to IDifferentiable.
    if (!witness)
        witness = tryGetDifferentiableWitness(builder, originalType);
    SLANG_RELEASE_ASSERT(witness);

    auto pairType = builder->getDifferentialPairType(
        (IRType*)primalType,
        witness);

    return pairType;
}

IRType* AutoDiffTranscriberBase::differentiateType(IRBuilder* builder, IRType* origType)
{
    if (isNoDiffType(origType))
        return nullptr;

    // Special-case for differentiable existential types.
    if (as<IRInterfaceType>(origType) || as<IRAssociatedType>(origType))
    {
        if (differentiableTypeConformanceContext.lookUpConformanceForType(origType))
            return autoDiffSharedContext->differentiableInterfaceType;
        else
            return nullptr;
    }

    auto primalType = lookupPrimalInst(builder, origType, origType);
    if (primalType->getOp() == kIROp_Param &&
        primalType->getParent() && primalType->getParent()->getParent() &&
        primalType->getParent()->getParent()->getOp() == kIROp_Generic)
    {
        auto diffType = (IRType*)differentiableTypeConformanceContext.getDifferentialForType(builder, origType);
        return (IRType*)findOrTranscribePrimalInst(builder, diffType);
    }
    return (IRType*)transcribe(builder, origType);
}

IRType* AutoDiffTranscriberBase::_differentiateTypeImpl(IRBuilder* builder, IRType* origType)
{
    if (isNoDiffType(origType))
        return nullptr;

    if (auto ptrType = as<IRPtrTypeBase>(origType))
        return builder->getPtrType(
            origType->getOp(),
            differentiateType(builder, ptrType->getValueType()));

    auto primalType = maybeCloneForPrimalInst(builder, origType);

    // Special case certain compound types (PtrType, FuncType, etc..)
    // otherwise try to lookup a differential definition for the given type.
    // If one does not exist, then we assume it's not differentiable.
    // 
    switch (primalType->getOp())
    {
    case kIROp_Param:
        if (as<IRTypeType>(primalType->getDataType()))
            return differentiateType(builder, origType);
        else if (as<IRWitnessTableType>(primalType->getDataType()))
            return (IRType*)primalType;
        else
            return nullptr;

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
            builder,
            differentiableTypeConformanceContext.getDiffTypeFromPairType(builder, primalPairType),
            differentiableTypeConformanceContext.getDiffTypeWitnessFromPairType(builder, primalPairType));
    }

    case kIROp_DifferentialPairUserCodeType:
    {
        auto primalPairType = as<IRDifferentialPairUserCodeType>(primalType);
        return builder->getDifferentialPairUserCodeType(
            (IRType*)differentiableTypeConformanceContext.getDiffTypeFromPairType(builder, primalPairType),
            differentiableTypeConformanceContext.getDiffTypeWitnessFromPairType(builder, primalPairType));
    }

    case kIROp_FuncType:
        return differentiateFunctionType(builder, nullptr, as<IRFuncType>(primalType));

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
    return (IRType*)maybeCloneForPrimalInst(
        builder,
        differentiableTypeConformanceContext.getDifferentialForType(builder, (IRType*)origType));
    }
}

bool AutoDiffTranscriberBase::isExistentialType(IRType *type)
{
    switch (type->getOp())
    {
        case kIROp_ExtractExistentialType:
        case kIROp_InterfaceType:
        case kIROp_AssociatedType:
            return true;
        default:
            return false;
    }
}

void AutoDiffTranscriberBase::copyOriginalDecorations(IRInst* origFunc, IRInst* diffFunc)
{
    for (auto decor : origFunc->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_ForceInlineDecoration:
            cloneDecoration(decor, diffFunc);
            break;
        }
    }
}

InstPair AutoDiffTranscriberBase::transcribeExtractExistentialWitnessTable(IRBuilder* builder, IRInst* origInst)
{
    IRInst* origBase = origInst->getOperand(0);
    auto primalBase = findOrTranscribePrimalInst(builder, origBase);
    auto primalType = (IRType*)findOrTranscribePrimalInst(builder, origInst->getDataType());

    IRInst* primalResult = builder->emitIntrinsicInst(
        primalType,
        origInst->getOp(),
        1,
        &primalBase);

    // Search for IDifferentiable conformance.
    auto interfaceType = as<IRInterfaceType>(
        unwrapAttributedType(cast<IRWitnessTableType>(origInst->getDataType())->getConformanceType()));
    
    if (!interfaceType)
        return InstPair(primalResult, nullptr);
    
    if (auto differentialWitnessTable = differentiableTypeConformanceContext.tryExtractConformanceFromInterfaceType(
            builder, interfaceType, (IRWitnessTable*)primalResult))
    {
        // `interfaceType` does conform to `IDifferentiable`.
        return InstPair(primalResult, differentialWitnessTable);
    }

    return InstPair(primalResult, nullptr);
}

void AutoDiffTranscriberBase::maybeMigrateDifferentiableDictionaryFromDerivativeFunc(IRBuilder* builder, IRInst* origFunc)
{
    auto decor = origFunc->findDecoration<IRDifferentiableTypeDictionaryDecoration>();
    if (decor)
        return;
    // A differentiable func must have `IRDifferentiableTypeDictionaryDecoration`, except it has a
    // `IRUserDefinedBackwardDerivativeDecoration`.
    auto udfDecor = origFunc->findDecoration<IRUserDefinedBackwardDerivativeDecoration>();
    SLANG_RELEASE_ASSERT(udfDecor);
    // We need to migrate the dictionary from the backward derivative func so we can properly
    // differentiate the function header.
    IRBuilder subBuilder = *builder;
    subBuilder.setInsertBefore(origFunc);

    auto derivative = udfDecor->getBackwardDerivativeFunc();
    if (auto specialize = as<IRSpecialize>(derivative))
    {
        auto derivativeGeneric = cast<IRGeneric>(specialize->getBase());
        GenericChildrenMigrationContext migrationContext;
        migrationContext.init(derivativeGeneric, cast<IRGeneric>(findOuterGeneric(origFunc)), origFunc);
        auto derivativeFunc = findGenericReturnVal(derivativeGeneric);
        auto derivativeBlock = cast<IRBlock>(derivativeFunc->getParent());
        for (auto dInst = derivativeBlock->getFirstOrdinaryInst(); dInst != derivativeFunc;
             dInst = dInst->getNextInst())
        {
            migrationContext.cloneInst(&subBuilder, dInst);
        }
        auto udfDictDecor = derivativeFunc->findDecoration<IRDifferentiableTypeDictionaryDecoration>();
        SLANG_RELEASE_ASSERT(udfDictDecor);
        subBuilder.setInsertBefore(origFunc->getFirstDecorationOrChild());
        migrationContext.cloneInst(&subBuilder, udfDictDecor);
        eliminateDeadCode(origFunc->getParent());
    }
    else
    {
        auto udfDictDecor = derivative->findDecoration< IRDifferentiableTypeDictionaryDecoration>();
        if (udfDictDecor)
        {
            cloneDecoration(udfDictDecor, origFunc);
        }
    }
}

IRType* AutoDiffTranscriberBase::differentiateExtractExistentialType(IRBuilder* builder, IRExtractExistentialType* origType, IRInst*& outWitnessTable)
{
    outWitnessTable = nullptr;

    // Search for IDifferentiable conformance.
    auto interfaceType = as<IRInterfaceType>(unwrapAttributedType(origType->getOperand(0)->getDataType()));
    if (!interfaceType)
        return nullptr;
    List<IRInterfaceRequirementEntry*> lookupKeyPath = differentiableTypeConformanceContext.findDifferentiableInterfaceLookupPath(
        autoDiffSharedContext->differentiableInterfaceType, interfaceType);

    if (lookupKeyPath.getCount())
    {
        // `interfaceType` does conform to `IDifferentiable`.
        outWitnessTable = builder->emitExtractExistentialWitnessTable(lookupPrimalInstIfExists(builder, origType->getOperand(0)));
        for (auto node : lookupKeyPath)
        {
            outWitnessTable = builder->emitLookupInterfaceMethodInst((IRType*)node->getRequirementVal(), outWitnessTable, node->getRequirementKey());
        }
        auto diffType = builder->emitLookupInterfaceMethodInst(builder->getTypeType(), outWitnessTable, autoDiffSharedContext->differentialAssocTypeStructKey);
        return (IRType*)diffType;
    }
    return nullptr;
}

IRType* AutoDiffTranscriberBase::tryGetDiffPairType(IRBuilder* builder, IRType* originalType)
{
    // If this is a PtrType (out, inout, etc..), then create diff pair from
    // value type and re-apply the appropropriate PtrType wrapper.
    // 
    if (auto origPtrType = as<IRPtrTypeBase>(originalType))
    {
        if (auto diffPairValueType = tryGetDiffPairType(builder, origPtrType->getValueType()))
            return builder->getPtrType(originalType->getOp(), diffPairValueType);
        else
            return nullptr;
    }
    auto diffType = differentiateType(builder, originalType);
    if (diffType)
        return (IRType*)getOrCreateDiffPairType(builder, originalType);
    return nullptr;
}

InstPair AutoDiffTranscriberBase::transcribeParam(IRBuilder* builder, IRParam* origParam)
{
    auto primalDataType = findOrTranscribePrimalInst(builder, origParam->getDataType());
    // Do not differentiate generic type (and witness table) parameters
    if (isGenericParam(origParam))
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

InstPair AutoDiffTranscriberBase::transcribeLookupInterfaceMethod(IRBuilder* builder, IRLookupWitnessMethod* lookupInst)
{
    auto primalWt = findOrTranscribePrimalInst(builder, lookupInst->getWitnessTable());
    auto primalKey = findOrTranscribePrimalInst(builder, lookupInst->getRequirementKey());
    auto primalType = findOrTranscribePrimalInst(builder, lookupInst->getFullType());
    auto primal = (IRSpecialize*)builder->emitLookupInterfaceMethodInst((IRType*)primalType, primalWt, primalKey);

    auto interfaceType = as<IRInterfaceType>(unwrapAttributedType(as<IRWitnessTableTypeBase>(lookupInst->getWitnessTable()->getDataType())->getConformanceType()));
    if (!interfaceType)
    {
        return InstPair(primal, nullptr);
    }
    if (interfaceType == autoDiffSharedContext->differentiableInterfaceType)
    {
        if (primalKey == autoDiffSharedContext->differentialAssocTypeStructKey)
        {
            return InstPair(primal, primal);
        }
        else if (primalKey == autoDiffSharedContext->differentialAssocTypeWitnessStructKey)
        {
            return InstPair(primal, primal);
        }
        else
        {
            // We can't really differentiate a call to a IDifferentiable method here.
            // They need to be specialized first.
            return InstPair(primal, nullptr);
        }
    }
    else if (auto returnWitnessType = as<IRWitnessTableTypeBase>(lookupInst->getDataType()))
    {
        // T.Diff_Is_IDifferential ==> T.Diff_Is_IDifferential.Diff_Is_IDifferential
        if (returnWitnessType->getConformanceType() == autoDiffSharedContext->differentiableInterfaceType)
        {
            auto primalDiffType = builder->emitLookupInterfaceMethodInst(
                builder->getTypeKind(),
                primal,
                autoDiffSharedContext->differentialAssocTypeStructKey);
            auto diffWitness = builder->emitLookupInterfaceMethodInst(
                (IRType*)primalDiffType,
                primal,
                autoDiffSharedContext->differentialAssocTypeWitnessStructKey);

            // Mark both as primal since we're working with types 
            // (which don't need transposing)
            // 
            builder->markInstAsPrimal(primalDiffType);
            builder->markInstAsPrimal(diffWitness);

            return InstPair(primal, diffWitness);
        }
    }
    auto decor = 
        lookupInst->getRequirementKey()->findDecorationImpl(
            getInterfaceRequirementDerivativeDecorationOp());
    if (!decor)
    {
        return InstPair(primal, nullptr);
    }

    auto diffKey = decor->getOperand(0);
    if (auto diffType = findInterfaceRequirement(interfaceType, diffKey))
    {
        auto diff = builder->emitLookupInterfaceMethodInst((IRType*)diffType, primalWt, diffKey);
        return InstPair(primal, diff);
    }
    return InstPair(primal, nullptr);
}

// In differential computation, the 'default' differential value is always zero.
// This is a consequence of differential computing being inherently linear. As a 
// result, it's useful to have a method to generate zero literals of any (arithmetic) type.
// The current implementation requires that types are defined linearly.
// 
IRInst* AutoDiffTranscriberBase::getDifferentialZeroOfType(
    IRBuilder* builder, IRType* originalType)
{
    originalType = (IRType*)unwrapAttributedType(originalType);
    auto primalType = (IRType*)lookupPrimalInst(builder, originalType);
    if (auto diffType = differentiateType(builder, originalType))
    {
        IRInst* diffWitnessTable = nullptr;
        IRType* diffOuterType = nullptr;
        if (isExistentialType(diffType))
        {
            // Emit null differential & pack it into an IDifferentiable existential.

            auto nullDiffValue = differentiableTypeConformanceContext.emitNullDifferential(builder);
            builder->markInstAsDifferential(nullDiffValue, autoDiffSharedContext->nullDifferentialStructType);

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
                    getDifferentialZeroOfType(builder, as<IRDifferentialPairType>(diffType)->getValueType()),
                    getDifferentialZeroOfType(builder, as<IRDifferentialPairType>(diffType)->getValueType()));
                builder->markInstAsDifferential(makeDiffPair, as<IRDifferentialPairType>(diffType)->getValueType());
                return makeDiffPair;
            }
        case kIROp_DifferentialPairUserCodeType:
        {
            auto makeDiffPair = builder->emitMakeDifferentialPairUserCode(
                diffType,
                getDifferentialZeroOfType(builder, as<IRDifferentialPairUserCodeType>(diffType)->getValueType()),
                getDifferentialZeroOfType(builder, as<IRDifferentialPairUserCodeType>(diffType)->getValueType()));
            builder->markInstAsDifferential(makeDiffPair, as<IRDifferentialPairUserCodeType>(diffType)->getValueType());
            return makeDiffPair;
        }
        }

        if (auto arrayType = as<IRArrayType>(originalType))
        {
            auto diffElementType =
                (IRType*)differentiableTypeConformanceContext.getDifferentialForType(
                    builder, arrayType->getElementType());
            SLANG_RELEASE_ASSERT(diffElementType);
            auto diffArrayType = builder->getArrayType(diffElementType, arrayType->getElementCount());
            auto diffElementZero = getDifferentialZeroOfType(builder, arrayType->getElementType());
            auto result = builder->emitMakeArrayFromElement(diffArrayType, diffElementZero);
            builder->markInstAsDifferential(result, primalType);
            return result;
        }

        // Since primalType has a corresponding differential type, we can lookup the 
        // definition for zero().
        IRInst* zeroMethod = nullptr;
        if (auto lookupInterface = as<IRLookupWitnessMethod>(diffType))
        {
            // if the differential type itself comes from a witness lookup, we can just lookup the
            // zero method from the same witness table.
            auto wt = lookupInterface->getWitnessTable();
            zeroMethod = builder->emitLookupInterfaceMethodInst(builder->getFuncType(List<IRType*>(), diffType), wt, autoDiffSharedContext->zeroMethodStructKey);
            builder->markInstAsPrimal(zeroMethod);
        }
        else
        {
            zeroMethod = differentiableTypeConformanceContext.getZeroMethodForType(builder, originalType);
        }
        SLANG_RELEASE_ASSERT(zeroMethod);

        auto emptyArgList = List<IRInst*>();

        auto callInst = builder->emitCallInst((IRType*)diffType, zeroMethod, emptyArgList);
        builder->markInstAsDifferential(callInst, primalType);

        if (diffOuterType && isExistentialType(diffOuterType))
        {
            // Need to wrap the result back into an existential.
            auto existentialZero = builder->emitMakeExistential(
                diffOuterType,
                callInst,
                diffWitnessTable);
            builder->markInstAsDifferential(existentialZero, primalType);
            return existentialZero;
        }
        else
            return callInst;
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

InstPair AutoDiffTranscriberBase::transcribeBlockImpl(IRBuilder* builder, IRBlock* origBlock, HashSet<IRInst*>& instsToSkip)
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

InstPair AutoDiffTranscriberBase::transcribeNonDiffInst(IRBuilder* builder, IRInst* origInst)
{
    auto primal = cloneInst(&cloneEnv, builder, origInst);
    return InstPair(primal, nullptr);
}

InstPair AutoDiffTranscriberBase::transcribeReturn(IRBuilder* builder, IRReturn* origReturn)
{
    IRInst* origReturnVal = origReturn->getVal();

    auto returnDataType = (IRType*)findOrTranscribePrimalInst(builder, origReturnVal->getDataType());
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

static void _markGenericChildrenWithoutRelaventUse(IRGeneric* origGeneric, HashSet<IRInst*>& outInstsToSkip)
{
    for (;;)
    {
        bool changed = false;
        for (auto inst = origGeneric->getFirstBlock()->getFirstOrdinaryInst(); inst;
            inst = inst->getNextInst())
        {
            // If an inst is only referenced by a UserDefinedDerivativeDecoration, we need to skip
            // its transcription.
            switch (inst->getOp())
            {
            case kIROp_Return:
                continue;
            default:
                break;
            }

            bool hasRelaventUse = false;
            for (auto use = inst->firstUse; use; use = use->nextUse)
            {
                switch (use->getUser()->getOp())
                {
                case kIROp_UserDefinedBackwardDerivativeDecoration:
                case kIROp_ForwardDerivativeDecoration:
                case kIROp_BackwardDerivativeDecoration:
                case kIROp_BackwardDerivativeIntermediateTypeDecoration:
                case kIROp_BackwardDerivativePrimalContextDecoration:
                case kIROp_BackwardDerivativePrimalDecoration:
                case kIROp_BackwardDerivativePropagateDecoration:
                case kIROp_PrimalSubstituteDecoration:
                    break;
                default:
                    if (!outInstsToSkip.contains(use->getUser()))
                    {
                        hasRelaventUse = true;
                    }
                    break;
                }
            }
            if (!hasRelaventUse)
            {
                if (outInstsToSkip.add(inst))
                {
                    changed = true;
                }
            }
        }
        if (!changed)
            break;
    }
}

// Transcribe a generic definition
InstPair AutoDiffTranscriberBase::transcribeGeneric(IRBuilder* inBuilder, IRGeneric* origGeneric)
{
    auto innerVal = findInnerMostGenericReturnVal(origGeneric);
    if (auto innerFunc = as<IRFunc>(innerVal))
    {
        maybeMigrateDifferentiableDictionaryFromDerivativeFunc(inBuilder, innerFunc);
        if (!innerFunc->findDecoration<IRDifferentiableTypeDictionaryDecoration>())
            return InstPair(origGeneric, nullptr);
        differentiableTypeConformanceContext.setFunc(innerFunc);
    }
    else if (const auto funcType = as<IRFuncType>(innerVal))
    {
    }
    else
    {
        return InstPair(origGeneric, nullptr);
    }

    IRGeneric* primalGeneric = origGeneric;

    IRBuilder builder = *inBuilder;
    builder.setInsertBefore(origGeneric);

    auto diffGeneric = builder.emitGeneric();
    
    mapDifferentialInst(origGeneric, diffGeneric);

    // Process type of generic. If the generic is a function, then it's type will also be a 
    // generic and this logic will transcribe that generic first before continuing with the 
    // function itself.
    // 
    auto primalType = primalGeneric->getFullType();

    IRType* diffType = nullptr;
    if (primalType)
    {
        diffType = (IRType*)findOrTranscribeDiffInst(&builder, primalType);
    }

    diffGeneric->setFullType(diffType);

    HashSet<IRInst*> instsToSkip;
    _markGenericChildrenWithoutRelaventUse(origGeneric, instsToSkip);

    // Transcribe children from origFunc into diffFunc.
    builder.setInsertInto(diffGeneric);
    auto bodyBlock = builder.emitBlock();
    mapPrimalInst(origGeneric->getFirstBlock(), bodyBlock);
    mapDifferentialInst(origGeneric->getFirstBlock(), bodyBlock);
    transcribeBlockImpl(&builder, origGeneric->getFirstBlock(), instsToSkip);

    return InstPair(primalGeneric, diffGeneric);
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

IRInst* AutoDiffTranscriberBase::transcribe(IRBuilder* builder, IRInst* origInst)
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

    if (auto primalInst = pair.primal)
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
                if (auto primalNameHint = primalInst->findDecoration<IRNameHintDecoration>())
                {
                    StringBuilder sb;
                    sb << "s_diff_" << primalNameHint->getName();
                    builder->addNameHintDecoration(pair.differential, sb.getUnownedSlice());
                }

                // Automatically tag the primal and differential results
                // if they haven't already been handled by the 
                // code.
                // 
                if (pair.primal != pair.differential)
                {
                    if (!pair.differential->findDecoration<IRAutodiffInstDecoration>()
                        && !as<IRConstant>(pair.differential))
                    {
                        auto primalType = (IRType*)(pair.primal->getDataType());
                        builder->markInstAsDifferential(pair.differential, primalType);
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
                        builder->markInstAsMixedDifferential(pair.primal, mixedType);
                    }
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

InstPair AutoDiffTranscriberBase::transcribeInst(IRBuilder* builder, IRInst* origInst)
{
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

    // At this point we should not see any global insts that are differentiable.
    // If the inst's parent is IRModule, return (inst, null).
    // 
    if (as<IRModuleInst>(origInst->getParent()) && !as<IRType>(origInst))
        return InstPair(origInst, nullptr);

    auto result = transcribeInstImpl(builder, origInst);

    if (result.primal == nullptr && result.differential == nullptr)
    {
        if (auto origType = as<IRType>(origInst))
        {
            // If this is a generic type, transcibe the parent 
            // generic and derive the type from the transcribed generic's
            // return value.
            // 
            if (as<IRGeneric>(origType->getParent()->getParent()) &&
                findInnerMostGenericReturnVal(as<IRGeneric>(origType->getParent()->getParent())) == origType &&
                !instsInProgress.contains(origType->getParent()->getParent()))
            {
                auto origGenericType = origType->getParent()->getParent();
                auto diffGenericType = findOrTranscribeDiffInst(builder, origGenericType);
                auto innerDiffGenericType = findInnerMostGenericReturnVal(as<IRGeneric>(diffGenericType));
                result = InstPair(
                    origGenericType,
                    innerDiffGenericType
                );
            }
            else
            {
                IRInst* primal = maybeCloneForPrimalInst(builder, origType);
                auto diffType = _differentiateTypeImpl(builder, origType);
                result = InstPair(primal, diffType);
            }
        }
    }

    if (result.primal == nullptr && result.differential == nullptr)
    {
        // If we reach this statement, the instruction type is likely unhandled.
        getSink()->diagnose(origInst->sourceLoc,
            Diagnostics::unimplemented,
            "this instruction cannot be differentiated");
    }

    return result;
}

}
