#include "slang-ir-autodiff-unzip.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"
#include "slang-ir-autodiff-rev.h"

namespace Slang
{

struct ExtractPrimalFuncContext
{
    SharedIRBuilder* sharedBuilder;
    AutoDiffTranscriberBase* backwardPrimalTranscriber;

    void init(SharedIRBuilder* inSharedBuilder, AutoDiffTranscriberBase* transcriber)
    {
        sharedBuilder = inSharedBuilder;
        backwardPrimalTranscriber = transcriber;
    }

    IRInst* cloneGenericHeader(IRBuilder& builder, IRCloneEnv& cloneEnv, IRGeneric* gen)
    {
        auto newGeneric = builder.emitGeneric();
        newGeneric->setFullType(builder.getTypeKind());
        for (auto decor : gen->getDecorations())
            cloneDecoration(decor, newGeneric);
        builder.emitBlock();
        auto originalBlock = gen->getFirstBlock();
        for (auto child = originalBlock->getFirstChild(); child != originalBlock->getLastParam();
             child = child->getNextInst())
        {
            cloneInst(&cloneEnv, &builder, child);
        }
        return newGeneric;
    }

    IRInst* createGenericIntermediateType(IRGeneric* gen)
    {
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(gen);
        IRCloneEnv intermediateTypeCloneEnv;
        auto clonedGen = cloneGenericHeader(builder, intermediateTypeCloneEnv, gen);
        auto structType = builder.createStructType();
        builder.emitReturn(structType);
        auto func = findGenericReturnVal(gen);
        if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
        {
            StringBuilder newName;
            newName << nameHint->getName() << "_Intermediates";
            builder.addNameHintDecoration(structType, UnownedStringSlice(newName.getBuffer()));
        }
        return clonedGen;
    }

    IRInst* createIntermediateType(IRGlobalValueWithCode* func)
    {
        if (func->getOp() == kIROp_Generic)
            return createGenericIntermediateType(as<IRGeneric>(func));
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(func);
        auto intermediateType = builder.createStructType();
        if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
        {
            StringBuilder newName;
            newName << nameHint->getName() << "_Intermediates";
            builder.addNameHintDecoration(
                intermediateType, UnownedStringSlice(newName.getBuffer()));
        }
        return intermediateType;
    }

