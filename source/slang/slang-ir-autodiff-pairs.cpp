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

// Return the instruction the lowered pair struct must be inserted after so its
// definition follows both field types it references. The struct is built in
// `diffType`'s block, so anchor after `diffType`, or after the primal type
// `origBaseType` when it shares that block and is defined later. Returns null when
// `diffType` is module-scoped, where out-of-order references are permitted.
static IRInst* getPairStructInsertAnchor(IRInst* origBaseType, IRInst* diffType)
{
    if (!as<IRBlock>(diffType->getParent()))
        return nullptr;

    if (origBaseType->getParent() == diffType->getParent())
        for (auto inst = diffType->getNextInst(); inst; inst = inst->getNextInst())
            if (inst == origBaseType)
                return origBaseType;

    return diffType;
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
                if (const auto genericBasePairStructType = as<IRStructType>(genericType);
                    genericBasePairStructType)
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
            return emitFieldAccessor(
                builder,
                baseInst,
                builder->getBuiltinStructKey(KnownBuiltinDeclName::DifferentialPairPrimal));
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
            return emitFieldAccessor(
                builder,
                baseInst,
                builder->getBuiltinStructKey(KnownBuiltinDeclName::DifferentialPairDifferential));
        }
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
        // The struct's only potentially block-local operands are its two field types
        // (the struct keys are module-scoped and the name hint is a string literal),
        // so anchoring after the later-defined of them satisfies every same-block operand.
        if (auto anchor = getPairStructInsertAnchor(origBaseType, diffType))
            setInsertAfterOrdinaryInst(&builder, anchor);
        else
            builder.setInsertBefore(diffType);

        auto pairStructType = builder.createStructType();
        StringBuilder nameBuilder;
        nameBuilder << "DiffPair_";
        getTypeNameHint(nameBuilder, origBaseType);
        builder.addNameHintDecoration(pairStructType, nameBuilder.toString().getUnownedSlice());

        builder.createStructField(
            pairStructType,
            builder.getBuiltinStructKey(KnownBuiltinDeclName::DifferentialPairPrimal),
            origBaseType);
        builder.createStructField(
            pairStructType,
            builder.getBuiltinStructKey(KnownBuiltinDeclName::DifferentialPairDifferential),
            diffType);
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

            if (auto witnessPack = as<IRMakeWitnessPack>(packWitness))
            {
                // Variadic autodiff can lower `each T : IDifferentiable` as a pack witness rather
                // than a concrete witness table. Decompose through the scalar pair path so the
                // element witness remains the source of truth for value-pair vs pointer-pair
                // lowering.
                SLANG_RELEASE_ASSERT(witnessPack->getOperandCount() == typePack->getOperandCount());

                List<IRType*> args;
                for (UInt i = 0; i < typePack->getOperandCount(); i++)
                {
                    auto type = (IRType*)typePack->getOperand(i);

                    if (pairTypeCache.tryGetValue(type, result))
                    {
                        args.add((IRType*)result);
                        continue;
                    }

                    auto witness = witnessPack->getOperand(i);
                    auto witnessType = as<IRWitnessTableTypeBase>(witness->getDataType());
                    if (!witnessType)
                        SLANG_UNEXPECTED(
                            "unexpected non-witness element in differential pair witness pack");

                    IRType* elemPairType = nullptr;
                    auto conformanceType = witnessType->getConformanceType();
                    if (conformanceType == this->sharedContext->differentiableInterfaceType)
                    {
                        elemPairType = builder->getDifferentialPairType(type, witness);
                    }
                    else if (conformanceType == this->sharedContext->differentiablePtrInterfaceType)
                    {
                        elemPairType = builder->getDifferentialPtrPairType(type, witness);
                    }
                    else
                    {
                        SLANG_UNEXPECTED("unexpected witness in differential pair type pack");
                    }

                    auto loweredPairType = (IRType*)lowerDiffPairType(builder, elemPairType);
                    args.add(loweredPairType);
                }

                auto loweredTypePack = builder->getTypePack(args.getCount(), args.getBuffer());
                pairTypeCache.add(cacheKey, loweredTypePack);

                return loweredTypePack;
            }

            // If the pack witness is still generic, specialization has not given this pass enough
            // information to create per-element pair structs yet. Leave the pair type unchanged,
            // matching the scalar Param/Specialize cases below.
            auto concretePackWitness = as<IRWitnessTable>(packWitness);
            if (!concretePackWitness)
                return result;

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
                auto diffType = (IRType*)diffTypePack->getOperand(i);

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


    IRInst* genericDiffPairType = nullptr;

    List<IRInst*> generatedTypeList;

    AutoDiffSharedContext* sharedContext = nullptr;

    IRInterfaceType* commonDiffPairInterface = nullptr;
};

