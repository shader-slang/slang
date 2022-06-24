// slang-ir-diff-jvp.cpp
#include "slang-ir-diff-jvp.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"

namespace Slang
{

struct JVPTranscriber
{

    // Stores the mapping of arbitrary 'R-value' instructions to instructions that represent
    // their differential values.
    Dictionary<IRInst*, IRInst*> instMapD;

    // Cloning environment to hold mapping from old to new copies for the primal
    // instructions.
    IRCloneEnv cloneEnv;

    void mapDifferentialInst(IRInst* instP, IRInst* instD)
    {
        instMapD.Add(instP, instD);
    }

    IRInst* getDifferentialInst(IRInst* instP)
    {
        return instMapD[instP];
    }

    IRFuncType* differentiateFunctionType(IRBuilder* builder, IRFuncType* funcType)
    {
        List<IRType*> parameterTypesD;
        IRType* returnTypeD;

        // Add all primal parameters to the list.
        for (UIndex i = 0; i < funcType->getParamCount(); i++)
        {   
            parameterTypesD.add(funcType->getParamType(i));
        }

        // Add differential versions for the types we support.
        for (UIndex i = 0; i < funcType->getParamCount(); i++)
        {   
            if (auto typeD = differentiateType(builder, funcType->getParamType(i)))
                parameterTypesD.add(typeD);
        }

        // Transcribe return type. 
        // This will be void if the primal return type is non-differentiable.
        //
        returnTypeD = differentiateType(builder, funcType->getResultType());
        if (!returnTypeD)
            returnTypeD = builder->getVoidType();

        return builder->getFuncType(parameterTypesD, returnTypeD);
    }

    IRType* differentiateType(IRBuilder* builder, IRType* typeP)
    {
        switch (typeP->getOp())
        {
            case kIROp_HalfType:
            case kIROp_FloatType:
            case kIROp_DoubleType:
                return builder->getType(typeP->getOp());
            
            default:
                return nullptr;
        }
    }
    
    IRInst* differentiateParam(IRBuilder* builder, IRParam* paramP)
    {
        if (IRType* typeD = differentiateType(builder, paramP->getFullType()))
        {
            IRParam* paramD = builder->emitParam(typeD);
            SLANG_ASSERT(paramD);
            return paramD;
        }
        return nullptr;
    }

    List<IRParam*> transcribeParams(IRBuilder* builder, IRInstList<IRParam> paramListP)
    {
        // Clone (and emit) all the primal parameters.
        List<IRParam*> newParamListP;
        for (auto paramP : paramListP)
        {
            newParamListP.add(as<IRParam>(cloneInst(&cloneEnv, builder, paramP)));
        }

        // Now emit differentials.
        List<IRParam*> newParamListD;
        for (auto paramP : newParamListP)
        {
            IRParam* paramD = as<IRParam>(differentiateParam(builder, paramP));
            mapDifferentialInst(paramP, paramD);
            newParamListD.add(paramD);
        }

        return newParamListD;
    }

    IRInst* differentiateVar(IRBuilder* builder, IRVar* varP)
    {
        if (IRType* typeD = differentiateType(builder, varP->getDataType()->getValueType()))
        {
            IRVar* varD = builder->emitVar(typeD);
            SLANG_ASSERT(varD);
            return varD;
        }
        return nullptr;
    }

    IRInst* differentiateBinaryArith(IRBuilder* builder, IRInst* arith)
    {
        SLANG_ASSERT(arith->getOperandCount() == 2);
        
        auto leftP = arith->getOperand(0);
        auto rightP = arith->getOperand(1);

        auto leftD = getDifferentialInst(leftP);
        auto rightD = getDifferentialInst(rightP);

        auto leftZero = builder->getFloatValue(leftP->getDataType(), 0.0);
        auto rightZero = builder->getFloatValue(rightP->getDataType(), 0.0);

        if (leftD || rightD)
        {
            leftD = leftD ? leftD : leftZero;
            rightD = rightD ? rightD : rightZero;

            // Might have to do special-case handling for non-scalar types,
            // like float3 or float3x3
            // 
            auto resultType = arith->getDataType();
            switch(arith->getOp())
            {
            case kIROp_Add:
                return builder->emitAdd(resultType, leftD, rightD);
            case kIROp_Mul:
                return builder->emitAdd(resultType,
                    builder->emitMul(resultType, leftD, rightP),
                    builder->emitMul(resultType, leftP, rightD));
            case kIROp_Sub:
                return builder->emitSub(resultType, leftD, rightD);
            case kIROp_Div:
                return builder->emitDiv(resultType, 
                    builder->emitSub(
                        resultType,
                        builder->emitMul(resultType, leftD, rightP),
                        builder->emitMul(resultType, leftP, rightD)),
                    builder->emitMul(
                        rightP->getDataType(), rightP, rightP
                    ));
            default:
                SLANG_UNEXPECTED("Attempting to differentiate unsupported arithmetic");
            }
        }

        return nullptr;
    }

    IRInst* differentiateLoad(IRBuilder* builder, IRLoad* loadP)
    {
        if (auto varP = as<IRVar>(loadP->getPtr()))
        {   
            // If the loaded parameter has a differential version, 
            // emit a load instruction for the differential parameter.
            // Otherwise, emit nothing since there's nothing to load.
            // 
            if (auto varD = as<IRVar>(getDifferentialInst(varP)))
            {
                IRLoad* loadD = as<IRLoad>(builder->emitLoad(varD));
                SLANG_ASSERT(loadD);
                return loadD;
            }
            return nullptr;
        }

        SLANG_UNEXPECTED("Attempting to differentiate an unsupported load instruction");
    }

