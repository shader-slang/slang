#include "slang-ir-autodiff.h"

#include "../core/slang-performance-profiler.h"
#include "slang-ir-address-analysis.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-pairs.h"
#include "slang-ir-autodiff-rev.h"
#include "slang-ir-inline.h"
#include "slang-ir-single-return.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-validate.h"

namespace Slang
{

bool isBackwardDifferentiableFunc(IRInst* func)
{
    for (auto decorations : func->getDecorations())
    {
        switch (decorations->getOp())
        {
        case kIROp_BackwardDifferentiableDecoration:
        case kIROp_UserDefinedBackwardDerivativeDecoration:
            return true;
        }
    }
    return false;
}

IRInst* _lookupWitness(
    IRBuilder* builder,
    IRInst* witness,
    IRInst* requirementKey,
    IRType* resultType)
{
    if (auto witnessTable = as<IRWitnessTable>(witness))
    {
        for (auto entry : witnessTable->getEntries())
        {
            if (entry->getRequirementKey() == requirementKey)
                return entry->getSatisfyingVal();
        }

        if (witnessTable->getConformanceType()->getOp() == kIROp_VoidType)
        {
            return builder->getVoidValue();
        }

        SLANG_UNEXPECTED("requirement not found in concrete witness table");
    }
    else if (auto interfaceType = as<IRInterfaceType>(witness))
    {
        for (UIndex ii = 0; ii < interfaceType->getOperandCount(); ii++)
        {
            auto entry = cast<IRInterfaceRequirementEntry>(interfaceType->getOperand(ii));
            if (entry->getRequirementKey() == requirementKey)
                return entry->getRequirementVal();
        }
        SLANG_UNEXPECTED("requirement entry not found in concrete interface type");
    }
    else
    {
        // Existential witness table. Emit a lookup instruction.
        SLANG_ASSERT(resultType);
        return builder->emitLookupInterfaceMethodInst(resultType, witness, requirementKey);
    }
    return nullptr;
}

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

static IRInst* _getDiffTypeWitnessFromPairType(
    AutoDiffSharedContext* sharedContext,
    IRBuilder* builder,
    IRDifferentialPairTypeBase* type)
{
    auto witnessTable = type->getWitness();

    if (as<IRDifferentialPairType>(type))
        return _lookupWitness(
            builder,
            witnessTable,
            sharedContext->differentialAssocTypeWitnessStructKey,
            sharedContext->differentialAssocTypeWitnessTableType);
    else if (as<IRDifferentialPtrPairType>(type))
        return _lookupWitness(
            builder,
            witnessTable,
            sharedContext->differentialAssocRefTypeWitnessStructKey,
            sharedContext->differentialAssocRefTypeWitnessTableType);
    else
        SLANG_UNEXPECTED("Unexpected differential pair type");
}

bool isNoDiffType(IRType* paramType)
{
    while (paramType)
    {
        if (auto attrType = as<IRAttributedType>(paramType))
        {
            if (attrType->findAttr<IRNoDiffAttr>())
                return true;

            paramType = attrType->getBaseType();
        }
        else if (auto ptrType = asRelevantPtrType(paramType))
        {
            paramType = ptrType->getValueType();
        }
        else
        {
            return false;
        }
    }
    return false;
}

// Return true if the result type and all the parameter types are no_diff
bool isNeverDiffFuncType(IRFuncType* const funcType)
{
    const auto resultType = funcType->getResultType();
    if (!isNoDiffType(resultType))
        return false;
    for (const auto p : funcType->getParamTypes())
    {
        if (!isNoDiffType(p))
            return false;
    }
    return true;
}

bool isExistentialOrRuntimeInst(IRInst* inst)
{
    if (auto lookup = as<IRLookupWitnessMethod>(inst))
    {
        return isExistentialOrRuntimeInst(lookup->getWitnessTable());
    }

    return as<IRExtractExistentialType>(inst) || as<IRExtractExistentialWitnessTable>(inst) ||
           as<IRMakeExistential>(inst) || as<IRInterfaceType>(inst->getDataType());
}

bool isRuntimeType(IRType* type)
{
    if (as<IRExtractExistentialType>(type))
        return true;

    if (auto lookup = as<IRLookupWitnessMethod>(type))
    {
        return isExistentialOrRuntimeInst(lookup->getWitnessTable());
    }

    return false;
}

IRInst* getExistentialBaseWitnessTable(IRBuilder* builder, IRType* type)
{
    if (auto lookupWitnessMethod = as<IRLookupWitnessMethod>(type))
    {
        return lookupWitnessMethod->getWitnessTable();
    }
    else if (auto extractExistentialType = as<IRExtractExistentialType>(type))
    {
        return builder->emitExtractExistentialWitnessTable(extractExistentialType->getOperand(0));
    }
    else
    {
        SLANG_UNEXPECTED("Unexpected existential type");
    }
}

IRInst* getCacheKey(IRBuilder* builder, IRInst* primalType)
{
    if (auto lookupWitness = as<IRLookupWitnessMethod>(primalType))
        return lookupWitness->getRequirementKey();
    else if (auto extractExistentialType = as<IRExtractExistentialType>(primalType))
    {
        auto interfaceType = extractExistentialType->getOperand(0)->getDataType();

        // We will cache on the interface's this-type, since the interface type itself can be
        // deallocated during the lowering process.
        //
        return builder->getThisType(interfaceType);
    }

    return primalType;
}


IRInterfaceType* findDifferentiableRefInterface(IRModuleInst* moduleInst)
{
    for (auto inst : moduleInst->getGlobalInsts())
    {
        if (auto interfaceType = as<IRInterfaceType>(inst))
        {
            if (auto decor = interfaceType->findDecoration<IRNameHintDecoration>())
            {
                if (decor->getName() == "IDifferentiablePtrType")
                {
                    return interfaceType;
                }
            }
        }
    }
    return nullptr;
}

AutoDiffSharedContext::AutoDiffSharedContext(IRModuleInst* inModuleInst)
    : moduleInst(inModuleInst)
{
    differentiableInterfaceType = as<IRInterfaceType>(findDifferentiableInterface());
    if (differentiableInterfaceType)
    {
        differentialAssocTypeStructKey = findDifferentialTypeStructKey();
        differentialAssocTypeWitnessStructKey = findDifferentialTypeWitnessStructKey();
        differentialAssocTypeWitnessTableType = findDifferentialTypeWitnessTableType();

        zeroMethodStructKey = findZeroMethodStructKey();
        zeroMethodType = cast<IRFuncType>(
            getInterfaceEntryAtIndex(differentiableInterfaceType, 2)->getRequirementVal());
        addMethodStructKey = findAddMethodStructKey();
        // 3 and 4 would be dzero : IForwardDifferentiable and dzero : IBackwardDifferentiable
        addMethodType = cast<IRFuncType>(
            getInterfaceEntryAtIndex(differentiableInterfaceType, 5)->getRequirementVal());

        nullDifferentialStructType = findNullDifferentialStructType();
        nullDifferentialWitness = findNullDifferentialWitness();

        forwardDifferentiableInterfaceType = cast<IRGeneric>(findForwardDifferentiableInterface());
        backwardDifferentiableInterfaceType =
            cast<IRGeneric>(findBackwardDifferentiableInterface());
        backwardCallableInterfaceType = cast<IRGeneric>(findBackwardCallableInterface());

        IRBuilder builder(moduleInst);
        builder.addKeepAliveDecoration(forwardDifferentiableInterfaceType);
        builder.addKeepAliveDecoration(backwardDifferentiableInterfaceType);
        builder.addKeepAliveDecoration(backwardCallableInterfaceType);

        isInterfaceAvailable = true;
    }

    differentiablePtrInterfaceType =
        as<IRInterfaceType>(findDifferentiableRefInterface(inModuleInst));

    if (differentiablePtrInterfaceType)
    {
        differentialAssocRefTypeStructKey = findDifferentialPtrTypeStructKey();
        differentialAssocRefTypeWitnessStructKey = findDifferentialPtrTypeWitnessStructKey();
        differentialAssocRefTypeWitnessTableType = findDifferentialPtrTypeWitnessTableType();

        isPtrInterfaceAvailable = true;
    }
}

IRInst* AutoDiffSharedContext::findDifferentiableInterface()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto intf = as<IRInterfaceType>(globalInst))
            {
                if (auto decor = intf->findDecoration<IRKnownBuiltinDecoration>())
                {
                    if (decor->getName() == KnownBuiltinDeclName::IDifferentiable)
                    {
                        return globalInst;
                    }
                }
            }
        }
    }
    return nullptr;
}


