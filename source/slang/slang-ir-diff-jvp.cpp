// slang-ir-diff-jvp.cpp
#include "slang-ir-diff-jvp.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"

namespace Slang
{



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
            // TODO (Big change required: The layout of the generic args goes T1, T2, T3 ..., WT1, WT2, WT3)
            // Right now, we're accepting: T1, WT1, T2, WT2, ...
            // 
            SLANG_ASSERT(false);

            IRInst* currGenericTypeParam = nullptr;
            for (auto genericParam : generic->getParams())
            {
                if ((genericParam->getDataType()->getOp() == kIROp_WitnessTableType))
                {
                    if (cast<IRWitnessTableType>(genericParam->getDataType())->getConformanceType() ==
                        interfaceType)
                    SLANG_ASSERT(currGenericTypeParam);
                    SLANG_ASSERT(!witnessTableMap.ContainsKey(currGenericTypeParam));
                    witnessTableMap.Add(currGenericTypeParam, genericParam);
                }
                else if (as<IRTypeType>(genericParam->getDataType()))
                {
                    currGenericTypeParam = genericParam;
                }
                else if (genericParam->getDataType()->getOp() == kIROp_IntType)
                {
                    currGenericTypeParam = nullptr;
                }
                else
                {
                    SLANG_UNEXPECTED("Unhandled generic param type");
                }
            }
        }

    }

};

struct DifferentialPairTypeBuilder
{
    
    DifferentialPairTypeBuilder(DifferentiableTypeConformanceContext* diffConformanceContext) :
        diffConformanceContext(diffConformanceContext)
    {}

