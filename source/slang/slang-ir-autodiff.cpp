#include "slang-ir-autodiff.h"
#include "slang-ir-address-analysis.h"
#include "slang-ir-autodiff-rev.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-pairs.h"
#include "slang-ir-single-return.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-validate.h"
#include "../core/slang-performance-profiler.h"

namespace Slang
{

bool isBackwardDifferentiableFunc(IRInst* func)
{
    for (auto decorations : func->getDecorations())
    {
        switch (decorations->getOp())
        {
        case kIROp_BackwardDifferentiableDecoration:
        case kIROp_UserDefinedBackwardDerivativeDecoration:
            return true;
        }
    }
    return false;
}

IRInst* _lookupWitness(IRBuilder* builder, IRInst* witness, IRInst* requirementKey)
{
    if (auto witnessTable = as<IRWitnessTable>(witness))
    {
        for (auto entry : witnessTable->getEntries())
        {
            if (entry->getRequirementKey() == requirementKey)
                return entry->getSatisfyingVal();
        }
    }
    else if (auto interfaceType = as<IRInterfaceType>(witness))
    {
        for (UIndex ii = 0; ii < interfaceType->getOperandCount(); ii++)
        {
            auto entry = cast<IRInterfaceRequirementEntry>(interfaceType->getOperand(ii));
            if (entry->getRequirementKey() == requirementKey)
                return entry->getRequirementVal();
        }
    }
    else
    {
        return builder->emitLookupInterfaceMethodInst(
            builder->getTypeKind(),
            witness,
            requirementKey);
    }
    return nullptr;
}

static IRInst* _getDiffTypeFromPairType(AutoDiffSharedContext*sharedContext, IRBuilder* builder, IRDifferentialPairTypeBase* type)
{
    auto witness = type->getWitness();
    SLANG_RELEASE_ASSERT(witness);

    // Special case when the primal type is an InterfaceType/AssociatedType
    if (as<IRInterfaceType>(type->getValueType()) || as<IRAssociatedType>(type->getValueType()))
    {
        // The differential type is the IDifferentiable interface type.
        return sharedContext->differentiableInterfaceType;
    }
    
    return _lookupWitness(builder, witness, sharedContext->differentialAssocTypeStructKey);
}

static IRInst* _getDiffTypeWitnessFromPairType(AutoDiffSharedContext* sharedContext, IRBuilder* builder, IRDifferentialPairTypeBase* type)
{
    auto witnessTable = type->getWitness();
    return _lookupWitness(builder, witnessTable, sharedContext->differentialAssocTypeWitnessStructKey);
}

bool isNoDiffType(IRType* paramType)
{
    while (auto ptrType = as<IRPtrTypeBase>(paramType))
        paramType = ptrType->getValueType();
    while (auto attrType = as<IRAttributedType>(paramType))
    {
        if (attrType->findAttr<IRNoDiffAttr>())
        {
            return true;
        }
    }
    return false;
}

IRInst* lookupForwardDerivativeReference(IRInst* primalFunction)
{
    if (auto jvpDefinition = primalFunction->findDecoration<IRForwardDerivativeDecoration>())
        return jvpDefinition->getForwardDerivativeFunc();
    return nullptr;
}

IRInst* DifferentialPairTypeBuilder::findSpecializationForParam(IRInst* specializeInst, IRInst* genericParam)
{
    // Get base generic that's being specialized.
    auto genericType = as<IRGeneric>(as<IRSpecialize>(specializeInst)->getBase());
    SLANG_ASSERT(genericType);
    
    // Find the index of genericParam in the base generic.
    int paramIndex = -1;
    int currentIndex = 0;
    for (auto param : genericType->getParams())
    {
        if (param == genericParam)
            paramIndex = currentIndex;
        currentIndex ++;
    }

    SLANG_ASSERT(paramIndex >= 0);

    // Return the corresponding operand in the specialization inst.
    return specializeInst->getOperand(1 + paramIndex);
}

IRInst* DifferentialPairTypeBuilder::emitFieldAccessor(IRBuilder* builder, IRInst* baseInst, IRStructKey* key)
{
    IRInst* pairType = nullptr;
    if (auto basePtrType = as<IRPtrTypeBase>(baseInst->getDataType()))
    {
        auto loweredType = lowerDiffPairType(builder, basePtrType->getValueType());

        pairType = builder->getPtrType(kIROp_PtrType, (IRType*)loweredType);
    }
    else
    {
        auto loweredType = lowerDiffPairType(builder, baseInst->getDataType());
        pairType = loweredType;
    }

    if (auto basePairStructType = as<IRStructType>(pairType))
    {
        return as<IRFieldExtract>(builder->emitFieldExtract(
                findStructField(basePairStructType, key)->getFieldType(),
                baseInst,
                key
            ));
    }
    else if (auto ptrType = as<IRPtrTypeBase>(pairType))
    {
        if (auto ptrInnerSpecializedType = as<IRSpecialize>(ptrType->getValueType()))
        {
            auto genericType = findInnerMostGenericReturnVal(as<IRGeneric>(ptrInnerSpecializedType->getBase()));
            if (const auto genericBasePairStructType = as<IRStructType>(genericType))
            {
                return as<IRFieldAddress>(builder->emitFieldAddress(
                    builder->getPtrType((IRType*)
                        findSpecializationForParam(
                            ptrInnerSpecializedType,
                            findStructField(ptrInnerSpecializedType, key)->getFieldType())),
                    baseInst,
                    key
                ));
            }
        }
        else if (auto ptrBaseStructType = as<IRStructType>(ptrType->getValueType()))
        {
            return as<IRFieldAddress>(builder->emitFieldAddress(
                builder->getPtrType((IRType*)
                        findStructField(ptrBaseStructType, key)->getFieldType()),
                baseInst,
                key));
        }
    }
    else if (auto specializedType = as<IRSpecialize>(pairType))
    {
        // TODO: Stopped here -> The type being emitted is incorrect. don't emit the generic's
        // type, emit the specialization type.
        // 
        auto genericType = findInnerMostGenericReturnVal(as<IRGeneric>(specializedType->getBase()));
        if (auto genericBasePairStructType = as<IRStructType>(genericType))
        {
            return as<IRFieldExtract>(builder->emitFieldExtract(
                (IRType*)findSpecializationForParam(
                    specializedType,
                    findStructField(genericBasePairStructType, key)->getFieldType()),
                baseInst,
                key
            ));
        }
        else if (auto genericPtrType = as<IRPtrTypeBase>(genericType))
        {
            if (auto genericPairStructType = as<IRStructType>(genericPtrType->getValueType()))
            {
                return as<IRFieldAddress>(builder->emitFieldAddress(
                        builder->getPtrType((IRType*)
                            findSpecializationForParam(
                                specializedType,
                                findStructField(genericPairStructType, key)->getFieldType())),
                        baseInst,
                        key
                    ));
            }
        }
    }
    else
    {
        SLANG_UNEXPECTED("Unrecognized field. Cannot emit field accessor");
    }
    return nullptr;
}

IRInst* DifferentialPairTypeBuilder::emitPrimalFieldAccess(IRBuilder* builder, IRInst* baseInst)
{
    return emitFieldAccessor(builder, baseInst, this->globalPrimalKey);
}

IRInst* DifferentialPairTypeBuilder::emitDiffFieldAccess(IRBuilder* builder, IRInst* baseInst)
{
    return emitFieldAccessor(builder, baseInst, this->globalDiffKey);
}

IRStructKey* DifferentialPairTypeBuilder::_getOrCreateDiffStructKey()
{
    if (!this->globalDiffKey)
    {
        IRBuilder builder(sharedContext->moduleInst);
        // Insert directly at top level (skip any generic scopes etc.)
        builder.setInsertInto(sharedContext->moduleInst);

        this->globalDiffKey = builder.createStructKey();
        builder.addNameHintDecoration(this->globalDiffKey , UnownedTerminatedStringSlice("differential"));
    }

    return this->globalDiffKey;
}

IRStructKey* DifferentialPairTypeBuilder::_getOrCreatePrimalStructKey()
{
    if (!this->globalPrimalKey)
    {
        // Insert directly at top level (skip any generic scopes etc.)
        IRBuilder builder(sharedContext->moduleInst);
        builder.setInsertInto(sharedContext->moduleInst);

        this->globalPrimalKey = builder.createStructKey();
        builder.addNameHintDecoration(this->globalPrimalKey , UnownedTerminatedStringSlice("primal"));
    }

    return this->globalPrimalKey;
}

IRInst* DifferentialPairTypeBuilder::_createDiffPairType(IRType* origBaseType, IRType* diffType)
{
    switch (origBaseType->getOp())
    {
    case kIROp_LookupWitness:
    case kIROp_Specialize:
    case kIROp_Param:
        return nullptr;
    default:
        break;
    }

    IRBuilder builder(sharedContext->moduleInst);
    builder.setInsertBefore(diffType);

    auto pairStructType = builder.createStructType();
    StringBuilder nameBuilder;
    nameBuilder << "DiffPair_";
    getTypeNameHint(nameBuilder, origBaseType);
    builder.addNameHintDecoration(pairStructType, nameBuilder.toString().getUnownedSlice());

    builder.createStructField(pairStructType, _getOrCreatePrimalStructKey(), origBaseType);
    builder.createStructField(pairStructType, _getOrCreateDiffStructKey(), (IRType*)diffType);
    return pairStructType;
}

IRInst* DifferentialPairTypeBuilder::lowerDiffPairType(
    IRBuilder* builder, IRType* originalPairType)
{
    IRInst* result = nullptr;
    auto pairType = as<IRDifferentialPairTypeBase>(originalPairType);
    if (!pairType)
        return originalPairType;

    // We make our type cache keyed on the primal type, not the pair type.
    // This is because there may be duplicate pair types for the same
    // primal type but different witness tables, and we don't want to treat
    // them as distinct.
    // We might want to consider making witness tables part of IR
    // deduplication (make them HOISTABLE insts), but that is a bigger
    // change. Another alternative is to make the witness operand of
    // `IRDifferentialPairTypeBase` be child instead of an operand
    // so that it is not considered part of the type for deduplication
    // purposes.

    auto primalType = pairType->getValueType();
    if (pairTypeCache.tryGetValue(primalType, result))
        return result;
    if (!pairType)
    {
        result = originalPairType;
        return result;
    }
    if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(primalType))
    {
        result = nullptr;
        return result;
    }

