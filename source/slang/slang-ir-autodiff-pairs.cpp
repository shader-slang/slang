#include "slang-ir-autodiff-pairs.h"

namespace Slang
{

// TODO: deduplicate this
static IRInst* _getDiffTypeFromPairType(
    AutoDiffSharedContext* sharedContext,
    IRBuilder* builder,
    IRDifferentialPairTypeBase* type)
{
    auto witness = type->getWitness();
    SLANG_RELEASE_ASSERT(witness);

    // Special case when the primal type is an InterfaceType/AssociatedType
    if (as<IRInterfaceType>(type->getValueType()) || as<IRAssociatedType>(type->getValueType()))
    {
        // The differential type is the IDifferentiable interface type.
        if (as<IRDifferentialPairType>(type))
            return sharedContext->differentiableInterfaceType;
        else if (as<IRDifferentialPtrPairType>(type))
            return sharedContext->differentiablePtrInterfaceType;
        else
            SLANG_UNEXPECTED("Unexpected differential pair type");
    }

    if (as<IRDifferentialPairType>(type))
        return _lookupWitness(
            builder,
            witness,
            sharedContext->differentialAssocTypeStructKey,
            builder->getTypeKind());
    else if (as<IRDifferentialPtrPairType>(type))
        return _lookupWitness(
            builder,
            witness,
            sharedContext->differentialAssocRefTypeStructKey,
            builder->getTypeKind());
    else
        SLANG_UNEXPECTED("Unexpected differential pair type");
}


struct DifferentialPairTypeBuilder
{
    DifferentialPairTypeBuilder() = default;

    DifferentialPairTypeBuilder(AutoDiffSharedContext* sharedContext)
        : sharedContext(sharedContext)
    {
    }

    IRInst* findSpecializationForParam(IRInst* specializeInst, IRInst* genericParam)
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
            currentIndex++;
        }

        SLANG_ASSERT(paramIndex >= 0);

