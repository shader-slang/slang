// slang-ir-diff-jvp.h
#pragma once

#include "slang-ir.h"
#include "slang-compiler.h"

namespace Slang
{
    struct IRModule;


    template<typename P, typename D>
    struct Pair
    {
        P primal;
        D differential;
        Pair() = default;
        Pair(P primal, D differential) : primal(primal), differential(differential)
        {}
        HashCode getHashCode() const
        {
            Hasher hasher;
            hasher << primal << differential;
            return hasher.getResult();
        }
        bool operator ==(const Pair& other) const
        {
            return primal == other.primal && differential == other.differential;
        }
    };

    typedef Pair<IRInst*, IRInst*> InstPair;

    struct AutoDiffSharedContext
    {
        IRModuleInst* moduleInst = nullptr;

        SharedIRBuilder* sharedBuilder = nullptr;

        // A reference to the builtin IDifferentiable interface type.
        // We use this to look up all the other types (and type exprs)
        // that conform to a base type.
        // 
        IRInterfaceType* differentiableInterfaceType = nullptr;

        // The struct key for the 'Differential' associated type
        // defined inside IDifferential. We use this to lookup the differential
        // type in the conformance table associated with the concrete type.
        // 
        IRStructKey* differentialAssocTypeStructKey = nullptr;

        // The struct key for the witness that `Differential` associated type conforms to
        // `IDifferential`.
        IRStructKey* differentialAssocTypeWitnessStructKey = nullptr;


        // The struct key for the 'zero()' associated type
        // defined inside IDifferential. We use this to lookup the 
        // implementation of zero() for a given type.
        // 
        IRStructKey* zeroMethodStructKey = nullptr;

        // The struct key for the 'add()' associated type
        // defined inside IDifferential. We use this to lookup the 
        // implementation of add() for a given type.
        // 
        IRStructKey* addMethodStructKey = nullptr;

        IRStructKey* mulMethodStructKey = nullptr;


        // Modules that don't use differentiable types
        // won't have the IDifferentiable interface type available. 
        // Set to false to indicate that we are uninitialized.
        // 
        bool                                    isInterfaceAvailable = false;


        AutoDiffSharedContext(IRModuleInst* inModuleInst)
            : moduleInst(inModuleInst)
        {
            differentiableInterfaceType = as<IRInterfaceType>(findDifferentiableInterface());
            if (differentiableInterfaceType)
            {
                differentialAssocTypeStructKey = findDifferentialTypeStructKey();
                differentialAssocTypeWitnessStructKey = findDifferentialTypeWitnessStructKey();
                zeroMethodStructKey = findZeroMethodStructKey();
                addMethodStructKey = findAddMethodStructKey();
                mulMethodStructKey = findMulMethodStructKey();

                if (differentialAssocTypeStructKey)
                    isInterfaceAvailable = true;
            }
        }

    private:

        IRInst* findDifferentiableInterface()
        {
            if (auto module = as<IRModuleInst>(moduleInst))
            {
                for (auto globalInst : module->getGlobalInsts())
                {
                    // TODO: This seems like a particularly dangerous way to look for an interface.
                    // See if we can lower IDifferentiable to a separate IR inst.
                    //
                    if (globalInst->getOp() == kIROp_InterfaceType &&
                        as<IRInterfaceType>(globalInst)->findDecoration<IRNameHintDecoration>()->getName() == "IDifferentiable")
                    {
                        return globalInst;
                    }
                }
            }
            return nullptr;
        }

        IRStructKey* findDifferentialTypeStructKey()
        {
            return getIDifferentiableStructKeyAtIndex(0);
        }

        IRStructKey* findDifferentialTypeWitnessStructKey()
        {
            return getIDifferentiableStructKeyAtIndex(1);
        }

        IRStructKey* findZeroMethodStructKey()
        {
            return getIDifferentiableStructKeyAtIndex(2);
        }

