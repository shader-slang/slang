// slang-ir-witness-table-wrapper.cpp
#include "slang-ir-witness-table-wrapper.h"

#include "slang-ir-clone.h"
#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct GenericsLoweringContext;
IRFunc* emitWitnessTableWrapper(
    IRModule* module,
    IRInst* funcInst,
    IRInst* interfaceRequirementVal);

struct GenerateWitnessTableWrapperContext
{
    SharedGenericsLoweringContext* sharedContext;

    void lowerWitnessTable(IRWitnessTable* witnessTable)
    {
        auto interfaceType = as<IRInterfaceType>(witnessTable->getConformanceType());
        if (!interfaceType)
            return;
        if (isBuiltin(interfaceType))
            return;
        if (isComInterfaceType(interfaceType))
            return;

        // We need to consider whether the concrete type that is conforming
        // in this witness table actually fits within the declared any-value
        // size for the interface.
        //
        // If the type doesn't fit then it would be invalid to use for dynamic
        // dispatch, and the packing/unpacking operations we emit would fail
        // to generate valid code.
        //
        // Such a type might still be useful for static specialization, so
        // we can't consider this case a hard error.
        //
        auto concreteType = witnessTable->getConcreteType();
        IRIntegerValue typeSize, sizeLimit;
        bool isTypeOpaque = false;
        if (!sharedContext->doesTypeFitInAnyValue(
                concreteType,
                interfaceType,
                &typeSize,
                &sizeLimit,
                &isTypeOpaque))
        {
            HashSet<IRType*> visited;
            if (isTypeOpaque)
            {
                sharedContext->sink->diagnose(
                    concreteType,
                    Diagnostics::typeCannotBePackedIntoAnyValue,
                    concreteType);
            }
            else
            {
                sharedContext->sink->diagnose(
                    concreteType,
                    Diagnostics::typeDoesNotFitAnyValueSize,
                    concreteType);
                sharedContext->sink->diagnoseWithoutSourceView(
                    concreteType,
                    Diagnostics::typeAndLimit,
                    concreteType,
                    typeSize,
                    sizeLimit);
            }
            return;
        }

        for (auto child : witnessTable->getChildren())
        {
            auto entry = as<IRWitnessTableEntry>(child);
            if (!entry)
                continue;
            auto interfaceRequirementVal = sharedContext->findInterfaceRequirementVal(
                interfaceType,
                entry->getRequirementKey());
            if (auto ordinaryFunc = as<IRFunc>(entry->getSatisfyingVal()))
            {
                auto wrapper = emitWitnessTableWrapper(
                    sharedContext->module,
                    ordinaryFunc,
                    interfaceRequirementVal);
                entry->satisfyingVal.set(wrapper);
                sharedContext->addToWorkList(wrapper);
            }
        }
    }

    void processInst(IRInst* inst)
    {
        if (auto witnessTable = as<IRWitnessTable>(inst))
        {
            lowerWitnessTable(witnessTable);
        }
    }

    void processModule()
    {
        sharedContext->addToWorkList(sharedContext->module->getModuleInst());

        while (sharedContext->workList.getCount() != 0)
        {
            IRInst* inst = sharedContext->workList.getLast();

            sharedContext->workList.removeLast();
            sharedContext->workListSet.remove(inst);

            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                sharedContext->addToWorkList(child);
            }
        }
    }
};


// DUPLICATES... put into common file.

static bool isTaggedUnionType(IRInst* type)
{
    if (auto tupleType = as<IRTupleType>(type))
        return as<IRCollectionTagType>(tupleType->getOperand(0)) != nullptr;

    return false;
}

static UCount getCollectionCount(IRCollectionBase* collection)
{
    if (!collection)
        return 0;
    return collection->getOperandCount();
}

static UCount getCollectionCount(IRCollectionTagType* tagType)
{
    auto collection = tagType->getOperand(0);
    return getCollectionCount(as<IRCollectionBase>(collection));
}

