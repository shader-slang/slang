#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-rev.h"
#include "slang-ir-autodiff-fwd.h"

namespace Slang
{

// TODO: Put into a nameless namespace.
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
    else if (auto witnessTableParam = as<IRParam>(witness))
    {
        return builder->emitLookupInterfaceMethodInst(
            builder->getTypeKind(),
            witnessTableParam,
            requirementKey);
    }
    return nullptr;
}

IRStructField* DifferentialPairTypeBuilder::findField(IRInst* type, IRStructKey* key)
{
    if (auto irStructType = as<IRStructType>(type))
    {
        for (auto field : irStructType->getFields())
        {
            if (field->getKey() == key)
            {
                return field;
            }
        }
    }
    else if (auto irSpecialize = as<IRSpecialize>(type))
    {
        if (auto irGeneric = as<IRGeneric>(irSpecialize->getBase()))
        {
            if (auto irGenericStructType = as<IRStructType>(findInnerMostGenericReturnVal(irGeneric)))
            {
                return findField(irGenericStructType, key);
            }
        }
    }

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
        auto baseTypeInfo = lowerDiffPairType(builder, basePtrType->getValueType());

        // TODO(sai): Not sure at the moment how to handle diff-bottom pointer types,
        // especially since we probably don't need diff bottom anymore.
        // 
        SLANG_ASSERT(!baseTypeInfo.isTrivial);

        pairType = builder->getPtrType(kIROp_PtrType, (IRType*)baseTypeInfo.loweredType);
    }
    else
    {
        auto baseTypeInfo = lowerDiffPairType(builder, baseInst->getDataType());
        if (baseTypeInfo.isTrivial)
        {
            if (key == globalPrimalKey)
                return baseInst;
            else
                return builder->getDifferentialBottom();
        }

        pairType = baseTypeInfo.loweredType;
    }

    if (auto basePairStructType = as<IRStructType>(pairType))
    {
        return as<IRFieldExtract>(builder->emitFieldExtract(
                findField(basePairStructType, key)->getFieldType(),
                baseInst,
                key
            ));
    }
    else if (auto ptrType = as<IRPtrTypeBase>(pairType))
    {
        if (auto ptrInnerSpecializedType = as<IRSpecialize>(ptrType->getValueType()))
        {
            auto genericType = findInnerMostGenericReturnVal(as<IRGeneric>(ptrInnerSpecializedType->getBase()));
            if (auto genericBasePairStructType = as<IRStructType>(genericType))
            {
                return as<IRFieldAddress>(builder->emitFieldAddress(
                    builder->getPtrType((IRType*)
                        findSpecializationForParam(
                            ptrInnerSpecializedType,
                            findField(ptrInnerSpecializedType, key)->getFieldType())),
                    baseInst,
                    key
                ));
            }
        }
        else if (auto ptrBaseStructType = as<IRStructType>(ptrType->getValueType()))
        {
            return as<IRFieldAddress>(builder->emitFieldAddress(
                builder->getPtrType((IRType*)
                        findField(ptrBaseStructType, key)->getFieldType()),
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
                    findField(genericBasePairStructType, key)->getFieldType()),
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
                                findField(genericPairStructType, key)->getFieldType())),
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
        IRBuilder builder(sharedContext->sharedBuilder);
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
        IRBuilder builder(sharedContext->sharedBuilder);
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
    case kIROp_lookup_interface_method:
    case kIROp_Specialize:
    case kIROp_Param:
        return nullptr;
    default:
        break;
    }
    if (diffType->getOp() != kIROp_DifferentialBottomType)
    {
        IRBuilder builder(sharedContext->sharedBuilder);
        builder.setInsertBefore(diffType);

        auto pairStructType = builder.createStructType();
        builder.createStructField(pairStructType, _getOrCreatePrimalStructKey(), origBaseType);
        builder.createStructField(pairStructType, _getOrCreateDiffStructKey(), (IRType*)diffType);
        return pairStructType;
    }
    return origBaseType;
}

struct LoweredPairTypeInfo
{
    IRInst* loweredType;
    bool isTrivial;
};

IRInst* DifferentialPairTypeBuilder::getDiffTypeFromPairType(IRBuilder* builder, IRDifferentialPairType* type)
{
    auto witnessTable = type->getWitness();
    return _lookupWitness(builder, witnessTable, sharedContext->differentialAssocTypeStructKey);
}

IRInst* DifferentialPairTypeBuilder::getDiffTypeWitnessFromPairType(IRBuilder* builder, IRDifferentialPairType* type)
{
    auto witnessTable = type->getWitness();
    return _lookupWitness(builder, witnessTable, sharedContext->differentialAssocTypeWitnessStructKey);
}

LoweredPairTypeInfo DifferentialPairTypeBuilder::lowerDiffPairType(IRBuilder* builder, IRType* originalPairType)
{
    LoweredPairTypeInfo result = {};
    
    if (pairTypeCache.TryGetValue(originalPairType, result))
        return result;
    auto pairType = as<IRDifferentialPairType>(originalPairType);
    if (!pairType)
    {
        result.isTrivial = true;
        result.loweredType = originalPairType;
        return result;
    }
    auto primalType = pairType->getValueType();
    if (as<IRParam>(primalType))
    {
        result.isTrivial = false;
        result.loweredType = nullptr;
        return result;
    }

    auto diffType = getDiffTypeFromPairType(builder, pairType);
    if (!diffType)
        return result;
    result.loweredType = _createDiffPairType(pairType->getValueType(), (IRType*)diffType);
    result.isTrivial = (diffType->getOp() == kIROp_DifferentialBottomType);
    pairTypeCache.Add(originalPairType, result);

    return result;
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

bool processAutodiffCalls(
    IRModule*                           module,
    DiagnosticSink*                     sink,
    IRAutodiffPassOptions const&        options = IRAutodiffPassOptions())
{
    // Simplify module to remove dead code.
    IRDeadCodeEliminationOptions dceOptions;
    dceOptions.keepExportsAlive = true;
    dceOptions.keepLayoutsAlive = true;
    eliminateDeadCode(module, options);

    bool modified = false;

    // Process forward derivative calls.
    modified |= processForwardDerivativeCalls(module, sink);

    // Process reverse derivative calls.
    modified |= processReverseDerivativeCalls(module, sink);

    modified |= simplifyDifferentialBottomType(builder);

    // Replaces IRDifferentialPairType with an auto-generated struct,
    // IRDifferentialPairGetDifferential with 'differential' field access,
    // IRDifferentialPairGetPrimal with 'primal' field access, and
    // IRMakeDifferentialPair with an IRMakeStruct.
    // 
    modified |= processPairTypes(builder, module->getModuleInst());

    modified |= eliminateDifferentialBottomType(builder);

    // Remove auto-diff related decorations.
    stripAutoDiffDecorations(module);

}


}