IRInst* AutoDiffSharedContext::findForwardDifferentiableInterface()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto generic = as<IRGeneric>(globalInst))
            {
                auto inner = getGenericReturnVal(generic);
                if (auto decor = inner->findDecoration<IRKnownBuiltinDecoration>())
                {
                    if (decor->getName() == KnownBuiltinDeclName::IForwardDifferentiable)
                    {
                        return globalInst;
                    }
                }
            }
        }
    }
    return nullptr;
}


IRInst* AutoDiffSharedContext::findBackwardDifferentiableInterface()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto generic = as<IRGeneric>(globalInst))
            {
                auto inner = getGenericReturnVal(generic);
                if (auto decor = inner->findDecoration<IRKnownBuiltinDecoration>())
                {
                    if (decor->getName() == KnownBuiltinDeclName::IBackwardDifferentiable)
                    {
                        return globalInst;
                    }
                }
            }
        }
    }
    return nullptr;
}


IRInst* AutoDiffSharedContext::findBackwardCallableInterface()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto generic = as<IRGeneric>(globalInst))
            {
                auto inner = getGenericReturnVal(generic);
                if (auto decor = inner->findDecoration<IRKnownBuiltinDecoration>())
                {
                    if (decor->getName() == KnownBuiltinDeclName::IBwdCallable)
                    {
                        return globalInst;
                    }
                }
            }
        }
    }
    return nullptr;
}

IRStructType* AutoDiffSharedContext::findNullDifferentialStructType()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            // TODO: Also a particularly dangerous way to look for a struct...
            if (auto structType = as<IRStructType>(globalInst))
            {
                if (auto decor = structType->findDecoration<IRNameHintDecoration>())
                {
                    if (decor->getName() == toSlice("NullDifferential"))
                    {
                        return structType;
                    }
                }
            }
        }
    }
    return nullptr;
}

IRInst* AutoDiffSharedContext::findNullDifferentialWitness()
{
    if (auto module = as<IRModuleInst>(moduleInst))
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto witnessTable = as<IRWitnessTable>(globalInst))
            {
                if (witnessTable->getConformanceType() == differentiableInterfaceType &&
                    witnessTable->getConcreteType() == nullDifferentialStructType)
                    return witnessTable;
            }
        }
    }

    return nullptr;
}


IRInterfaceRequirementEntry* AutoDiffSharedContext::getInterfaceEntryAtIndex(
    IRInterfaceType* interface,
    UInt index)
{
    if (as<IRModuleInst>(moduleInst) && interface)
    {
        // Assume for now that IDifferentiable has exactly five fields.
        // SLANG_ASSERT(interface->getOperandCount() == 5);
        if (auto entry = as<IRInterfaceRequirementEntry>(interface->getOperand(index)))
            return entry;
        else
        {
            SLANG_UNEXPECTED("IDifferentiable interface entry unexpected type");
        }
    }

    return nullptr;
}