    auto diffType = _getDiffTypeFromPairType(sharedContext, builder, pairType);
    if (!diffType)
        return result;
    result = _createDiffPairType(pairType->getValueType(), (IRType*)diffType);
    pairTypeCache.add(primalType, result);

    return result;
}

AutoDiffSharedContext::AutoDiffSharedContext(TargetProgram* target, IRModuleInst* inModuleInst)
    : moduleInst(inModuleInst), targetProgram(target)
{
    differentiableInterfaceType = as<IRInterfaceType>(findDifferentiableInterface());
    if (differentiableInterfaceType)
    {
        differentialAssocTypeStructKey = findDifferentialTypeStructKey();
        differentialAssocTypeWitnessStructKey = findDifferentialTypeWitnessStructKey();
        zeroMethodStructKey = findZeroMethodStructKey();
        addMethodStructKey = findAddMethodStructKey();
        mulMethodStructKey = findMulMethodStructKey();
        nullDifferentialStructType = findNullDifferentialStructType();
        nullDifferentialWitness = findNullDifferentialWitness();

        if (differentialAssocTypeStructKey)
            isInterfaceAvailable = true;
    }
}

IRInst* AutoDiffSharedContext::findDifferentiableInterface()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            // TODO: This seems like a particularly dangerous way to look for an interface.
            // See if we can lower IDifferentiable to a separate IR inst.
            //
            if (auto intf = as<IRInterfaceType>(globalInst))
            {
                if (auto decor = intf->findDecoration<IRNameHintDecoration>())
                {
                    if (decor->getName() == toSlice("IDifferentiable"))
                    {
                        return globalInst;
                    }
                }
            }
        }
    }
    return nullptr;
}

IRStructType* AutoDiffSharedContext::findNullDifferentialStructType()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            // TODO: Also a particularly dangerous way to look for a struct...
            if (auto structType = as<IRStructType>(globalInst))
            {
                if (auto decor = structType->findDecoration<IRNameHintDecoration>())
                {
                    if (decor->getName() == toSlice("NullDifferential"))
                    {
                        return structType;
                    }
                }
            }
        }
    }
    return nullptr;
}

IRInst* AutoDiffSharedContext::findNullDifferentialWitness()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto witnessTable = as<IRWitnessTable>(globalInst))
            {
                if (witnessTable->getConformanceType() == differentiableInterfaceType
                    && witnessTable->getConcreteType() == nullDifferentialStructType)
                    return witnessTable;
            }
        }
    }

    return nullptr;
}


IRStructKey* AutoDiffSharedContext::getIDifferentiableStructKeyAtIndex(UInt index)
{
    if (as<IRModuleInst>(moduleInst) && differentiableInterfaceType)
    {
        // Assume for now that IDifferentiable has exactly five fields.
        SLANG_ASSERT(differentiableInterfaceType->getOperandCount() == 5);
        if (auto entry = as<IRInterfaceRequirementEntry>(differentiableInterfaceType->getOperand(index)))
            return as<IRStructKey>(entry->getRequirementKey());
        else
        {
            SLANG_UNEXPECTED("IDifferentiable interface entry unexpected type");
        }
    }

    return nullptr;
}

void DifferentiableTypeConformanceContext::setFunc(IRGlobalValueWithCode* func)
{
    parentFunc = func;


    auto decor = func->findDecoration<IRDifferentiableTypeDictionaryDecoration>();
    SLANG_RELEASE_ASSERT(decor);

    // Build lookup dictionary for type witnesses.
    for (auto child = decor->getFirstChild(); child; child = child->next)
    {
        if (auto item = as<IRDifferentiableTypeDictionaryItem>(child))
        {
            auto existingItem = differentiableWitnessDictionary.tryGetValue(item->getConcreteType());
            if (existingItem)
            {
                *existingItem = item->getWitness();
            }
            else
            {
                differentiableWitnessDictionary.add((IRType*)item->getConcreteType(), item->getWitness());

                // Also register the type's differential type with the same witness.
                IRBuilder subBuilder(item->getConcreteType());
                if (!as<IRInterfaceType>(item->getConcreteType()))
                {
                    differentiableWitnessDictionary.addIfNotExists(
                        (IRType*)_lookupWitness(&subBuilder, item->getWitness(), sharedContext->differentialAssocTypeStructKey), 
                        item->getWitness());
                }

                if (auto diffPairType = as<IRDifferentialPairTypeBase>(item->getConcreteType()))
                {
                    // For differential pair types, register the differential type as well.
                    IRBuilder builder(diffPairType);
                    builder.setInsertAfter(diffPairType->getWitness());
                    auto diffType = _lookupWitness(&builder, diffPairType->getWitness(), sharedContext->differentialAssocTypeStructKey);
                    auto diffWitness = _lookupWitness(&builder, diffPairType->getWitness(), sharedContext->differentialAssocTypeWitnessStructKey);
                    if (diffType && diffWitness)
                    {
                        differentiableWitnessDictionary.addIfNotExists((IRType*)diffType, diffWitness);
                    }
                }
            }
        }
    }
}

IRInst* DifferentiableTypeConformanceContext::lookUpConformanceForType(IRInst* type)
{
    IRInst* foundResult = nullptr;
    differentiableWitnessDictionary.tryGetValue(type, foundResult);
    return foundResult;
}

IRInst* DifferentiableTypeConformanceContext::lookUpInterfaceMethod(IRBuilder* builder, IRType* origType, IRStructKey* key)
{
    if (auto conformance = tryGetDifferentiableWitness(builder, origType))
        return _lookupWitness(builder, conformance, key);
    return nullptr;
}

IRInst* DifferentiableTypeConformanceContext::getDifferentialTypeFromDiffPairType(
    IRBuilder*, IRDifferentialPairTypeBase*)
{
    SLANG_UNIMPLEMENTED_X("");
}

IRInst* DifferentiableTypeConformanceContext::getDiffTypeFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type)
{
    return _getDiffTypeFromPairType(sharedContext, builder, type);
}

IRInst* DifferentiableTypeConformanceContext::getDiffTypeWitnessFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type)
{
    return _getDiffTypeWitnessFromPairType(sharedContext, builder, type);
}

IRInst* DifferentiableTypeConformanceContext::getDiffZeroMethodFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type)
{
    auto witnessTable = type->getWitness();
    return _lookupWitness(builder, witnessTable, sharedContext->zeroMethodStructKey);
}

IRInst* DifferentiableTypeConformanceContext::getDiffAddMethodFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type)
{
    auto witnessTable = type->getWitness();
    return _lookupWitness(builder, witnessTable, sharedContext->addMethodStructKey);
}

IRInst *DifferentiableTypeConformanceContext::tryExtractConformanceFromInterfaceType(IRBuilder *builder, IRInterfaceType *interfaceType, IRWitnessTable *witnessTable)
{
    SLANG_RELEASE_ASSERT(interfaceType);

    List<IRInterfaceRequirementEntry*> lookupKeyPath = findDifferentiableInterfaceLookupPath(
        sharedContext->differentiableInterfaceType, interfaceType);

    IRInst* differentialTypeWitness = witnessTable;
    if (lookupKeyPath.getCount())
    {
        // `interfaceType` does conform to `IDifferentiable`.
        for (auto node : lookupKeyPath)
        {
            differentialTypeWitness = builder->emitLookupInterfaceMethodInst((IRType*)node->getRequirementVal(), differentialTypeWitness, node->getRequirementKey());
            // Lookup insts are always primal values.
            builder->markInstAsPrimal(differentialTypeWitness);
        }
        return differentialTypeWitness;
    }

    return nullptr;
}

