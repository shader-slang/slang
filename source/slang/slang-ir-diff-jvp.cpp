// slang-ir-diff-jvp.cpp
#include "slang-ir-diff-jvp.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"

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

    Pair(P primal, D differential) : primal(primal), differential(differential)
    {}
};

typedef Pair<IRInst*, IRInst*> InstPair;

struct DifferentiableTypeConformanceContext
{
    Dictionary<IRInst*, IRInst*>            witnessTableMap;

    IRInst*                                 inst = nullptr;

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
    
    // Modules that don't use differentiable types
    // won't have the IDifferentiable interface type available. 
    // Set to false to indicate that we are uninitialized.
    // 
    bool                                    isInterfaceAvailable = false;

    // For handling generic blocks, we use a parent pointer to allow
    // looking up types in all relevant scopes.
    DifferentiableTypeConformanceContext*   parent = nullptr;

    DifferentiableTypeConformanceContext(DifferentiableTypeConformanceContext* parent, IRInst* inst) : parent(parent), inst(inst)
    {
        if (parent)
        {
            differentiableInterfaceType = parent->differentiableInterfaceType;
            differentialAssocTypeStructKey = parent->differentialAssocTypeStructKey;
            zeroMethodStructKey = parent->zeroMethodStructKey;
            addMethodStructKey = parent->addMethodStructKey;

            isInterfaceAvailable = parent->isInterfaceAvailable;
        }
        else
        {
            differentiableInterfaceType = as<IRInterfaceType>(findDifferentiableInterface());
            if (differentiableInterfaceType)
            {
                differentialAssocTypeStructKey = findDifferentialTypeStructKey();
                zeroMethodStructKey = findZeroMethodStructKey();
                addMethodStructKey = findAddMethodStructKey();

                if (differentialAssocTypeStructKey)
                    isInterfaceAvailable = true;
            }
        }
    }

    DifferentiableTypeConformanceContext(IRInst* inst) :
        DifferentiableTypeConformanceContext(nullptr, inst)
    {}

    // Lookup a witness table for the concreteType. One should exist if concreteType
    // inherits (successfully) from IDifferentiable.
    // 
    IRInst* lookUpConformanceForType(IRBuilder* builder, IRInst* type)
    {
        SLANG_ASSERT(isInterfaceAvailable);
        // TODO: Cache the returned value to avoid repeatedly scanning through
        // blocks looking for the type entries.
        // 
        if (auto irWitness = builder->findDifferentiableTypeEntry(type, type->getParent()))
        {
            return irWitness;
        }

        return nullptr;
    }