IRInst* DifferentiableTypeConformanceContext::tryGetAssociationOfKind(
    IRInst* target,
    ValAssociationKind kind)
{
    IRBuilder builder(sharedContext->moduleInst);
    return builder.tryLookupAnnotation(target, kind);

    /*
    if (!target)
        return nullptr;

    ValAssociationCacheKey key = {target, kind};
    if (IRInst* cachedResult; associationCache.tryGetValue(key, cachedResult))
        return cachedResult;

    IRInst* result = nullptr;
    for (auto use = target->firstUse; use; use = use->nextUse)
    {
        if (auto annotation = as<IRAssociatedInstAnnotation>(use->getUser()))
        {
            if (annotation->getConformanceID() == (IRIntegerValue)kind &&
                annotation->getTarget() == target)
            {
                result = annotation->getInst();
                break;
            }
        }
    }

    associationCache.add(key, result);
    return result;
    */
}


IRInst* DifferentiableTypeConformanceContext::getDiffTypeFromPairType(
    IRBuilder* builder,
    IRDifferentialPairTypeBase* type)
{
    return _getDiffTypeFromPairType(sharedContext, builder, type);
}

IRInst* DifferentiableTypeConformanceContext::getDiffTypeWitnessFromPairType(
    IRBuilder* builder,
    IRDifferentialPairTypeBase* type)
{
    return _getDiffTypeWitnessFromPairType(sharedContext, builder, type);
}


IRFunc* DifferentiableTypeConformanceContext::getOrCreateExistentialDAddMethod()
{
    if (this->existentialDAddFunc)
        return this->existentialDAddFunc;

    SLANG_ASSERT(sharedContext->differentiableInterfaceType);
    SLANG_ASSERT(sharedContext->nullDifferentialWitness);

    auto builder = IRBuilder(this->sharedContext->moduleInst);

    existentialDAddFunc = builder.createFunc();
    existentialDAddFunc->setFullType(builder.getFuncType(
        List<IRType*>({
            sharedContext->differentiableInterfaceType,
            sharedContext->differentiableInterfaceType,
        }),
        sharedContext->differentiableInterfaceType));

    builder.setInsertInto(existentialDAddFunc);
    auto entryBlock = builder.emitBlock();

    builder.setInsertInto(entryBlock);

    // Insert parameters.
    auto aObj = builder.emitParam(sharedContext->differentiableInterfaceType);
    auto bObj = builder.emitParam(sharedContext->differentiableInterfaceType);

    // Check if a.type == null_differential.type
    auto aObjWitnessIsNull = builder.emitIsDifferentialNull(aObj);

    // If aObjWitnessTable is null, return bObj.
    auto aObjWitnessIsNullBlock = builder.emitBlock();
    builder.setInsertInto(aObjWitnessIsNullBlock);
    builder.emitReturn(bObj);

    auto aObjWitnessIsNotNullBlock = builder.emitBlock();
    builder.setInsertInto(aObjWitnessIsNotNullBlock);

    // Check if b.type == null_differential.type
    auto bObjWitnessIsNull = builder.emitIsDifferentialNull(bObj);

    // If bObjWitnessTable is null, return aObj.
    auto bObjWitnessIsNullBlock = builder.emitBlock();
    builder.setInsertInto(bObjWitnessIsNullBlock);
    builder.emitReturn(aObj);

    auto bObjWitnessIsNotNullBlock = builder.emitBlock();

    // Emit aObj.type::dadd(aObj.val, bObj.val)
    //
    // Important: we're looking up dadd on the differential type, and
    // not the primal type. This assumes that the two methods are identical,
    // which (mathematically) they should be.
    //
    auto concreteDiffTypeWitnessTable = builder.emitExtractExistentialWitnessTable(aObj);

    // Extract func type from the witness table type.
    IRFuncType* dAddFuncType = nullptr;
    for (UIndex ii = 0; ii < sharedContext->differentiableInterfaceType->getOperandCount(); ii++)
    {
        auto entry = cast<IRInterfaceRequirementEntry>(
            sharedContext->differentiableInterfaceType->getOperand(ii));
        if (entry->getRequirementKey() == sharedContext->addMethodStructKey)
        {
            dAddFuncType = cast<IRFuncType>(entry->getRequirementVal());
            break;
        }
    }

    SLANG_ASSERT(dAddFuncType);

    auto dAddMethod = builder.emitLookupInterfaceMethodInst(
        dAddFuncType,
        concreteDiffTypeWitnessTable,
        sharedContext->addMethodStructKey);

    // Call
    auto dAddResult = builder.emitCallInst(
        dAddFuncType->getResultType(),
        dAddMethod,
        List<IRInst*>(
            {builder.emitExtractExistentialValue(dAddFuncType->getParamType(0), aObj),
             builder.emitExtractExistentialValue(dAddFuncType->getParamType(1), bObj)}));

    // Wrap result in existential.
    auto existentialDiffType = builder.emitMakeExistential(
        sharedContext->differentiableInterfaceType,
        dAddResult,
        concreteDiffTypeWitnessTable);

    builder.emitReturn(existentialDiffType);

    // Emit an unreachable block to act as the after block.
    auto unreachableBlock = builder.emitBlock();
    builder.setInsertInto(unreachableBlock);
    builder.emitUnreachable();

    // Link up conditional blocks.
    builder.setInsertInto(entryBlock);
    builder.emitIfElse(
        aObjWitnessIsNull,
        aObjWitnessIsNullBlock,
        aObjWitnessIsNotNullBlock,
        unreachableBlock);

    builder.setInsertInto(aObjWitnessIsNotNullBlock);
    builder.emitIfElse(
        bObjWitnessIsNull,
        bObjWitnessIsNullBlock,
        bObjWitnessIsNotNullBlock,
        unreachableBlock);

    builder.addNameHintDecoration(existentialDAddFunc, UnownedStringSlice("__existential_dadd"));
    builder.addBackwardDifferentiableDecoration(existentialDAddFunc);

    return existentialDAddFunc;
}