// Given an interface type, return the lookup path from a witness table of `type` to a witness table of `IDifferentiable`.
static bool _findDifferentiableInterfaceLookupPathImpl(
    HashSet<IRInst*>& processedTypes,
    IRInterfaceType* idiffType,
    IRInterfaceType* type,
    List<IRInterfaceRequirementEntry*>& currentPath)
{
    if (processedTypes.contains(type))
        return false;
    processedTypes.add(type);

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

List<IRInterfaceRequirementEntry *> DifferentiableTypeConformanceContext::findDifferentiableInterfaceLookupPath(IRInterfaceType *idiffType, IRInterfaceType *type)
{
    List<IRInterfaceRequirementEntry*> currentPath;
    HashSet<IRInst*> processedTypes;
    _findDifferentiableInterfaceLookupPathImpl(processedTypes, idiffType, type, currentPath);
    return currentPath;
}

IRFunc *DifferentiableTypeConformanceContext::getOrCreateExistentialDAddMethod()
{
    if (this->existentialDAddFunc)
        return this->existentialDAddFunc;

    SLANG_ASSERT(sharedContext->differentiableInterfaceType);
    SLANG_ASSERT(sharedContext->nullDifferentialWitness);
    
    auto builder = IRBuilder(this->sharedContext->moduleInst);

    existentialDAddFunc = builder.createFunc();
    existentialDAddFunc->setFullType(builder.getFuncType(
        List<IRType*>({
            sharedContext->differentiableInterfaceType,
            sharedContext->differentiableInterfaceType,
        }),
        sharedContext->differentiableInterfaceType));
    
    builder.setInsertInto(existentialDAddFunc);
    auto entryBlock = builder.emitBlock();

    builder.setInsertInto(entryBlock);

    // Insert parameters.
    auto aObj = builder.emitParam(sharedContext->differentiableInterfaceType);
    auto bObj = builder.emitParam(sharedContext->differentiableInterfaceType);
    
    // Check if a.type == null_differential.type
    auto aObjWitnessIsNull = builder.emitIsDifferentialNull(aObj);
    
    // If aObjWitnessTable is null, return bObj.
    auto aObjWitnessIsNullBlock = builder.emitBlock();
    builder.setInsertInto(aObjWitnessIsNullBlock);
    builder.emitReturn(bObj);

    auto aObjWitnessIsNotNullBlock = builder.emitBlock();
    builder.setInsertInto(aObjWitnessIsNotNullBlock);
    
    // Check if b.type == null_differential.type
    auto bObjWitnessIsNull = builder.emitIsDifferentialNull(bObj);

    // If bObjWitnessTable is null, return aObj.
    auto bObjWitnessIsNullBlock = builder.emitBlock();
    builder.setInsertInto(bObjWitnessIsNullBlock);
    builder.emitReturn(aObj);

    auto bObjWitnessIsNotNullBlock = builder.emitBlock();

    // Emit aObj.type::dadd(aObj.val, bObj.val)
    //
    // Important: we're looking up dadd on the differential type, and 
    // not the primal type. This assumes that the two methods are identical,
    // which (mathematically) they should be.
    // 
    auto concreteDiffTypeWitnessTable = builder.emitExtractExistentialWitnessTable(aObj);

    // Extract func type from the witness table type.
    IRFuncType* dAddFuncType = nullptr;
    for (UIndex ii = 0; ii < sharedContext->differentiableInterfaceType->getOperandCount(); ii++)
    {
        auto entry = cast<IRInterfaceRequirementEntry>(sharedContext->differentiableInterfaceType->getOperand(ii));
        if (entry->getRequirementKey() == sharedContext->addMethodStructKey)
        {
            dAddFuncType = cast<IRFuncType>(entry->getRequirementVal());
            break;
        }
    }

    SLANG_ASSERT(dAddFuncType);

    auto dAddMethod = builder.emitLookupInterfaceMethodInst(
        dAddFuncType,
        concreteDiffTypeWitnessTable,
        sharedContext->addMethodStructKey);
    
    // Call
    auto dAddResult = builder.emitCallInst(
        dAddFuncType->getResultType(),
        dAddMethod,
        List<IRInst*>({
            builder.emitExtractExistentialValue(dAddFuncType->getParamType(0), aObj),
            builder.emitExtractExistentialValue(dAddFuncType->getParamType(1), bObj)}));
    
    // Wrap result in existential.
    auto existentialDiffType = builder.emitMakeExistential(
        sharedContext->differentiableInterfaceType,
        dAddResult,
        concreteDiffTypeWitnessTable);
    
    builder.emitReturn(existentialDiffType);

    // Emit an unreachable block to act as the after block.
    auto unreachableBlock = builder.emitBlock();
    builder.setInsertInto(unreachableBlock);
    builder.emitUnreachable();

    // Link up conditional blocks.
    builder.setInsertInto(entryBlock);
    builder.emitIfElse(
        aObjWitnessIsNull,
        aObjWitnessIsNullBlock,
        aObjWitnessIsNotNullBlock,
        unreachableBlock);
    
    builder.setInsertInto(aObjWitnessIsNotNullBlock);
    builder.emitIfElse(
        bObjWitnessIsNull,
        bObjWitnessIsNullBlock,
        bObjWitnessIsNotNullBlock,
        unreachableBlock);

    builder.addNameHintDecoration(existentialDAddFunc, UnownedStringSlice("__existential_dadd"));
    builder.addBackwardDifferentiableDecoration(existentialDAddFunc);
    
    return existentialDAddFunc;
}

void DifferentiableTypeConformanceContext::buildGlobalWitnessDictionary()
{
    for (auto globalInst : sharedContext->moduleInst->getChildren())
    {
        if (auto pairType = as<IRDifferentialPairTypeBase>(globalInst))
        {
            differentiableWitnessDictionary.addIfNotExists(pairType->getValueType(), pairType->getWitness());
        }
    }
}

IRType* DifferentiableTypeConformanceContext::differentiateType(IRBuilder* builder, IRInst* primalType)
{
    if (auto ptrType = as<IRPtrTypeBase>(primalType))
        return builder->getPtrType(
            primalType->getOp(),
            differentiateType(builder, ptrType->getValueType()));

    // Special case certain compound types (PtrType, FuncType, etc..)
    // otherwise try to lookup a differential definition for the given type.
    // If one does not exist, then we assume it's not differentiable.
    // 
    switch (primalType->getOp())
    {
    case kIROp_Param:
        if (as<IRTypeType>(primalType->getDataType()))
            return differentiateType(builder, primalType);
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
            getDiffTypeFromPairType(builder, primalPairType),
            getDiffTypeWitnessFromPairType(builder, primalPairType));
    }

    case kIROp_DifferentialPairUserCodeType:
    {
        auto primalPairType = as<IRDifferentialPairUserCodeType>(primalType);
        return builder->getDifferentialPairUserCodeType(
            (IRType*)getDiffTypeFromPairType(builder, primalPairType),
            getDiffTypeWitnessFromPairType(builder, primalPairType));
    }

    case kIROp_FuncType:
    {
        SLANG_UNIMPLEMENTED_X("Impl");
    }

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
        SLANG_UNIMPLEMENTED_X("Impl");
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
        return (IRType*)getDifferentialForType(builder, (IRType*)primalType);
    }
}

IRInst* DifferentiableTypeConformanceContext::tryGetDifferentiableWitness(IRBuilder* builder, IRInst* primalType)
{
    if (isNoDiffType((IRType*)primalType))
        return nullptr;

    IRInst* witness = lookUpConformanceForType((IRType*)primalType);
    if (witness)
    {
        SLANG_RELEASE_ASSERT(witness || as<IRArrayType>(primalType));
    }

    if (!witness)
    {
        SLANG_RELEASE_ASSERT(primalType);
        if (auto primalPairType = as<IRDifferentialPairTypeBase>(primalType))
        {
            witness = getOrCreateDifferentiablePairWitness(builder, primalPairType);
        }
        else if (auto arrayType = as<IRArrayType>(primalType))
        {
            witness = getArrayWitness(builder, arrayType);
        }
        else if (auto extractExistential = as<IRExtractExistentialType>(primalType))
        {
            witness = getExtractExistensialTypeWitness(builder, extractExistential);
        }
    }
    return witness;
}

IRType* DifferentiableTypeConformanceContext::getOrCreateDiffPairType(IRBuilder* builder, IRInst* primalType, IRInst* witness)
{
    return builder->getDifferentialPairType(
        (IRType*)primalType,
        witness);
}

IRInst* DifferentiableTypeConformanceContext::getOrCreateDifferentiablePairWitness(IRBuilder* builder, IRDifferentialPairTypeBase* pairType)
{
    // Differentiate the pair type to get it's differential (which is itself a pair)
    auto diffDiffPairType = (IRType*)differentiateType(builder, (IRType*)pairType);

    auto addMethod = builder->createFunc();
    auto zeroMethod = builder->createFunc();

    auto table = builder->createWitnessTable(this->sharedContext->differentiableInterfaceType, (IRType*)pairType);

    // And place it in the synthesized witness table.
    builder->createWitnessTableEntry(table, sharedContext->differentialAssocTypeStructKey, diffDiffPairType);
    builder->createWitnessTableEntry(table, sharedContext->differentialAssocTypeWitnessStructKey, table);
    builder->createWitnessTableEntry(table, sharedContext->addMethodStructKey, addMethod);
    builder->createWitnessTableEntry(table, sharedContext->zeroMethodStructKey, zeroMethod);

    bool isUserCodeType = as<IRDifferentialPairUserCodeType>(pairType) ? true : false;

    // Fill in differential method implementations.
    auto elementType = as<IRDifferentialPairTypeBase>(pairType)->getValueType();
    auto innerWitness = as<IRDifferentialPairTypeBase>(pairType)->getWitness();

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
        auto innerAdd = _lookupWitness(&b, innerWitness, sharedContext->addMethodStructKey);
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
        auto innerZero = _lookupWitness(&b, innerWitness, sharedContext->zeroMethodStructKey);
        auto zeroVal = b.emitCallInst(elementType, innerZero, 0, nullptr);
        auto retVal =
            isUserCodeType
            ? b.emitMakeDifferentialPairUserCode(diffDiffPairType, zeroVal, zeroVal)
            : b.emitMakeDifferentialPair(diffDiffPairType, zeroVal, zeroVal);
        b.emitReturn(retVal);
    }
    
    // Record this in the context for future lookups
    differentiableWitnessDictionary[(IRType*)pairType] = table;

    return table;
}

