// slang-ir-diff-jvp.cpp
#include "slang-ir-diff-jvp.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"

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
    Dictionary<IRInst*, IRInst*>    witnessTableMap;

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
            isInterfaceAvailable = parent->isInterfaceAvailable;
        }
        else
        {
            differentiableInterfaceType = as<IRInterfaceType>(findDifferentiableInterface());
            if (differentiableInterfaceType)
            {
                differentialAssocTypeStructKey = findDifferentialTypeStructKey();

                if (differentialAssocTypeStructKey)
                    isInterfaceAvailable = true;
            }
        }

        if (isInterfaceAvailable)
        {
            // Load all witness tables corresponding to the IDifferentiable interface.
            loadWitnessTablesForInterface(differentiableInterfaceType);
        }
    }

    DifferentiableTypeConformanceContext(IRInst* inst) :
        DifferentiableTypeConformanceContext(nullptr, inst)
    {}

    // Lookup a witness table for the concreteType. One should exist if concreteType
    // inherits (successfully) from IDifferentiable.
    // 
    IRInst* lookUpConformanceForType(IRInst* type)
    {
        SLANG_ASSERT(isInterfaceAvailable);

        if (witnessTableMap.ContainsKey(type))
            return witnessTableMap[type];
        else if (parent)
            return parent->lookUpConformanceForType(type);
        else
            return nullptr;
    }
    
    // Lookup and return the 'Differential' type declared in the concrete type
    // in order to conform to the IDifferentiable interface.
    // Note that inside a generic block, this will be a witness table lookup instruction
    // that gets resolved during the specialization pass.
    // 
    IRInst* getDifferentialForType(IRBuilder* builder, IRType* origType)
    {
        SLANG_ASSERT(isInterfaceAvailable);

        if (auto conformance = lookUpConformanceForType(origType))
        {
            if (auto witnessTable = as<IRWitnessTable>(conformance))
            {
                for (auto entry : witnessTable->getEntries())
                {
                    if (entry->getRequirementKey() == differentialAssocTypeStructKey)
                        return as<IRType>(entry->getSatisfyingVal());
                }
            }
            else if (auto witnessTableParam = as<IRParam>(conformance))
            {
                return builder->emitLookupInterfaceMethodInst(
                    builder->getTypeKind(),
                    witnessTableParam,
                    differentialAssocTypeStructKey);
            }
        }

        return nullptr;
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
        if (as<IRModuleInst>(inst) && differentiableInterfaceType)
        {
            // Assume for now that IDifferentiable has exactly one field: the 'Differential' associated type.
            SLANG_ASSERT(differentiableInterfaceType->getOperandCount() == 1);
            if (auto entry = as<IRInterfaceRequirementEntry>(differentiableInterfaceType->getOperand(0)))
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
            
            UCount tableIndex = 0;
            while (genericParam)
            {
                SLANG_ASSERT(!as<IRTypeType>(genericParam->getDataType()));
                if (auto witnessTableType = as<IRWitnessTableType>(genericParam->getDataType()))
                {
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

struct DifferentialPairTypeBuilder
{
    
    DifferentialPairTypeBuilder(DifferentiableTypeConformanceContext* diffConformanceContext) :
        diffConformanceContext(diffConformanceContext)
    {}

    IRInst* emitPrimalFieldAccess(IRBuilder* builder, IRInst* baseInst)
    {
        if (auto basePairStructType = as<IRStructType>(baseInst->getDataType()))
        {
            auto primalField = as<IRStructField>(basePairStructType->getFirstChild());
            SLANG_ASSERT(primalField);

            return as<IRFieldExtract>(builder->emitFieldExtract(
                    primalField->getFieldType(),
                    baseInst,
                    primalField->getKey()
                ));
        }
        else if (auto ptrType = as<IRPtrTypeBase>(baseInst->getDataType()))
        {
            if (auto pairStructType = as<IRStructType>(ptrType->getValueType()))
            {
                auto primalField = as<IRStructField>(pairStructType->getFirstChild());
                SLANG_ASSERT(primalField);
                
                return as<IRFieldAddress>(builder->emitFieldAddress(
                        builder->getPtrType(primalField->getFieldType()),
                        baseInst,
                        primalField->getKey()
                    ));
            }
        }
        else
        {
            SLANG_UNREACHABLE("basePairType must be an IRStructType or PtrType<IRStructType>");
        }
        return nullptr;
    }

    IRInst* emitDiffFieldAccess(IRBuilder* builder, IRInst* baseInst)
    {
        if (auto basePairStructType = as<IRStructType>(baseInst->getDataType()))
        {
            auto diffField = as<IRStructField>(basePairStructType->getFirstChild()->getNextInst());
            SLANG_ASSERT(diffField);

            return as<IRFieldExtract>(builder->emitFieldExtract(
                    diffField->getFieldType(),
                    baseInst,
                    diffField->getKey()
                ));
        }
        else if (auto ptrType = as<IRPtrTypeBase>(baseInst->getDataType()))
        {
            if (auto pairStructType = as<IRStructType>(ptrType->getValueType()))
            {
                auto diffField = as<IRStructField>(pairStructType->getFirstChild()->getNextInst());
                SLANG_ASSERT(diffField);
                
                return as<IRFieldAddress>(builder->emitFieldAddress(
                        builder->getPtrType(diffField->getFieldType()),
                        baseInst,
                        diffField->getKey()
                    ));
            }
        }
        else
        {
            SLANG_UNREACHABLE("basePairType must be an IRStructType or PtrType<IRStructType>");
        }
        return nullptr;
    }
    
    IRStructType* _createDiffPairType(IRBuilder* builder, IRType* origBaseType)
    {
        if (auto diffBaseType = diffConformanceContext->getDifferentialForType(builder, origBaseType))
        {
            auto diffPairType = builder->createStructType();

            // Create a keys for the primal and differential fields.
            IRStructKey* origKey = builder->createStructKey();
            builder->addNameHintDecoration(origKey, UnownedTerminatedStringSlice("primal"));
            builder->createStructField(diffPairType, origKey, origBaseType);

            IRStructKey* diffKey = builder->createStructKey();
            builder->addNameHintDecoration(diffKey, UnownedTerminatedStringSlice("differential"));
            builder->createStructField(diffPairType, diffKey, (IRType*)(diffBaseType));

            return diffPairType;
        }
        return nullptr;
    }

    IRStructType* getOrCreateDiffPairType(IRBuilder* builder, IRType* origBaseType)
    {
        if (pairTypeCache.ContainsKey(origBaseType))
            return pairTypeCache[origBaseType];

        auto pairType = _createDiffPairType(builder, origBaseType);
        pairTypeCache.Add(origBaseType, pairType);

        return pairType;
    }

    Dictionary<IRType*, IRStructType*> pairTypeCache;

    DifferentiableTypeConformanceContext* diffConformanceContext;

};

struct JVPTranscriber
{

    // Stores the mapping of arbitrary 'R-value' instructions to instructions that represent
    // their differential values.
    Dictionary<IRInst*, IRInst*>            instMapD;

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
        instMapD.Add(origInst, diffInst);
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
            if (auto diffPairType = tryGetDiffPairType(builder, origType))
                newParameterTypes.add(diffPairType);
            else
                newParameterTypes.add(origType);
        }

        // Transcribe return type to a pair.
        // This will be void if the primal return type is non-differentiable.
        //
        if (auto returnPairType = tryGetDiffPairType(builder, funcType->getResultType()))
            diffReturnType = returnPairType;
        else
            diffReturnType = builder->getVoidType();

        return builder->getFuncType(newParameterTypes, diffReturnType);
    }

    IRType* differentiateType(IRBuilder* builder, IRType* origType)
    {
        switch (origType->getOp())
        {
            case kIROp_HalfType:
            case kIROp_FloatType:
            case kIROp_DoubleType:
            case kIROp_VectorType:
                return (IRType*)(diffConformanceContext->getDifferentialForType(builder, origType));
            case kIROp_OutType:
                return builder->getOutType(differentiateType(builder, as<IROutType>(origType)->getValueType()));
            case kIROp_InOutType:
                return builder->getInOutType(differentiateType(builder, as<IRInOutType>(origType)->getValueType()));
            default:
                return nullptr;
        }
    }
    
    IRType* tryGetDiffPairType(IRBuilder* builder, IRType* origType)
    {
        // If this is a PtrType (out, inout, etc..), then create diff pair from
        // value type and re-apply the appropropriate PtrType wrapper.
        // 
        if (auto origPtrType = as<IRPtrTypeBase>(origType))
        {   
            if (auto diffPairValueType = tryGetDiffPairType(builder, origPtrType->getValueType()))
                return builder->getPtrType(origType->getOp(), diffPairValueType);
            else 
                return nullptr;
        }

        return pairBuilder->getOrCreateDiffPairType(builder, origType);
    }

    InstPair transcribeParam(IRBuilder* builder, IRParam* origParam)
    {
        if (auto diffPairType = tryGetDiffPairType(builder, origParam->getFullType()))
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

        auto leftZero = builder->getFloatValue(origLeft->getDataType(), 0.0);
        auto rightZero = builder->getFloatValue(origRight->getDataType(), 0.0);

        if (diffLeft || diffRight)
        {
            diffLeft = diffLeft ? diffLeft : leftZero;
            diffRight = diffRight ? diffRight : rightZero;

            auto resultType = origArith->getDataType();
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

    InstPair transcribeLoad(IRBuilder* builder, IRLoad* origLoad)
    {
        auto origPtr = origLoad->getPtr();
        
        auto primalLoad = cloneInst(&cloneEnv, builder, origLoad);

        if (auto diffPtr = lookupDiffInst(origPtr, nullptr))
        {
            IRLoad* diffLoad = as<IRLoad>(builder->emitLoad(diffPtr));
            SLANG_ASSERT(diffLoad);

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

        // If the stored value has a differential version, 
        // emit a store instruction for the differential parameter.
        // Otherwise, emit nothing since there's nothing to load.
        // 
        if (diffStoreLocation && diffStoreVal)
        {
            IRStore* diffStore = as<IRStore>(
                    builder->emitStore(diffStoreLocation, diffStoreVal));
            SLANG_ASSERT(diffStore);
            
            return InstPair(primalStore, diffStore);
        }

        return InstPair(primalStore, nullptr);
    }

    InstPair transcribeReturn(IRBuilder* builder, IRReturn* origReturn)
    {
        IRInst* origReturnVal = origReturn->getVal();
        
        if (auto pairType = tryGetDiffPairType(builder, origReturnVal->getDataType()))
        {   
            IRInst* primalReturnVal = findOrTranscribePrimalInst(builder, origReturnVal);
        
            IRInst* diffReturnVal = findOrTranscribeDiffInst(builder, origReturnVal);
            if(!diffReturnVal)
                diffReturnVal = getZeroOfType(builder, origReturnVal->getDataType());

            auto diffPair = builder->emitMakeDifferentialPair(pairType, primalReturnVal, diffReturnVal);
            IRReturn* pairReturn = as<IRReturn>(builder->emitReturn(diffPair));
            return InstPair(pairReturn, pairReturn);
        }
        else
        {
            // If the differential return value is not available, emit a 
            // void return.
            IRInst* voidReturn = builder->emitReturn();
            return InstPair(voidReturn, voidReturn);
        }
    }

    // Since int/float literals are sometimes nested inside an IRConstructor
    // instruction, we check to make sure that the nested instr is a constant
    // and then return nullptr. Literals do not need to be differentiated.
    //
    InstPair transcribeConstruct(IRBuilder* builder, IRInst* origConstruct)
    {   
        IRInst* primalConstruct = cloneInst(&cloneEnv, builder, origConstruct);

        if (as<IRConstant>(origConstruct->getOperand(0)) && origConstruct->getOperandCount() == 1)
            return InstPair(primalConstruct, nullptr);
        else
            getSink()->diagnose(origConstruct->sourceLoc,
                    Diagnostics::unimplemented,
                    "this construct instruction cannot be differentiated");

        return InstPair(primalConstruct, nullptr);
    }

    // Differentiating a call instruction here is primarily about generating
    // an appropriate call list based on whichever parameters have differentials 
    // in the current transcription context.
    // 
    InstPair transcribeCall(IRBuilder* builder, IRCall* origCall)
    {   
        if (auto origCallee = as<IRFunc>(origCall->getCallee()))
        {
            
            // Build the differential callee
            IRInst* diffCall = builder->emitJVPDifferentiateInst(
                differentiateFunctionType(builder, as<IRFuncType>(origCallee->getFullType())),
                origCallee);
            
            List<IRInst*> args;
            // Go over the parameter list and create pairs for each input (if required)
            for (UIndex ii = 0; ii < origCall->getArgCount(); ii++)
            {
                auto origArg = origCall->getArg(ii);
                auto primalArg = findOrTranscribePrimalInst(builder, origArg);
                SLANG_ASSERT(primalArg);

                auto origType = origArg->getDataType();
                if (auto pairType = tryGetDiffPairType(builder, origType))
                {
                    
                    auto diffArg = findOrTranscribeDiffInst(builder, origArg);

                    // TODO(sai): This part is flawed. Replace with a call to the 
                    // 'zero()' interface method.
                    if (!diffArg)
                        diffArg = getZeroOfType(builder, origType);
                    
                    auto diffPair = builder->emitMakeDifferentialPair(pairType, primalArg, diffArg);

                    args.add(diffPair);
                }
                else
                {
                    // Add original/primal argument.
                    args.add(primalArg);
                }
            }
            
            auto callInst = builder->emitCallInst(
                tryGetDiffPairType(builder, origCall->getFullType()),
                diffCall,
                args);
            
            return InstPair(
                pairBuilder->emitPrimalFieldAccess(builder, callInst),
                pairBuilder->emitDiffFieldAccess(builder, callInst));
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
                    differentiateType(builder, origSwizzle->getDataType()),
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
                differentiateType(builder, origInst->getDataType()),
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

                // Branches with extra operands not handled currently.
                if (origBranch->getOperandCount() > 1)
                    break;

                IRInst* diffBranch = nullptr;

                if (auto diffBlock = lookupDiffInst(origBranch->getTargetBlock(), nullptr))
                    diffBranch = builder->emitBranch(as<IRBlock>(diffBlock));

                // For now, every block in the original fn must have a corresponding
                // block to compute both primals and derivatives.
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
                return InstPair(origInst, nullptr);
        }

        getSink()->diagnose(
            origInst->sourceLoc,
            Diagnostics::unimplemented,
            "attempting to differentiate unhandled const type");

        return InstPair(nullptr, nullptr);
    }

    // In differential computation, the 'default' differential value is always zero.
    // This is a consequence of differential computing being inherently linear. As a 
    // result, it's useful to have a method to generate zero literals of any (arithmetic) type.
    // 
    IRInst* getZeroOfType(IRBuilder* builder, IRType* type)
    {
        switch (type->getOp())
        {
            case kIROp_FloatType:
            case kIROp_HalfType:
            case kIROp_DoubleType:
                return builder->getFloatValue(type, 0.0);
            case kIROp_IntType:
                return builder->getIntValue(type, 0);
            case kIROp_VectorType:
            {
                IRInst* args[] = {getZeroOfType(builder, as<IRVectorType>(type)->getElementType())};
                return builder->emitIntrinsicInst(
                    type,
                    kIROp_constructVectorFromScalar,
                    1,
                    args);
            }
            default:
                getSink()->diagnose(type->sourceLoc,
                    Diagnostics::internalCompilerError,
                    "could not generate zero value for given type");
                return nullptr;       
        }
    }

    IRInst* transcribe(IRBuilder* builder, IRInst* origInst)
    {
        InstPair pair = transcribeInst(builder, origInst);

        if (auto primalInst = pair.primal)
        {
            mapPrimalInst(origInst, pair.primal);

            mapDifferentialInst(origInst, pair.differential);
            return pair.differential;
        }

        getSink()->diagnose(origInst->sourceLoc,
                    Diagnostics::internalCompilerError,
                    "failed to transcibe instruction");
        return nullptr;
    }

    InstPair transcribeInst(IRBuilder* builder, IRInst* origInst)
    {
        // Handle common operations
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

        case kIROp_Construct:
            return transcribeConstruct(builder, origInst);
        
        case kIROp_Call:
            return transcribeCall(builder, as<IRCall>(origInst));
        
        case kIROp_swizzle:
            return transcribeSwizzle(builder, as<IRSwizzle>(origInst));
        
        case kIROp_constructVectorFromScalar:
            return transcribeByPassthrough(builder, origInst);

        case kIROp_unconditionalBranch:
        case kIROp_conditionalBranch:
            return transcribeControlFlow(builder, origInst);

        case kIROp_FloatLit:
            return transcribeConst(builder, origInst);

        }
    
        // If none of the cases have been hit, check if the instruction is a
        // type.
        // For now we don't have logic to differentiate types that appear in blocks.
        // So, we clone and avoid differentiating them.
        //
        if (auto origType = as<IRType>(origInst))
            return InstPair(cloneInst(&cloneEnv, builder, origType), nullptr);
        
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

        return modified;
    }

    IRInst* lookupJVPReference(IRInst* primalFunction)
    {
        if(auto jvpDefinition = primalFunction->findDecoration<IRJVPDerivativeReferenceDecoration>())
            return jvpDefinition->getJVPFunc();
        
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
                    auto baseFunction = jvpDiffInst->getBaseFn();
                    // If the JVP Reference already exists, no need to
                    // differentiate again.
                    //
                    if(lookupJVPReference(baseFunction)) continue;

                    if (isFunctionMarkedForJVP(as<IRGlobalValueWithCode>(baseFunction)))
                    {
                        IRFunc* jvpFunction = emitJVPFunction(builder, as<IRFunc>(baseFunction));
                        builder->addJVPDerivativeReferenceDecoration(baseFunction, jvpFunction);
                        workQueue->push(jvpFunction);
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

    // Run through all the global-level instructions, 
    // looking for callables.
    // Note: We're only processing global callables (IRGlobalValueWithCode)
    // for now.
    // 
    bool processMarkedGlobalFunctions(IRBuilder* builder)
    {
        for (auto inst : module->getGlobalInsts())
        {
            // If the instr is a callable, get all the basic blocks
            if (auto callable = as<IRGlobalValueWithCode>(inst))
            {
                if (isFunctionMarkedForJVP(callable))
                {   
                    SLANG_ASSERT(as<IRFunc>(callable));

                    IRFunc* jvpFunction = emitJVPFunction(builder, as<IRFunc>(callable));
                    builder->addJVPDerivativeReferenceDecoration(callable, jvpFunction);

                    unmarkForJVP(callable);
                }
            }
        }
        return true;
    }

    IRInst* lowerPairType(IRBuilder* builder, IRType* type, DifferentiableTypeConformanceContext* diffContext)
    {
        if (diffContext->isInterfaceAvailable)
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

            auto makeStructInst = builder->emitMakeStruct(as<IRStructType>(diffPairStructType), operands);
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

    // Checks decorators to see if the function should
    // be differentiated (kIROp_JVPDerivativeMarkerDecoration)
    // 
    bool isFunctionMarkedForJVP(IRGlobalValueWithCode* callable)
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

    List<IRParam*> emitFuncParameters(IRBuilder* builder, IRFuncType* dataType)
    {
        List<IRParam*> params;
        for(UIndex i = 0; i < dataType->getParamCount(); i++)
        {
            params.add(
                builder->emitParam(dataType->getParamType(i)));
        }
        return params;
    }

    // Perform forward-mode automatic differentiation on 
    // the intstructions.
    //
    IRFunc* emitJVPFunction(IRBuilder* builder,
                            IRFunc*    primalFn)
    {
        
        builder->setInsertBefore(primalFn->getNextInst()); 

        auto jvpFn = builder->createFunc();
        
        SLANG_ASSERT(as<IRFuncType>(primalFn->getFullType()));
        IRType* jvpFuncType = transcriberStorage.differentiateFunctionType(
            builder,
            as<IRFuncType>(primalFn->getFullType()));
        jvpFn->setFullType(jvpFuncType);

        if (auto jvpName = getJVPFuncName(builder, primalFn))
            builder->addNameHintDecoration(jvpFn, jvpName);

        builder->setInsertInto(jvpFn);
        
        // Emit a block instruction for every block in the function, and map it as the 
        // corresponding differential.
        //
        for (auto block = primalFn->getFirstBlock(); block; block = block->getNextBlock())
        {
            auto jvpBlock = builder->emitBlock();
            transcriberStorage.mapDifferentialInst(block, jvpBlock);
            transcriberStorage.mapPrimalInst(block, jvpBlock);
        }

        // Go back over the blocks, and process the children of each block.
        for (auto block = primalFn->getFirstBlock(); block; block = block->getNextBlock())
        {
            auto jvpBlock = as<IRBlock>(transcriberStorage.lookupDiffInst(block, block));
            SLANG_ASSERT(jvpBlock);
            emitJVPBlock(builder, block, jvpBlock);
        }

        return jvpFn;
    }

    IRStringLit* getJVPFuncName(IRBuilder*    builder,
                                IRFunc*       func)
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

    IRBlock* emitJVPBlock(IRBuilder*    builder, 
                          IRBlock*      origBlock,
                          IRBlock*      jvpBlock = nullptr)
    {   
        JVPTranscriber* transcriber = &(transcriberStorage);

        // Create if not already created, and then insert into new block.
        if (!jvpBlock)
            jvpBlock = builder->emitBlock();
        else
            builder->setInsertInto(jvpBlock);

        
        // First transcribe every parameter in the block.
        for (auto param = origBlock->getFirstParam(); param; param = param->getNextParam())
        {
            transcriber->transcribe(builder, param);
        }

        // Then, run through every instruction and use the transcriber to generate the appropriate
        // derivative code.
        //
        for (auto child = origBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
        {
            transcriber->transcribe(builder, child);
        }

        return jvpBlock;
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