IRInst* DifferentiableTypeConformanceContext::buildDifferentiablePairWitness(
    IRBuilder* builder,
    IRDifferentialPairTypeBase* pairType,
    DiffConformanceKind target)
{
    IRWitnessTable* table = nullptr;
    if (target == DiffConformanceKind::Value)
    {
        // Differentiate the pair type to get it's differential (which is itself a pair)

        // Fill in differential method implementations.
        auto elementType = as<IRDifferentialPairTypeBase>(pairType)->getValueType();
        auto diffType = (IRType*)tryGetDifferentiableValueType(builder, elementType);
        SLANG_ASSERT(diffType);
        auto innerWitness = as<IRDifferentialPairTypeBase>(pairType)->getWitness();

        auto diffDiffPairType =
            (IRType*)tryGetAssociationOfKind(diffType, ValAssociationKind::DifferentialPairType);

        auto addMethod = builder->createFunc();
        auto zeroMethod = builder->createFunc();

        table = builder->createWitnessTable(
            sharedContext->differentiableInterfaceType,
            (IRType*)pairType);

        // Add WitnessTableEntry only once
        if (!table->hasDecorationOrChild())
        {
            // And place it in the synthesized witness table.
            builder->createWitnessTableEntry(
                table,
                sharedContext->differentialAssocTypeStructKey,
                diffDiffPairType);
            builder->createWitnessTableEntry(
                table,
                sharedContext->differentialAssocTypeWitnessStructKey,
                table);
            builder->createWitnessTableEntry(table, sharedContext->addMethodStructKey, addMethod);
            builder->createWitnessTableEntry(table, sharedContext->zeroMethodStructKey, zeroMethod);
        }


        {
            // Add method.
            IRBuilder b = *builder;
            b.setInsertInto(addMethod);
            IRType* paramTypes[2] = {diffDiffPairType, diffDiffPairType};
            addMethod->setFullType(b.getFuncType(2, paramTypes, diffDiffPairType));
            b.emitBlock();
            auto p0 = b.emitParam(diffDiffPairType);
            auto p1 = b.emitParam(diffDiffPairType);

            // Since we are already dealing with a DiffPair<T>.Differnetial type, we know that
            // value type == diff type.
            auto innerAdd = _lookupWitness(
                &b,
                innerWitness,
                sharedContext->addMethodStructKey,
                sharedContext->addMethodType);
            IRInst* argsPrimal[2] = {
                b.emitDifferentialValuePairGetPrimal(diffType, p0),
                b.emitDifferentialValuePairGetPrimal(diffType, p1)};
            auto primalPart = b.emitCallInst(diffType, innerAdd, 2, argsPrimal);
            IRInst* argsDiff[2] = {
                b.emitDifferentialValuePairGetDifferential(diffType, p0),
                b.emitDifferentialValuePairGetDifferential(diffType, p1)};
            auto diffPart = b.emitCallInst(diffType, innerAdd, 2, argsDiff);
            auto retVal = b.emitMakeDifferentialValuePair(diffDiffPairType, primalPart, diffPart);
            b.emitReturn(retVal);
        }
        {
            // Zero method.
            IRBuilder b = *builder;
            b.setInsertInto(zeroMethod);
            zeroMethod->setFullType(b.getFuncType(0, nullptr, diffDiffPairType));
            b.emitBlock();
            auto innerZero = _lookupWitness(
                &b,
                innerWitness,
                sharedContext->zeroMethodStructKey,
                sharedContext->zeroMethodType);
            auto zeroVal = b.emitCallInst(diffType, innerZero, 0, nullptr);
            auto retVal = b.emitMakeDifferentialValuePair(diffDiffPairType, zeroVal, zeroVal);
            b.emitReturn(retVal);
        }
    }
    else if (target == DiffConformanceKind::Ptr)
    {
        // TODO: This logic is incorrect (will cause infinite loop)
        // Differentiate the pair type to get it's differential (which is itself a pair)
        auto diffDiffPairType = (IRType*)getDifferentialForType(builder, (IRType*)pairType);

        table = builder->createWitnessTable(
            sharedContext->differentiablePtrInterfaceType,
            (IRType*)pairType);

        // Add WitnessTableEntry only once
        if (!table->hasDecorationOrChild())
        {
            // And place it in the synthesized witness table.
            builder->createWitnessTableEntry(
                table,
                sharedContext->differentialAssocRefTypeStructKey,
                diffDiffPairType);
            builder->createWitnessTableEntry(
                table,
                sharedContext->differentialAssocRefTypeWitnessStructKey,
                table);
        }
    }

    return table;
}

IRInst* DifferentiableTypeConformanceContext::emitDAddOfDiffInstType(
    IRBuilder* builder,
    IRType* primalType,
    IRInst* op1,
    IRInst* op2)
{
    if (as<IRInterfaceType>(primalType))
    {
        // If our type is existential, we need to handle the case where
        // one or both of our operands are null-type.
        //
        return emitDAddForExistentialType(builder, primalType, op1, op2);
    }
    else if (as<IRAssociatedType>(primalType))
    {
        // Should not happen. associated type does not have any additional info, we can't
        // lookup the necessary methods.
        //
        SLANG_UNEXPECTED("unexpected associated type during transposition");
    }

    auto addMethod = this->getAddMethodForType(builder, primalType);

    // Should exist.
    SLANG_ASSERT(addMethod);

    return builder->emitCallInst(
        (IRType*)this->getDifferentialForType(builder, primalType),
        addMethod,
        List<IRInst*>(op1, op2));
}