    IRFieldExtract* emitPrimalFieldExtract(IRBuilder* builder, IRInst* baseInst)
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
        else
        {
            SLANG_UNREACHABLE("basePairType must be an IRStructType");
        }
    }

    IRFieldExtract* emitDiffFieldExtract(IRBuilder* builder, IRInst* baseInst)
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
        else
        {
            SLANG_UNREACHABLE("basePairType must be an IRStructType");
        }
    }
    
    IRStructType* createDiffPairType(IRBuilder* builder, IRType* origBaseType)
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

        auto pairType = createDiffPairType(builder, origBaseType);
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

    IRInst* getDifferentialInst(IRInst* origInst)
    {
        return instMapD[origInst];
    }

    IRInst* getDifferentialInst(IRInst* origInst, IRInst* defaultInst)
    {
        return (hasDifferentialInst(origInst)) ? instMapD[origInst] : defaultInst;
    }

    bool hasDifferentialInst(IRInst* origInst)
    {
        return instMapD.ContainsKey(origInst);
    }

    IRFuncType* differentiateFunctionType(IRBuilder* builder, IRFuncType* funcType)
    {
        List<IRType*> newParameterTypes;
        IRType* diffReturnType;

        for (UIndex i = 0; i < funcType->getParamCount(); i++)
        {
            auto origType = funcType->getParamType(i);
            if (auto diffPairType = tryCreateDifferentialPairFromType(builder, origType))
                newParameterTypes.add(diffPairType);
            else
                newParameterTypes.add(origType);
        }

        // Transcribe return type. 
        // This will be void if the primal return type is non-differentiable.
        //
        if (tryCreateDifferentialPairFromType(builder, funcType->getResultType()))
        {
            diffReturnType = (IRType*)(diffConformanceContext->getDifferentialForType(builder, funcType->getResultType()));
        }
        else
        {
            diffReturnType = builder->getVoidType();
        }

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
    
    IRType* tryCreateDifferentialPairFromType(IRBuilder* builder, IRType* origType)
    {
        // If this is a PtrType (out, inout, etc..), then create diff pair from
        // value type and re-apply the appropropriate PtrType wrapper.
        // 
        if (auto origPtrType = as<IRPtrType>(origType))
        {   
            if (auto diffPairValueType = tryCreateDifferentialPairFromType(builder, origPtrType->getValueType()))
                return builder->getPtrType(origType->getOp(), diffPairValueType);
            else 
                return nullptr;
        }

        return pairBuilder->getOrCreateDiffPairType(builder, origType);
    }
    
    IRInst* differentiateParam(IRBuilder* builder, IRParam* origParam)
    {
        // Try to get a differential pair, if the original type implements IDifferentiable
        if (auto diffPairType = tryCreateDifferentialPairFromType(builder, origParam->getFullType()))
        {
            IRParam* diffParam = builder->emitParam(diffPairType);

            // auto diffNameHint = getDiffPairVarName(origParam);
            // if (diffNameHint.getLength() > 0)
            //     builder->addNameHintDecoration(diffParam, diffNameHint.getUnownedSlice());

            SLANG_ASSERT(diffParam);
            return diffParam;
        }
        return nullptr;
    }

    List<IRParam*> transcribeParams(IRBuilder* builder, IRInstList<IRParam> origParamList)
    {
        List<IRParam*> origParams;
        List<IRParam*> newParamList;
        List<bool> isDiff;

        // Go through all parameters and generate derivative versions.
        // Note that this is one place where the primal (original) inst needs special
        // handling. Instead of adding a new inst for the differential and cloning the 
        // primal one, we have a single param inst for both, with a pair type. 
        // Therefore, we emit a member access instruction for the primal and differential.
        // 

        // First emit all the parameters 
        for (auto origParam : origParamList)
        {
            origParams.add(origParam);
            if (IRInst* diffParam = differentiateParam(builder, origParam))
            {
                newParamList.add(as<IRParam>(diffParam));
                isDiff.add(true);
            }
            else
            {
                newParamList.add(as<IRParam>(cloneInst(&cloneEnv, builder, origParam)));
                isDiff.add(false);
            }
        }

        // Go back over them and emit accessors.
        for (Index ii = 0; ii < newParamList.getCount(); ii++)
        {
            IRInst* newInst = nullptr;
            IRInst* diffInst = nullptr;

            if (isDiff[ii])
            {
                auto diffPairParam = newParamList[ii];
                newInst = pairBuilder->emitPrimalFieldExtract(builder, diffPairParam);
                diffInst = pairBuilder->emitDiffFieldExtract(builder, diffPairParam);
            }
            else
            {
                newInst = newParamList[ii];
                diffInst = nullptr;
            }

            cloneEnv.mapOldValToNew[origParams[ii]] = newInst;
            mapDifferentialInst(newInst, diffInst);
        }

        return newParamList;

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

    IRInst* differentiateVar(IRBuilder* builder, IRVar* origVar)
    {
        if (IRType* diffType = differentiateType(builder, origVar->getDataType()->getValueType()))
        {
            IRVar* diffVar = builder->emitVar(diffType);
            SLANG_ASSERT(diffVar);

            auto diffNameHint = getJVPVarName(origVar);
            if (diffNameHint.getLength() > 0)
                builder->addNameHintDecoration(diffVar, diffNameHint.getUnownedSlice());

            return diffVar;
        }
        return nullptr;
    }

    IRInst* differentiateBinaryArith(IRBuilder* builder, IRInst* origArith)
    {
        SLANG_ASSERT(origArith->getOperandCount() == 2);
        
        auto origLeft = origArith->getOperand(0);
        auto origRight = origArith->getOperand(1);

        auto diffLeft = getDifferentialInst(origLeft);
        auto diffRight = getDifferentialInst(origRight);

        auto leftZero = builder->getFloatValue(origLeft->getDataType(), 0.0);
        auto rightZero = builder->getFloatValue(origRight->getDataType(), 0.0);

        if (diffLeft || diffRight)
        {
            diffLeft = diffLeft ? diffLeft : leftZero;
            diffRight = diffRight ? diffRight : rightZero;

            // Might have to do special-case handling for non-scalar types,
            // like float3 or float3x3
            // 
            auto resultType = origArith->getDataType();
            switch(origArith->getOp())
            {
            case kIROp_Add:
                return builder->emitAdd(resultType, diffLeft, diffRight);
            case kIROp_Mul:
                return builder->emitAdd(resultType,
                    builder->emitMul(resultType, diffLeft, origRight),
                    builder->emitMul(resultType, origLeft, diffRight));
            case kIROp_Sub:
                return builder->emitSub(resultType, diffLeft, diffRight);
            case kIROp_Div:
                return builder->emitDiv(resultType, 
                    builder->emitSub(
                        resultType,
                        builder->emitMul(resultType, diffLeft, origRight),
                        builder->emitMul(resultType, origLeft, diffRight)),
                    builder->emitMul(
                        origRight->getDataType(), origRight, origRight
                    ));
            default:
                getSink()->diagnose(origArith->sourceLoc,
                    Diagnostics::unimplemented,
                    "this arithmetic instruction cannot be differentiated");
            }
        }

        return nullptr;
    }

    IRInst* differentiateLoad(IRBuilder* builder, IRLoad* origLoad)
    {
        auto origPtr = origLoad->getPtr();
        if (as<IRVar>(origPtr) || as<IRParam>(origPtr))
        {   
            // If the loaded parameter has a differential version, 
            // emit a load instruction for the differential parameter.
            // Otherwise, emit nothing since there's nothing to load.
            // 
            if (auto diffPtr = getDifferentialInst(origPtr, nullptr))
            {
                IRLoad* diffLoad = as<IRLoad>(builder->emitLoad(diffPtr));
                SLANG_ASSERT(diffLoad);
                return diffLoad;
            }
            return nullptr;
        }
        else
            getSink()->diagnose(origLoad->sourceLoc,
                    Diagnostics::unimplemented,
                    "this load instruction cannot be differentiated");
        return nullptr;
    }

    IRInst* differentiateStore(IRBuilder* builder, IRStore* origStore)
    {
        IRInst* storeLocation = origStore->getPtr();
        IRInst* storeVal = origStore->getVal();
        if (as<IRVar>(storeLocation) || as<IRParam>(storeLocation))
        {   
            // If the stored value has a differential version, 
            // emit a store instruction for the differential parameter.
            // Otherwise, emit nothing since there's nothing to load.
            // 
            IRInst* diffStoreVal = getDifferentialInst(storeVal);
            IRInst* diffStoreLocation = getDifferentialInst(storeLocation);
            if (diffStoreVal && diffStoreLocation)
            {
                IRStore* diffStore = as<IRStore>(
                    builder->emitStore(diffStoreLocation, diffStoreVal));
                SLANG_ASSERT(diffStore);
                return diffStore;
            }
            return nullptr;
        }
        else
            getSink()->diagnose(origStore->sourceLoc,
                    Diagnostics::unimplemented,
                    "this store instruction cannot be differentiated");
        return nullptr;
    }

    IRInst* differentiateReturn(IRBuilder* builder, IRReturn* origReturn)
    {
        IRInst* returnVal = origReturn->getVal();
        if (auto diffReturnVal = getDifferentialInst(returnVal, nullptr))
        {   
            IRReturn* diffReturn = as<IRReturn>(builder->emitReturn(diffReturnVal));
            SLANG_ASSERT(diffReturn);
            return diffReturn;
        }
        else
        {
            // If the differential return value is not available, emit a 
            // void return.
            return builder->emitReturn();
        }
    }

    // Since int/float literals are sometimes nested inside an IRConstructor
    // instruction, we check to make sure that the nested instr is a constant
    // and then return nullptr. Literals do not need to be differentiated.
    //
    IRInst* differentiateConstruct(IRBuilder*, IRInst* origConstruct)
    {   
        if (as<IRConstant>(origConstruct->getOperand(0)) && origConstruct->getOperandCount() == 1)
            return nullptr;
        else
            getSink()->diagnose(origConstruct->sourceLoc,
                    Diagnostics::unimplemented,
                    "this construct instruction cannot be differentiated");
        return nullptr;
    }

    // Differentiating a call instruction here is primarily about generating
    // an appropriate call list based on whichever parameters have differentials 
    // in the current transcription context.
    // Note(sai): Currently we don't look at modifiers (in, out, const etc..) in the function
    // type, and so only support 'plain' parameters. We need to validte this somewhere to
    // avoid weird behaviour
    // 
    IRInst* differentiateCall(IRBuilder* builder, IRCall* origCall)
    {   
        if (auto origCallee = as<IRFunc>(origCall->getCallee()))
        {
            
            // Build the differential callee
            IRInst* diffCall = builder->emitJVPDifferentiateInst(
                differentiateFunctionType(builder, as<IRFuncType>(origCallee->getFullType())),
                origCallee);
            
            List<IRInst*> args;
            // Go over the parameter list and all primal arguments.
            for (UIndex ii = 0; ii < origCall->getArgCount(); ii++)
            {
                args.add(origCall->getArg(ii));
            }

            {
                IRParam* param = origCallee->getFirstParam();
                // Go over the parameter list again and arguments for types that need differentials.
                for (UIndex ii = 0; ii < origCall->getArgCount(); ii++)
                {
                    // Look the parameter up in the callee's signature. If it requires a derivative, proceed.
                    // Otherwise, continue.
                    //
                    if (differentiateType(builder, param->getDataType()))
                    {
                        // If the corresponding argument does not have a differential, create and place a
                        // 0 argument.
                        //
                        auto origArg = origCall->getArg(ii);
                        if (auto diffArg = getDifferentialInst(origArg, nullptr))
                            args.add(diffArg);
                        else
                            args.add(getZeroOfType(builder, origArg->getDataType()));
                    }

                    param = param->getNextParam();
                }
            }
            
            return builder->emitCallInst(differentiateType(builder, origCall->getFullType()),
                                         diffCall,
                                         args);
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
        return nullptr;
    }

    IRInst* differentiateSwizzle(IRBuilder* builder, IRSwizzle* origSwizzle)
    {
        if (auto diffBase = getDifferentialInst(origSwizzle->getBase(), nullptr))
        {
            List<IRInst*> swizzleIndices;
            for (UIndex ii = 0; ii < origSwizzle->getElementCount(); ii++)
                swizzleIndices.add(origSwizzle->getElementIndex(ii));
            
            return builder->emitSwizzle(differentiateType(builder, origSwizzle->getDataType()),
                                        diffBase,
                                        origSwizzle->getElementCount(),
                                        swizzleIndices.getBuffer());
        }
        return nullptr;
    }

    IRInst* differentiateByPassthrough(IRBuilder* builder, IRInst* origInst)
    {
        UCount operandCount = origInst->getOperandCount();

        List<IRInst*> diffOperands;
        for (UIndex ii = 0; ii < operandCount; ii++)
        {
            // If the operand has a differential version, replace the original with the 
            // differential.
            // Otherwise, abandon the differentiation attempt and assume that origInst 
            // cannot (or does not need to) be differentiated.
            // 
            if (auto diffInst = getDifferentialInst(origInst->getOperand(ii), nullptr))
                diffOperands.add(diffInst);
            else
                return nullptr;
        }
        
        return builder->emitIntrinsicInst(
                    differentiateType(builder, origInst->getDataType()),
                    origInst->getOp(),
                    operandCount,
                    diffOperands.getBuffer());
    }

    IRInst* handleControlFlow(IRBuilder* builder, IRInst* origInst)
    {
        switch(origInst->getOp())
        {
            case kIROp_unconditionalBranch:
                auto origBranch = as<IRUnconditionalBranch>(origInst);

                // Branches with extra operands not handled currently.
                if (origBranch->getOperandCount() > 1)
                    break;

                if (auto diffBlock = getDifferentialInst(origBranch->getTargetBlock(), nullptr))
                    return builder->emitBranch(as<IRBlock>(diffBlock));
                else
                    return nullptr;
        }

        getSink()->diagnose(
            origInst->sourceLoc,
            Diagnostics::unimplemented,
            "attempting to differentiate unhandled control flow");
        return nullptr;
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
            default:
                getSink()->diagnose(type->sourceLoc,
                    Diagnostics::internalCompilerError,
                    "could not generate zero value for given type");
                return nullptr;       
        }
    }

    // Logic for whether a primal instruction needs to be replicated
    // in the differential function. We detect and avoid replicating 
    // 'side-effect' instructions.
    // 
    bool isPurelyFunctional(IRBuilder*, IRInst* origInst)
    {
        if (as<IRTerminatorInst>(origInst))
            return false;
        else if (auto origParam = as<IRParam>(origInst))
        {
            // Out-type parameters are discarded from the parameter list,
            // since pure JVP functions to not write to primal outputs.
            // 
            if (as<IROutType>(origParam->getDataType()))
                return false;
        }
        else if (auto origStore = as<IRStore>(origInst))
        {
            IRInst* storeLocation = origStore->getPtr();

            // Writing to a parameter is a side-effect that should be avoided.
            if(as<IRParam>(storeLocation))
                return false;

            // If attempting to store to a location without a clone, 
            // then this instruction likely has side-effects external to the
            // current function.
            // 
            if(!lookUp(&cloneEnv, storeLocation))
                return false;
        }
        
        return true;
    }

    IRInst* transcribe(IRBuilder* builder, IRInst* origInstOld)
    {

        // Clone the old instruction into the new differential function.
        // 
        IRInst* origInst = cloneInst(&cloneEnv, builder, origInstOld);

        SLANG_ASSERT(origInst);

        IRInst* diffInst = differentiateInst(builder, origInst);
        
        // In case it's not safe to clone the old instruction, 
        // remove it from the graph.
        // For instance, instructions that handle control flow 
        // (return statements) shouldn't be replicated.
        //
        if (isPurelyFunctional(builder, origInstOld))
            mapDifferentialInst(origInst, diffInst);
        else
        {
            // This inst should never have been used.
            SLANG_ASSERT(origInst->firstUse == nullptr);

            origInst->removeAndDeallocate();
            mapDifferentialInst(origInstOld, diffInst);
        }

        return diffInst;
    }

    IRInst* differentiateInst(IRBuilder* builder, IRInst* origInst)
    {
        // Handle common operations
        switch (origInst->getOp())
        {
        case kIROp_Var:
            return differentiateVar(builder, as<IRVar>(origInst));

        case kIROp_Load:
            return differentiateLoad(builder, as<IRLoad>(origInst));

        case kIROp_Store:
            return differentiateStore(builder, as<IRStore>(origInst));

        case kIROp_Return:
            return differentiateReturn(builder, as<IRReturn>(origInst));

        case kIROp_Add:
        case kIROp_Mul:
        case kIROp_Sub:
        case kIROp_Div:
            return differentiateBinaryArith(builder, origInst);

        case kIROp_Construct:
            return differentiateConstruct(builder, origInst);
        
        case kIROp_Call:
            return differentiateCall(builder, as<IRCall>(origInst));
        
        case kIROp_swizzle:
            return differentiateSwizzle(builder, as<IRSwizzle>(origInst));
        
        case kIROp_constructVectorFromScalar:
            return differentiateByPassthrough(builder, origInst);

        case kIROp_unconditionalBranch:
        case kIROp_conditionalBranch:
            return handleControlFlow(builder, origInst);

        }
    
        // If none of the cases have been hit, check if the instruction is a
        // type.
        // For now we don't have logic to differentiate types that appear in blocks.
        // So, we ignore them.
        //
        if (as<IRType>(origInst))
            return nullptr;
        
        
        // If we reach this statement, the instruction type is likely unhandled.
        getSink()->diagnose(origInst->sourceLoc,
                    Diagnostics::unimplemented,
                    "this instruction cannot be differentiated");
        return nullptr;
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
        // Temporarily, this also replaces any uses of DifferentialPair<T> with an auto-generated struct type.
        // This is mainly for user-defined derivative code, since auto-generated code
        // uses the struct type by default.
        //
        bool modified = processReferencedFunctions(builder);

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
            auto nextChild = child->getNextInst();

            if (auto diffPairType = as<IRDifferentialPairType>(child))
            {
                if (subContext.isInterfaceAvailable)
                {
                    
                    auto diffPairStructType = (&pairBuilderStorage)->createDiffPairType(builder, diffPairType->getValueType());

                    diffPairType->replaceUsesWith(diffPairStructType);
                    diffPairType->removeAndDeallocate();

                    modified = true;
                }
            }
            else if (child->getFirstChild())
            {
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
        }

        // Go back over the blocks, and process the children of each block.
        for (auto block = primalFn->getFirstBlock(); block; block = block->getNextBlock())
        {
            auto jvpBlock = as<IRBlock>(transcriberStorage.getDifferentialInst(block, block));
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
                          IRBlock*      primalBlock,
                          IRBlock*      jvpBlock = nullptr)
    {   
        JVPTranscriber* transcriber = &(transcriberStorage);

        // Create if not already created, and then insert into new block.
        if (!jvpBlock)
            jvpBlock = builder->emitBlock();
        else
            builder->setInsertInto(jvpBlock);

        // First transcribe the parameter list. This is done separately because we
        // want all the derivative parameters emitted after the primal parameters
        // rather than interleaved with one another.
        //
        transcriber->transcribeParams(builder, primalBlock->getParams());

        // Run through every instruction and use the transcriber to generate the appropriate
        // derivative code.
        //
        for(auto child = primalBlock->getFirstOrdinaryInst(); child; child = child->getNextInst())
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