static IRInst* upcastCollection(IRBuilder* builder, IRInst* arg, IRType* destInfo)
{
    auto argInfo = arg->getDataType();
    if (!argInfo || !destInfo)
        return arg;

    if (isTaggedUnionType(argInfo) && isTaggedUnionType(destInfo))
    {
        auto argTupleType = as<IRTupleType>(argInfo);
        auto destTupleType = as<IRTupleType>(destInfo);

        List<IRInst*> upcastedElements;
        bool hasUpcastedElements = false;

        // Upcast each element of the tuple
        for (UInt i = 0; i < argTupleType->getOperandCount(); ++i)
        {
            auto argElementType = argTupleType->getOperand(i);
            auto destElementType = destTupleType->getOperand(i);

            // If the element types are different, we need to reinterpret
            if (argElementType != destElementType)
            {
                hasUpcastedElements = true;
                upcastedElements.add(upcastCollection(
                    builder,
                    builder->emitGetTupleElement((IRType*)argElementType, arg, i),
                    (IRType*)destElementType));
            }
            else
            {
                upcastedElements.add(builder->emitGetTupleElement((IRType*)argElementType, arg, i));
            }
        }

        if (hasUpcastedElements)
        {
            return builder->emitMakeTuple(upcastedElements);
        }
    }
    else if (as<IRCollectionTagType>(argInfo) && as<IRCollectionTagType>(destInfo))
    {
        if (getCollectionCount(as<IRCollectionTagType>(argInfo)) !=
            getCollectionCount(as<IRCollectionTagType>(destInfo)))
        {
            return builder
                ->emitIntrinsicInst((IRType*)destInfo, kIROp_GetTagForSuperCollection, 1, &arg);
        }
    }
    else if (as<IRCollectionBase>(argInfo) && as<IRCollectionBase>(destInfo))
    {
        if (getCollectionCount(as<IRCollectionBase>(argInfo)) !=
            getCollectionCount(as<IRCollectionBase>(destInfo)))
        {
            // If the sets of witness tables are not equal, reinterpret to the parameter type
            return builder->emitReinterpret((IRType*)destInfo, arg);
        }
    }
    else if (!as<IRCollectionBase>(argInfo) && as<IRCollectionBase>(destInfo))
    {
        return builder->emitPackAnyValue((IRType*)destInfo, arg);
    }

    return arg; // Can use as-is.
}


// Represents a work item for packing `inout` or `out` arguments after a concrete call.
struct ArgumentPackWorkItem
{
    enum Kind
    {
        Pack,
        UpCast,
    } kind = Pack;

    // A `AnyValue` typed destination.
    IRInst* dstArg = nullptr;
    // A concrete value to be packed.
    IRInst* concreteArg = nullptr;
};

bool isAnyValueType(IRType* type)
{
    if (as<IRAnyValueType>(type) || as<IRTypeCollection>(type))
        return true;
    return false;
}

// Unpack an `arg` of `IRAnyValue` into concrete type if necessary, to make it feedable into the
// parameter. If `arg` represents a AnyValue typed variable passed in to a concrete `out`
// parameter, this function indicates that it needs to be packed after the call by setting
// `packAfterCall`.
IRInst* maybeUnpackArg(
    IRBuilder* builder,
    IRType* paramType,
    IRInst* arg,
    ArgumentPackWorkItem& packAfterCall)
{
    packAfterCall.dstArg = nullptr;
    packAfterCall.concreteArg = nullptr;

    // If either paramType or argType is a pointer type
    // (because of `inout` or `out` modifiers), we extract
    // the underlying value type first.
    IRType* paramValType = paramType;
    IRType* argValType = arg->getDataType();
    IRInst* argVal = arg;
    if (auto ptrType = as<IRPtrTypeBase>(paramType))
    {
        paramValType = ptrType->getValueType();
    }
    auto argType = arg->getDataType();
    if (auto argPtrType = as<IRPtrTypeBase>(argType))
    {
        argValType = argPtrType->getValueType();
    }


    // Unpack `arg` if the parameter expects concrete type but
    // `arg` is an AnyValue.
    if (!isAnyValueType(paramValType) && isAnyValueType(argValType))
    {
        // if parameter expects an `out` pointer, store the unpacked val into a
        // variable and pass in a pointer to that variable.
        if (as<IRPtrTypeBase>(paramType))
        {
            auto tempVar = builder->emitVar(paramValType);
            if (as<IRInOutType>(paramType))
                builder->emitStore(
                    tempVar,
                    builder->emitUnpackAnyValue(paramValType, builder->emitLoad(arg)));

            // tempVar needs to be unpacked into original var after the call.
            packAfterCall.kind = ArgumentPackWorkItem::Kind::Pack;
            packAfterCall.dstArg = arg;
            packAfterCall.concreteArg = tempVar;
            return tempVar;
        }
        else
        {
            return builder->emitUnpackAnyValue(paramValType, argVal);
        }
    }

    // Reinterpret 'arg' if it is being passed to a parameter with
    // a different type collection. For now, we'll approximate this
    // by checking if the types are different, but this should be
    // encoded in the types.
    //
    if (isTaggedUnionType(paramValType) && isTaggedUnionType(argValType) &&
        paramValType != argValType)
    {
        // if parameter expects an `out` pointer, store the unpacked val into a
        // variable and pass in a pointer to that variable.
        if (as<IROutType>(paramType))
        {
            auto tempVar = builder->emitVar(paramValType);

            // tempVar needs to be unpacked into original var after the call.
            packAfterCall.kind = ArgumentPackWorkItem::Kind::UpCast;
            packAfterCall.dstArg = arg;
            packAfterCall.concreteArg = tempVar;
            return tempVar;
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected upcast for non-out parameter");
        }
    }
    return arg;
}