    IRInst* generatePrimalFuncType(
        IRGlobalValueWithCode* destFunc, IRGlobalValueWithCode* originalFunc, IRInst*& outIntermediateType)
    {
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(destFunc);
        IRFuncType* originalFuncType = nullptr;
        outIntermediateType = createIntermediateType(destFunc);

        GenericChildrenMigrationContext migrationContext;
        migrationContext.init(as<IRGeneric>(findOuterGeneric(originalFunc)), as<IRGeneric>(findOuterGeneric(destFunc)), destFunc);

        originalFuncType = as<IRFuncType>(originalFunc->getDataType());

        SLANG_RELEASE_ASSERT(originalFuncType);
        List<IRType*> paramTypes;
        for (UInt i = 0; i < originalFuncType->getParamCount() - 1; i++)
            paramTypes.add((IRType*)migrationContext.cloneInst(&builder, originalFuncType->getParamType(i)));
        paramTypes.add(builder.getInOutType((IRType*)outIntermediateType));
        auto resultType = (IRType*)migrationContext.cloneInst(&builder, originalFuncType->getResultType());
        auto newFuncType = builder.getFuncType(paramTypes, resultType);
        return newFuncType;
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

    IRInst* insertIntoReturnBlock(IRBuilder& builder, IRInst* inst)
    {
        if (!isDiffInst(inst))
            return inst;

        switch (inst->getOp())
        {
        case kIROp_Return:
            {
                IRInst* val = builder.getVoidValue();
                if (inst->getOperandCount() != 0)
                {
                    val = insertIntoReturnBlock(builder, inst->getOperand(0));
                }
                return builder.emitReturn(val);
            }
        case kIROp_MakeDifferentialPair:
            {
                auto diff = builder.emitDefaultConstruct(inst->getOperand(1)->getDataType());
                auto primal = insertIntoReturnBlock(builder, inst->getOperand(0));
                return builder.emitMakeDifferentialPair(inst->getDataType(), primal, diff);
            }
        default:
            SLANG_UNREACHABLE("unknown case of mixed inst.");
        }
    }

    bool shouldStoreInst(IRInst* inst)
    {
        if (!inst->getDataType())
        {
            return false;
        }

        // Only store allowed types.
        if (isScalarIntegerType(inst->getDataType()))
        {
        }
        else if (as<IRResourceTypeBase>(inst->getDataType()))
        {
        }
        else
        {
            switch (inst->getDataType()->getOp())
            {
            case kIROp_StructType:
            case kIROp_OptionalType:
            case kIROp_TupleType:
            case kIROp_ArrayType:
            case kIROp_DifferentialPairType:
            case kIROp_InterfaceType:
            case kIROp_AnyValueType:
            case kIROp_ClassType:
            case kIROp_FloatType:
            case kIROp_HalfType:
            case kIROp_DoubleType:
            case kIROp_VectorType:
            case kIROp_MatrixType:
            case kIROp_BoolType:
            case kIROp_Param:
            case kIROp_Specialize:
            case kIROp_LookupWitness:
                break;
            default:
                return false;
            }
        }

        // Never store certain opcodes.
        switch (inst->getOp())
        {
        case kIROp_CastFloatToInt:
        case kIROp_CastIntToFloat:
        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_MakeVectorFromScalar:
        case kIROp_MakeMatrixFromScalar:
        case kIROp_Reinterpret:
        case kIROp_BitCast:
        case kIROp_DefaultConstruct:
        case kIROp_MakeStruct:
        case kIROp_MakeTuple:
        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
        case kIROp_MakeDifferentialPair:
        case kIROp_MakeOptionalNone:
        case kIROp_MakeOptionalValue:
        case kIROp_DifferentialPairGetDifferential:
        case kIROp_DifferentialPairGetPrimal:
            return false;
        case kIROp_GetElement:
        case kIROp_FieldExtract:
        case kIROp_swizzle:
        case kIROp_UpdateElement:
        case kIROp_OptionalHasValue:
        case kIROp_GetOptionalValue:
        case kIROp_MatrixReshape:
        case kIROp_VectorReshape:
            // If the operand is already stored, don't store the result of these insts.
            if (inst->getOperand(0)->findDecoration<IRPrimalValueStructKeyDecoration>())
            {
                return false;
            }
            break;
        default:
            break;
        }

        // Only store if the inst has differential inst user.
        bool hasDiffUser = false;
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (isDiffInst(user))
            {
                // Ignore uses that is a return or MakeDiffPair
                switch (user->getOp())
                {
                case kIROp_Return:
                    continue;
                case kIROp_MakeDifferentialPair:
                    if (!user->hasMoreThanOneUse() && user->firstUse &&
                        user->firstUse->getUser()->getOp() == kIROp_Return)
                        continue;
                    break;
                default:
                    break;
                }
                hasDiffUser = true;
                break;
            }
        }
        if (!hasDiffUser)
            return false;

        return true;
    }

    IRStructField* addIntermediateContextField(IRInst* type, IRInst* intermediateOutput)
    {
        IRBuilder genTypeBuilder(sharedBuilder);
        auto ptrStructType = as<IRPtrTypeBase>(intermediateOutput->getDataType());
        SLANG_RELEASE_ASSERT(ptrStructType);
        auto structType = as<IRStructType>(ptrStructType->getValueType());
        if (auto outerGen = findOuterGeneric(structType))
            genTypeBuilder.setInsertBefore(outerGen);
        else
            genTypeBuilder.setInsertBefore(structType);
        auto fieldType = type;
        SLANG_RELEASE_ASSERT(structType);
        auto structKey = genTypeBuilder.createStructKey();
        genTypeBuilder.setInsertInto(structType);

        if (fieldType->getParent() != structType->getParent() &&
            isChildInstOf(fieldType->getParent(), structType->getParent()))
        {
            IRCloneEnv cloneEnv;
            fieldType = cloneInst(&cloneEnv, &genTypeBuilder, fieldType);
        }
        return genTypeBuilder.createStructField(structType, structKey, (IRType*)fieldType);
    }

    void storeInst(
        IRBuilder& builder,
        IRInst* inst,
        IRInst* intermediateOutput)
    {
        auto field = addIntermediateContextField(inst->getDataType(), intermediateOutput);
        auto key = field->getKey();
        if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
            cloneDecoration(nameHint, key);
        builder.addPrimalValueStructKeyDecoration(inst, key);
        builder.emitStore(
            builder.emitFieldAddress(
                builder.getPtrType(inst->getFullType()), intermediateOutput, key),
            inst);
    }