IRInst* DifferentiableTypeConformanceContext::getArrayWitness(IRBuilder* builder, IRArrayType* arrayType)
{
    // Differentiate the pair type to get it's differential (which is itself a pair)
    auto diffArrayType = (IRType*)differentiateType(builder, (IRType*)arrayType);

    if (!diffArrayType)
        return nullptr;

    auto innerWitness = tryGetDifferentiableWitness(builder, as<IRArrayTypeBase>(arrayType)->getElementType());

    auto addMethod = builder->createFunc();
    auto zeroMethod = builder->createFunc();

    auto table = builder->createWitnessTable(sharedContext->differentiableInterfaceType, (IRType*)arrayType);

    // And place it in the synthesized witness table.
    builder->createWitnessTableEntry(table, sharedContext->differentialAssocTypeStructKey, diffArrayType);
    builder->createWitnessTableEntry(table, sharedContext->differentialAssocTypeWitnessStructKey, table);
    builder->createWitnessTableEntry(table, sharedContext->addMethodStructKey, addMethod);
    builder->createWitnessTableEntry(table, sharedContext->zeroMethodStructKey, zeroMethod);

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
        auto innerAdd = _lookupWitness(&b, innerWitness, sharedContext->addMethodStructKey);
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

        auto innerZero = _lookupWitness(&b, innerWitness, sharedContext->zeroMethodStructKey);
        auto zeroVal = b.emitCallInst(elementType, innerZero, 0, nullptr);
        auto retVal = b.emitMakeArrayFromElement(diffArrayType, zeroVal);
        b.emitReturn(retVal);
    }

    // Record this in the context for future lookups
    differentiableWitnessDictionary[(IRType*)arrayType] = table;

    return table;
}

IRInst* DifferentiableTypeConformanceContext::getExtractExistensialTypeWitness(
    IRBuilder* builder,
    IRExtractExistentialType* extractExistentialType)
{
    // Check that the type's base is differentiable
    if (differentiateType(builder, extractExistentialType->getOperand(0)->getDataType()))
    {
        return tryExtractConformanceFromInterfaceType(
            builder,
            cast<IRInterfaceType>(extractExistentialType->getOperand(0)->getDataType()),
            (IRWitnessTable*)builder->emitExtractExistentialWitnessTable(extractExistentialType->getOperand(0)));
    }
    
    return nullptr;
}


void copyCheckpointHints(IRBuilder* builder, IRGlobalValueWithCode* oldInst, IRGlobalValueWithCode* newInst)
{
    for (auto decor = oldInst->getFirstDecoration(); decor; decor = decor->getNextDecoration())
    {
        if (auto chkHint = as<IRCheckpointHintDecoration>(decor))
        {
            SLANG_ASSERT(chkHint->getOperandCount() == 0);
            builder->addDecoration(newInst, chkHint->getOp());
        }
    }
}

void stripDerivativeDecorations(IRInst* inst)
{
    for (auto decor = inst->getFirstDecoration(); decor; )
    {
        auto next = decor->getNextDecoration();
        switch (decor->getOp())
        {
        case kIROp_ForwardDerivativeDecoration:
        case kIROp_DerivativeMemberDecoration:
        case kIROp_BackwardDerivativeDecoration:
        case kIROp_BackwardDerivativeIntermediateTypeDecoration:
        case kIROp_BackwardDerivativePropagateDecoration:
        case kIROp_BackwardDerivativePrimalDecoration:
        case kIROp_UserDefinedBackwardDerivativeDecoration:
        case kIROp_AutoDiffOriginalValueDecoration:
            decor->removeAndDeallocate();
            break;
        default:
            break;
        }
        decor = next;
    }
}


void stripAutoDiffDecorationsFromChildren(IRInst* parent)
{
    for (auto inst : parent->getChildren())
    {
        for (auto decor = inst->getFirstDecoration(); decor; )
        {
            auto next = decor->getNextDecoration();
            switch (decor->getOp())
            {
            case kIROp_ForwardDerivativeDecoration:
            case kIROp_DerivativeMemberDecoration:
            case kIROp_DifferentiableTypeDictionaryDecoration:
            case kIROp_PrimalInstDecoration:
            case kIROp_DifferentialInstDecoration:
            case kIROp_MixedDifferentialInstDecoration:
            case kIROp_RecomputeBlockDecoration:
            case kIROp_LoopCounterDecoration:
            case kIROp_LoopCounterUpdateDecoration:
            case kIROp_BackwardDerivativeDecoration:
            case kIROp_BackwardDerivativeIntermediateTypeDecoration:
            case kIROp_BackwardDerivativePropagateDecoration:
            case kIROp_BackwardDerivativePrimalDecoration:
            case kIROp_BackwardDerivativePrimalContextDecoration:
            case kIROp_BackwardDerivativePrimalReturnDecoration:
            case kIROp_AutoDiffOriginalValueDecoration:
            case kIROp_UserDefinedBackwardDerivativeDecoration:
            case kIROp_IntermediateContextFieldDifferentialTypeDecoration:
                decor->removeAndDeallocate();
                break;
            default:
                break;
            }
            decor = next;
        }

        if (inst->getFirstChild() != nullptr)
        {
            stripAutoDiffDecorationsFromChildren(inst);
        }
    }
}

void stripAutoDiffDecorations(IRModule* module)
{
    stripAutoDiffDecorationsFromChildren(module->getModuleInst());
}


void stripTempDecorations(IRInst* inst)
{
    for (auto decor = inst->getFirstDecoration(); decor; )
    {
        auto next = decor->getNextDecoration();
        switch (decor->getOp())
        {
        case kIROp_DifferentialInstDecoration:
        case kIROp_MixedDifferentialInstDecoration:
        case kIROp_RecomputeBlockDecoration:
        case kIROp_AutoDiffOriginalValueDecoration:
        case kIROp_BackwardDerivativePrimalReturnDecoration:
        case kIROp_PrimalValueStructKeyDecoration:
        case kIROp_PrimalElementTypeDecoration:
            decor->removeAndDeallocate();
            break;
        default:
            break;
        }
        decor = next;
    }
    for (auto child : inst->getChildren())
    {
        stripTempDecorations(child);
    }
}


struct StripNoDiffTypeAttributePass : InstPassBase
{
    StripNoDiffTypeAttributePass(IRModule* module) :
        InstPassBase(module)
    {
    }
    void processModule()
    {
        processInstsOfType<IRAttributedType>(kIROp_AttributedType, [&](IRAttributedType* attrType)
            {
                if (attrType->getAllAttrs().getCount() == 1)
                {
                    if (attrType->findAttr<IRNoDiffAttr>())
                    {
                        attrType->replaceUsesWith(attrType->getBaseType());
                        attrType->removeAndDeallocate();
                    }
                }
            });
    }
};

void stripNoDiffTypeAttribute(IRModule* module)
{
    StripNoDiffTypeAttributePass pass(module);
    pass.processModule();
}

bool isDifferentiableType(DifferentiableTypeConformanceContext& context, IRInst* typeInst)
{
    if (!typeInst)
        return false;

    if (context.isDifferentiableType((IRType*)typeInst))
        return true;
    // Look for equivalent types.
    for (auto type : context.differentiableWitnessDictionary)
    {
        if (isTypeEqual(type.key, (IRType*)typeInst))
        {
            context.differentiableWitnessDictionary[(IRType*)typeInst] = type.value;
            return true;
        }
    }
    return false;
}

bool canTypeBeStored(IRInst* type)
{
    if (!type)
        return false;

    if (as<IRBasicType>(type))
        return true;

    switch (type->getOp())
    {
    case kIROp_StructType:
    case kIROp_OptionalType:
    case kIROp_TupleType:
    case kIROp_ArrayType:
    case kIROp_DifferentialPairType:
    case kIROp_DifferentialPairUserCodeType:
    case kIROp_InterfaceType:
    case kIROp_AssociatedType:
    case kIROp_AnyValueType:
    case kIROp_ClassType:
    case kIROp_FloatType:
    case kIROp_VectorType:
    case kIROp_MatrixType:
    case kIROp_BackwardDiffIntermediateContextType:
        return true;
    case kIROp_AttributedType:
        return canTypeBeStored(type->getOperand(0));
    default:
        return false;
    }
}

struct AutoDiffPass : public InstPassBase
{
    DiagnosticSink* getSink()
    {
        return sink;
    }

    bool processModule()
    {
        // TODO(sai): Move this call.
        forwardTranscriber.differentiableTypeConformanceContext.buildGlobalWitnessDictionary();

        IRBuilder builderStorage(module);
        IRBuilder* builder = &builderStorage;

        // Process all ForwardDifferentiate and BackwardDifferentiate instructions by 
        // generating derivative code for the referenced function.
        //
        bool modified = processReferencedFunctions(builder);

        return modified;
    }