IRInst* DifferentiableTypeConformanceContext::emitDAddForExistentialType(
    IRBuilder* builder,
    IRType* primalType,
    IRInst* op1,
    IRInst* op2)
{
    return builder->emitCallInst(
        (IRType*)this->getDifferentialForType(builder, primalType),
        this->getOrCreateExistentialDAddMethod(),
        List<IRInst*>({op1, op2}));
}

IRInst* DifferentiableTypeConformanceContext::emitDZeroOfDiffInstType(
    IRBuilder* builder,
    IRType* primalType)
{
    if (as<IRInterfaceType>(primalType))
    {
        // Pack a null value into an existential type.
        auto existentialZero = builder->emitMakeExistential(
            this->sharedContext->differentiableInterfaceType,
            this->emitNullDifferential(builder),
            this->sharedContext->nullDifferentialWitness);

        return existentialZero;
    }
    else if (as<IRAssociatedType>(primalType))
    {
        // Should never see this case.
        SLANG_UNEXPECTED("unexpected associated type during transposition");
    }

    auto zeroMethod = this->getZeroMethodForType(builder, primalType);

    // Should exist.
    SLANG_ASSERT(zeroMethod);

    return builder->emitCallInst(
        (IRType*)this->getDifferentialForType(builder, primalType),
        zeroMethod,
        List<IRInst*>());
}


void DifferentiableTypeConformanceContext::markDiffTypeInst(
    IRBuilder* builder,
    IRInst* diffInst,
    IRType* primalType)
{
    // Ignore module-level insts.
    if (as<IRModuleInst>(diffInst->getParent()))
        return;

    // Also ignore generic-container-level insts.
    if (as<IRBlock>(diffInst->getParent()) && as<IRGeneric>(diffInst->getParent()->getParent()))
        return;

    // TODO: This logic is a bit of a hack. We need to determine if the type is
    // relevant to ptr-type computation or not, or more complex applications
    // that use dynamic dispatch + ptr types will fail.
    //
    if (as<IRType>(diffInst))
    {
        builder->markInstAsDifferential(diffInst, nullptr);
        return;
    }

    SLANG_ASSERT(diffInst);
    SLANG_ASSERT(primalType);

    if (isDifferentiableValueType(primalType))
    {
        builder->markInstAsDifferential(diffInst, primalType);
    }
    else if (isDifferentiablePtrType(primalType))
    {
        builder->markInstAsPrimal(diffInst);
    }
    else
    {
        // Stop-gap solution to go with differential inst for now.
        builder->markInstAsDifferential(diffInst, primalType);
    }
}

void DifferentiableTypeConformanceContext::markDiffPairTypeInst(
    IRBuilder* builder,
    IRInst* diffPairInst,
    IRType* pairType)
{
    SLANG_ASSERT(diffPairInst);
    SLANG_ASSERT(pairType);
    SLANG_ASSERT(as<IRDifferentialPairTypeBase>(pairType));

    if (as<IRDifferentialPairType>(pairType))
    {
        builder->markInstAsMixedDifferential(diffPairInst, pairType);
    }
    else if (as<IRDifferentialPtrPairType>(pairType))
    {
        builder->markInstAsPrimal(diffPairInst);
    }
    else
    {
        SLANG_UNEXPECTED("unexpected differentiable type");
    }
}

void copyCheckpointHints(
    IRBuilder* builder,
    IRGlobalValueWithCode* oldInst,
    IRGlobalValueWithCode* newInst)
{
    for (auto decor = oldInst->getFirstDecoration(); decor; decor = decor->getNextDecoration())
    {
        if (auto chkHint = as<IRCheckpointHintDecoration>(decor))
        {
            cloneCheckpointHint(builder, chkHint, newInst);
        }
    }
}

void cloneCheckpointHint(
    IRBuilder* builder,
    IRCheckpointHintDecoration* chkHint,
    IRGlobalValueWithCode* target)
{
    // Grab all the operands
    List<IRInst*> operands;
    for (UCount operand = 0; operand < chkHint->getOperandCount(); operand++)
    {
        operands.add(chkHint->getOperand(operand));
    }

    builder->addDecoration(target, chkHint->getOp(), operands.getBuffer(), operands.getCount());
}

void stripDerivativeDecorations(IRInst* inst)
{
    for (auto decor = inst->getFirstDecoration(); decor;)
    {
        auto next = decor->getNextDecoration();
        switch (decor->getOp())
        {
        case kIROp_ForwardDerivativeDecoration:
        case kIROp_DerivativeMemberDecoration:
        case kIROp_BackwardDerivativeDecoration:
        case kIROp_BackwardDerivativeIntermediateTypeDecoration:
        case kIROp_BackwardDerivativePropagateDecoration:
        case kIROp_BackwardDerivativePrimalDecoration:
        case kIROp_UserDefinedBackwardDerivativeDecoration:
        case kIROp_AutoDiffOriginalValueDecoration:
            decor->removeAndDeallocate();
            break;
        default:
            break;
        }
        decor = next;
    }
}


