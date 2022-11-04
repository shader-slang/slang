// slang-ir-diff-jvp.cpp
#include "slang-ir-diff-jvp.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-util.h"
#include "slang-ir-inst-pass-base.h"

// origX, primalX, diffX
// origX -> primalX (cloneEnv)
// origX -> diffX (instMapD)

namespace Slang
{

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
    IRModuleInst*                          moduleInst = nullptr;

    // A reference to the builtin IDifferentiable interface type.
    // We use this to look up all the other types (and type exprs)
    // that conform to a base type.
    // 
    IRInterfaceType*                        differentiableInterfaceType = nullptr;

    // The struct key for the 'Differential' associated type
    // defined inside IDifferential. We use this to lookup the differential
    // type in the conformance table associated with the concrete type.
    // 
    IRStructKey*                            differentialAssocTypeStructKey = nullptr;

    // The struct key for the witness that `Differential` associated type conforms to
    // `IDifferential`.
    IRStructKey*                            differentialAssocTypeWitnessStructKey = nullptr;


    // The struct key for the 'zero()' associated type
    // defined inside IDifferential. We use this to lookup the 
    // implementation of zero() for a given type.
    // 
    IRStructKey*                            zeroMethodStructKey = nullptr;
    
    // The struct key for the 'add()' associated type
    // defined inside IDifferential. We use this to lookup the 
    // implementation of add() for a given type.
    // 
    IRStructKey*                            addMethodStructKey = nullptr;

    IRStructKey*                            mulMethodStructKey = nullptr;

    
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
            currentIndex ++;
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

    IRStructKey* _getOrCreateDiffStructKey(IRBuilder* builder)
    {
        if (!this->globalDiffKey)
        {
            // Insert directly at top level (skip any generic scopes etc.)
            auto insertLoc = builder->getInsertLoc();
            builder->setInsertInto(builder->getModule()->getModuleInst());

            this->globalDiffKey = builder->createStructKey();
            builder->addNameHintDecoration(this->globalDiffKey , UnownedTerminatedStringSlice("differential"));

            builder->setInsertLoc(insertLoc);
        }

        return this->globalDiffKey;
    }

    IRStructKey* _getOrCreatePrimalStructKey(IRBuilder* builder)
    {
        if (!this->globalPrimalKey)
        {
            // Insert directly at top level (skip any generic scopes etc.)
            auto insertLoc = builder->getInsertLoc();
            builder->setInsertInto(builder->getModule()->getModuleInst());

            this->globalPrimalKey = builder->createStructKey();
            builder->addNameHintDecoration(this->globalPrimalKey , UnownedTerminatedStringSlice("primal"));

            builder->setInsertLoc(insertLoc);
        }

        return this->globalPrimalKey;
    }