    IRInst* processIntermediateContextTypeBase(IRBuilder* builder, IRInst* base)
    {
        if (auto spec = as<IRSpecialize>(base))
        {
            List<IRInst*> args;
            auto subBase = processIntermediateContextTypeBase(builder, spec->getBase());
            if (!subBase)
                return nullptr;
            for (UInt a = 0; a < spec->getArgCount(); a++)
                args.add(spec->getArg(a));
            auto actualType = builder->emitSpecializeInst(
                builder->getTypeKind(),
                subBase,
                args.getCount(),
                args.getBuffer());
            return actualType;
        }
        else if (auto baseGeneric = as<IRGeneric>(base))
        {
            auto inner = findGenericReturnVal(baseGeneric);
            if (auto typeDecor = inner->findDecoration<IRBackwardDerivativeIntermediateTypeDecoration>())
            {
                if (!isTypeFullyDifferentiated(typeDecor->getBackwardDerivativeIntermediateType()))
                    return nullptr;

                return typeDecor->getBackwardDerivativeIntermediateType();
            }
        }
        else if (auto func = as<IRFunc>(base))
        {
            if (auto typeDecor = func->findDecoration<IRBackwardDerivativeIntermediateTypeDecoration>())
            {
                if (!isTypeFullyDifferentiated(typeDecor->getBackwardDerivativeIntermediateType()))
                    return nullptr;
                return typeDecor->getBackwardDerivativeIntermediateType();
            }
        }
        else if (auto lookup = as<IRLookupWitnessMethod>(base))
        {
            auto key = lookup->getRequirementKey();
            if (auto typeDecor = key->findDecoration<IRBackwardDerivativeIntermediateTypeDecoration>())
            {
                auto typeKey = typeDecor->getBackwardDerivativeIntermediateType();
                auto typeLookup = builder->emitLookupInterfaceMethodInst(builder->getTypeKind(), lookup->getWitnessTable(), typeKey);
                return typeLookup;
            }
        }
        return nullptr;
    }

    bool lowerIntermediateContextType(IRBuilder* builder)
    {
        bool result = false;
        OrderedHashSet<IRInst*> loweredIntermediateTypes;

        // Replace all `BackwardDiffIntermediateContextType` insts with the struct type
        // that we generated during backward diff pass.
        for (;;)
        {
            bool changed = false;
            processAllInsts([&](IRInst* inst)
                {
                    switch (inst->getOp())
                    {
                    case kIROp_BackwardDiffIntermediateContextType:
                    {
                        auto differentiateInst = as<IRBackwardDiffIntermediateContextType>(inst);

                        auto baseFunc = differentiateInst->getOperand(0);
                        IRBuilder subBuilder = *builder;
                        subBuilder.setInsertBefore(inst);
                        auto type = processIntermediateContextTypeBase(&subBuilder, baseFunc);
                        if (type)
                        {
                            loweredIntermediateTypes.add(type);
                            inst->replaceUsesWith(type);
                            inst->removeAndDeallocate();
                            changed = true;
                        }
                    }
                    break;
                    default:
                        break;
                    }
                });
            result |= changed;
            if (!changed)
                break;
        }
        // Now we generate the differential type for the intermediate context type
        // to allow higher order differentiation.
        generateDifferentialImplementationForContextType(loweredIntermediateTypes);
        return result;
    }

    // Utility function for topology sorting the intermediate context types.
    bool isIntermediateContextTypeReadyForProcess(OrderedHashSet<IRInst*>& contextTypes, OrderedHashSet<IRInst*>& sortedSet, IRInst* t)
    {
        if (!contextTypes.contains(t))
            return true;

        switch (t->getOp())
        {
        case kIROp_StructType:
            {
                bool canAddNow = true;
                for (auto f : as<IRStructType>(t)->getFields())
                {
                    if (!isIntermediateContextTypeReadyForProcess(contextTypes, sortedSet, f->getFieldType()))
                    {
                        canAddNow = false;
                        break;
                    }
                }
                return canAddNow;
            }
        case kIROp_Specialize:
            return isIntermediateContextTypeReadyForProcess(contextTypes, sortedSet, as<IRSpecialize>(t)->getBase());
        case kIROp_Generic:
            return isIntermediateContextTypeReadyForProcess(contextTypes, sortedSet, findGenericReturnVal(as<IRGeneric>(t)));
        default:
            return true;
        }
    }

    struct IntermediateContextTypeDifferentialInfo
    {
        IRInst* diffType = nullptr;
        IRInst* diffWitness = nullptr;
        IRInst* diffDiffWitness = nullptr;
        IRInst* zeroMethod = nullptr;
        IRInst* addMethod = nullptr;
    };

    // Register the differential type for an intermediate context type to the derivative functions that uses the type.
    void registerDiffContextType(
        IRBuilder& builder,
        IRDifferentiableTypeDictionaryDecoration* diffDecor,
        OrderedDictionary<IRInst*, IntermediateContextTypeDifferentialInfo>& diffTypes,
        IRInst* origType)
    {
        HashSet<IRInst*> registeredType;
        for (auto entry : diffDecor->getChildren())
        {
            if (auto e = as<IRDifferentiableTypeDictionaryItem>(entry))
            {
                registeredType.add(e->getOperand(0));
            }
        }
        // Use a work list to recursively walk through all sub fields of the struct type.
        List<IRInst*> wlist;
        wlist.add(origType);
        for (Index i = 0; i < wlist.getCount(); i++)
        {
            auto t = wlist[i];
            IntermediateContextTypeDifferentialInfo diffInfo;
            if (!diffTypes.tryGetValue(t, diffInfo))
                continue;
            if (registeredType.add(t))
                builder.addDifferentiableTypeEntry(diffDecor, t, diffInfo.diffWitness);
            else
                continue;

            if (auto structType = as<IRStructType>(getResolvedInstForDecorations(t)))
            {
                for (auto f : structType->getFields())
                {
                    wlist.add(f->getFieldType());
                }
            }
        }
    }

    void generateDifferentialImplementationForContextType(OrderedHashSet<IRInst*>& contextTypes)
    {
        // First we are going to topology sort all intermediate context types.
        OrderedHashSet<IRInst*> sortedContextTypes;
        for (;;)
        {
            auto lastCount = sortedContextTypes.getCount();
            for (auto t : contextTypes)
            {
                if (sortedContextTypes.contains(t))
                    continue;
                // Have all dependent types been added yet?
                if (isIntermediateContextTypeReadyForProcess(contextTypes, sortedContextTypes, t))
                    sortedContextTypes.add(t);
            }
            if (lastCount == sortedContextTypes.getCount())
                break;
        }

        // After the types are sorted, we start to generate the differential type and IDifferentiable witnesses
        // for them.

        OrderedDictionary<IRInst*, IntermediateContextTypeDifferentialInfo> diffTypes;
        IRBuilder builder(module);
        for (auto t : sortedContextTypes)
        {
            if (t->getOp() == kIROp_Generic || t->getOp() == kIROp_StructType)
            {
                // For generics/struct types, we will generate a new generic/struct type representing the differntial.

                SLANG_RELEASE_ASSERT(t->getParent() && t->getParent()->getOp() == kIROp_Module);
                builder.setInsertBefore(t);
                auto diffInfo = fillDifferentialTypeImplementation(diffTypes, t);
                diffTypes[t] = diffInfo;
            }
            else if (auto specialize = as<IRSpecialize>(t))
            {
                // A specialize of a context type translates to a specialize of its differential type/witness.

                IntermediateContextTypeDifferentialInfo baseInfo;
                SLANG_RELEASE_ASSERT(diffTypes.tryGetValue(specialize->getBase(), baseInfo));
                builder.setInsertBefore(t);
                List<IRInst*> args;
                for (UInt i = 0; i < specialize->getArgCount(); i++)
                    args.add(specialize->getArg(i));
                IntermediateContextTypeDifferentialInfo info;
                info.diffType = builder.emitSpecializeInst(
                    builder.getTypeKind(), baseInfo.diffType, (UInt)args.getCount(), args.getBuffer());
                info.diffWitness = builder.emitSpecializeInst(
                    builder.getWitnessTableType(autodiffContext->differentiableInterfaceType),
                    baseInfo.diffWitness,
                    (UInt)args.getCount(),
                    args.getBuffer());
                diffTypes[t] = info;
            }
            else
            {
                // If `t` is not a specialize, it'd better be processed by now.
                // We currently don't support the `LookupInterfaceMethod` case, since it can't
                // appear in a derivative function because we will only call the backward diff function without a intermediate-type
                // via an interface.
                SLANG_RELEASE_ASSERT(diffTypes.containsKey(t));
            }
        }

        // Register the differential types into the conformance dictionaries of the functions that uses them.
        for (auto t : diffTypes)
        {
            HashSet<IRFunc*> registeredFuncs;
            for (auto use = t.key->firstUse; use; use = use->nextUse)
            {
                auto parentFunc = getParentFunc(use->getUser());
                if (!parentFunc)
                    continue;
                if (!registeredFuncs.add(parentFunc))
                    continue;
                if (auto dictDecor = parentFunc->findDecoration<IRDifferentiableTypeDictionaryDecoration>())
                {
                    registerDiffContextType(builder, dictDecor, diffTypes, t.key);
                }
            }
        }
    }
    