void stripAutoDiffDecorationsFromChildren(IRInst* parent)
{
    for (auto inst : parent->getChildren())
    {
        bool shouldRemoveKeepAliveDecorations = false;
        for (auto decor = inst->getFirstDecoration(); decor;)
        {
            auto next = decor->getNextDecoration();
            switch (decor->getOp())
            {
            case kIROp_ForwardDerivativeDecoration:
            case kIROp_DerivativeMemberDecoration:
            case kIROp_DifferentiableTypeDictionaryDecoration:
            case kIROp_PrimalInstDecoration:
            case kIROp_DifferentialInstDecoration:
            case kIROp_MixedDifferentialInstDecoration:
            case kIROp_RecomputeBlockDecoration:
            case kIROp_LoopCounterDecoration:
            case kIROp_LoopCounterUpdateDecoration:
            case kIROp_BackwardDerivativeDecoration:
            case kIROp_BackwardDerivativeIntermediateTypeDecoration:
            case kIROp_BackwardDerivativePropagateDecoration:
            case kIROp_BackwardDerivativePrimalDecoration:
            case kIROp_BackwardDerivativePrimalContextDecoration:
            case kIROp_BackwardDerivativePrimalReturnDecoration:
            case kIROp_PrimalSubstituteDecoration:
            case kIROp_AutoDiffOriginalValueDecoration:
            case kIROp_UserDefinedBackwardDerivativeDecoration:
            case kIROp_IntermediateContextFieldDifferentialTypeDecoration:
            case kIROp_CheckpointIntermediateDecoration:
                decor->removeAndDeallocate();
                break;
            case kIROp_AutoDiffBuiltinDecoration:
                // Remove the builtin decoration, and also remove any export/keep-alive
                // decorations.
                shouldRemoveKeepAliveDecorations = true;
                decor->removeAndDeallocate();
            default:
                break;
            }
            decor = next;
        }

        if (shouldRemoveKeepAliveDecorations)
        {
            for (auto decor = inst->getFirstDecoration(); decor;)
            {
                auto next = decor->getNextDecoration();
                switch (decor->getOp())
                {
                case kIROp_ExportDecoration:
                case kIROp_HLSLExportDecoration:
                case kIROp_KeepAliveDecoration:
                    decor->removeAndDeallocate();
                    break;
                }
                decor = next;
            }
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


void stripTempDecorations(IRInst* inst)
{
    for (auto decor = inst->getFirstDecoration(); decor;)
    {
        auto next = decor->getNextDecoration();
        switch (decor->getOp())
        {
        case kIROp_DifferentialInstDecoration:
        case kIROp_MixedDifferentialInstDecoration:
        case kIROp_RecomputeBlockDecoration:
        case kIROp_AutoDiffOriginalValueDecoration:
        case kIROp_BackwardDerivativePrimalReturnDecoration:
        case kIROp_BackwardDerivativePrimalContextDecoration:
        case kIROp_PrimalValueStructKeyDecoration:
        case kIROp_PrimalElementTypeDecoration:
            decor->removeAndDeallocate();
            break;
        default:
            break;
        }
        decor = next;
    }
    for (auto child : inst->getChildren())
    {
        stripTempDecorations(child);
    }
}

ParameterDirectionInfo transposeDirection(ParameterDirectionInfo direction)
{
    switch (direction.kind)
    {
    case ParameterDirectionInfo::Kind::In:
        return ParameterDirectionInfo(ParameterDirectionInfo::Kind::Out);
    case ParameterDirectionInfo::Kind::Out:
        return ParameterDirectionInfo(ParameterDirectionInfo::Kind::In);
    case ParameterDirectionInfo::Kind::BorrowInOut:
        return ParameterDirectionInfo(ParameterDirectionInfo::Kind::BorrowInOut);
    case ParameterDirectionInfo::Kind::Ref:
        return ParameterDirectionInfo(ParameterDirectionInfo::Kind::Ref, direction.addressSpace);
    case ParameterDirectionInfo::Kind::BorrowIn:
        return ParameterDirectionInfo(
            ParameterDirectionInfo::Kind::BorrowIn,
            direction.addressSpace);
    default:
        SLANG_UNREACHABLE("Invalid parameter direction");
    }
}

struct StripNoDiffTypeAttributePass : InstPassBase
{
    StripNoDiffTypeAttributePass(IRModule* module)
        : InstPassBase(module)
    {
    }
    void processModule()
    {
        processInstsOfType<IRAttributedType>(
            kIROp_AttributedType,
            [&](IRAttributedType* attrType)
            {
                if (attrType->getAllAttrs().getCount() == 1)
                {
                    if (attrType->findAttr<IRNoDiffAttr>())
                    {
                        attrType->replaceUsesWith(attrType->getBaseType());
                        attrType->removeAndDeallocate();
                    }
                }
            });
    }
};

void stripNoDiffTypeAttribute(IRModule* module)
{
    StripNoDiffTypeAttributePass pass(module);
    pass.processModule();
}

bool canTypeBeStored(IRInst* type)
{
    if (!type)
        return false;

    if (as<IRBasicType>(type))
        return true;

    switch (type->getOp())
    {
    case kIROp_StructType:
    case kIROp_OptionalType:
    case kIROp_TupleType:
    case kIROp_ArrayType:
    case kIROp_DifferentialPairType:
    case kIROp_InterfaceType:
    case kIROp_AssociatedType:
    case kIROp_AnyValueType:
    case kIROp_ClassType:
    case kIROp_FloatType:
    case kIROp_VectorType:
    case kIROp_CoopVectorType:
    case kIROp_MatrixType:
    case kIROp_BackwardDiffIntermediateContextType:
        return true;
    case kIROp_AttributedType:
        return canTypeBeStored(type->getOperand(0));
    default:
        return false;
    }
}

void checkAutodiffPatterns(TargetProgram* target, IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(target);

    enum SideEffectBehavior
    {
        Warn = 0,
        Allow = 1,
    };

    // For now, we have only 1 check to see if methods that have side-effects
    // are marked with prefer-recompute
    //
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            if (func->sourceLoc.isValid() && // Don't diagnose for synthesized functions
                func->findDecoration<IRPreferRecomputeDecoration>())
            {
                // If we don't have any side-effect behavior, we should warn (note: read-none is
                // a stronger guarantee than no-side-effect)
                //
                if (func->findDecoration<IRNoSideEffectDecoration>() ||
                    func->findDecoration<IRReadNoneDecoration>())
                    continue;

                auto preferRecomputeDecor = func->findDecoration<IRPreferRecomputeDecoration>();
                auto sideEffectBehavior =
                    as<IRIntLit>(preferRecomputeDecor->getOperand(0))->getValue();

                if (sideEffectBehavior == SideEffectBehavior::Allow)
                    continue;

                // Find function name. (don't diagnose on nameless functions)
                if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
                {
                    sink->diagnose(
                        func,
                        Diagnostics::potentialIssuesWithPreferRecomputeOnSideEffectMethod,
                        nameHint->getName());
                }
            }
        }
    }
}

struct RemoveDetachInstsPass : InstPassBase
{
    RemoveDetachInstsPass(IRModule* module)
        : InstPassBase(module)
    {
    }
    void processModule()
    {
        processInstsOfType<IRDetachDerivative>(
            kIROp_DetachDerivative,
            [&](IRDetachDerivative* detach) { detach->replaceUsesWith(detach->getBase()); });
    }
};

void removeDetachInsts(IRModule* module)
{
    RemoveDetachInstsPass pass(module);
    pass.processModule();
}


struct RemoveTypeAnnotationInstsPass : InstPassBase
{
    RemoveTypeAnnotationInstsPass(IRModule* module)
        : InstPassBase(module)
    {
    }
    void processModule()
    {
        processInstsOfType<IRAssociatedInstAnnotation>(
            kIROp_AssociatedInstAnnotation,
            [&](IRAssociatedInstAnnotation* annotation) { annotation->removeAndDeallocate(); });
    }
};

void removeTypeAnnotations(IRModule* module)
{
    RemoveTypeAnnotationInstsPass pass(module);
    pass.processModule();
}

struct LowerNullCheckPass : InstPassBase
{
    LowerNullCheckPass(IRModule* module, AutoDiffSharedContext* context)
        : InstPassBase(module), context(context)
    {
    }
    void processModule()
    {
        List<IRInst*> nullCheckInsts;
        processInstsOfType<IRIsDifferentialNull>(
            kIROp_IsDifferentialNull,
            [&](IRIsDifferentialNull* isDiffNullInst)
            {
                IRBuilder builder(module);
                builder.setInsertBefore(isDiffNullInst);

                // Extract existential type from the operand.
                auto operand = isDiffNullInst->getBase();
                auto operandConcreteWitness = builder.emitExtractExistentialWitnessTable(operand);
                auto witnessID = builder.emitGetSequentialIDInst(operandConcreteWitness);

                auto nullDiffWitnessTable = context->nullDifferentialWitness;
                auto nullDiffWitnessID = builder.emitGetSequentialIDInst(nullDiffWitnessTable);

                // Compare the concrete type with the null differential witness table.
                auto isDiffNull = builder.emitEql(witnessID, nullDiffWitnessID);

                isDiffNullInst->replaceUsesWith(isDiffNull);
                nullCheckInsts.add(isDiffNullInst);
            });

        for (auto nullCheckInst : nullCheckInsts)
        {
            nullCheckInst->removeAndDeallocate();
        }
    }

private:
    AutoDiffSharedContext* context;
};

void lowerNullCheckInsts(IRModule* module, AutoDiffSharedContext* context)
{
    LowerNullCheckPass pass(module, context);
    pass.processModule();
}

void releaseNullDifferentialType(AutoDiffSharedContext* context)
{
    if (auto nullStruct = context->nullDifferentialStructType)
    {
        if (auto keepAliveDecoration = nullStruct->findDecoration<IRKeepAliveDecoration>())
            keepAliveDecoration->removeAndDeallocate();
        if (auto exportDecoration = nullStruct->findDecoration<IRHLSLExportDecoration>())
            exportDecoration->removeAndDeallocate();
    }

    if (auto nullWitness = context->nullDifferentialWitness)
    {
        if (auto keepAliveDecoration = nullWitness->findDecoration<IRKeepAliveDecoration>())
            keepAliveDecoration->removeAndDeallocate();
        if (auto exportDecoration = nullWitness->findDecoration<IRHLSLExportDecoration>())
            exportDecoration->removeAndDeallocate();
    }
}

void releaseDifferentiableInterfaces(AutoDiffSharedContext* context)
{
    // Remove all KeepAlive decorations on the differentiable interfaces
    List<IRInst*> decorationsToRemove;
    if (context->forwardDifferentiableInterfaceType)
        for (auto decoration : context->forwardDifferentiableInterfaceType->getDecorations())
        {
            if (auto keepAliveDecoration = decoration->findDecoration<IRKeepAliveDecoration>())
                decorationsToRemove.add(keepAliveDecoration);
        }

    if (context->backwardDifferentiableInterfaceType)
        for (auto decoration : context->backwardDifferentiableInterfaceType->getDecorations())
        {
            if (auto keepAliveDecoration = decoration->findDecoration<IRKeepAliveDecoration>())
                decorationsToRemove.add(keepAliveDecoration);
        }

    if (context->backwardCallableInterfaceType)
        for (auto decoration : context->backwardCallableInterfaceType->getDecorations())
        {
            if (auto keepAliveDecoration = decoration->findDecoration<IRKeepAliveDecoration>())
                decorationsToRemove.add(keepAliveDecoration);
        }

    for (auto decoration : decorationsToRemove)
        decoration->removeAndDeallocate();
}

bool finalizeAutoDiffPass(TargetProgram* target, IRModule* module)
{
    bool modified = false;

    // Create shared context for all auto-diff related passes
    AutoDiffSharedContext autodiffContext(module->getModuleInst());

    // Replaces IRDifferentialPairType with an auto-generated struct,
    // IRDifferentialPairGetDifferential with 'differential' field access,
    // IRDifferentialPairGetPrimal with 'primal' field access, and
    // IRMakeDifferentialPair with an IRMakeStruct.
    //
    modified |= processPairTypes(&autodiffContext);

    removeDetachInsts(module);

    removeTypeAnnotations(module);

    lowerNullCheckInsts(module, &autodiffContext);

    stripNoDiffTypeAttribute(module);

    stripAutoDiffDecorations(module);

    releaseDifferentiableInterfaces(&autodiffContext);

    return true;
}

UIndex addPhiOutputArg(
    IRBuilder* builder,
    IRBlock* block,
    IRInst*& inoutTerminatorInst,
    IRInst* arg)
{
    SLANG_RELEASE_ASSERT(as<IRUnconditionalBranch>(block->getTerminator()));

    auto branchInst = as<IRUnconditionalBranch>(block->getTerminator());
    List<IRInst*> phiArgs;

    for (UIndex ii = 0; ii < branchInst->getArgCount(); ii++)
        phiArgs.add(branchInst->getArg(ii));

    phiArgs.add(arg);

    builder->setInsertInto(block);
    switch (branchInst->getOp())
    {
    case kIROp_UnconditionalBranch:
        inoutTerminatorInst = builder->emitBranch(
            branchInst->getTargetBlock(),
            phiArgs.getCount(),
            phiArgs.getBuffer());
        break;

    case kIROp_Loop:
        {
            auto newLoop = builder->emitLoop(
                as<IRLoop>(branchInst)->getTargetBlock(),
                as<IRLoop>(branchInst)->getBreakBlock(),
                as<IRLoop>(branchInst)->getContinueBlock(),
                phiArgs.getCount(),
                phiArgs.getBuffer());
            branchInst->transferDecorationsTo(newLoop);
            branchInst->replaceUsesWith(newLoop);
            inoutTerminatorInst = newLoop;
        }
        break;

    default:
        SLANG_UNEXPECTED("Unexpected branch-type for phi replacement");
    }

    branchInst->removeAndDeallocate();
    return phiArgs.getCount() - 1;
}

bool isDifferentialOrRecomputeBlock(IRBlock* block)
{
    if (!block)
        return false;
    for (auto decor : block->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_DifferentialInstDecoration:
        case kIROp_RecomputeBlockDecoration:
            return true;
        default:
            break;
        }
    }
    return false;
}