        IRStructKey* findAddMethodStructKey()
        {
            return getIDifferentiableStructKeyAtIndex(3);
        }

        IRStructKey* findMulMethodStructKey()
        {
            return getIDifferentiableStructKeyAtIndex(4);
        }

        IRStructKey* getIDifferentiableStructKeyAtIndex(UInt index)
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
    };

    namespace
    {

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

    }

    struct DifferentiableTypeConformanceContext
    {
        AutoDiffSharedContext* sharedContext;

        IRGlobalValueWithCode* parentFunc = nullptr;
        Dictionary<IRType*, IRInst*> differentiableWitnessDictionary;

        DifferentiableTypeConformanceContext(AutoDiffSharedContext* shared)
            : sharedContext(shared)
        {}

        void setFunc(IRGlobalValueWithCode* func)
        {
            parentFunc = func;

            auto decor = func->findDecoration<IRDifferentiableTypeDictionaryDecoration>();
            SLANG_RELEASE_ASSERT(decor);

            // Build lookup dictionary for type witnesses.
            for (auto child = decor->getFirstChild(); child; child = child->next)
            {
                if (auto item = as<IRDifferentiableTypeDictionaryItem>(child))
                {
                    auto existingItem = differentiableWitnessDictionary.TryGetValue(item->getConcreteType());
                    if (existingItem)
                    {
                        if (auto witness = as<IRWitnessTable>(item->getWitness()))
                        {
                            if (witness->getConcreteType()->getOp() == kIROp_DifferentialBottomType)
                                continue;
                        }
                        *existingItem = item->getWitness();
                    }
                    else
                    {
                        differentiableWitnessDictionary.Add((IRType*)item->getConcreteType(), item->getWitness());
                    }
                }
            }
        }


        // Lookup a witness table for the concreteType. One should exist if concreteType
        // inherits (successfully) from IDifferentiable.
        // 
        IRInst* lookUpConformanceForType(IRInst* type)
        {
            IRInst* foundResult = nullptr;
            differentiableWitnessDictionary.TryGetValue(type, foundResult);
            return foundResult;
        }

        IRInst* lookUpInterfaceMethod(IRBuilder* builder, IRType* origType, IRStructKey* key)
        {
            if (auto conformance = lookUpConformanceForType(origType))
            {
                return _lookupWitness(builder, conformance, key);
            }
            return nullptr;
        }

        // Lookup and return the 'Differential' type declared in the concrete type
        // in order to conform to the IDifferentiable interface.
        // Note that inside a generic block, this will be a witness table lookup instruction
        // that gets resolved during the specialization pass.
        // 
        IRInst* getDifferentialForType(IRBuilder* builder, IRType* origType)
        {
            switch (origType->getOp())
            {
            case kIROp_FloatType:
            case kIROp_HalfType:
            case kIROp_DoubleType:
            case kIROp_VectorType:
                return origType;
            }
            return lookUpInterfaceMethod(builder, origType, sharedContext->differentialAssocTypeStructKey);
        }

        IRInst* getZeroMethodForType(IRBuilder* builder, IRType* origType)
        {
            return lookUpInterfaceMethod(builder, origType, sharedContext->zeroMethodStructKey);
        }

        IRInst* getAddMethodForType(IRBuilder* builder, IRType* origType)
        {
            return lookUpInterfaceMethod(builder, origType, sharedContext->addMethodStructKey);
        }

    };

    struct DifferentialPairTypeBuilder
    {

        IRStructField* findField(IRInst* type, IRStructKey* key)
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
            auto baseTypeInfo = lowerDiffPairType(builder, baseInst->getDataType());
            if (baseTypeInfo.isTrivial)
            {
                if (key == globalPrimalKey)
                    return baseInst;
                else
                    return builder->getDifferentialBottom();
            }