    IRInst* _createDiffPairType(IRBuilder* builder, IRType* origBaseType, IRType* diffType)
    {
        SLANG_ASSERT(!as<IRParam>(origBaseType));
        SLANG_ASSERT(diffType);
        if (diffType->getOp() != kIROp_DifferentialBottomType)
        {
            auto pairStructType = builder->createStructType();
            builder->createStructField(pairStructType, _getOrCreatePrimalStructKey(builder), origBaseType);
            builder->createStructField(pairStructType, _getOrCreateDiffStructKey(builder), (IRType*)diffType);
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
        result.loweredType = _createDiffPairType(builder, pairType->getValueType(), (IRType*)diffType);
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

struct JVPTranscriber
{

    // Stores the mapping of arbitrary 'R-value' instructions to instructions that represent
    // their differential values.
    Dictionary<IRInst*, IRInst*>            instMapD;

    // Set of insts currently being transcribed. Used to avoid infinite loops.
    HashSet<IRInst*>                        instsInProgress;

    // Cloning environment to hold mapping from old to new copies for the primal
    // instructions.
    IRCloneEnv                              cloneEnv;

    // Diagnostic sink for error messages.
    DiagnosticSink*                         sink;

    // Type conformance information.
    AutoDiffSharedContext*                  autoDiffSharedContext;

    // Builder to help with creating and accessing the 'DifferentiablePair<T>' struct
    DifferentialPairTypeBuilder*            pairBuilder;

    DifferentiableTypeConformanceContext    differentiableTypeConformanceContext;

    List<InstPair>                          followUpFunctionsToTranscribe;

    SharedIRBuilder* sharedBuilder;
    // Witness table that `DifferentialBottom:IDifferential`.
    IRWitnessTable* differentialBottomWitness = nullptr;
    Dictionary<InstPair, IRInst*> differentialPairTypes;

    JVPTranscriber(AutoDiffSharedContext* shared, SharedIRBuilder* inSharedBuilder)
        : differentiableTypeConformanceContext(shared), sharedBuilder(inSharedBuilder)
    {}

    DiagnosticSink* getSink()
    {
        SLANG_ASSERT(sink);
        return sink;
    }

    void mapDifferentialInst(IRInst* origInst, IRInst* diffInst)
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

    void mapPrimalInst(IRInst* origInst, IRInst* primalInst)
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

    IRInst* lookupDiffInst(IRInst* origInst)
    {
        return instMapD[origInst];
    }

    IRInst* lookupDiffInst(IRInst* origInst, IRInst* defaultInst)
    {
        return (hasDifferentialInst(origInst)) ? instMapD[origInst] : defaultInst;
    }

    bool hasDifferentialInst(IRInst* origInst)
    {
        return instMapD.ContainsKey(origInst);
    }

    IRInst* lookupPrimalInst(IRInst* origInst)
    {
        return cloneEnv.mapOldValToNew[origInst];
    }

    IRInst* lookupPrimalInst(IRInst* origInst, IRInst* defaultInst)
    {
        return (hasPrimalInst(origInst)) ? lookupPrimalInst(origInst) : defaultInst;
    }

    bool hasPrimalInst(IRInst* origInst)
    {
        return cloneEnv.mapOldValToNew.ContainsKey(origInst);
    }
    
    IRInst* findOrTranscribeDiffInst(IRBuilder* builder, IRInst* origInst)
    {
        if (!hasDifferentialInst(origInst))
        {
            transcribe(builder, origInst);
            SLANG_ASSERT(hasDifferentialInst(origInst));
        }

        return lookupDiffInst(origInst);
    }

    IRInst* findOrTranscribePrimalInst(IRBuilder* builder, IRInst* origInst)
    {
        if (!hasPrimalInst(origInst))
        {
            transcribe(builder, origInst);
            SLANG_ASSERT(hasPrimalInst(origInst));
        }

        return lookupPrimalInst(origInst);
    }

    IRFuncType* differentiateFunctionType(IRBuilder* builder, IRFuncType* funcType)
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
            diffReturnType = builder->getVoidType();

        return builder->getFuncType(newParameterTypes, diffReturnType);
    }

    IRWitnessTable* getDifferentialBottomWitness()
    {
        IRBuilder builder(sharedBuilder);
        builder.setInsertInto(sharedBuilder->getModule()->getModuleInst());
        auto result =
            as<IRWitnessTable>(differentiableTypeConformanceContext.lookUpConformanceForType(
                builder.getDifferentialBottomType()));
        SLANG_ASSERT(result);
        return result;
    }

    // Get or construct `:IDifferentiable` conformance for a DifferentiablePair.
    IRWitnessTable* getDifferentialPairWitness(IRInst* inDiffPairType)
    {
        IRBuilder builder(sharedBuilder);
        builder.setInsertInto(inDiffPairType->parent);
        auto diffPairType = as<IRDifferentialPairType>(inDiffPairType);
        SLANG_ASSERT(diffPairType);
        auto result =
            as<IRWitnessTable>(differentiableTypeConformanceContext.lookUpConformanceForType(
                builder.getDifferentialBottomType()));
        if (result)
            return result;

        auto table = builder.createWitnessTable(autoDiffSharedContext->differentiableInterfaceType, diffPairType);
        auto diffType = differentiateType(&builder, diffPairType->getValueType());
        auto differentialType = builder.getDifferentialPairType(diffType, getDifferentialBottomWitness());
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
        if (!witness)
            witness = getDifferentialBottomWitness();
        return builder.getDifferentialPairType(
            (IRType*)primalType,
            witness);
    }

    IRType* differentiateType(IRBuilder* builder, IRType* origType)
    {
        IRInst* diffType = nullptr;
        if (!instMapD.TryGetValue(origType, diffType))
        {
            diffType = _differentiateTypeImpl(builder, origType);
            instMapD[origType] = diffType;
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
        auto primalDataType = lookupPrimalInst(origParam->getDataType(), origParam->getDataType());
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

    // Returns "d<var-name>" to use as a name hint for variables and parameters.
    // If no primal name is available, returns a blank string.
    // 
    String getJVPVarName(IRInst* origVar)
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
    String makeDiffPairName(IRInst* origVar)
    {
        if (auto namehintDecoration = origVar->findDecoration<IRNameHintDecoration>())
        {
            return ("dp" + String(namehintDecoration->getName()));
        }

        return String("");
    }

    InstPair transcribeVar(IRBuilder* builder, IRVar* origVar)
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

    InstPair transcribeBinaryArith(IRBuilder* builder, IRInst* origArith)
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

    
    InstPair transcribeBinaryLogic(IRBuilder* builder, IRInst* origLogic)
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

    InstPair transcribeLoad(IRBuilder* builder, IRLoad* origLoad)
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

    InstPair transcribeStore(IRBuilder* builder, IRStore* origStore)
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

    InstPair transcribeReturn(IRBuilder* builder, IRReturn* origReturn)
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
    InstPair transcribeConstruct(IRBuilder* builder, IRInst* origConstruct)
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
    InstPair transcribeCall(IRBuilder* builder, IRCall* origCall)
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

        if (auto derivativeReferenceDecor = primalCallee->findDecoration<IRForwardDerivativeDecoration>())
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
        else
        {
            // The callee is non differentiable, just return primal value with null diff value.
            IRInst* primalCall = cloneInst(&cloneEnv, builder, origCall);
            return InstPair(primalCall, nullptr);
        }

        List<IRInst*> args;
        // Go over the parameter list and create pairs for each input (if required)
        for (UIndex ii = 0; ii < origCall->getArgCount(); ii++)
        {
            auto origArg = origCall->getArg(ii);
            auto primalArg = findOrTranscribePrimalInst(builder, origArg);
            SLANG_ASSERT(primalArg);

            auto primalType = primalArg->getDataType();
            auto diffArg = findOrTranscribeDiffInst(builder, origArg);

            if (!diffArg)
                diffArg = getDifferentialZeroOfType(builder, primalType);

            if (auto pairType = tryGetDiffPairType(builder, primalType))
            {
                // If a pair type can be formed, this must be non-null.
                SLANG_RELEASE_ASSERT(diffArg);
                auto diffPair = builder->emitMakeDifferentialPair(pairType, primalArg, diffArg);
                args.add(diffPair);
            }
            else
            {
                // Add original/primal argument.
                args.add(primalArg);
            }
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

    InstPair transcribeSwizzle(IRBuilder* builder, IRSwizzle* origSwizzle)
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

    InstPair transcribeByPassthrough(IRBuilder* builder, IRInst* origInst)
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

    InstPair transcribeControlFlow(IRBuilder* builder, IRInst* origInst)
    {
        switch(origInst->getOp())
        {
            case kIROp_unconditionalBranch:
                auto origBranch = as<IRUnconditionalBranch>(origInst);

                // Grab the differentials for any phi nodes.
                List<IRInst*> pairArgs;
                for (UIndex ii = 0; ii < origBranch->getArgCount(); ii++)
                {
                    auto origArg = origBranch->getArg(ii);

                    IRInst* pairArg = nullptr;
                    if (auto diffPairType = tryGetDiffPairType(builder, (IRType*)origArg->getDataType()))
                    {
                        auto diffArg = lookupDiffInst(origArg, nullptr);
                        if (!diffArg)
                        {
                            diffArg = getDifferentialZeroOfType(builder, (IRType*)origArg->getDataType());
                        }
                        
                        pairArg = builder->emitMakeDifferentialPair(
                            diffPairType,
                            lookupPrimalInst(origArg),
                            diffArg);
                    }
                    else
                    {
                        pairArg = lookupPrimalInst(origArg);
                    }
                    pairArgs.add(pairArg);
                }

                IRInst* diffBranch = nullptr;
                if (auto diffBlock = findOrTranscribeDiffInst(builder, origBranch->getTargetBlock()))
                {
                    diffBranch = builder->emitBranch(
                        as<IRBlock>(diffBlock),
                        pairArgs.getCount(),
                        pairArgs.getBuffer());
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

    InstPair transcribeConst(IRBuilder* builder, IRInst* origInst)
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

    InstPair transcribeSpecialize(IRBuilder*, IRSpecialize* origSpecialize)
    {
        // In general, we should not see any specialize insts at this stage.
        // The exceptions are target intrinsics.
        auto genericInnerVal = findInnerMostGenericReturnVal(as<IRGeneric>(origSpecialize->getBase()));
        if (genericInnerVal->findDecoration<IRTargetIntrinsicDecoration>())
        {
            // Look for an IRForwardDerivativeDecoration on the specialize inst.
            // (Normally, this would be on the inner IRFunc, but in this case only the JVP func
            // can be specialized, so we put a decoration on the IRSpecialize)
            //
            if (auto jvpFuncDecoration = origSpecialize->findDecoration<IRForwardDerivativeDecoration>())
            {
                auto jvpFunc = jvpFuncDecoration->getForwardDerivativeFunc();

                // Make sure this isn't itself a specialize .
                SLANG_RELEASE_ASSERT(!as<IRSpecialize>(jvpFunc));

                return InstPair(jvpFunc, jvpFunc);
            }
        }
        else
        {
            getSink()->diagnose(origSpecialize->sourceLoc,
                    Diagnostics::unexpected,
                    "should not be attempting to differentiate anything specialized here.");
        }
        
        return InstPair(nullptr, nullptr);
    }

    InstPair transcibeLookupInterfaceMethod(IRBuilder* builder, IRLookupWitnessMethod* origLookup)
    {
        // This is slightly counter-intuitive, but we don't perform any differentiation
        // logic here. We simple clone the original lookup which points to the original function,
        // or the cloned version in case we're inside a generic scope.
        // The differentiation logic is inserted later when this is used in an IRCall.
        // This decision is mostly to maintain a uniform convention of ForwardDifferentiate(Lookup(Table))
        // rather than have Lookup(ForwardDifferentiate(Table))
        // 
        auto diffLookup = cloneInst(&cloneEnv, builder, origLookup);
        return InstPair(diffLookup, diffLookup);
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
        auto oldLoc = builder->getInsertLoc();
        
        IRInst* diffBlock = builder->emitBlock();
        
        // Note: for blocks, we setup the mapping _before_
        // processing the children since we could encounter
        // a lookup while processing the children.
        // 
        mapPrimalInst(origBlock, diffBlock);
        mapDifferentialInst(origBlock, diffBlock);

        builder->setInsertInto(diffBlock);

        // First transcribe every parameter in the block.
        for (auto param = origBlock->getFirstParam(); param; param = param->getNextParam())
            this->transcribe(builder, param);

        // Then, run through every instruction and use the transcriber to generate the appropriate
        // derivative code.
        //
        for (auto child = origBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
            this->transcribe(builder, child);

        builder->setInsertLoc(oldLoc);

        return InstPair(diffBlock, diffBlock);
    }

    InstPair transcribeFieldExtract(IRBuilder* builder, IRInst* originalInst)
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

    InstPair transcribeGetElement(IRBuilder* builder, IRInst* origGetElementPtr)
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

    InstPair transcribeLoop(IRBuilder* builder, IRLoop* origLoop)
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

    InstPair transcribeIfElse(IRBuilder* builder, IRIfElse* origIfElse)
    {
        // The loop comes with three blocks.. we just need to transcribe each one
        // and assemble the new loop instruction.
        
        // Transcribe the target block (this is the 'condition' part of the loop, which
        // will branch into the loop body).
        // Note that for the condition we use the primal inst (condition values should not have a 
        // differential)
        auto primalConditionBlock = findOrTranscribePrimalInst(builder, origIfElse->getCondition());
        SLANG_ASSERT(primalConditionBlock);

        // Transcribe the break block (this is the block after the exiting the loop)
        auto diffTrueBlock = findOrTranscribeDiffInst(builder, origIfElse->getTrueBlock());
        SLANG_ASSERT(diffTrueBlock);

        // Transcribe the continue block (this is the 'update' part of the loop, which will
        // branch into the condition block)
        auto diffFalseBlock = findOrTranscribeDiffInst(builder, origIfElse->getFalseBlock());
        SLANG_ASSERT(diffFalseBlock);

        // Transcribe the continue block (this is the 'update' part of the loop, which will
        // branch into the condition block)
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

    InstPair transcribeMakeDifferentialPair(IRBuilder* builder, IRMakeDifferentialPair* origInst)
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

    InstPair transcribeDifferentialPairGetElement(IRBuilder* builder, IRInst* origInst)
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
    InstPair transcribeFuncHeader(IRBuilder* builder, IRFunc* origFunc)
    {
        auto oldLoc = builder->getInsertLoc();

        IRFunc* primalFunc = origFunc;

        differentiableTypeConformanceContext.setFunc(origFunc);

        builder->setInsertBefore(origFunc);
        primalFunc = origFunc;

        auto diffFunc = builder->createFunc();

        SLANG_ASSERT(as<IRFuncType>(origFunc->getFullType()));
        IRType* diffFuncType = this->differentiateFunctionType(
            builder,
            as<IRFuncType>(origFunc->getFullType()));
        diffFunc->setFullType(diffFuncType);

        if (auto nameHint = origFunc->findDecoration<IRNameHintDecoration>())
        {
            auto originalName = nameHint->getName();
            StringBuilder newNameSb;
            newNameSb << "s_jvp_" << originalName;
            builder->addNameHintDecoration(diffFunc, newNameSb.getUnownedSlice());
        }
        builder->addForwardDerivativeDecoration(origFunc, diffFunc);

        // Mark the generated derivative function itself as differentiable.
        builder->addForwardDifferentiableDecoration(diffFunc);

        // Find and clone `DifferentiableTypeDictionaryDecoration` to the new diffFunc.
        if (auto dictDecor = origFunc->findDecoration<IRDifferentiableTypeDictionaryDecoration>())
        {
            cloneDecoration(dictDecor, diffFunc);
        }

        // Reset builder position
        builder->setInsertLoc(oldLoc);
        auto result = InstPair(primalFunc, diffFunc);
        followUpFunctionsToTranscribe.add(result);
        return result;
    }

    // Transcribe a function definition.
    InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc)
    {
        auto oldLoc = builder->getInsertLoc();

        differentiableTypeConformanceContext.setFunc(primalFunc);
        // Transcribe children from origFunc into diffFunc
        builder->setInsertInto(diffFunc);
        for (auto block = primalFunc->getFirstBlock(); block; block = block->getNextBlock())
            this->transcribe(builder, block);
        
        // Reset builder position
        builder->setInsertLoc(oldLoc);

        return InstPair(primalFunc, diffFunc);
    }

    // Transcribe a generic definition
    InstPair transcribeGeneric(IRBuilder* builder, IRGeneric* origGeneric)
    {
        auto innerVal = findInnerMostGenericReturnVal(origGeneric);
        if (auto innerFunc = as<IRFunc>(innerVal))
        {
            differentiableTypeConformanceContext.setFunc(innerFunc);
        }
        else
        {
            return InstPair(origGeneric, nullptr);
        }

        // For now, we assume there's only one generic layer. So this inst must be top level
        bool isTopLevel = (as<IRModuleInst>(origGeneric->getParent()) != nullptr);
        SLANG_RELEASE_ASSERT(isTopLevel);

        IRGeneric* primalGeneric = origGeneric;

        auto oldLoc = builder->getInsertLoc();
        builder->setInsertBefore(origGeneric);

        auto diffGeneric = builder->emitGeneric();

        // Process type of generic. If the generic is a function, then it's type will also be a 
        // generic and this logic will transcribe that generic first before continuing with the 
        // function itself.
        // 
        auto primalType =  primalGeneric->getFullType();

        IRType* diffType = nullptr;
        if (primalType)
        {
            diffType = (IRType*) findOrTranscribeDiffInst(builder, primalType);
        }

        diffGeneric->setFullType(diffType);

        // TODO(sai): Replace naming scheme
        // if (auto jvpName = this->getJVPFuncName(builder, primalFn))
        //    builder->addNameHintDecoration(diffFunc, jvpName);
        
        // Transcribe children from origFunc into diffFunc.
        builder->setInsertInto(diffGeneric);
        for (auto block = origGeneric->getFirstBlock(); block; block = block->getNextBlock())
            this->transcribe(builder, block);
        
        // Reset builder position.
        builder->setInsertLoc(oldLoc);

        return InstPair(primalGeneric, diffGeneric);
    }

    IRInst* transcribe(IRBuilder* builder, IRInst* origInst)
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

        if (auto primalInst = pair.primal)
        {
            mapPrimalInst(origInst, pair.primal);
            mapDifferentialInst(origInst, pair.differential);
            if (pair.differential)
            {
                // Generate name hint for the inst.
                if (auto primalNameHint = primalInst->findDecoration<IRNameHintDecoration>())
                {
                    StringBuilder sb;
                    sb << "s_diff_" << primalNameHint->getName();
                    builder->addNameHintDecoration(pair.differential, sb.getUnownedSlice());
                }
            }
            return pair.differential;
        }
        instsInProgress.Remove(origInst);

        getSink()->diagnose(origInst->sourceLoc,
                    Diagnostics::internalCompilerError,
                    "failed to transcibe instruction");
        return nullptr;
    }

    InstPair transcribeInst(IRBuilder* builder, IRInst* origInst)
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

        case kIROp_lookup_interface_method:
            return transcibeLookupInterfaceMethod(builder, as<IRLookupWitnessMethod>(origInst));

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
};

struct JVPDerivativeContext : public InstPassBase
{

    DiagnosticSink* getSink()
    {
        return sink;
    }

    bool processModule()
    {
        // We start by initializing our shared IR building state,
        // since we will re-use that state for any code we
        // generate along the way.
        //
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->init(module);
        sharedBuilder->deduplicateAndRebuildGlobalNumberingMap();
    
        IRBuilder builderStorage(sharedBuilderStorage);
        IRBuilder* builder = &builderStorage;

        // Process all ForwardDifferentiate instructions (kIROp_ForwardDifferentiate), by 
        // generating derivative code for the referenced function.
        //
        bool modified = processReferencedFunctions(builder);

        // Replaces IRDifferentialPairType with an auto-generated struct,
        // IRDifferentialPairGetDifferential with 'differential' field access,
        // IRDifferentialPairGetPrimal with 'primal' field access, and
        // IRMakeDifferentialPair with an IRMakeStruct.
        // 
        modified |= simplifyDifferentialBottomType(builder);

        modified |= processPairTypes(builder, module->getModuleInst());

        modified |= eliminateDifferentialBottomType(builder);

        return modified;
    }

    IRInst* lookupJVPReference(IRInst* primalFunction)
    {
        if(auto jvpDefinition = primalFunction->findDecoration<IRForwardDerivativeDecoration>())
            return jvpDefinition->getForwardDerivativeFunc();
        return nullptr;
    }

    // Recursively process instructions looking for JVP calls (kIROp_ForwardDifferentiate),
    // then check that the referenced function is marked correctly for differentiation.
    //
    bool processReferencedFunctions(IRBuilder* builder)
    {
        List<IRForwardDifferentiate*> autoDiffWorkList;

        for (;;)
        {
            // Collect all `ForwardDifferentiate` insts from the module.
            autoDiffWorkList.clear();
            processInstsOfType<IRForwardDifferentiate>(kIROp_ForwardDifferentiate, [&](IRForwardDifferentiate* fwdDiffInst)
            {
                autoDiffWorkList.add(fwdDiffInst);
            });

            if (autoDiffWorkList.getCount() == 0)
                break;

            // Process collected `ForwardDifferentiate` insts and replace them with placeholders for
            // differentiated functions.
            transcriberStorage.followUpFunctionsToTranscribe.clear();

            for (auto fwdDiffInst : autoDiffWorkList)
            {
                auto baseInst = fwdDiffInst->getBaseFn();
                if (auto baseFunction = as<IRGlobalValueWithCode>(baseInst))
                {
                    if (auto existingDiffFunc = lookupJVPReference(baseFunction))
                    {
                        fwdDiffInst->replaceUsesWith(existingDiffFunc);
                        fwdDiffInst->removeAndDeallocate();
                    }
                    else if (isMarkedForForwardDifferentiation(baseFunction))
                    {
                        if (as<IRFunc>(baseFunction) || as<IRGeneric>(baseFunction))
                        {
                            IRInst* diffFunc = transcriberStorage.transcribe(builder, baseFunction);
                            SLANG_ASSERT(diffFunc);
                            fwdDiffInst->replaceUsesWith(diffFunc);
                            fwdDiffInst->removeAndDeallocate();
                        }
                        else
                        {
                            // TODO(Sai): This would probably be better with a more specific
                            // error code.
                            getSink()->diagnose(fwdDiffInst->sourceLoc,
                                Diagnostics::internalCompilerError,
                                "Unexpected instruction. Expected func or generic");
                        }
                    }
                    else
                    {
                        // TODO(Sai): This would probably be better with a more specific
                        // error code.
                        getSink()->diagnose(fwdDiffInst->sourceLoc,
                            Diagnostics::internalCompilerError,
                            "Cannot differentiate functions not marked for differentiation");
                    }
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
        return true;
    }

    IRInst* lowerPairType(IRBuilder* builder, IRType* pairType, bool* isTrivial = nullptr)
    {
        builder->setInsertBefore(pairType);
        auto loweredPairTypeInfo = (&pairBuilderStorage)->lowerDiffPairType(
            builder,
            pairType);
        if (isTrivial)
            *isTrivial = loweredPairTypeInfo.isTrivial;
        return loweredPairTypeInfo.loweredType;
    }

    IRInst* lowerMakePair(IRBuilder* builder, IRInst* inst)
    {
        
        if (auto makePairInst = as<IRMakeDifferentialPair>(inst))
        {
            bool isTrivial = false;
            auto pairType = as<IRDifferentialPairType>(makePairInst->getDataType());
            if (auto loweredPairType = lowerPairType(builder, pairType, &isTrivial))
            {
                builder->setInsertBefore(makePairInst);
                IRInst* result = nullptr;
                if (isTrivial)
                {
                    result = makePairInst->getPrimalValue();
                }
                else
                {
                    IRInst* operands[2] = { makePairInst->getPrimalValue(), makePairInst->getDifferentialValue() };
                    result = builder->emitMakeStruct((IRType*)(loweredPairType), 2, operands);
                }
                makePairInst->replaceUsesWith(result);
                makePairInst->removeAndDeallocate();
                return result;
            }
        }
        
        return nullptr;
    }

    IRInst* lowerPairAccess(IRBuilder* builder, IRInst* inst)
    {
        if (auto getDiffInst = as<IRDifferentialPairGetDifferential>(inst))
        {
            if (lowerPairType(builder, getDiffInst->getBase()->getDataType(), nullptr))
            {
                builder->setInsertBefore(getDiffInst);
                IRInst* diffFieldExtract = nullptr;
                diffFieldExtract = (&pairBuilderStorage)->emitDiffFieldAccess(builder, getDiffInst->getBase());
                getDiffInst->replaceUsesWith(diffFieldExtract);
                getDiffInst->removeAndDeallocate();
                return diffFieldExtract;
            }
        }
        else if (auto getPrimalInst = as<IRDifferentialPairGetPrimal>(inst))
        {
            if (lowerPairType(builder, getPrimalInst->getBase()->getDataType(), nullptr))
            {
                builder->setInsertBefore(getPrimalInst);

                IRInst* primalFieldExtract = nullptr;
                primalFieldExtract = (&pairBuilderStorage)->emitPrimalFieldAccess(builder, getPrimalInst->getBase());
                getPrimalInst->replaceUsesWith(primalFieldExtract);
                getPrimalInst->removeAndDeallocate();
                return primalFieldExtract;
            }
        }
        
        return nullptr;
    }

    bool processPairTypes(IRBuilder* builder, IRInst* instWithChildren)
    {
        bool modified = false;
        // Hoist all pair types to global scope when possible.
        auto moduleInst = module->getModuleInst();
        processInstsOfType<IRDifferentialPairType>(kIROp_DifferentialPairType, [&](IRInst* originalPairType)
            {
                if (originalPairType->parent != moduleInst)
                {
                    originalPairType->removeFromParent();
                    ShortList<IRInst*> operands;
                    for (UInt i = 0; i < originalPairType->getOperandCount(); i++)
                    {
                        operands.add(originalPairType->getOperand(i));
                    }
                    auto newPairType = builder->findOrEmitHoistableInst(
                        originalPairType->getFullType(),
                        originalPairType->getOp(),
                        originalPairType->getOperandCount(),
                        operands.getArrayView().getBuffer());
                    originalPairType->replaceUsesWith(newPairType);
                    originalPairType->removeAndDeallocate();
                }
            });

        sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();

        processAllInsts([&](IRInst* inst)
            {
                // Make sure the builder is at the right level.
                builder->setInsertInto(instWithChildren);

                switch (inst->getOp())
                {
                case kIROp_DifferentialPairGetDifferential:
                case kIROp_DifferentialPairGetPrimal:
                    lowerPairAccess(builder, inst);
                    modified = true;
                    break;

                case kIROp_MakeDifferentialPair:
                    lowerMakePair(builder, inst);
                    modified = true;
                    break;

                default:
                    break;
                }
            });

        processInstsOfType<IRDifferentialPairType>(kIROp_DifferentialPairType, [&](IRDifferentialPairType* inst)
            {
                if (auto loweredType = lowerPairType(builder, inst))
                {
                    inst->replaceUsesWith(loweredType);
                    inst->removeAndDeallocate();
                }
            });
        return modified;
    }

    bool simplifyDifferentialBottomType(IRBuilder* builder)
    {
        bool modified = false;
        auto diffBottom = builder->getDifferentialBottom();

        bool changed = true;
        List<IRUse*> uses;
        while (changed)
        {
            changed = false;
            // Replace all insts whose type is `DifferentialBottomType` to `diffBottom`.
            processAllInsts([&](IRInst* inst)
                {
                    if (inst->getDataType() && inst->getDataType()->getOp() == kIROp_DifferentialBottomType)
                    {
                        if (inst != diffBottom)
                        {
                            inst->replaceUsesWith(diffBottom);
                            inst->removeAndDeallocate();
                            modified = true;
                        }
                    }
                });
            // Go through all uses of diffBottom and run simplification.
            processAllInsts([&](IRInst* inst)
                {
                    if (!inst->hasUses())
                        return;

                    builder->setInsertBefore(inst);
                    IRInst* valueToReplace = nullptr;
                    switch (inst->getOp())
                    {
                    case kIROp_Store:
                        if (as<IRStore>(inst)->getVal() == diffBottom)
                        {
                            inst->removeAndDeallocate();
                            changed = true;
                        }
                        return;
                    case kIROp_MakeDifferentialPair:
                        // Our simplification could lead to a situation where
                        // bottom is used to make a pair that has a non-bottom differential type,
                        // in this case we should use zero instead.
                        if (inst->getOperand(1) == diffBottom)
                        {
                            // Only apply if we are the second operand.
                            auto pairType = as<IRDifferentialPairType>(inst->getDataType());
                            if (pairBuilderStorage.getDiffTypeFromPairType(builder, pairType)->getOp() != kIROp_DifferentialBottomType)
                            {
                                auto zero = transcriberStorage.getDifferentialZeroOfType(builder, pairType->getValueType());
                                inst->setOperand(1, zero);
                                changed = true;
                            }
                        }
                        return;
                    case kIROp_DifferentialPairGetDifferential:
                        if (inst->getOperand(0)->getOp() == kIROp_MakeDifferentialPair)
                        {
                            valueToReplace = inst->getOperand(0)->getOperand(1);
                        }
                        break;
                    case kIROp_DifferentialPairGetPrimal:
                        if (inst->getOperand(0)->getOp() == kIROp_MakeDifferentialPair)
                        {
                            valueToReplace = inst->getOperand(0)->getOperand(0);
                        }
                        break;
                    case kIROp_Add:
                        if (inst->getOperand(0) == diffBottom)
                        {
                            valueToReplace = inst->getOperand(1);
                        }
                        else if (inst->getOperand(1) == diffBottom)
                        {
                            valueToReplace = inst->getOperand(0);
                        }
                        break;
                    case kIROp_Sub:
                        if (inst->getOperand(0) == diffBottom)
                        {
                            // If left is bottom, and right is not bottom, then we should return -right.
                            // However we can't possibly run into that case since both side of - operator
                            // must be at the same order of differentiation.
                            valueToReplace = diffBottom;
                        }
                        else if (inst->getOperand(1) == diffBottom)
                        {
                            valueToReplace = inst->getOperand(0);
                        }
                        break;
                    case kIROp_Mul:
                    case kIROp_Div:
                        if (inst->getOperand(0) == diffBottom)
                        {
                            valueToReplace = diffBottom;
                        }
                        else if (inst->getOperand(1) == diffBottom)
                        {
                            valueToReplace = diffBottom;
                        }
                        break;
                    default:
                        break;
                    }
                    if (valueToReplace)
                    {
                        inst->replaceUsesWith(valueToReplace);
                        changed = true;
                    }
                });
            modified |= changed;
        }

        return modified;
    }

    bool eliminateDifferentialBottomType(IRBuilder* builder)
    {
        simplifyDifferentialBottomType(builder);

        bool modified = false;
        auto diffBottom = builder->getDifferentialBottom();
        auto diffBottomType = diffBottom->getDataType();
        diffBottom->replaceUsesWith(builder->getVoidValue());
        diffBottom->removeAndDeallocate();
        diffBottomType->replaceUsesWith(builder->getVoidType());

        return modified;
    }

    // Checks decorators to see if the function should
    // be differentiated (kIROp_ForwardDifferentiableDecoration)
    // 
    bool isMarkedForForwardDifferentiation(IRGlobalValueWithCode* callable)
    {
        for(auto decoration = callable->getFirstDecoration(); 
            decoration;
            decoration = decoration->getNextDecoration())
        {
            if (decoration->getOp() == kIROp_ForwardDifferentiableDecoration)
            {
                return true;
            }
        }
        return false;
    }

    IRStringLit* getForwardDerivativeFuncName(IRBuilder*    builder,
                                IRInst*       func)
    {
        auto oldLoc = builder->getInsertLoc();
        builder->setInsertBefore(func);
        
        IRStringLit* name = nullptr;
        if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
        {
            name = builder->getStringValue((String(linkageDecoration->getMangledName()) + "_fwd_diff").getUnownedSlice());
        }
        else if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
        {
            name = builder->getStringValue((String(namehintDecoration->getName()) + "_fwd_diff").getUnownedSlice());
        }

        builder->setInsertLoc(oldLoc);

        return name;
    }

    JVPDerivativeContext(IRModule* module, DiagnosticSink* sink) :
        InstPassBase(module),
        sink(sink),
        autoDiffSharedContextStorage(module->getModuleInst()),
        transcriberStorage(&autoDiffSharedContextStorage, &sharedBuilderStorage)
    {
        pairBuilderStorage.sharedContext = &autoDiffSharedContextStorage;
        transcriberStorage.sink = sink;
        transcriberStorage.autoDiffSharedContext = &(autoDiffSharedContextStorage);
        transcriberStorage.pairBuilder = &(pairBuilderStorage);
    }

protected:
    // A transcriber object that handles the main job of 
    // processing instructions while maintaining state.
    //
    JVPTranscriber                  transcriberStorage;
    
    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink*                 sink;

    // Context to find and manage the witness tables for types 
    // implementing `IDifferentiable`
    AutoDiffSharedContext           autoDiffSharedContextStorage;

    // Builder for dealing with differential pair types.
    DifferentialPairTypeBuilder     pairBuilderStorage;

};

// Set up context and call main process method.
//
bool processForwardDifferentiableFuncs(
        IRModule*                           module,
        DiagnosticSink*                     sink,
        IRJVPDerivativePassOptions const&)
{   
    // Simplify module to remove dead code.
    IRDeadCodeEliminationOptions options;
    options.keepExportsAlive = true;
    options.keepLayoutsAlive = true;
    eliminateDeadCode(module, options);

    JVPDerivativeContext context(module, sink);
    bool changed = context.processModule();
    return changed;
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


}