    IntermediateContextTypeDifferentialInfo fillDifferentialTypeImplementationForStruct(
        OrderedDictionary<IRInst*, IntermediateContextTypeDifferentialInfo>& diffTypes,
        IRStructType* originalType,
        IRStructType* diffType)
    {
        IntermediateContextTypeDifferentialInfo result;
        result.diffType = diffType;

        IRBuilder builder(diffType);
        builder.setInsertInto(diffType);

        // Generate the fields for all differentiable members of the original struct type.
        struct FieldInfo
        {
            IRStructField* field;
            IRInst* witness;
        };
        List<FieldInfo> diffFields;

        for (auto field : originalType->getFields())
        {
            IRInst* diffFieldWitness = nullptr;
            if (auto diffDecor = field->findDecoration<IRIntermediateContextFieldDifferentialTypeDecoration>())
            {
                diffFieldWitness = diffDecor->getDifferentialWitness();
            }
            else
            {
                IntermediateContextTypeDifferentialInfo diffFieldTypeInfo;
                diffTypes.tryGetValue(field->getFieldType(), diffFieldTypeInfo);
                diffFieldWitness = diffFieldTypeInfo.diffWitness;
            }
            if (diffFieldWitness)
            {
                FieldInfo info;
                IRBuilder keyBuilder = builder;
                keyBuilder.setInsertBefore(maybeFindOuterGeneric(originalType));
                auto diffKey = keyBuilder.createStructKey();
                auto diffFieldType = _lookupWitness(&keyBuilder, diffFieldWitness, autodiffContext->differentialAssocTypeStructKey);
                info.field = builder.createStructField(diffType, diffKey, (IRType*)diffFieldType);
                info.witness = diffFieldWitness;
                builder.addDecoration(field->getKey(), kIROp_DerivativeMemberDecoration, diffKey);
                builder.addDecoration(diffKey, kIROp_DerivativeMemberDecoration, diffKey);
                diffFields.add(info);
            }
        }
        
        builder.setInsertAfter(diffType);

        // Implement `dadd` and `dzero` methods.
        IRInst* zeroMethod = nullptr;
        {
            auto zeroMethodType = builder.getFuncType(List<IRType*>(), diffType);
            zeroMethod = builder.createFunc();
            zeroMethod->setFullType(zeroMethodType);
            result.zeroMethod = zeroMethod;
            builder.setInsertInto(zeroMethod);
            builder.emitBlock();
            List<IRInst*> fieldVals;
            for (auto info : diffFields)
            {
                auto innerZeroMethod = _lookupWitness(&builder, info.witness, autodiffContext->zeroMethodStructKey);
                IRInst* val = builder.emitCallInst(info.field->getFieldType(), innerZeroMethod, 0, nullptr);
                fieldVals.add(val);
            }
            builder.emitReturn(builder.emitMakeStruct(diffType, fieldVals));
        }

        builder.setInsertAfter(zeroMethod);
        IRInst* addMethod = nullptr;
        {
            List<IRType*> paramTypes;
            paramTypes.add(diffType);
            paramTypes.add(diffType);
            auto addMethodType = builder.getFuncType(List<IRType*>(), diffType);
            addMethod = builder.createFunc();
            result.addMethod = addMethod;
            addMethod->setFullType(addMethodType);
            builder.setInsertInto(addMethod);
            builder.emitBlock();
            auto param1 = builder.emitParam(diffType);
            auto param2 = builder.emitParam(diffType);
            List<IRInst*> fieldVals;
            for (auto info : diffFields)
            {
                auto innerAddMethod = _lookupWitness(&builder, info.witness, autodiffContext->addMethodStructKey);
                IRInst* args[2] = {
                    builder.emitFieldExtract(info.field->getFieldType(), param1, info.field->getKey()),
                    builder.emitFieldExtract(info.field->getFieldType(), param2, info.field->getKey()),
                };
                IRInst* val = builder.emitCallInst(info.field->getFieldType(), innerAddMethod, 2, args);
                fieldVals.add(val);
            }
            builder.emitReturn(builder.emitMakeStruct(diffType, fieldVals));
        }

        builder.setInsertAfter(addMethod);
        auto diffTypeIsDiffWitness = builder.createWitnessTable(autodiffContext->differentiableInterfaceType, diffType);
        auto origTypeIsDiffWitness = builder.createWitnessTable(autodiffContext->differentiableInterfaceType, originalType);
        result.diffWitness = origTypeIsDiffWitness;

        builder.createWitnessTableEntry(origTypeIsDiffWitness, autodiffContext->differentialAssocTypeStructKey, diffType);
        builder.createWitnessTableEntry(origTypeIsDiffWitness, autodiffContext->differentialAssocTypeWitnessStructKey, diffTypeIsDiffWitness);
        builder.createWitnessTableEntry(origTypeIsDiffWitness, autodiffContext->zeroMethodStructKey, zeroMethod);
        builder.createWitnessTableEntry(origTypeIsDiffWitness, autodiffContext->addMethodStructKey, addMethod);

        builder.createWitnessTableEntry(diffTypeIsDiffWitness, autodiffContext->differentialAssocTypeStructKey, diffType);
        builder.createWitnessTableEntry(diffTypeIsDiffWitness, autodiffContext->differentialAssocTypeWitnessStructKey, diffTypeIsDiffWitness);
        builder.createWitnessTableEntry(diffTypeIsDiffWitness, autodiffContext->zeroMethodStructKey, zeroMethod);
        builder.createWitnessTableEntry(diffTypeIsDiffWitness, autodiffContext->addMethodStructKey, addMethod);
        return result;
    }

    IntermediateContextTypeDifferentialInfo fillDifferentialTypeImplementation(
        OrderedDictionary<IRInst*, IntermediateContextTypeDifferentialInfo>& diffTypes,
        IRInst* originalType)
    {
        if (originalType->getOp() == kIROp_StructType)
        {
            IRBuilder builder(originalType);
            builder.setInsertBefore(originalType);
            auto diffType = builder.createStructType();
            return fillDifferentialTypeImplementationForStruct(
                diffTypes,
                as<IRStructType>(originalType),
                as<IRStructType>(diffType));
        }
        else if (auto genType = as<IRGeneric>(originalType))
        {
            // For generics, we process the inner struct type as normal,
            // and then hoist the additional insts we created from the generic.

            auto structType = as<IRStructType>(findGenericReturnVal(genType));
            SLANG_RELEASE_ASSERT(structType);

            auto innerResult = fillDifferentialTypeImplementation(diffTypes, structType);
            IRBuilder builder(originalType);
            builder.setInsertBefore(originalType);

            // Now we hoist the new values from the generic to form their independent generics.
            IRInst* specInst = nullptr;
            IntermediateContextTypeDifferentialInfo result;
            if (innerResult.diffType)
                result.diffType = hoistValueFromGeneric(builder, innerResult.diffType, specInst, true);
            if (innerResult.zeroMethod)
            {
                hoistValueFromGeneric(builder, innerResult.zeroMethod->getFullType(), specInst, true);
                result.zeroMethod = hoistValueFromGeneric(builder, innerResult.zeroMethod, specInst, true);
            }
            if (innerResult.addMethod)
            {
                hoistValueFromGeneric(builder, innerResult.addMethod->getFullType(), specInst, true);
                result.addMethod = hoistValueFromGeneric(builder, innerResult.addMethod, specInst, true);
            }
            if (innerResult.diffDiffWitness)
                result.diffDiffWitness = hoistValueFromGeneric(builder, innerResult.diffDiffWitness, specInst, true);
            if (innerResult.diffWitness)
            {
                builder.setInsertBefore(innerResult.diffWitness);
                List<IRInst*> args;
                for (auto param : genType->getParams())
                    args.add(param);
                as<IRWitnessTable>(innerResult.diffWitness)->setConcreteType((IRType*)builder.emitSpecializeInst(
                    builder.getTypeKind(), originalType, (UInt)args.getCount(), args.getBuffer()));
                result.diffWitness = hoistValueFromGeneric(builder, innerResult.diffWitness, specInst, true);
            }
            return result;
        }
        return IntermediateContextTypeDifferentialInfo();
    }

    HashSet<IRInst*> fullyDifferentiatedInsts;

    // Returns true if `type` is fully differentiated, i.e. does not have
    // any unmaterialized intermediate context types.
    bool isTypeFullyDifferentiated(IRInst* type)
    {
        if (fullyDifferentiatedInsts.contains(type))
            return true;
        if (type->getOp() == kIROp_BackwardDiffIntermediateContextType)
            return false;
        if (auto structType = as<IRStructType>(type))
        {
            for (auto f : structType->getFields())
                if (!isTypeFullyDifferentiated(f->getFieldType()))
                    return false;
        }
        else if (auto genType = as<IRGeneric>(type))
        {
            bool result = isTypeFullyDifferentiated(findGenericReturnVal(genType));
            if (result)
                fullyDifferentiatedInsts.add(genType);
            return result;
        }
        switch (type->getOp())
        {
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
        case kIROp_InOutType:
        case kIROp_OutType:
        case kIROp_PtrType:
        case kIROp_DifferentialPairType:
        case kIROp_DifferentialPairUserCodeType:
        case kIROp_AttributedType:
            for (UInt i = 0; i < type->getOperandCount(); i++)
                if (!isTypeFullyDifferentiated(type->getOperand(i)))
                    return false;
            [[fallthrough]];
        default:
            fullyDifferentiatedInsts.add(type);
            return true;
        }
    }