            if (auto basePairStructType = as<IRStructType>(baseTypeInfo.loweredType))
            {
                return as<IRFieldExtract>(builder->emitFieldExtract(
                    findField(basePairStructType, key)->getFieldType(),
                    baseInst,
                    key
                ));
            }
            else if (auto ptrType = as<IRPtrTypeBase>(baseTypeInfo.loweredType))
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
            else if (auto specializedType = as<IRSpecialize>(baseTypeInfo.loweredType))
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

        IRInst* emitPrimalFieldAccess(IRBuilder* builder, IRInst* baseInst)
        {
            return emitFieldAccessor(builder, baseInst, this->globalPrimalKey);
        }

        IRInst* emitDiffFieldAccess(IRBuilder* builder, IRInst* baseInst)
        {
            return emitFieldAccessor(builder, baseInst, this->globalDiffKey);
        }

        IRStructKey* _getOrCreateDiffStructKey()
        {
            if (!this->globalDiffKey)
            {
                IRBuilder builder(sharedContext->sharedBuilder);
                // Insert directly at top level (skip any generic scopes etc.)
                builder.setInsertInto(sharedContext->moduleInst);

                this->globalDiffKey = builder.createStructKey();
                builder.addNameHintDecoration(this->globalDiffKey, UnownedTerminatedStringSlice("differential"));
            }

            return this->globalDiffKey;
        }

        IRStructKey* _getOrCreatePrimalStructKey()
        {
            if (!this->globalPrimalKey)
            {
                // Insert directly at top level (skip any generic scopes etc.)
                IRBuilder builder(sharedContext->sharedBuilder);
                builder.setInsertInto(sharedContext->moduleInst);

                this->globalPrimalKey = builder.createStructKey();
                builder.addNameHintDecoration(this->globalPrimalKey, UnownedTerminatedStringSlice("primal"));
            }

            return this->globalPrimalKey;
        }

        IRInst* _createDiffPairType(IRType* origBaseType, IRType* diffType)
        {
            SLANG_ASSERT(!as<IRParam>(origBaseType));
            SLANG_ASSERT(diffType);
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

        IRInst* getDiffTypeFromPairType(IRBuilder* builder, IRDifferentialPairType* type)
        {
            auto witnessTable = type->getWitness();
            return _lookupWitness(builder, witnessTable, sharedContext->differentialAssocTypeStructKey);
        }

        IRInst* getDiffTypeWitnessFromPairType(IRBuilder* builder, IRDifferentialPairType* type)
        {
            auto witnessTable = type->getWitness();
            return _lookupWitness(builder, witnessTable, sharedContext->differentialAssocTypeWitnessStructKey);
        }

        LoweredPairTypeInfo lowerDiffPairType(IRBuilder* builder, IRType* originalPairType)
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
            result.loweredType = _createDiffPairType(pairType->getValueType(), (IRType*)diffType);
            result.isTrivial = (diffType->getOp() == kIROp_DifferentialBottomType);
            pairTypeCache.Add(originalPairType, result);

            return result;
        }

        Dictionary<IRInst*, LoweredPairTypeInfo> pairTypeCache;

        IRStructKey* globalPrimalKey = nullptr;

        IRStructKey* globalDiffKey = nullptr;

        IRInst* genericDiffPairType = nullptr;

        List<IRInst*> generatedTypeList;

        AutoDiffSharedContext* sharedContext = nullptr;
    };

    struct IRJVPDerivativePassOptions
    {
        // Nothing for now..
    };

    bool processForwardDifferentiableFuncs(
        IRModule*                           module,
        DiagnosticSink*                     sink,
        IRJVPDerivativePassOptions const&   options = IRJVPDerivativePassOptions());

    bool processBackwardDifferentiableFuncs(
        IRModule*                           module,
        DiagnosticSink*                     sink,
        IRJVPDerivativePassOptions const&   options = IRJVPDerivativePassOptions());

    void stripAutoDiffDecorations(IRModule* module);
}