IRUse* findUniqueStoredVal(IRVar* var)
{
    if (isDerivativeContextVar(var))
    {
        IRUse* primalCallUse = nullptr;
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            if (const auto callInst = as<IRCall>(use->getUser()))
            {
                // Ignore uses from differential blocks.
                if (callInst->getParent()->findDecoration<IRDifferentialInstDecoration>())
                    continue;
                // Should not see more than one IRCall. If we do
                // we'll need to pick the primal call.
                //
                SLANG_RELEASE_ASSERT(!primalCallUse);
                primalCallUse = use;
            }
        }
        return primalCallUse;
    }
    else
    {
        IRUse* storeUse = nullptr;
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            if (const auto storeInst = as<IRStore>(use->getUser()))
            {
                // Ignore uses from differential blocks.
                if (storeInst->getParent()->findDecoration<IRDifferentialInstDecoration>())
                    continue;
                // Should not see more than one IRStore
                SLANG_RELEASE_ASSERT(!storeUse);
                storeUse = use;
            }
        }
        return storeUse;
    }
}

// Given a local var that is supposed to have a unique write, find the last inst
// that writes to it. Note: if var is intended for an inout argument, it will
// have exactly one store that sets its initial value and one call that writes
// the final value to it, this method will return the call inst for this case.
IRUse* findLatestUniqueWriteUse(IRVar* var)
{
    IRUse* callUse = nullptr;
    for (auto use = var->firstUse; use; use = use->nextUse)
    {
        if (const auto callInst = as<IRCall>(use->getUser()))
        {
            // Ignore uses from differential blocks.
            if (callInst->getParent()->findDecoration<IRDifferentialInstDecoration>())
                continue;
            SLANG_RELEASE_ASSERT(!callUse);
            callUse = use;
        }
    }

    if (callUse)
        return callUse;

    // If no unique call found, try to look for a store.
    return findUniqueStoredVal(var);
}