    // Returns true if `func` is fully differentiated, i.e. does not have
    // any differentiate insts.
    bool isFullyDifferentiated(IRFunc* func)
    {
        if (fullyDifferentiatedInsts.contains(func))
            return true;

        for (auto block : func->getBlocks())
        {
            for (auto ii : block->getChildren())
            {
                switch (ii->getOp())
                {
                case kIROp_ForwardDifferentiate:
                case kIROp_BackwardDifferentiate:
                case kIROp_BackwardDifferentiatePrimal:
                case kIROp_BackwardDifferentiatePropagate:
                case kIROp_BackwardDiffIntermediateContextType:
                    return false;
                }
                if (ii->getDataType() && !isTypeFullyDifferentiated(ii->getDataType()))
                    return false;
            }
        }
        fullyDifferentiatedInsts.add(func);
        return true;
    }

    // Process all differentiate calls, and recursively generate code for forward and backward
    // derivative functions.
    //
    bool processReferencedFunctions(IRBuilder* builder)
    {
        fullyDifferentiatedInsts.clear();
        bool hasChanges = false;
        for (;;)
        {
            bool changed = false;
            List<IRInst*> autoDiffWorkList;
            // Collect all `ForwardDifferentiate`/`BackwardDifferentiate` insts from the call graph.
            processAllReachableInsts([&](IRInst* inst)
                {
                    switch (inst->getOp())
                    {
                    case kIROp_ForwardDifferentiate:
                    case kIROp_BackwardDifferentiate:
                    case kIROp_BackwardDifferentiatePrimal:
                    case kIROp_BackwardDifferentiatePropagate:
                    case kIROp_BackwardDiffIntermediateContextType:
                        // Only process now if the operand is a materialized function.
                        switch (inst->getOperand(0)->getOp())
                        {
                        case kIROp_Func:
                        case kIROp_Specialize:
                        case kIROp_LookupWitness:
                            if (auto innerFunc = as<IRFunc>(getResolvedInstForDecorations(inst->getOperand(0))))
                            {
                                // Skip functions whose body still has a differentiate inst (higher order func).
                                if (!isFullyDifferentiated(innerFunc))
                                {
                                    addToWorkList(inst->getOperand(0));
                                    return;
                                }
                            }
                            autoDiffWorkList.add(inst);
                            break;
                        default:
                            autoDiffWorkList.add(inst->getOperand(0));
                            break;
                        }
                        break;
                    case kIROp_PrimalSubstitute:
                        // Explicit primal subst operator is not yet supported.
                        SLANG_UNIMPLEMENTED_X("explicit primal_subst operator.");
                    default:
                        for (UInt i = 0; i < inst->getOperandCount(); i++)
                        {
                            auto operand = inst->getOperand(i);
                            addToWorkList(operand);
                        }
                        break;
                    }
                });

            // Process collected differentiate insts and replace them with placeholders for
            // differentiated functions.

            for (Index i = 0; i < autoDiffWorkList.getCount(); i++)
            {
                auto differentiateInst = autoDiffWorkList[i];

                IRInst* diffFunc = nullptr;
                IRBuilder subBuilder(*builder);
                subBuilder.setInsertBefore(differentiateInst);
                switch (differentiateInst->getOp())
                {
                case kIROp_ForwardDifferentiate:
                    {
                        auto baseFunc = as<IRForwardDifferentiate>(differentiateInst)->getBaseFn();
                        diffFunc = forwardTranscriber.transcribe(&subBuilder, baseFunc);
                    }
                    break;
                case kIROp_BackwardDifferentiatePrimal:
                    {
                        auto baseFunc = differentiateInst->getOperand(0);
                        diffFunc = backwardPrimalTranscriber.transcribe(&subBuilder, baseFunc);
                    }
                    break;
                case kIROp_BackwardDifferentiatePropagate:
                    {
                        auto baseFunc = differentiateInst->getOperand(0);
                        diffFunc = backwardPropagateTranscriber.transcribe(&subBuilder, baseFunc);
                    }
                    break;
                case kIROp_BackwardDifferentiate:
                    {
                        auto baseFunc = differentiateInst->getOperand(0);
                        diffFunc = backwardTranscriber.transcribe(&subBuilder, baseFunc);
                    }
                    break;
                default:
                    break;
                }

                if (diffFunc)
                {
                    SLANG_ASSERT(diffFunc);
                    differentiateInst->replaceUsesWith(diffFunc);
                    differentiateInst->removeAndDeallocate();
                    changed = true;
                }
            }

            // Run transcription logic to generate the body of forward/backward derivatives functions.
            // While doing so, we may discover new functions to differentiate, so we keep running until
            // the worklist goes dry.
            List<IRFunc*> autodiffCleanupList;
            while (autodiffContext->followUpFunctionsToTranscribe.getCount() != 0)
            {
                changed = true;
                auto followUpWorkList = _Move(autodiffContext->followUpFunctionsToTranscribe);
                for (auto task : followUpWorkList)
                {
                    auto diffFunc = as<IRFunc>(task.resultFunc);
                    SLANG_ASSERT(diffFunc);

                    // We're running in to some situations where the follow-up task
                    // has already been completed (diffFunc has been generated, processed,
                    // and deallocated). Skip over these for now.
                    // 
                    if (!diffFunc->getDataType())
                        continue;

                    auto primalFunc = as<IRFunc>(task.originalFunc);
                    SLANG_ASSERT(primalFunc);
                    switch (task.type)
                    {
                    case FuncBodyTranscriptionTaskType::Forward:
                        forwardTranscriber.transcribeFunc(builder, primalFunc, diffFunc);
                        break;
                    case FuncBodyTranscriptionTaskType::BackwardPrimal:
                        backwardPrimalTranscriber.transcribeFunc(builder, primalFunc, diffFunc);
                        break;
                    case FuncBodyTranscriptionTaskType::BackwardPropagate:
                        backwardPropagateTranscriber.transcribeFunc(builder, primalFunc, diffFunc);
                        break;
                    default:
                        break;
                    }

                    autodiffCleanupList.add(diffFunc);
                }
            }

            // Get rid of block-level decorations that are used to keep track of 
            // different block types. These don't work well with the IR simplification
            // passes since they don't expect decorations in blocks.
            // 
            for (auto diffFunc : autodiffCleanupList)
                stripTempDecorations(diffFunc);

            autodiffCleanupList.clear();

#if _DEBUG
            validateIRModule(module, sink);
#endif

            if (!changed)
                break;

            if (lowerIntermediateContextType(builder))
            {
                hasChanges = true;
            }

            // We have done transcribing the functions, now it is time to demote all DifferentialPair types
            // and their operations down to DifferentialPairUserCodeType and *UserCode operations so they
            // can be treated just like normal types with no special semantics in future processing, and won't
            // be confused with the semantics of a DifferentialPair type during future autodiff code gen.
            rewriteDifferentialPairToUserCode(module);

            hasChanges |= changed;
        }

        return hasChanges;
    }

    IRStringLit* getDerivativeFuncName(IRInst* func, const char* postFix)
    {
        IRBuilder builder(autodiffContext->moduleInst);
        builder.setInsertBefore(func);

        IRStringLit* name = nullptr;
        if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
        {
            name = builder.getStringValue((String(linkageDecoration->getMangledName()) + postFix).getUnownedSlice());
        }
        else if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
        {
            name = builder.getStringValue((String(namehintDecoration->getName()) + postFix).getUnownedSlice());
        }

        return name;
    }

    IRStringLit* getForwardDerivativeFuncName(IRInst* func)
    {
        return getDerivativeFuncName(func, "_fwd_diff");
    }

    IRStringLit* getBackwardDerivativeFuncName(IRInst* func)
    {
        return getDerivativeFuncName(func, "_bwd_diff");
    }

    AutoDiffPass(AutoDiffSharedContext* context, DiagnosticSink* sink) :
        InstPassBase(context->moduleInst->getModule()),
        sink(sink),
        forwardTranscriber(context, sink),
        backwardPrimalTranscriber(context, sink),
        backwardPropagateTranscriber(context, sink),
        backwardTranscriber(context, sink),
        pairBuilderStorage(context),
        autodiffContext(context)
    {
        // We start by initializing our shared IR building state,
        // since we will re-use that state for any code we
        // generate along the way.
        //
        forwardTranscriber.pairBuilder = &pairBuilderStorage;
        backwardPrimalTranscriber.pairBuilder = &pairBuilderStorage;
        backwardPropagateTranscriber.pairBuilder = &pairBuilderStorage;
        backwardTranscriber.pairBuilder = &pairBuilderStorage;

        // Make the transcribers available to all sub passes via shared context.
        context->transcriberSet.primalTranscriber = &backwardPrimalTranscriber;
        context->transcriberSet.propagateTranscriber = &backwardPropagateTranscriber;
        context->transcriberSet.forwardTranscriber = &forwardTranscriber;
        context->transcriberSet.backwardTranscriber = &backwardTranscriber;
    }

protected:
    // A transcriber object that handles the main job of 
    // processing instructions while maintaining state.
    //
    ForwardDiffTranscriber forwardTranscriber;

    BackwardDiffPrimalTranscriber backwardPrimalTranscriber;

    BackwardDiffPropagateTranscriber backwardPropagateTranscriber;