    IRInst* differentiateStore(IRBuilder* builder, IRStore* storeP)
    {
        IRInst* storeLocation = storeP->getPtr();
        IRInst* storeVal = storeP->getVal();
        if (auto destParam = as<IRVar>(storeLocation))
        {   
            // If the stored value has a differential version, 
            // emit a store instruction for the differential parameter.
            // Otherwise, emit nothing since there's nothing to load.
            // 
            IRInst* storeValD = getDifferentialInst(storeVal);
            IRVar*  storeLocationD = as<IRVar>(getDifferentialInst(destParam));
            if (storeValD && storeLocationD)
            {
                IRStore* storeD = as<IRStore>(
                    builder->emitStore(storeLocationD, storeValD));
                SLANG_ASSERT(storeD);
                return storeD;
            }
            return nullptr;
        }
        
        SLANG_UNEXPECTED("Attempting to differentiate an unsupported store instruction");
    }

    IRInst* differentiateReturn(IRBuilder* builder, IRReturn* returnP)
    {
        IRInst* returnVal = findCloneForOperand(&cloneEnv, returnP->getVal());
        if (auto returnValD = getDifferentialInst(returnVal))
        {   
            IRReturn* returnD = as<IRReturn>(builder->emitReturn(returnValD));
            SLANG_ASSERT(returnD);
            return returnD;
        }
        return nullptr;
    }

    // Since int/float literals are sometimes nested inside an IRConstructor
    // instruction, we check to make sure that the nested instr is a constant
    // and then return nullptr. Literals do not need to be differentiated.
    //
    IRInst* differentiateConstruct(IRBuilder*, IRInst* consP)
    {   
    
        if (as<IRConstant>(consP->getOperand(0)) && consP->getOperandCount() == 1)
        {
            return nullptr;
        }
        SLANG_UNEXPECTED("Attempting to differentiate unsupported constructor");
    }

    // Logic for whether a primal instruction needs to be replicated
    // in the differential function. For puerly functional blocks with
    // no side-effects, it's safe to replicate everything except the
    // return instruction.
    //
    bool requiresPrimalClone(IRBuilder*, IRInst* instP)
    {
        if (as<IRReturn>(instP))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    IRInst* transcribe(IRBuilder* builder, IRInst* oldInstP)
    {
        IRInst* instP = oldInstP;

        // Clone the old instruction, but only if it's safe to do so.
        // For instance, instructions that handle control flow 
        // (return statements) shouldn't be replicated.
        //
        if (requiresPrimalClone(builder, oldInstP))
            instP = cloneInst(&cloneEnv, builder, oldInstP);
        SLANG_ASSERT(instP);

        IRInst* instD = differentiateInst(builder, instP);

        mapDifferentialInst(instP, instD);

        return instD;
    }

    IRInst* differentiateInst(IRBuilder* builder, IRInst* instP)
    {
        switch (instP->getOp())
        {
        case kIROp_Var:
            return differentiateVar(builder, as<IRVar>(instP));

        case kIROp_Load:
            return differentiateLoad(builder, as<IRLoad>(instP));

        case kIROp_Store:
            return differentiateStore(builder, as<IRStore>(instP));

        case kIROp_Return:
            return differentiateReturn(builder, as<IRReturn>(instP));

        case kIROp_Add:
        case kIROp_Mul:
        case kIROp_Sub:
            return differentiateBinaryArith(builder, instP);

        case kIROp_Construct:
            return differentiateConstruct(builder, instP);

        default:
            SLANG_UNEXPECTED("Attempting to differentiate unrecognized instruction");
        }
    }
};

struct JVPDerivativeContext
{
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

    bool processModule()
    {
        // We start by initializing our shared IR building state,
        // since we will re-use that state for any code we
        // generate along the way.
        //
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->init(module);

        // Run through all the global-level instructions, 
        // looking for callables.
        // Note: We're only processing global callables (IRGlobalValueWithCode)
        // for now.
        // 
        IRBuilder builderStorage(sharedBuilderStorage);
        IRBuilder* builder = &builderStorage;
        for (auto inst : module->getGlobalInsts())
        {
            // If the instr is a callable, get all the basic blocks
            if (auto callable = as<IRGlobalValueWithCode>(inst))
            {
                if (isFunctionMarkedForJVP(callable))
                {   
                    SLANG_ASSERT(as<IRFunc>(callable));
                    IRFunc* jvpFunction = emitJVPFunction(&builderStorage, as<IRFunc>(callable));
                    builder->addJVPDerivativeReferenceDecoration(callable, jvpFunction);

                    unmarkForJVP(callable);
                }
            }
        }
        return true;
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

        // Start with _extremely_ basic functions
        SLANG_ASSERT(primalFn->getFirstBlock() == primalFn->getLastBlock());
        
        for (auto block = primalFn->getFirstBlock(); block; block = block->getNextBlock())
        {
            emitJVPBlock(builder, primalFn->getFirstBlock());
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

};

// Set up context and call main process method.
//
bool processJVPDerivativeMarkers(
        IRModule*                           module,
        IRJVPDerivativePassOptions const&)
{
    JVPDerivativeContext context;
    context.module = module;

    return context.processModule();
}

}