    IRFunc* turnUnzippedFuncIntoPrimalFunc(IRFunc* unzippedFunc, IRFunc* originalFunc, HashSet<IRInst*>& primalParams, IRInst*& outIntermediateType)
    {
        IRBuilder builder(sharedBuilder);

        IRFunc* func = unzippedFunc;
        IRInst* intermediateType = nullptr;
        auto newFuncType = generatePrimalFuncType(unzippedFunc, originalFunc, intermediateType);
        outIntermediateType = intermediateType;
        func->setFullType((IRType*)newFuncType);

        auto paramBlock = func->getFirstBlock();
        builder.setInsertInto(paramBlock);
        auto oldIntermediateParam = func->getLastParam();
        auto outIntermediary =
            builder.emitParam(builder.getInOutType((IRType*)intermediateType));
        oldIntermediateParam->transferDecorationsTo(outIntermediary);
        primalParams.Add(outIntermediary);
        oldIntermediateParam->replaceUsesWith(outIntermediary);
        oldIntermediateParam->removeAndDeallocate();

        auto firstBlock = *(paramBlock->getSuccessors().begin());

        List<IRBlock*> diffBlocksList;
        List<IRBlock*> primalBlocksList;

        for (auto block : func->getBlocks())
        {
            if (block == paramBlock)
                continue;

            if (isDiffInst(block))
                diffBlocksList.add(block);
            else
                primalBlocksList.add(block);
        }
        
        // Go over primal blocks and store insts.
        for (auto block : primalBlocksList)
        {
            // For primal insts, decide whether or not to store its result in
            // output intermediary struct.
            for (auto inst : block->getChildren())
            {
                if (shouldStoreInst(inst))
                {
                    if (as<IRParam>(inst))
                        builder.setInsertBefore(block->getFirstOrdinaryInst());
                    else
                        builder.setInsertAfter(inst);
                    storeInst(builder, inst, outIntermediary);
                }
                else if (inst->getOp() == kIROp_Var)
                {
                    // Always store intermediate context var.
                    if (inst->findDecoration<IRBackwardDerivativePrimalContextDecoration>())
                    {
                        auto field = addIntermediateContextField(cast<IRPtrTypeBase>(inst->getDataType())->getValueType(), outIntermediary);
                        builder.setInsertBefore(inst);
                        auto fieldAddr = builder.emitFieldAddress(
                            inst->getFullType(), outIntermediary, field->getKey());
                        inst->replaceUsesWith(fieldAddr);
                        builder.addPrimalValueStructKeyDecoration(inst, field->getKey());
                    }
                }
            }
        }

        for (auto block : primalBlocksList)
        {
            auto term = block->getTerminator();
            builder.setInsertBefore(term);
            if (auto decor = term->findDecoration<IRBackwardDerivativePrimalReturnDecoration>())
            {
                builder.emitReturn(decor->getBackwardDerivativePrimalReturnValue());
                term->removeAndDeallocate();
            }
        }
       
        List<IRBlock*> unusedBlocks;
        for (auto block : func->getBlocks())
        {
            if (isDiffInst(block))
                unusedBlocks.add(block);
        }
        for (auto block : unusedBlocks)
            block->removeAndDeallocate();

        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
        auto defVal = builder.emitDefaultConstructRaw((IRType*)intermediateType);
        builder.emitStore(outIntermediary, defVal);

        // Remove any parameters not in `primalParams` set.
        List<IRInst*> params;
        for (auto param = func->getFirstParam(); param;)
        {
            auto nextParam = param->getNextParam();
            if (!primalParams.Contains(param))
            {
                param->replaceUsesWith(builder.getVoidValue());
                param->removeAndDeallocate();
            }
            param = nextParam;
        }
        return unzippedFunc;
    }
};

static void copyPrimalValueStructKeyDecorations(IRInst* inst, IRCloneEnv& cloneEnv)
{
    IRInst* newInst = nullptr;
    if (cloneEnv.mapOldValToNew.TryGetValue(inst, newInst))
    {
        if (auto decor = newInst->findDecoration<IRPrimalValueStructKeyDecoration>())
        {
            cloneDecoration(decor, inst);
        }
    }

    for (auto child : inst->getChildren())
    {
        copyPrimalValueStructKeyDecorations(child, cloneEnv);
    }
}

IRFunc* DiffUnzipPass::extractPrimalFunc(
    IRFunc* func,
    IRFunc* originalFunc,
    ParameterBlockTransposeInfo& paramInfo,
    IRInst*& intermediateType)
{
    IRBuilder builder(this->autodiffContext->sharedBuilder);
    builder.setInsertBefore(func);

    IRCloneEnv subEnv;
    subEnv.squashChildrenMapping = true;
    subEnv.parent = &cloneEnv;
    auto clonedFunc = as<IRFunc>(cloneInst(&subEnv, &builder, func));

    // Remove [KeepAlive] decorations in clonedFunc.
    for (auto block : clonedFunc->getBlocks())
        for (auto inst : block->getChildren())
            if (auto decor = inst->findDecoration<IRKeepAliveDecoration>())
                decor->removeAndDeallocate();

    // Remove propagate func specific primal insts from cloned func.
    for (auto inst : paramInfo.propagateFuncSpecificPrimalInsts)
    {
        auto newInst = subEnv.mapOldValToNew[inst].GetValue();
        newInst->removeAndDeallocate();
    }

    HashSet<IRInst*> newPrimalParams;
    for (auto param : func->getParams())
    {
        if (paramInfo.primalFuncParams.Contains(param))
            newPrimalParams.Add(subEnv.mapOldValToNew[param].GetValue());
    }

    ExtractPrimalFuncContext context;
    context.init(autodiffContext->sharedBuilder, autodiffContext->transcriberSet.primalTranscriber);

    intermediateType = nullptr;
    auto primalFunc = context.turnUnzippedFuncIntoPrimalFunc(clonedFunc, originalFunc, newPrimalParams, intermediateType);

    if (auto nameHint = primalFunc->findDecoration<IRNameHintDecoration>())
    {
        nameHint->removeAndDeallocate();
    }
    if (auto originalNameHint = originalFunc->findDecoration<IRNameHintDecoration>())
    {
        auto primalName = String("s_bwd_primal_") + UnownedStringSlice(originalNameHint->getName());
        builder.addNameHintDecoration(primalFunc, builder.getStringValue(primalName.getUnownedSlice()));
    }

    // Copy PrimalValueStructKey decorations from primal func.
    copyPrimalValueStructKeyDecorations(func, subEnv);
    
    auto paramBlock = func->getFirstBlock();
    auto firstBlock = *(paramBlock->getSuccessors().begin());
    builder.setInsertBefore(firstBlock->getFirstInst());
    auto intermediateVar = func->getLastParam();

    // Replace all insts that has intermediate results with a load of the intermediate.
    List<IRInst*> instsToRemove;
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getOrdinaryInsts())
        {
            if (auto structKeyDecor = inst->findDecoration<IRPrimalValueStructKeyDecoration>())
            {
                builder.setInsertBefore(inst);
                if (inst->getOp() == kIROp_Var)
                {
                    // This is a var for intermediate context.
                    // Replace all loads of the var with a field extract.
                    // Other type of uses will get a temp var that stores a copy of the field.
                    while (auto use = inst->firstUse)
                    {
                        if (as<IRDecoration>(use->getUser()))
                        {
                            use->set(builder.getVoidValue());
                            continue;
                        }
                        builder.setInsertBefore(use->getUser());
                        auto valType = cast<IRPtrTypeBase>(inst->getFullType())->getValueType();
                        auto val = builder.emitFieldExtract(
                            valType,
                            intermediateVar,
                            structKeyDecor->getStructKey());
                        if (use->getUser()->getOp() == kIROp_Load)
                        {
                            use->getUser()->replaceUsesWith(val);
                            use->getUser()->removeAndDeallocate();
                        }
                        else
                        {
                            auto tempVar =
                                builder.emitVar(valType);
                            builder.emitStore(tempVar, val);
                            use->set(tempVar);
                        }
                    }
                }
                else
                {
                    // Orindary value.
                    auto val = builder.emitFieldExtract(
                        inst->getFullType(),
                        intermediateVar,
                        structKeyDecor->getStructKey());
                    inst->replaceUsesWith(val);
                }
                instsToRemove.add(inst);
            }
            else if (auto primalCtx = inst->findDecoration<IRBackwardDerivativePrimalContextDecoration>())
            {
                if (inst->getOp() == kIROp_Call)
                {
                    builder.addSimpleDecoration<IRNoSideEffectDecoration>(inst);
                }
            }
        }
    }

    for (auto inst : instsToRemove)
    {
        inst->removeAndDeallocate();
    }
    
    stripTempDecorations(func);

    return primalFunc;
}
} // namespace Slang