    IRInst* lookUpInterfaceMethod(IRBuilder* builder, IRType* origType, IRStructKey* key)
    {
        if (auto conformance = lookUpConformanceForType(builder, origType))
        {
            if (auto witnessTable = as<IRWitnessTable>(conformance))
            {
                for (auto entry : witnessTable->getEntries())
                {
                    if (entry->getRequirementKey() == key)
                        return entry->getSatisfyingVal();
                }
            }
            else if (auto witnessTableParam = as<IRParam>(conformance))
            {
                return builder->emitLookupInterfaceMethodInst(
                    builder->getTypeKind(),
                    witnessTableParam,
                    key);
            }
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
        return lookUpInterfaceMethod(builder, origType, differentialAssocTypeStructKey);
    }

    IRInst* getZeroMethodForType(IRBuilder* builder, IRType* origType)
    {
        return lookUpInterfaceMethod(builder, origType, zeroMethodStructKey);
    }

    IRInst* getAddMethodForType(IRBuilder* builder, IRType* origType)
    {
        return lookUpInterfaceMethod(builder, origType, addMethodStructKey);
    }

    private:

    IRInst* findDifferentiableInterface()
    {
        if (auto module = as<IRModuleInst>(inst))
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

    IRStructKey* findZeroMethodStructKey()
    {
        return getIDifferentiableStructKeyAtIndex(1);
    }

    IRStructKey* findAddMethodStructKey()
    {
        return getIDifferentiableStructKeyAtIndex(2);
    }

    IRStructKey* getIDifferentiableStructKeyAtIndex(UInt index)
    {
        if (as<IRModuleInst>(inst) && differentiableInterfaceType)
        {
            // Assume for now that IDifferentiable has exactly three fields.
            SLANG_ASSERT(differentiableInterfaceType->getOperandCount() == 4);
            if (auto entry = as<IRInterfaceRequirementEntry>(differentiableInterfaceType->getOperand(index)))
                return as<IRStructKey>(entry->getRequirementKey());
            else
            {
                SLANG_UNEXPECTED("IDifferentiable interface entry unexpected type");
            }
        }

        return nullptr;
    }

    void loadWitnessTablesForInterface(IRInst* interfaceType)
    {
        
        if (auto module = as<IRModuleInst>(inst))
        {
            for (auto globalInst : module->getGlobalInsts())
            {
                if (globalInst->getOp() == kIROp_WitnessTable &&
                    cast<IRWitnessTableType>(globalInst->getDataType())->getConformanceType() ==
                        interfaceType)
                {
                    // TODO: Can we have multiple conformances for the same pair of types?
                    // TODO: Can type instrs be duplicated (i.e. two different float types)? And if they are duplicated, can
                    // we supply the dictionary with a custom equality rule that uses 'type1->equals(type2)'
                    witnessTableMap.Add(as<IRWitnessTable>(globalInst)->getConcreteType(), globalInst);
                }
            }
        }
        else if (auto generic = as<IRGeneric>(inst))
        {
            List<IRParam*> typeParams;

            auto genericParam = generic->getFirstParam();
            while (genericParam)
            {
                if (as<IRTypeType>(genericParam->getDataType()))
                {
                    typeParams.add(genericParam);
                }
                else
                    break;
                
                genericParam = genericParam->getNextParam();
            }
            
            Count tableIndex = 0;
            while (genericParam)
            {
                SLANG_ASSERT(!as<IRTypeType>(genericParam->getDataType()));

                if (tableIndex >= typeParams.getCount())
                    break;

                if (auto witnessTableType = as<IRWitnessTableType>(genericParam->getDataType()))
                {
                    // TODO(sai): Heavily flawed way to find the right witness table.
                    // Rewrite this part
                    if (witnessTableType->getConformanceType() == differentiableInterfaceType)
                        witnessTableMap.Add(typeParams[tableIndex], genericParam);
                }
                else
                    break;

                tableIndex += 1;
                genericParam = genericParam->getNextParam();
            }
            
        }

    }

};


IRInst* findGlobal(IRInst* inst)
{
    if (inst->getParent() != inst->getModule()->getModuleInst())
    {
        return findGlobal(inst->getParent());
    }

    return inst;
}

void moveGlobalToBeforeUses(IRBuilder*, IRInst* globalInst)
{
    HashSet<IRInst*> globalsOfUses;
    for (auto use = globalInst->firstUse; use; use = use->nextUse)
    {
        globalsOfUses.Add(findGlobal(use->getUser()));
    }

    IRInst* earliestUse = nullptr;
    for (auto cursor = globalInst; cursor; cursor = cursor->getPrevInst())
    {   
        if (globalsOfUses.Contains(cursor))
        {
            earliestUse = cursor;
        }
    }

    if (earliestUse)
    {
        globalInst->insertBefore(earliestUse);
    }
}

struct DifferentialPairTypeBuilder
{
    
    DifferentialPairTypeBuilder(DifferentiableTypeConformanceContext* diffConformanceContext) :
        diffConformanceContext(diffConformanceContext)
    {}

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
        if (auto basePairStructType = as<IRStructType>(baseInst->getDataType()))
        {
            return as<IRFieldExtract>(builder->emitFieldExtract(
                    findField(basePairStructType, key)->getFieldType(),
                    baseInst,
                    key
                ));
        }
        else if (auto ptrType = as<IRPtrTypeBase>(baseInst->getDataType()))
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
        else if (auto specializedType = as<IRSpecialize>(baseInst->getDataType()))
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

    void relocateNewTypes(IRBuilder* builder)
    {
        for (auto typeInst : generatedTypeList)
        {
            moveGlobalToBeforeUses(builder, typeInst);
        }
    }

    void _createGenericDiffPairType(IRBuilder* builder)
    {
        // Insert directly at top level (skip any generic scopes etc.)
        auto insertLoc = builder->getInsertLoc();
        builder->setInsertInto(builder->getModule()->getModuleInst());

        // Make a generic version of the pair struct.
        auto irGeneric = builder->emitGeneric();
        irGeneric->setFullType(builder->getTypeKind());
        builder->setInsertInto(irGeneric);

        generatedTypeList.add(irGeneric);

        auto irBlock = builder->emitBlock();
        builder->setInsertInto(irBlock);

        auto pTypeParam = builder->emitParam(builder->getTypeType());
        builder->addNameHintDecoration(pTypeParam, UnownedTerminatedStringSlice("pT"));

        auto dTypeParam = builder->emitParam(builder->getTypeType());
        builder->addNameHintDecoration(dTypeParam, UnownedTerminatedStringSlice("dT"));

        auto irStructType = builder->createStructType();
        builder->emitReturn(irStructType);

        auto primalKey = _getOrCreatePrimalStructKey(builder);
        builder->addNameHintDecoration(primalKey, UnownedTerminatedStringSlice("primal"));
        builder->createStructField(irStructType, primalKey, (IRType*) pTypeParam);

        auto diffKey = _getOrCreateDiffStructKey(builder);
        builder->addNameHintDecoration(diffKey, UnownedTerminatedStringSlice("differential"));
        builder->createStructField(irStructType, diffKey, (IRType*) dTypeParam);
        
        // Reset cursor when done.
        builder->setInsertLoc(insertLoc);

        this->genericDiffPairType = irGeneric;
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

    IRInst* _getOrCreateGenericDiffPairType(IRBuilder* builder)
    {
        if (!this->genericDiffPairType)
        {
            _createGenericDiffPairType(builder);
        }

        SLANG_ASSERT(this->genericDiffPairType);
        return this->genericDiffPairType;
    }
    
    IRInst* _createDiffPairType(IRBuilder* builder, IRType* origBaseType)
    {
        if (auto diffBaseType = diffConformanceContext->getDifferentialForType(builder, origBaseType))
        {
            SLANG_ASSERT(!as<IRParam>(origBaseType));

            auto pairStructType = builder->createStructType();
            builder->createStructField(pairStructType, _getOrCreatePrimalStructKey(builder), origBaseType);
            builder->createStructField(pairStructType, _getOrCreateDiffStructKey(builder), (IRType*) diffBaseType);

            return pairStructType;
        }
        return nullptr;
    }

    IRInst* getOrCreateDiffPairType(IRBuilder* builder, IRType* origBaseType)
    {
        if (pairTypeCache.ContainsKey(origBaseType))
            return pairTypeCache[origBaseType];

        auto pairType = _createDiffPairType(builder, origBaseType);
        pairTypeCache.Add(origBaseType, pairType);

        return pairType;
    }

    Dictionary<IRInst*, IRInst*> pairTypeCache;

    DifferentiableTypeConformanceContext* diffConformanceContext;
    
    IRStructKey* globalPrimalKey = nullptr;

    IRStructKey* globalDiffKey = nullptr;

    IRInst* genericDiffPairType = nullptr;

    List<IRInst*> generatedTypeList;
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
    DifferentiableTypeConformanceContext*   diffConformanceContext;

    // Builder to help with creating and accessing the 'DifferentiablePair<T>' struct
    DifferentialPairTypeBuilder*            pairBuilder;

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

    IRType* differentiateType(IRBuilder* builder, IRType* origType)
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
                return (IRType*)(diffConformanceContext->getDifferentialForType(
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
            return (IRType*)(diffConformanceContext->getDifferentialForType(builder, (IRType*)primalType));
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

        return (IRType*)pairBuilder->getOrCreateDiffPairType(builder, primalType);
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
            IRParam* diffPairParam = builder->emitParam(diffPairType);

            auto diffPairVarName = makeDiffPairName(origParam);
            if (diffPairVarName.getLength() > 0)
                builder->addNameHintDecoration(diffPairParam, diffPairVarName.getUnownedSlice());

            SLANG_ASSERT(diffPairParam);

            return InstPair(
                pairBuilder->emitPrimalFieldAccess(builder, diffPairParam),
                pairBuilder->emitDiffFieldAccess(builder, diffPairParam));
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
        
        auto primalLoad = cloneInst(&cloneEnv, builder, origLoad);

        IRInst* diffLoad = nullptr;

        if (auto diffPtr = lookupDiffInst(origPtr, nullptr))
        {
            // Default case, we're loading from a known differential inst.
            diffLoad = as<IRLoad>(builder->emitLoad(diffPtr));
            return InstPair(primalLoad, diffLoad);
        }
        return InstPair(primalLoad, nullptr);
    }

    InstPair transcribeStore(IRBuilder* builder, IRStore* origStore)
    {
        IRInst* origStoreLocation = origStore->getPtr();
        IRInst* origStoreVal = origStore->getVal();
        
        auto primalStore = cloneInst(&cloneEnv, builder, origStore);

        auto diffStoreLocation = lookupDiffInst(origStoreLocation, nullptr);
        auto diffStoreVal = lookupDiffInst(origStoreVal, nullptr);

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

        if (as<IRFunc>(origCall->getCallee()))
        {
            auto origCallee = origCall->getCallee();

            // Since concrete functions are globals, the primal callee is the same
            // as the original callee.
            //
            auto primalCallee = origCallee;

            IRInst* diffCallee = nullptr;
            bool isAccessor = false;

            if (auto accessorDecor = primalCallee->findDecoration<IRJVPDerivativeAccessorReferenceDecoration>())
            {
                diffCallee = accessorDecor->getJVPFunc();
                isAccessor = true;
            }
            else
            {
                // TODO: If inner is not differentiable, treat as non-differentiable call.
                // Build the differential callee
                diffCallee = builder->emitJVPDifferentiateInst(
                    differentiateFunctionType(builder, as<IRFuncType>(primalCallee->getFullType())),
                    primalCallee);
            }

            List<IRInst*> args;
            // Go over the parameter list and create pairs for each input (if required)
            for (UIndex ii = 0; ii < origCall->getArgCount(); ii++)
            {
                auto origArg = origCall->getArg(ii);
                auto primalArg = findOrTranscribePrimalInst(builder, origArg);
                SLANG_ASSERT(primalArg);

                auto primalType = primalArg->getDataType();
                if (auto pairType = tryGetDiffPairType(builder, primalType))
                {
                    auto diffArg = findOrTranscribeDiffInst(builder, origArg);

                    if (!diffArg)
                        diffArg = getDifferentialZeroOfType(builder, primalType);
                    
                    // If a pair type can be formed, this must be non-null.
                    SLANG_RELEASE_ASSERT(diffArg);

                    if (isAccessor)
                    {
                        // `dget` `dset` accessors takes in only diff values as parameter, so don't wrap them in
                        // a DifferentialPair.
                        args.add(diffArg);
                    }
                    else
                    {
                        auto diffPair = builder->emitMakeDifferentialPair(pairType, primalArg, diffArg);
                        args.add(diffPair);
                    }
                }
                else
                {
                    // Add original/primal argument.
                    args.add(primalArg);
                }
            }
            
            IRType* diffReturnType = nullptr;
            if (isAccessor)
                diffReturnType = as<IRFunc>(diffCallee)->getResultType();
            else
                diffReturnType = tryGetDiffPairType(builder, origCall->getFullType());
            SLANG_ASSERT(diffReturnType);

            auto callInst = builder->emitCallInst(
                diffReturnType,
                diffCallee,
                args);

            IRInst* primalResultValue = nullptr;
            IRInst* diffResultValue = nullptr;
            if (isAccessor)
            {
                primalResultValue = origCall;
                diffResultValue = callInst;
            }
            else
            {
                primalResultValue = pairBuilder->emitPrimalFieldAccess(builder, callInst);
                diffResultValue = pairBuilder->emitDiffFieldAccess(builder, callInst);
            }
            
            return InstPair(primalResultValue, diffResultValue);
        }
        else if(as<IRSpecialize>(origCall->getCallee()) ||
                as<IRLookupWitnessMethod>(origCall->getCallee()))
        {
            getSink()->diagnose(origCall->sourceLoc,
                Diagnostics::unimplemented,
                "attempting to differentiate unspecialized callee or an interface method");
        }
        else
        {
            // Note that this can only happen if the callee is a result
            // of a higher-order operation. For now, we assume that we cannot
            // differentiate such calls safely.
            // TODO(sai): Should probably get checked in the front-end.
            //
            getSink()->diagnose(origCall->sourceLoc,
                Diagnostics::internalCompilerError,
                "attempting to differentiate unresolved callee");
        }

        return InstPair(nullptr, nullptr);
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

    InstPair transcribeConst(IRBuilder*, IRInst* origInst)
    {
        switch(origInst->getOp())
        {
            case kIROp_FloatLit:
            case kIROp_VoidLit:
            case kIROp_IntLit:
                return InstPair(origInst, nullptr);
        }

        getSink()->diagnose(
            origInst->sourceLoc,
            Diagnostics::unimplemented,
            "attempting to differentiate unhandled const type");

        return InstPair(nullptr, nullptr);
    }

    InstPair transcribeSpecialize(IRBuilder* builder, IRSpecialize* origSpecialize)
    {
        // This is slightly counter-intuitive, but we don't perform any differentiation
        // logic here. We simple clone the original specialize which points to the original function,
        // or the cloned version in case we're inside a generic scope.
        // The differentiation logic is inserted later when this is used in an IRCall.
        // This decision is mostly to maintain a uniform convention of JVPDifferentiate(Specialize(Fn))
        // rather than have Specialize(JVPDifferentiate(Fn))
        // 
        auto diffSpecialize = cloneInst(&cloneEnv, builder, origSpecialize);
        return InstPair(diffSpecialize, diffSpecialize);
    }

    InstPair transcibeLookupInterfaceMethod(IRBuilder* builder, IRLookupWitnessMethod* origLookup)
    {
        // This is slightly counter-intuitive, but we don't perform any differentiation
        // logic here. We simple clone the original lookup which points to the original function,
        // or the cloned version in case we're inside a generic scope.
        // The differentiation logic is inserted later when this is used in an IRCall.
        // This decision is mostly to maintain a uniform convention of JVPDifferentiate(Lookup(Table))
        // rather than have Lookup(JVPDifferentiate(Table))
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
            // Since primalType has a corresponding differential type, we can lookup the 
            // definition for zero().
            auto zeroMethod = this->diffConformanceContext->getZeroMethodForType(builder, primalType);
            SLANG_ASSERT(zeroMethod);

            auto emptyArgList = List<IRInst*>();
            return builder->emitCallInst((IRType*)diffType, zeroMethod, emptyArgList);
        }
        else
        {
            // We special case a few non-differentiable types that sometimes appear in places
            // where we're forced to provide a differential zero value. For instance, 
            // float3(float, float, int) is accepted by the compiler, but is tricky in the context
            // of differentiation since int is non-differentiable, and should be cast to float first.
            // In the absence of such casts, this piece of code generates appropriate zero values.
            // 
            switch (primalType->getOp())
            {
                case kIROp_IntType:
                    return builder->getIntValue(primalType, 0);
                default:
                    getSink()->diagnose(primalType->sourceLoc,
                        Diagnostics::internalCompilerError,
                        "could not generate zero value for given type");
                    return nullptr;
            }
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

        // Look for the differentiable type dictionary and clone it (and anything else we might need).
        // TODO: This logic might have issues if there are additional instructions (say lookup_interface_requirement) 
        // that are operands.
        // TODO: This is currently cloning the global dictionary. Should only clone dictionaries in generic blocks.
        if (auto origDict = builder->findDifferentiableTypeDictionary(origBlock))
        {
            auto clonedDict = cloneInst(&cloneEnv, builder, origDict);
            mapPrimalInst(origDict, clonedDict);
            mapDifferentialInst(origDict, clonedDict);
        }

        // Then, run through every instruction and use the transcriber to generate the appropriate
        // derivative code.
        //
        for (auto child = origBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
            this->transcribe(builder, child);

        builder->setInsertLoc(oldLoc);

        return InstPair(diffBlock, diffBlock);
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

    // Transcribe a function definition.
    InstPair transcribeFunc(IRBuilder* builder, IRFunc* origFunc)
    {
        IRFunc* primalFunc = nullptr;

        auto oldLoc = builder->getInsertLoc();

        // If this is a top-level function, there is no need to clone it
        // since it is visible in all the scopes.
        // Otherwise, we need to clone it in case of generic scopes.
        // 
        // TODO(sai): Is this the correct thing to do? Can a function cloned inside a 
        // generic scope but is not the return value of that generic, be used within
        // that scope? Or do we have to call out to the original generic specialized with
        // the current generic params?
        // 
        bool isTopLevelFunc = (as<IRModuleInst>(origFunc->parent) != nullptr);
        if (isTopLevelFunc)
        {
            builder->setInsertBefore(origFunc);
            primalFunc = origFunc;
        }
        else
        {
            // TODO(sai): this might never be called, and it might never make sense
            // to call it either. Potentially remove this.
            primalFunc = as<IRFunc>(
                cloneInst(&cloneEnv, builder, origFunc));
        }

        auto diffFunc = builder->createFunc();
        
        SLANG_ASSERT(as<IRFuncType>(origFunc->getFullType()));
        IRType* diffFuncType = this->differentiateFunctionType(
            builder,
            as<IRFuncType>(origFunc->getFullType()));
        diffFunc->setFullType(diffFuncType);

        // TODO(sai): Replace naming scheme
        // if (auto jvpName = this->getJVPFuncName(builder, primalFn))
        //    builder->addNameHintDecoration(diffFunc, jvpName);
        
        // Transcribe children from origFunc into diffFunc
        builder->setInsertInto(diffFunc);
        for (auto block = origFunc->getFirstBlock(); block; block = block->getNextBlock())
            this->transcribe(builder, block);
        
        // Reset builder position
        builder->setInsertLoc(oldLoc);

        return InstPair(primalFunc, diffFunc);
    }

    // Transcribe a generic definition
    InstPair transcribeGeneric(IRBuilder* builder, IRGeneric* origGeneric)
    {
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
            getSink()->diagnose(origInst->sourceLoc,
                    Diagnostics::unexpected,
                    "should not be attempting to differentiate anything specialized here.");
            return InstPair(nullptr, nullptr);

        case kIROp_lookup_interface_method:
            return transcibeLookupInterfaceMethod(builder, as<IRLookupWitnessMethod>(origInst));

        case kIROp_FieldExtract:
        case kIROp_FieldAddress:
            getSink()->diagnose(origInst->sourceLoc,
                Diagnostics::unexpected,
                "should not be attempting to differentiate field extract/field address.");
            return InstPair(nullptr, nullptr);
        case kIROp_getElement:
        case kIROp_getElementPtr:
            return transcribeGetElement(builder, origInst);
        
        case kIROp_loop:
            return transcribeLoop(builder, as<IRLoop>(origInst));

        case kIROp_ifElse:
            return transcribeIfElse(builder, as<IRIfElse>(origInst));

        case kIROp_DifferentiableTypeDictionary:
            // Ignore dictionary insts.
            return InstPair(nullptr, nullptr);

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
            return transcribeFunc(builder, as<IRFunc>(origInst)); 

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

struct IRWorkQueue
{
    // Work list to hold the active set of insts whose children
    // need to be looked at.
    //
    List<IRInst*> workList;
    HashSet<IRInst*> workListSet;

    void push(IRInst* inst)
    {
        if(!inst) return;
        if(workListSet.Contains(inst)) return;
        
        workList.add(inst);
        workListSet.Add(inst);
    }

    IRInst* pop()
    {
        if (workList.getCount() != 0)
        {
            IRInst* topItem = workList.getFirst();
            // TODO(Sai): Repeatedly calling removeAt() can be really slow.
            // Consider a specialized data structure or using removeLast()
            // 
            workList.removeAt(0);
            workListSet.Remove(topItem);
            return topItem;
        }
        return nullptr;
    }

    IRInst* peek()
    {
        return workList.getFirst();
    }
};

struct JVPDerivativeContext
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
    
        IRBuilder builderStorage(sharedBuilderStorage);
        IRBuilder* builder = &builderStorage;

        // Process all JVPDifferentiate instructions (kIROp_JVPDifferentiate), by 
        // generating derivative code for the referenced function.
        //
        bool modified = processReferencedFunctions(builder);

        // Replaces IRDifferentialPairType with an auto-generated struct,
        // IRDifferentialPairGetDifferential with 'differential' field access,
        // IRDifferentialPairGetPrimal with 'primal' field access, and
        // IRMakeDifferentialPair with an IRMakeStruct.
        // 
        modified |= processPairTypes(builder, module->getModuleInst(), (&diffConformanceContextStorage));
        
        // Temporary fix: Move generated types, if any, to before their use locations.
        (&pairBuilderStorage)->relocateNewTypes(builder);

        // Remove all kIROp_DifferentiableTypeDictionary instructions and 
        // kIROp_DifferentialGetterDecoration decorations
        // 
        modified |= stripDiffTypeInformation(builder, module->getModuleInst());

        return modified;
    }

    IRInst* lookupJVPReference(IRInst* primalFunction)
    {
        if(auto jvpDefinition = primalFunction->findDecoration<IRJVPDerivativeReferenceDecoration>())
            return jvpDefinition->getJVPFunc();
        if (auto jvpGetter = primalFunction->findDecoration<IRJVPDerivativeAccessorReferenceDecoration>())
            return jvpGetter->getJVPFunc();
        return nullptr;
    }

    // Recursively process instructions looking for JVP calls (kIROp_JVPDifferentiate),
    // then check that the referenced function is marked correctly for differentiation.
    //
    bool processReferencedFunctions(IRBuilder* builder)
    {
        IRWorkQueue* workQueue = &(workQueueStorage);

        // Put the top-level inst into the queue.
        workQueue->push(module->getModuleInst());
        
        // Keep processing items until the queue is complete.
        while (IRInst* workItem = workQueue->pop())
        {   
            for(auto child = workItem->getFirstChild(); child; child = child->getNextInst())
            {
                // Either the child instruction has more children (func/block etc..)
                // and we add it to the work list for further processing, or 
                // it's an ordinary inst in which case we check if it's a JVPDifferentiate
                // instruction.
                //
                if (child->getFirstChild() != nullptr)
                    workQueue->push(child);
                
                if (auto jvpDiffInst = as<IRJVPDifferentiate>(child))
                {
                    auto baseInst = jvpDiffInst->getBaseFn();

                    IRGlobalValueWithCode* baseFunction = nullptr;

                    if (auto specializeInst = as<IRSpecialize>(baseInst))
                    {
                        baseFunction = as<IRGlobalValueWithCode>(specializeInst->getBase());
                    }
                    else if (auto globalValWithCode = as<IRGlobalValueWithCode>(baseInst))
                    {
                        baseFunction = globalValWithCode;
                    }

                    SLANG_ASSERT(baseFunction);

                    // If the JVP Reference already exists, no need to
                    // differentiate again.
                    //
                    if (lookupJVPReference(baseFunction)) continue;

                    if (isMarkedForJVP(baseFunction))
                    {
                        if (as<IRFunc>(baseFunction) || as<IRGeneric>(baseFunction))
                        {
                            IRInst* diffFunc = (&transcriberStorage)->transcribe(builder, baseFunction);
                            SLANG_ASSERT(diffFunc);
                            builder->addJVPDerivativeReferenceDecoration(baseFunction, diffFunc);
                            workQueue->push(diffFunc);
                        } 
                        else
                        {
                            // TODO(Sai): This would probably be better with a more specific
                            // error code.
                            getSink()->diagnose(jvpDiffInst->sourceLoc,
                                Diagnostics::internalCompilerError,
                                "Unexpected instruction. Expected func or generic");
                        }
                    }
                    else 
                    {
                        // TODO(Sai): This would probably be better with a more specific
                        // error code.
                        getSink()->diagnose(jvpDiffInst->sourceLoc,
                            Diagnostics::internalCompilerError,
                            "Cannot differentiate functions not marked for differentiation");
                    }
                }
            }
        }

        return true;
    }

    IRInst* lowerPairType(IRBuilder* builder, IRType* type, DifferentiableTypeConformanceContext*)
    {
        
        if (auto pairType = as<IRDifferentialPairType>(type))
        {
            builder->setInsertBefore(pairType);

            auto diffPairStructType = (&pairBuilderStorage)->getOrCreateDiffPairType(
                builder,
                pairType->getValueType());

            pairType->replaceUsesWith(diffPairStructType);
            pairType->removeAndDeallocate();

            return diffPairStructType;
        }
        else if (auto loweredStructType = as<IRStructType>(type))
        {
            // Already lowered to struct.
            return loweredStructType;
        }
        else if (auto specializedStructType = as<IRSpecialize>(type))
        {
            // Already lowered to specialized struct.
            return specializedStructType;
        }
        
        return nullptr;
    }

    IRInst* lowerMakePair(IRBuilder* builder, IRInst* inst, DifferentiableTypeConformanceContext* diffContext)
    {
        
        if (auto makePairInst = as<IRMakeDifferentialPair>(inst))
        {
            auto diffPairStructType = lowerPairType(builder, makePairInst->getDataType(), diffContext);
            
            builder->setInsertBefore(makePairInst);
            
            List<IRInst*> operands;
            operands.add(makePairInst->getPrimalValue());
            operands.add(makePairInst->getDifferentialValue());

            auto makeStructInst = builder->emitMakeStruct((IRType*)(diffPairStructType), operands);
            makePairInst->replaceUsesWith(makeStructInst);
            makePairInst->removeAndDeallocate();

            return makeStructInst;
        }
        
        return nullptr;
    }

    IRInst* lowerPairAccess(IRBuilder* builder, IRInst* inst, DifferentiableTypeConformanceContext* diffContext)
    {
        
        if (auto getDiffInst = as<IRDifferentialPairGetDifferential>(inst))
        {
            lowerPairType(builder, getDiffInst->getBase()->getDataType(), diffContext);

            builder->setInsertBefore(getDiffInst);
            
            auto diffFieldExtract = (&pairBuilderStorage)->emitDiffFieldAccess(builder, getDiffInst->getBase());
            getDiffInst->replaceUsesWith(diffFieldExtract);
            getDiffInst->removeAndDeallocate();

            return diffFieldExtract;
        }
        else if (auto getPrimalInst = as<IRDifferentialPairGetPrimal>(inst))
        {
            lowerPairType(builder, getPrimalInst->getBase()->getDataType(), diffContext);

            builder->setInsertBefore(getPrimalInst);

            auto primalFieldExtract = (&pairBuilderStorage)->emitPrimalFieldAccess(builder, getPrimalInst->getBase());
            getPrimalInst->replaceUsesWith(primalFieldExtract);
            getPrimalInst->removeAndDeallocate();

            return primalFieldExtract;
        }
        
        return nullptr;
    }

    bool processPairTypes(IRBuilder* builder, IRInst* instWithChildren, DifferentiableTypeConformanceContext* diffContext)
    {
        bool modified = false;

        // Create a new sub-context to scan witness tables inside workItem 
        // (mainly relevant if instWithChildren is a generic scope)
        // 
        auto subContext = DifferentiableTypeConformanceContext(diffContext, instWithChildren);
        (&pairBuilderStorage)->diffConformanceContext = (&subContext);

        for (auto child = instWithChildren->getFirstChild(); child; )
        {
            // Make sure the builder is at the right level.
            builder->setInsertInto(instWithChildren);

            auto nextChild = child->getNextInst();

            switch (child->getOp())
            {
                case kIROp_DifferentialPairType:
                    lowerPairType(builder, as<IRType>(child), &subContext);
                    break;
                
                case kIROp_DifferentialPairGetDifferential:
                case kIROp_DifferentialPairGetPrimal:
                    lowerPairAccess(builder, child, &subContext);
                    break;
                
                case kIROp_MakeDifferentialPair:
                    lowerMakePair(builder, child, &subContext);
                    break;
                
                default:
                    if (child->getFirstChild())
                        modified = processPairTypes(builder, child, (&subContext)) | modified;
            }

            child = nextChild;
        }

        // Reset the context back to the parent.
        (&pairBuilderStorage)->diffConformanceContext = diffContext;

        return modified;
    }

    bool stripDiffTypeInformation(IRBuilder* builder, IRInst* parent)
    {
        bool modified = false;

        auto child = parent->getFirstChild();
        while (child)
        {
            auto nextChild = child->getNextInst();
            
            if (child->getOp() == kIROp_DifferentiableTypeDictionary)
            {
                child->removeAndDeallocate();
                child = nextChild;
                modified = true;
                continue;
            }

            if (child->getFirstChild() != nullptr)
            {
                modified |= stripDiffTypeInformation(builder, child);
            }

            child = nextChild;
        }

        return modified;
    }

    // Checks decorators to see if the function should
    // be differentiated (kIROp_JVPDerivativeMarkerDecoration)
    // 
    bool isMarkedForJVP(IRGlobalValueWithCode* callable)
    {
        for(auto decoration = callable->getFirstDecoration(); 
            decoration;
            decoration = decoration->getNextDecoration())
        {
            if (decoration->getOp() == kIROp_JVPDerivativeMarkerDecoration)
            {
                return true;
            }
        }
        return false;
    }

    // Removes the JVPDerivativeMarkerDecoration from the provided callable, 
    // if it exists.
    //
    void unmarkForJVP(IRGlobalValueWithCode* callable)
    {
        for(auto decoration = callable->getFirstDecoration(); 
            decoration;
            decoration = decoration->getNextDecoration())
        {
            if (decoration->getOp() == kIROp_JVPDerivativeMarkerDecoration)
            {
                decoration->removeAndDeallocate();
                return;
            }
        }
    }

    IRStringLit* getJVPFuncName(IRBuilder*    builder,
                                IRInst*       func)
    {
        auto oldLoc = builder->getInsertLoc();
        builder->setInsertBefore(func);
        
        IRStringLit* name = nullptr;
        if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
        {
            name = builder->getStringValue((String(linkageDecoration->getMangledName()) + "_jvp").getUnownedSlice());
        }
        else if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
        {
            name = builder->getStringValue((String(namehintDecoration->getName()) + "_jvp").getUnownedSlice());
        }

        builder->setInsertLoc(oldLoc);

        return name;
    }

    JVPDerivativeContext(IRModule* module, DiagnosticSink* sink) :
        module(module), sink(sink),
        diffConformanceContextStorage(module->getModuleInst()),
        pairBuilderStorage(&diffConformanceContextStorage)
    {
        transcriberStorage.sink = sink;
        transcriberStorage.diffConformanceContext = &(diffConformanceContextStorage);
        transcriberStorage.pairBuilder = &(pairBuilderStorage);
    }

    protected:

    // This type passes over the module and generates
    // forward-mode derivative versions of functions 
    // that are explicitly marked for it.
    //
    IRModule*                       module;

    // Shared builder state for our derivative passes.
    SharedIRBuilder                 sharedBuilderStorage;

    // A transcriber object that handles the main job of 
    // processing instructions while maintaining state.
    //
    JVPTranscriber                  transcriberStorage;
    
    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink*                 sink;

    // Work queue to hold a stream of instructions that need
    // to be checked for references to derivative functions.
    IRWorkQueue                     workQueueStorage;

    // Context to find and manage the witness tables for types 
    // implementing `IDifferentiable`
    DifferentiableTypeConformanceContext diffConformanceContextStorage;

    // Builder for dealing with differential pair types.
    DifferentialPairTypeBuilder     pairBuilderStorage;

};

// Set up context and call main process method.
//
bool processJVPDerivativeMarkers(
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

    return context.processModule();
}

}