        // Return the corresponding operand in the specialization inst.
        return specializeInst->getOperand(1 + paramIndex);
    }

    IRInst* emitFieldAccessor(IRBuilder* builder, IRInst* baseInst, IRStructKey* key)
    {
        IRInst* pairType = nullptr;
        if (auto basePtrType = asRelevantPtrType(baseInst->getDataType()))
        {
            auto loweredType = lowerDiffPairType(builder, basePtrType->getValueType());

            pairType = builder->getPtrType((IRType*)loweredType);
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
                key));
        }
        else if (auto ptrType = asRelevantPtrType(pairType))
        {
            if (auto ptrInnerSpecializedType = as<IRSpecialize>(ptrType->getValueType()))
            {
                auto genericType = findInnerMostGenericReturnVal(
                    as<IRGeneric>(ptrInnerSpecializedType->getBase()));
                if (const auto genericBasePairStructType = as<IRStructType>(genericType))
                {
                    return as<IRFieldAddress>(builder->emitFieldAddress(
                        builder->getPtrType((IRType*)findSpecializationForParam(
                            ptrInnerSpecializedType,
                            findStructField(ptrInnerSpecializedType, key)->getFieldType())),
                        baseInst,
                        key));
                }
            }
            else if (auto ptrBaseStructType = as<IRStructType>(ptrType->getValueType()))
            {
                return as<IRFieldAddress>(builder->emitFieldAddress(
                    builder->getPtrType(
                        (IRType*)findStructField(ptrBaseStructType, key)->getFieldType()),
                    baseInst,
                    key));
            }
        }
        else if (auto specializedType = as<IRSpecialize>(pairType))
        {
            auto genericType =
                findInnerMostGenericReturnVal(as<IRGeneric>(specializedType->getBase()));
            if (auto genericBasePairStructType = as<IRStructType>(genericType))
            {
                return as<IRFieldExtract>(builder->emitFieldExtract(
                    (IRType*)findSpecializationForParam(
                        specializedType,
                        findStructField(genericBasePairStructType, key)->getFieldType()),
                    baseInst,
                    key));
            }
            else if (auto genericPtrType = asRelevantPtrType(genericType))
            {
                if (auto genericPairStructType = as<IRStructType>(genericPtrType->getValueType()))
                {
                    return as<IRFieldAddress>(builder->emitFieldAddress(
                        builder->getPtrType((IRType*)findSpecializationForParam(
                            specializedType,
                            findStructField(genericPairStructType, key)->getFieldType())),
                        baseInst,
                        key));
                }
            }
        }
        else
        {
            SLANG_UNEXPECTED("Unrecognized field. Cannot emit field accessor");
        }
        return nullptr;
    }

    IRInst* emitPrimalFieldAccess(IRBuilder* builder, IRType* loweredPairType, IRInst* baseInst)
    {
        if (isRuntimeType(loweredPairType))
        {
            // Do nothing.
            return nullptr;
        }
        else
        {
            return emitFieldAccessor(builder, baseInst, this->globalPrimalKey);
        }
    }

    IRInst* emitDiffFieldAccess(IRBuilder* builder, IRType* loweredPairType, IRInst* baseInst)
    {
        if (isRuntimeType(loweredPairType))
        {
            // Do nothing.
            return nullptr;
        }
        else
        {
            return emitFieldAccessor(builder, baseInst, this->globalDiffKey);
        }
    }

    IRStructKey* _getOrCreateDiffStructKey()
    {
        if (!this->globalDiffKey)
        {
            IRBuilder builder(sharedContext->moduleInst);
            // Insert directly at top level (skip any generic scopes etc.)
            builder.setInsertInto(sharedContext->moduleInst);

            this->globalDiffKey = builder.createStructKey();
            builder.addNameHintDecoration(
                this->globalDiffKey,
                UnownedTerminatedStringSlice("differential"));
        }

        return this->globalDiffKey;
    }

    IRStructKey* _getOrCreatePrimalStructKey()
    {
        if (!this->globalPrimalKey)
        {
            // Insert directly at top level (skip any generic scopes etc.)
            IRBuilder builder(sharedContext->moduleInst);
            builder.setInsertInto(sharedContext->moduleInst);

            this->globalPrimalKey = builder.createStructKey();
            builder.addNameHintDecoration(
                this->globalPrimalKey,
                UnownedTerminatedStringSlice("primal"));
        }

        return this->globalPrimalKey;
    }

    IRInst* _createDiffPairType(IRType* origBaseType, IRType* diffType)
    {
        switch (origBaseType->getOp())
        {
        case kIROp_LookupWitnessMethod:
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


    IRInst* lowerDiffPairType(IRBuilder* builder, IRType* originalPairType)
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

        if (isRuntimeType(primalType))
        {
            // Do nothing.
            return originalPairType;
        }
        else if (auto typePack = as<IRTypePack>(primalType))
        {
            // Lower DiffPair(TypePack(a_0, a_1, ...), MakeWitnessPack(w_0, w_1, ...)) as
            // TypePack(DiffPair(a_0, w_0), DiffPair(a_1, w_1), ...)
            //
            auto cacheKey = primalType;
            if (pairTypeCache.tryGetValue(cacheKey, result))
                return result;

            auto packWitness = pairType->getWitness();

            // Right now we only support concrete witness tables for type packs.
            auto concretePackWitness = as<IRWitnessTable>(packWitness);
            SLANG_ASSERT(concretePackWitness);

            // Get diff type pack.
            IRTypePack* diffTypePack = nullptr;

            if (concretePackWitness->getConformanceType() ==
                this->sharedContext->differentiableInterfaceType)
                diffTypePack = as<IRTypePack>(findWitnessTableEntry(
                    concretePackWitness,
                    this->sharedContext->differentialAssocTypeStructKey));
            else if (
                concretePackWitness->getConformanceType() ==
                this->sharedContext->differentiablePtrInterfaceType)
                diffTypePack = as<IRTypePack>(findWitnessTableEntry(
                    concretePackWitness,
                    this->sharedContext->differentialAssocRefTypeStructKey));
            else
                SLANG_UNEXPECTED("Unexpected witness table");

            SLANG_ASSERT(diffTypePack);

            List<IRType*> args;
            for (UInt i = 0; i < typePack->getOperandCount(); i++)
            {
                auto type = (IRType*)typePack->getOperand(i);
                auto diffType = (IRType*)typePack->getOperand(i);

                if (pairTypeCache.tryGetValue(type, result))
                {
                    args.add((IRType*)result);
                    continue;
                }

                // Lower the diff pair type.
                auto loweredPairType = (IRType*)_createDiffPairType(type, diffType);

                pairTypeCache.add(type, loweredPairType);
                args.add(loweredPairType);
            }

            auto loweredTypePack = builder->getTypePack(args.getCount(), args.getBuffer());
            // TODO: Unify the cache between the three cases.
            pairTypeCache.add(cacheKey, loweredTypePack);

            return loweredTypePack;
        }
        else
        {
            auto cacheKey = primalType;
            if (pairTypeCache.tryGetValue(primalType, result))
                return result;

            if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(primalType))
            {
                result = nullptr;
                return result;
            }

            auto diffType = _getDiffTypeFromPairType(sharedContext, builder, pairType);
            if (!diffType)
                return result;

            // Concrete case.
            result = _createDiffPairType(primalType, (IRType*)diffType);
            pairTypeCache.add(cacheKey, result);

            return result;
        }
    }

    struct PairStructKey
    {
        IRInst* originalType;
        IRInst* diffType;
    };

    // Cache from pair types to lowered type.
    Dictionary<IRInst*, IRInst*> pairTypeCache;

    // Even more caches for easier access to original primal/diff types
    // (Only used for existential pair types). For regular pair types,
    // these are easy to find right on the type itself.
    //
    Dictionary<IRInst*, IRType*> primalTypeMap;
    Dictionary<IRInst*, IRType*> diffTypeMap;


    IRStructKey* globalPrimalKey = nullptr;

    IRStructKey* globalDiffKey = nullptr;

    IRInst* genericDiffPairType = nullptr;

    List<IRInst*> generatedTypeList;

    AutoDiffSharedContext* sharedContext = nullptr;

    IRInterfaceType* commonDiffPairInterface = nullptr;
};