struct DiffPairLoweringPass : InstPassBase
{
    enum class PairField
    {
        Primal,
        Differential,
    };

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

    IRInst* emitPairFieldAccess(
        IRBuilder* builder,
        IRType* loweredPairType,
        IRInst* base,
        PairField field)
    {
        if (field == PairField::Primal)
            return pairBuilder->emitPrimalFieldAccess(builder, loweredPairType, base);
        return pairBuilder->emitDiffFieldAccess(builder, loweredPairType, base);
    }

    /// Lowers a differential-pair field access while preserving field-sensitive loads.
    ///
    /// Consider `inout DifferentialPair<T> pair; T value = pair.p;`. The front end represents
    /// the source access as `GetPrimal(load(pair))`. Extracting the field after that load would
    /// read the entire pair, including `pair.d`, even when the backward-derivative ABI permits
    /// `pair.d` to remain uninitialized. Address the field first so downstream passes see
    /// `load(GetPrimal(pair))` and only read the requested field.
    IRInst* lowerPairFieldAccess(
        IRBuilder* builder,
        IRType* loweredPairType,
        IRInst* base,
        PairField field)
    {
        if (auto pairLoad = as<IRLoad>(base))
        {
            auto fieldAddress =
                emitPairFieldAccess(builder, loweredPairType, pairLoad->getPtr(), field);
            SLANG_RELEASE_ASSERT(asRelevantPtrType(fieldAddress->getDataType()));
            return builder->emitLoad(fieldAddress);
        }
        return emitPairFieldAccess(builder, loweredPairType, base, field);
    }

    IRInst* lowerPairAccess(IRBuilder* builder, IRInst* inst)
    {
        IRInst* base = nullptr;
        PairField field;
        if (auto getDiffInst = as<IRDifferentialPairGetDifferentialBase>(inst))
        {
            base = getDiffInst->getBase();
            field = PairField::Differential;
        }
        else if (auto getPrimalInst = as<IRDifferentialPairGetPrimalBase>(inst))
        {
            base = getPrimalInst->getBase();
            field = PairField::Primal;
        }
        else
        {
            return nullptr;
        }

        auto pairType = base->getDataType();
        if (auto pairPtrType = as<IRPtrTypeBase>(pairType))
        {
            pairType = pairPtrType->getValueType();
        }

        builder->setInsertBefore(inst);
        if (auto loweredType = lowerPairType(builder, pairType))
        {
            auto pairLoad = as<IRLoad>(base);
            auto loweredAccess = lowerPairFieldAccess(builder, (IRType*)loweredType, base, field);
            inst->replaceUsesWith(loweredAccess);
            inst->removeAndDeallocate();

            // The field-address load replaces the original whole-pair load. Remove it here when
            // this access was its last use: not every consumer runs DCE before interpreting the
            // lowered IR, and evaluating the dead load would still read an uninitialized sibling
            // field.
            if (pairLoad && !pairLoad->hasUses())
                pairLoad->removeAndDeallocate();
            return loweredAccess;
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
                        if (!pendingReplacements.containsKey(pairType))
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