// Given a local var that is supposed to have a unique write, find the last inst
// that writes to it. Note: if var is intended for an inout argument, it will
// have exactly one store that sets its initial value and one call that writes
// the final value to it, this method will return the store inst for this case.
IRUse* findEarliestUniqueWriteUse(IRVar* var)
{
    IRUse* storeUse = findUniqueStoredVal(var);
    if (storeUse)
        return storeUse;

    // If no unique store found, try to look for a call.
    for (auto use = var->firstUse; use; use = use->nextUse)
    {
        if (const auto callInst = as<IRCall>(use->getUser()))
        {
            // Ignore uses from differential blocks.
            if (callInst->getParent()->findDecoration<IRDifferentialInstDecoration>())
                continue;
            SLANG_RELEASE_ASSERT(!storeUse);
            storeUse = use;
        }
    }
    return storeUse;
}


bool isDerivativeContextVar(IRVar* var)
{
    return var->findDecoration<IRBackwardDerivativePrimalContextDecoration>();
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

void copyDebugInfo(IRInst* srcFunc, IRInst* destFunc)
{
    // Copy debug decorations.
    for (auto decor : srcFunc->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_DebugInlinedAt:
        case kIROp_DebugScope:
        case kIROp_DebugNoScope:
        case kIROp_DebugInlinedVariable:
        case kIROp_DebugFuncDecoration:
        case kIROp_DebugLocationDecoration:
            cloneDecoration(decor, destFunc);
            break;
        default:
            break;
        }
    }
}

void copyOriginalDecorations(IRInst* origFunc, IRInst* diffFunc)
{
    for (auto decor : origFunc->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_ForceInlineDecoration:
            cloneDecoration(decor, diffFunc);
            break;
        }
    }
}

} // namespace Slang
