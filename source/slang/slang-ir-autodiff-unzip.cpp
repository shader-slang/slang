#include "slang-ir-autodiff-unzip.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"

namespace Slang
{
struct ExtractPrimalFuncContext
{
    SharedIRBuilder* sharedBuilder;

    void init(SharedIRBuilder* inSharedBuilder)
    {
        sharedBuilder = inSharedBuilder;
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
        IRGlobalValueWithCode* destFunc, IRGlobalValueWithCode* fwdFunc, IRInst*& outIntermediateType)
    {
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(destFunc);
        IRFuncType* originalFuncType = nullptr;
        outIntermediateType = createIntermediateType(destFunc);

        originalFuncType = as<IRFuncType>(fwdFunc->getDataType());

        SLANG_RELEASE_ASSERT(originalFuncType);
        List<IRType*> paramTypes;
        for (UInt i = 0; i < originalFuncType->getParamCount() - 1; i++)
            paramTypes.add(originalFuncType->getParamType(i));
        paramTypes.add(builder.getInOutType((IRType*)outIntermediateType));
        auto newFuncType = builder.getFuncType(paramTypes, builder.getVoidType());
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
        case kIROp_MakeDifferentialPair:
        case kIROp_MakeOptionalNone:
        case kIROp_MakeOptionalValue:
        case kIROp_DifferentialPairGetDifferential:
        case kIROp_DifferentialPairGetPrimal:
            return false;
        case kIROp_GetElement:
        case kIROp_FieldExtract:
        case kIROp_swizzle:
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

    void storeInst(
        IRBuilder& builder,
        IRInst* inst,
        IRInst* intermediateOutput)
    {
        IRBuilder genTypeBuilder(sharedBuilder);
        auto ptrStructType = as<IRPtrTypeBase>(intermediateOutput->getDataType() );
        SLANG_RELEASE_ASSERT(ptrStructType);
        auto structType = as<IRStructType>(ptrStructType->getValueType());
        genTypeBuilder.setInsertBefore(structType);
        auto fieldType = inst->getDataType();
        SLANG_RELEASE_ASSERT(structType);
        auto structKey = genTypeBuilder.createStructKey();
        if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
            cloneDecoration(nameHint, structKey);
        genTypeBuilder.setInsertInto(structType);
        genTypeBuilder.createStructField(structType, structKey, (IRType*)fieldType);
        builder.addPrimalValueStructKeyDecoration(inst, structKey);
        builder.emitStore(
            builder.emitFieldAddress(
                builder.getPtrType(inst->getFullType()), intermediateOutput, structKey),
            inst);
    }

    IRFunc* turnUnzippedFuncIntoPrimalFunc(IRFunc* unzippedFunc, IRFunc* fwdFunc, IRInst*& outIntermediateType)
    {
        // Note: this transformation assumes the original func has only one return.

        IRBuilder builder(sharedBuilder);

        IRFunc* func = unzippedFunc;
        IRInst* intermediateType = nullptr;
        auto newFuncType = generatePrimalFuncType(unzippedFunc, fwdFunc, intermediateType);
        outIntermediateType = intermediateType;
        func->setFullType((IRType*)newFuncType);

        // Go through all the insts and preserve the primal blocks.
        // Create a return block to replace all branches into a non-primal block.
        builder.setInsertInto(func);
        auto returnBlock = builder.emitBlock();
        for (auto block : func->getBlocks())
        {
            auto term = block->getTerminator();
            if (auto ret = as<IRReturn>(term))
            {
                insertIntoReturnBlock(builder, ret);
                break;
            }
        }

        auto paramBlock = func->getFirstBlock();
        builder.setInsertInto(paramBlock);
        auto oldIntermediateParam = func->getLastParam();
        auto outIntermediary =
            builder.emitParam(builder.getInOutType((IRType*)intermediateType));
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
                    builder.setInsertAfter(inst);
                    storeInst(builder, inst, outIntermediary);
                }
            }
        }

        // Go over differential blocks and complete
        for (auto block : diffBlocksList)
        {
            
            if (block->getFirstParam() == nullptr)
            {
                // If the block does not have any PHI nodes, just remove it and
                // replace all its uses with returnBlock.

                // TODO: This invalides the next block in the chain. Make a list first.
                block->replaceUsesWith(returnBlock);
                block->removeAndDeallocate();
            }
            else
            {
                // If the block has Phi nodes, we can't directly replace it with
                // `returnBlock`, but we can turn the block into a trivial branch
                // into `returnBlock` to safely preserve the invariants of Phi nodes.
                auto inst = block->getLastParam()->getNextInst();
                for (; inst;)
                {
                    auto nextInst = inst->getNextInst();
                    inst->removeAndDeallocate();
                    inst = nextInst;
                }

                builder.setInsertInto(block);
                builder.emitBranch(returnBlock);
            }
        }

        List<IRBlock*> unusedBlocks;
        for (auto block : func->getBlocks())
        {
            if (!block->hasUses() && isDiffInst(block))
                unusedBlocks.add(block);
        }

        for (auto block : unusedBlocks)
            block->removeAndDeallocate();

        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
        auto defVal = builder.emitDefaultConstructRaw((IRType*)intermediateType);
        builder.emitStore(outIntermediary, defVal);
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
    IRFunc* func, IRFunc* fwdFunc, IRInst*& intermediateType)
{
    IRBuilder builder(this->autodiffContext->sharedBuilder);
    builder.setInsertBefore(func);

    IRCloneEnv subEnv;
    subEnv.squashChildrenMapping = true;
    subEnv.parent = &cloneEnv;
    auto clonedFunc = as<IRFunc>(cloneInst(&subEnv, &builder, func));

    ExtractPrimalFuncContext context;
    context.init(autodiffContext->sharedBuilder);

    intermediateType = nullptr;
    auto primalFunc = context.turnUnzippedFuncIntoPrimalFunc(clonedFunc, fwdFunc, intermediateType);

    if (auto nameHint = primalFunc->findDecoration<IRNameHintDecoration>())
    {
        auto primalName = String(nameHint->getName()) + "_primal";
        nameHint->setOperand(0, builder.getStringValue(primalName.getUnownedSlice()));
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
                auto addr = builder.emitFieldAddress(
                    builder.getPtrType(inst->getDataType()),
                    intermediateVar,
                    structKeyDecor->getStructKey());
                auto val = builder.emitLoad(addr);
                inst->replaceUsesWith(val);
                instsToRemove.add(inst);
            }
        }
    }

    for (auto inst : instsToRemove)
    {
        inst->removeAndDeallocate();
    }

    // Run simplification to DCE unnecessary insts.
    eliminateDeadCode(func);
    eliminateDeadCode(primalFunc);

    return primalFunc;
}
} // namespace Slang