IRStringLit* _getWitnessTableWrapperFuncName(IRModule* module, IRFunc* func)
{
    IRBuilder builderStorage(module);
    auto builder = &builderStorage;
    builder->setInsertBefore(func);
    if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
    {
        return builder->getStringValue(
            (String(linkageDecoration->getMangledName()) + "_wtwrapper").getUnownedSlice());
    }
    if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
    {
        return builder->getStringValue(
            (String(namehintDecoration->getName()) + "_wtwrapper").getUnownedSlice());
    }
    return nullptr;
}


IRFunc* emitWitnessTableWrapper(IRModule* module, IRInst* funcInst, IRInst* interfaceRequirementVal)
{
    auto funcTypeInInterface = cast<IRFuncType>(interfaceRequirementVal);
    auto targetFuncType = as<IRFuncType>(funcInst->getDataType());

    IRBuilder builderStorage(module);
    auto builder = &builderStorage;
    builder->setInsertBefore(funcInst);

    auto wrapperFunc = builder->createFunc();
    wrapperFunc->setFullType((IRType*)interfaceRequirementVal);
    if (auto func = as<IRFunc>(funcInst))
        if (auto name = _getWitnessTableWrapperFuncName(module, func))
            builder->addNameHintDecoration(wrapperFunc, name);

    builder->setInsertInto(wrapperFunc);
    auto block = builder->emitBlock();
    builder->setInsertInto(block);

    ShortList<IRParam*> params;
    for (UInt i = 0; i < funcTypeInInterface->getParamCount(); i++)
    {
        params.add(builder->emitParam(funcTypeInInterface->getParamType(i)));
    }

    List<IRInst*> args;
    List<ArgumentPackWorkItem> argsToPack;

    SLANG_ASSERT(params.getCount() == (Index)targetFuncType->getParamCount());
    for (UInt i = 0; i < targetFuncType->getParamCount(); i++)
    {
        auto wrapperParam = params[i];
        // Type of the parameter in the callee.
        auto funcParamType = targetFuncType->getParamType(i);

        // If the implementation expects a concrete type
        // (either in the form of a pointer for `out`/`inout` parameters,
        // or in the form a value for `in` parameters, while
        // the interface exposes an AnyValue type,
        // we need to unpack the AnyValue argument to the appropriate
        // concerete type.
        ArgumentPackWorkItem packWorkItem;
        auto newArg = maybeUnpackArg(builder, funcParamType, wrapperParam, packWorkItem);
        args.add(newArg);
        if (packWorkItem.concreteArg)
            argsToPack.add(packWorkItem);
    }
    auto call = builder->emitCallInst(targetFuncType->getResultType(), funcInst, args);

    // Pack all `out` arguments.
    for (auto item : argsToPack)
    {
        auto anyValType = cast<IRPtrTypeBase>(item.dstArg->getDataType())->getValueType();
        auto concreteVal = builder->emitLoad(item.concreteArg);
        auto packedVal = (item.kind == ArgumentPackWorkItem::Kind::Pack)
                             ? builder->emitPackAnyValue(anyValType, concreteVal)
                             : upcastCollection(builder, concreteVal, anyValType);
        builder->emitStore(item.dstArg, packedVal);
    }

    // Pack return value if necessary.
    if (!isAnyValueType(call->getDataType()) &&
        isAnyValueType(funcTypeInInterface->getResultType()))
    {
        auto pack = builder->emitPackAnyValue(funcTypeInInterface->getResultType(), call);
        builder->emitReturn(pack);
    }
    else if (
        isTaggedUnionType(call->getDataType()) &&
        isTaggedUnionType(funcTypeInInterface->getResultType()))
    {
        auto reinterpret = upcastCollection(builder, call, funcTypeInInterface->getResultType());
        builder->emitReturn(reinterpret);
    }
    else
    {
        if (call->getDataType()->getOp() == kIROp_VoidType)
            builder->emitReturn();
        else
            builder->emitReturn(call);
    }
    return wrapperFunc;
}

void generateWitnessTableWrapperFunctions(SharedGenericsLoweringContext* sharedContext)
{
    GenerateWitnessTableWrapperContext context;
    context.sharedContext = sharedContext;
    context.processModule();
}

} // namespace Slang