struct DiffPairLoweringPass : InstPassBase
{
    DiffPairLoweringPass(AutoDiffSharedContext* context)
        : InstPassBase(context->moduleInst->getModule()), pairBuilderStorage(context)
    {
        pairBuilder = &pairBuilderStorage;
    }

    IRInst* lowerPairType(IRBuilder* builder, IRType* pairType)
    {
        auto loweredPairType = pairBuilder->lowerDiffPairType(builder, pairType);
        return loweredPairType;
    }

    IRInst* lowerMakePair(IRBuilder* builder, IRInst* inst)
    {
        if (auto makePairInst = as<IRMakeDifferentialPairBase>(inst))
        {
            auto pairType = as<IRDifferentialPairTypeBase>(makePairInst->getDataType());
            builder->setInsertBefore(makePairInst);
            if (auto loweredPairType = (IRType*)lowerPairType(builder, pairType))
            {
                if (isRuntimeType(pairType->getValueType()))
                {
                    // Do nothing.
                    return makePairInst;
                }
                else
                {
                    IRInst* result = nullptr;

                    IRInst* operands[2] = {
                        makePairInst->getPrimalValue(),
                        makePairInst->getDifferentialValue()};
                    result = builder->emitMakeStruct((IRType*)(loweredPairType), 2, operands);

                    makePairInst->replaceUsesWith(result);
                    makePairInst->removeAndDeallocate();
                    return result;
                }
            }
        }

        return nullptr;
    }

    IRInst* lowerPairAccess(IRBuilder* builder, IRInst* inst)
    {
        if (auto getDiffInst = as<IRDifferentialPairGetDifferentialBase>(inst))
        {
            auto pairType = getDiffInst->getBase()->getDataType();
            if (auto pairPtrType = as<IRPtrTypeBase>(pairType))
            {
                pairType = pairPtrType->getValueType();
            }

            builder->setInsertBefore(getDiffInst);
            if (auto loweredType = lowerPairType(builder, pairType))
            {
                IRInst* diffFieldExtract = nullptr;
                diffFieldExtract = pairBuilder->emitDiffFieldAccess(
                    builder,
                    (IRType*)loweredType,
                    getDiffInst->getBase());
                getDiffInst->replaceUsesWith(diffFieldExtract);
                getDiffInst->removeAndDeallocate();
                return diffFieldExtract;
            }
        }
        else if (auto getPrimalInst = as<IRDifferentialPairGetPrimalBase>(inst))
        {
            auto pairType = getPrimalInst->getBase()->getDataType();
            if (auto pairPtrType = as<IRPtrTypeBase>(pairType))
            {
                pairType = pairPtrType->getValueType();
            }

            builder->setInsertBefore(getPrimalInst);
            if (auto loweredType = lowerPairType(builder, pairType))
            {
                IRInst* primalFieldExtract = nullptr;
                primalFieldExtract = pairBuilder->emitPrimalFieldAccess(
                    builder,
                    (IRType*)loweredType,
                    getPrimalInst->getBase());
                getPrimalInst->replaceUsesWith(primalFieldExtract);
                getPrimalInst->removeAndDeallocate();
                return primalFieldExtract;
            }
        }

        return nullptr;
    }

    bool processInstWithChildren(IRBuilder* builder, IRInst* instWithChildren)
    {
        bool modified = false;

        processAllInsts(
            [&](IRInst* inst)
            {
                // Make sure the builder is at the right level.
                builder->setInsertInto(instWithChildren);

                switch (inst->getOp())
                {
                case kIROp_DifferentialPairGetDifferential:
                case kIROp_DifferentialPairGetPrimal:
                case kIROp_DifferentialPtrPairGetDifferential:
                case kIROp_DifferentialPtrPairGetPrimal:
                    lowerPairAccess(builder, inst);
                    break;

                case kIROp_MakeDifferentialPtrPair:
                case kIROp_MakeDifferentialPair:
                    lowerMakePair(builder, inst);
                    break;

                default:
                    break;
                }
            });

        OrderedDictionary<IRInst*, IRInst*> pendingReplacements;
        processAllInsts(
            [&](IRInst* inst)
            {
                if (auto pairType = as<IRDifferentialPairTypeBase>(inst))
                {
                    if (auto loweredType = lowerPairType(builder, pairType))
                    {
                        pendingReplacements.add(pairType, loweredType);
                        modified = true;
                    }
                }
            });
        for (auto replacement : pendingReplacements)
        {
            replacement.key->replaceUsesWith(replacement.value);
            replacement.key->removeAndDeallocate();
        }

        return modified;
    }

    bool processModule()
    {
        IRBuilder builder(module);
        return processInstWithChildren(&builder, module->getModuleInst());
    }

private:
    DifferentialPairTypeBuilder* pairBuilder;

    DifferentialPairTypeBuilder pairBuilderStorage;
};

bool processPairTypes(AutoDiffSharedContext* context)
{
    DiffPairLoweringPass pairLoweringPass(context);
    return pairLoweringPass.processModule();
}


} // namespace Slang