    BackwardDiffTranscriber backwardTranscriber;


    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink* sink;

    // Shared context.
    AutoDiffSharedContext* autodiffContext;

    // Builder for dealing with differential pair types.
    DifferentialPairTypeBuilder     pairBuilderStorage;

};

bool processAutodiffCalls(
    TargetProgram* target,
    IRModule*                           module,
    DiagnosticSink*                     sink,
    IRAutodiffPassOptions const&)
{
    SLANG_PROFILE;
    bool modified = false;

    // Create shared context for all auto-diff related passes
    AutoDiffSharedContext autodiffContext(target, module->getModuleInst());

    AutoDiffPass pass(&autodiffContext, sink);

    modified |= pass.processModule();

    return modified;
}

struct RemoveDetachInstsPass : InstPassBase
{
    RemoveDetachInstsPass(IRModule* module) :
        InstPassBase(module)
    {
    }
    void processModule()
    {
        processInstsOfType<IRDetachDerivative>(kIROp_DetachDerivative, [&](IRDetachDerivative* detach)
            {
                detach->replaceUsesWith(detach->getBase());
            });
    }
};

void removeDetachInsts(IRModule* module)
{
    RemoveDetachInstsPass pass(module);
    pass.processModule();
}

struct LowerNullCheckPass : InstPassBase
{
    LowerNullCheckPass(IRModule* module, AutoDiffSharedContext* context) :
        InstPassBase(module), context(context)
    {
    }
    void processModule()
    {
        List<IRInst*> nullCheckInsts;
        processInstsOfType<IRIsDifferentialNull>(kIROp_IsDifferentialNull, [&](IRIsDifferentialNull* isDiffNullInst)
            {
                IRBuilder builder(module);
                builder.setInsertBefore(isDiffNullInst);

                // Extract existential type from the operand.
                auto operand = isDiffNullInst->getBase();
                auto operandConcreteWitness = builder.emitExtractExistentialWitnessTable(operand);
                auto witnessID = builder.emitGetSequentialIDInst(operandConcreteWitness);

                auto nullDiffWitnessTable = context->nullDifferentialWitness;
                auto nullDiffWitnessID = builder.emitGetSequentialIDInst(nullDiffWitnessTable);

                // Compare the concrete type with the null differential witness table.
                auto isDiffNull = builder.emitEql(witnessID, nullDiffWitnessID);

                isDiffNullInst->replaceUsesWith(isDiffNull);
                nullCheckInsts.add(isDiffNullInst);
            });

        for (auto nullCheckInst : nullCheckInsts)
        {
            nullCheckInst->removeAndDeallocate();
        }
    }

    private:
    AutoDiffSharedContext* context;
};

void lowerNullCheckInsts(IRModule* module, AutoDiffSharedContext* context)
{
    LowerNullCheckPass pass(module, context);
    pass.processModule();
}

void releaseNullDifferentialType(AutoDiffSharedContext* context)
{
    if (auto nullStruct = context->nullDifferentialStructType)
    {
        if (auto keepAliveDecoration = nullStruct->findDecoration<IRKeepAliveDecoration>())
            keepAliveDecoration->removeAndDeallocate();
    }

    if (auto nullWitness = context->nullDifferentialWitness)
    {
        if (auto keepAliveDecoration = nullWitness->findDecoration<IRKeepAliveDecoration>())
            keepAliveDecoration->removeAndDeallocate();
    }
}

bool finalizeAutoDiffPass(TargetProgram* target, IRModule* module)
{
    bool modified = false;

    // Create shared context for all auto-diff related passes
    AutoDiffSharedContext autodiffContext(target, module->getModuleInst());

    // Replaces IRDifferentialPairType with an auto-generated struct,
    // IRDifferentialPairGetDifferential with 'differential' field access,
    // IRDifferentialPairGetPrimal with 'primal' field access, and
    // IRMakeDifferentialPair with an IRMakeStruct.
    // 
    modified |= processPairTypes(&autodiffContext);

    removeDetachInsts(module);

    lowerNullCheckInsts(module, &autodiffContext);

    stripNoDiffTypeAttribute(module);

    // Remove auto-diff related decorations.
    stripAutoDiffDecorations(module);

    // Remove keep-alive decorations from null-differential type
    // so it can be DCE'd if unused.
    // 
    releaseNullDifferentialType(&autodiffContext);

    return modified;
}

UIndex addPhiOutputArg(IRBuilder* builder, IRBlock* block, IRInst*& inoutTerminatorInst, IRInst* arg)
{
    SLANG_RELEASE_ASSERT(as<IRUnconditionalBranch>(block->getTerminator()));
    
    auto branchInst = as<IRUnconditionalBranch>(block->getTerminator());
    List<IRInst*> phiArgs;
    
    for (UIndex ii = 0; ii < branchInst->getArgCount(); ii++)
        phiArgs.add(branchInst->getArg(ii));
    
    phiArgs.add(arg);

    builder->setInsertInto(block);
    switch (branchInst->getOp())
    {
        case kIROp_unconditionalBranch:
            inoutTerminatorInst = builder->emitBranch(
                branchInst->getTargetBlock(), phiArgs.getCount(), phiArgs.getBuffer());
            break;
        
        case kIROp_loop:
            {
                auto newLoop = builder->emitLoop(
                    as<IRLoop>(branchInst)->getTargetBlock(),
                    as<IRLoop>(branchInst)->getBreakBlock(),
                    as<IRLoop>(branchInst)->getContinueBlock(),
                    phiArgs.getCount(),
                    phiArgs.getBuffer());
                branchInst->transferDecorationsTo(newLoop);
                branchInst->replaceUsesWith(newLoop);
                inoutTerminatorInst = newLoop;
            }
            break;
        
        default:
            SLANG_UNEXPECTED("Unexpected branch-type for phi replacement");
    }

    branchInst->removeAndDeallocate();
    return phiArgs.getCount() - 1;
}

bool isDifferentialOrRecomputeBlock(IRBlock* block)
{
    if (!block)
        return false;
    for (auto decor : block->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_DifferentialInstDecoration:
        case kIROp_RecomputeBlockDecoration:
            return true;
        default:
            break;
        }
    }
    return false;
}

IRUse* findUniqueStoredVal(IRVar* var)
{
    if (isDerivativeContextVar(var))
    {
        IRUse* primalCallUse = nullptr;
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            if (const auto callInst = as<IRCall>(use->getUser()))
            {
                // Ignore uses from differential blocks.
                if (callInst->getParent()->findDecoration<IRDifferentialInstDecoration>())
                    continue;
                // Should not see more than one IRCall. If we do
                // we'll need to pick the primal call.
                // 
                SLANG_RELEASE_ASSERT(!primalCallUse);
                primalCallUse = use;
            }
        }
        return primalCallUse;
    }
    else
    {
        IRUse* storeUse = nullptr;
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            if (const auto storeInst = as<IRStore>(use->getUser()))
            {
                // Ignore uses from differential blocks.
                if (storeInst->getParent()->findDecoration<IRDifferentialInstDecoration>())
                    continue;
                // Should not see more than one IRStore
                SLANG_RELEASE_ASSERT(!storeUse);
                storeUse = use;
            }
        }
        return storeUse;
    }
}

// Given a local var that is supposed to have a unique write, find the last inst
// that writes to it. Note: if var is intended for an inout argument, it will
// have exactly one store that sets its initial value and one call that writes
// the final value to it, this method will return the call inst for this case.
IRUse* findLatestUniqueWriteUse(IRVar* var)
{
    IRUse* callUse = nullptr;
    for (auto use = var->firstUse; use; use = use->nextUse)
    {
        if (const auto callInst = as<IRCall>(use->getUser()))
        {
            // Ignore uses from differential blocks.
            if (callInst->getParent()->findDecoration<IRDifferentialInstDecoration>())
                continue;
            SLANG_RELEASE_ASSERT(!callUse);
            callUse = use;
        }
    }

    if (callUse)
        return callUse;

    // If no unique call found, try to look for a store.
    return findUniqueStoredVal(var);
}

// Given a local var that is supposed to have a unique write, find the last inst
// that writes to it. Note: if var is intended for an inout argument, it will
// have exactly one store that sets its initial value and one call that writes
// the final value to it, this method will return the store inst for this case.
IRUse* findEarliestUniqueWriteUse(IRVar* var)
{
    IRUse* storeUse = findUniqueStoredVal(var);
    if (storeUse)
        return storeUse;

    // If no unique store found, try to look for a call.
    for (auto use = var->firstUse; use; use = use->nextUse)
    {
        if (const auto callInst = as<IRCall>(use->getUser()))
        {
            // Ignore uses from differential blocks.
            if (callInst->getParent()->findDecoration<IRDifferentialInstDecoration>())
                continue;
            SLANG_RELEASE_ASSERT(!storeUse);
            storeUse = use;
        }
    }
    return storeUse;
}


bool isDerivativeContextVar(IRVar* var)
{
    return var->findDecoration<IRBackwardDerivativePrimalContextDecoration>();
}

bool isDiffInst(IRInst* inst)
{
    if (inst->findDecoration<IRDifferentialInstDecoration>() ||
        inst->findDecoration<IRMixedDifferentialInstDecoration>())
        return true;

    if (auto block = as<IRBlock>(inst->getParent()))
        return isDiffInst(block);

    return false;
}

}
