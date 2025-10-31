#include "slang-ir-lower-typeflow-insts.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-specialize.h"
#include "slang-ir-typeflow-collection.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

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
    if (as<IRAnyValueType>(type) || as<IRUntaggedUnionType>(type))
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
            if (as<IRBorrowInOutParamType>(paramType))
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
    if (as<IRTaggedUnionType>(paramValType) && as<IRTaggedUnionType>(argValType) &&
        paramValType != argValType)
    {
        // if parameter expects an `out` pointer, store the unpacked val into a
        // variable and pass in a pointer to that variable.
        if (as<IROutParamType>(paramType))
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
                             : upcastSet(builder, concreteVal, anyValType);
        builder->emitStore(item.dstArg, packedVal);
    }

    // Pack return value if necessary.
    if (!isAnyValueType(call->getDataType()) &&
        isAnyValueType(funcTypeInInterface->getResultType()))
    {
        auto pack = builder->emitPackAnyValue(funcTypeInInterface->getResultType(), call);
        builder->emitReturn(pack);
    }
    else if (call->getDataType() != funcTypeInInterface->getResultType())
    {
        auto reinterpret = upcastSet(builder, call, funcTypeInInterface->getResultType());
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

UInt getUniqueID(IRBuilder* builder, IRInst* inst)
{
    // Fallback.
    return builder->getUniqueID(inst);
}

// Generate a single function that dispatches to each function in the collection.
// The resulting function will have one additional parameter to accept the tag
// indicating which function to call.
//
IRFunc* createDispatchFunc(IRFuncType* dispatchFuncType, Dictionary<IRInst*, IRInst*>& mapping)
{
    // Create a dispatch function with switch-case for each function
    IRBuilder builder(dispatchFuncType->getModule());

    // Consume the first parameter of the expected function type
    List<IRType*> innerParamTypes;
    for (auto paramType : dispatchFuncType->getParamTypes())
        innerParamTypes.add(paramType);
    innerParamTypes.removeAt(0); // Remove the first parameter (ID)

    auto resultType = dispatchFuncType->getResultType();
    auto innerFuncType = builder.getFuncType(innerParamTypes, resultType);

    auto func = builder.createFunc();
    builder.setInsertInto(func);
    func->setFullType(dispatchFuncType);

    auto entryBlock = builder.emitBlock();
    builder.setInsertInto(entryBlock);

    auto idParam = builder.emitParam(builder.getUIntType());

    // Create parameters for the original function arguments
    List<IRInst*> originalParams;
    for (Index i = 0; i < innerParamTypes.getCount(); i++)
    {
        originalParams.add(builder.emitParam(innerParamTypes[i]));
    }

    // Create default block
    auto defaultBlock = builder.emitBlock();
    builder.setInsertInto(defaultBlock);
    if (resultType->getOp() == kIROp_VoidType)
    {
        builder.emitReturn();
    }
    else
    {
        // Return a default-constructed value
        auto defaultValue = builder.emitDefaultConstruct(resultType);
        builder.emitReturn(defaultValue);
    }

    // Go back to entry block and create switch
    builder.setInsertInto(entryBlock);

    // Create case blocks for each function
    List<IRInst*> caseValues;
    List<IRBlock*> caseBlocks;

    for (auto kvPair : mapping)
    {
        auto funcInst = kvPair.second;
        auto funcTag = kvPair.first;

        auto wrapperFunc = emitWitnessTableWrapper(funcInst->getModule(), funcInst, innerFuncType);

        // Create case block
        auto caseBlock = builder.emitBlock();
        builder.setInsertInto(caseBlock);

        List<IRInst*> callArgs;
        auto wrappedFuncType = as<IRFuncType>(wrapperFunc->getDataType());
        for (Index ii = 0; ii < originalParams.getCount(); ii++)
        {
            callArgs.add(originalParams[ii]);
        }

        // Call the specific function
        auto callResult =
            builder.emitCallInst(wrappedFuncType->getResultType(), wrapperFunc, callArgs);

        if (resultType->getOp() == kIROp_VoidType)
        {
            builder.emitReturn();
        }
        else
        {
            builder.emitReturn(callResult);
        }

        caseValues.add(funcTag);
        caseBlocks.add(caseBlock);
    }

    // Create flattened case arguments array
    List<IRInst*> flattenedCaseArgs;
    for (Index i = 0; i < caseValues.getCount(); i++)
    {
        flattenedCaseArgs.add(caseValues[i]);
        flattenedCaseArgs.add(caseBlocks[i]);
    }

    // Create an unreachable block for the break block.
    auto unreachableBlock = builder.emitBlock();
    builder.setInsertInto(unreachableBlock);
    builder.emitUnreachable();

    // Go back to entry and emit switch
    builder.setInsertInto(entryBlock);
    builder.emitSwitch(
        idParam,
        unreachableBlock,
        defaultBlock,
        flattenedCaseArgs.getCount(),
        flattenedCaseArgs.getBuffer());

    return func;
}

// Create a function that maps input integers to output integers based on the provided mapping.
IRFunc* createIntegerMappingFunc(IRModule* module, Dictionary<UInt, UInt>& mapping, UInt defaultVal)
{
    // Emit a switch statement with the inputs as case labels and outputs as return values.

    IRBuilder builder(module);

    auto funcType =
        builder.getFuncType(List<IRType*>({builder.getUIntType()}), builder.getUIntType());
    auto func = builder.createFunc();
    builder.setInsertInto(func);
    func->setFullType(funcType);

    auto entryBlock = builder.emitBlock();
    builder.setInsertInto(entryBlock);

    auto param = builder.emitParam(builder.getUIntType());

    // Create default block that returns defaultVal
    auto defaultBlock = builder.emitBlock();
    builder.setInsertInto(defaultBlock);
    builder.emitReturn(builder.getIntValue(builder.getUIntType(), defaultVal));

    // Go back to entry block and create switch
    builder.setInsertInto(entryBlock);

    // Create case blocks for each input table
    List<IRInst*> caseValues;
    List<IRBlock*> caseBlocks;

    for (auto item : mapping)
    {
        // Create case block
        auto caseBlock = builder.emitBlock();
        builder.setInsertInto(caseBlock);
        builder.emitReturn(builder.getIntValue(builder.getUIntType(), item.second));

        caseValues.add(builder.getIntValue(builder.getUIntType(), item.first));
        caseBlocks.add(caseBlock);
    }

    // Create flattened case arguments array
    List<IRInst*> flattenedCaseArgs;
    for (Index i = 0; i < caseValues.getCount(); i++)
    {
        flattenedCaseArgs.add(caseValues[i]);
        flattenedCaseArgs.add(caseBlocks[i]);
    }

    // Emit an unreachable block for the break block.
    auto unreachableBlock = builder.emitBlock();
    builder.setInsertInto(unreachableBlock);
    builder.emitUnreachable();

    // Go back to entry and emit switch
    builder.setInsertInto(entryBlock);
    builder.emitSwitch(
        param,
        unreachableBlock,
        defaultBlock,
        flattenedCaseArgs.getCount(),
        flattenedCaseArgs.getBuffer());

    return func;
}

// This context lowers `GetTagOfElementInSet`,
// `GetTagForSuperSet`, and `GetTagForMappedSet` instructions,
//
struct TagOpsLoweringContext : public InstPassBase
{
    // Our strategy for lowering tag operations is to
    // assign each element to a unique integer ID that is stable
    // across the same module. This is acheived via `getUniqueID`,
    // on the IRBuilder, which uses a dicionary on the module inst
    // to keep track of assignments.
    //
    // Then, tag operations can be lowered to mapping functions that
    // take an integer in and return an integer out, based on the
    // input and output sets (and any other operands)
    //
    TagOpsLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerGetTagForSuperSet(IRGetTagForSuperSet* inst)
    {
        // `GetTagForSuperSet` is a no-op since we want to translate the tag
        // for an element in the sub-set to a tag for the same element in the super-set.
        //
        // Since all elements have a unique ID across the module, this is the identity operation.
        //

        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);
        inst->replaceUsesWith(builder.emitCast(inst->getDataType(), inst->getOperand(0), true));
        inst->removeAndDeallocate();
    }

    void lowerGetTagForMappedSet(IRGetTagForMappedSet* inst)
    {
        // `GetTagForMappedSet` turns into a integer mapping from
        // the unique ID of each input set element to the unique ID of the
        // corresponding element (as determined by witness table lookup) in the destination set.
        //
        auto srcSet = cast<IRWitnessTableSet>(
            cast<IRSetTagType>(inst->getOperand(0)->getDataType())->getOperand(0));
        auto destSet = cast<IRSetBase>(cast<IRSetTagType>(inst->getDataType())->getOperand(0));
        auto key = cast<IRStructKey>(inst->getOperand(1));

        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        Dictionary<UInt, UInt> mapping;
        for (UInt i = 0; i < srcSet->getCount(); i++)
        {
            // Find in destSet
            bool found = false;
            auto srcMappedElement =
                findWitnessTableEntry(cast<IRWitnessTable>(srcSet->getElement(i)), key);
            for (UInt j = 0; j < destSet->getCount(); j++)
            {
                auto destElement = destSet->getElement(j);
                if (srcMappedElement == destElement)
                {
                    found = true;
                    // We rely on the fact that if the element ever appeared in a collection,
                    // it must have been assigned a unique ID.
                    //
                    mapping.add(
                        getUniqueID(&builder, srcSet->getElement(i)),
                        getUniqueID(&builder, destElement));
                    break; // Found the index
                }
            }

            if (!found)
            {
                // destSet must be a super-set
                SLANG_UNEXPECTED("Element not found in destination collection");
            }
        }

        // Create an index mapping func and call that.
        auto mappingFunc = createIntegerMappingFunc(inst->getModule(), mapping, 0);

        auto resultID = builder.emitCallInst(
            inst->getDataType(),
            mappingFunc,
            List<IRInst*>({inst->getOperand(0)}));
        inst->replaceUsesWith(resultID);
        inst->removeAndDeallocate();
    }

    void lowerGetTagOfElementInSet(IRGetTagOfElementInSet* inst)
    {
        // `GetTagOfElementInSet` simply gets replaced by the element's
        //  unique ID (as an integer literal value)
        //
        // Note: the element must be a concrete global inst (cannot by a
        // dynamic value)
        //
        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        auto uniqueId = getUniqueID(&builder, inst->getOperand(0));
        auto resultValue = builder.getIntValue(inst->getDataType(), uniqueId);
        inst->replaceUsesWith(resultValue);
        inst->removeAndDeallocate();
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_GetTagForSuperSet:
            lowerGetTagForSuperSet(as<IRGetTagForSuperSet>(inst));
            break;
        case kIROp_GetTagForMappedSet:
            lowerGetTagForMappedSet(as<IRGetTagForMappedSet>(inst));
            break;
        case kIROp_GetTagOfElementInSet:
            lowerGetTagOfElementInSet(as<IRGetTagOfElementInSet>(inst));
            break;
        default:
            break;
        }
    }

    void processModule()
    {
        processAllInsts([&](IRInst* inst) { return processInst(inst); });
    }
};

struct DispatcherLoweringContext : public InstPassBase
{
    DispatcherLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerGetDispatcher(IRGetDispatcher* dispatcher)
    {
        // Replace the `IRGetDispatcher` with a dispatch function,
        // which takes an extra first parameter for the tag (i.e. ID)
        //
        // We'll also replace the callee in all 'call' insts.
        //
        // The generated dispatch function uses a switch-case to call the
        // appropriate function based on the integer tag. Since tags
        // may not yet be lowered into actual integers, we use `GetTagOfElementInSet`
        // as a placeholder literal.
        //
        // Note that before each function is called, it needs to be wrapped in a
        // method (a 'witness table wrapper') that handles marshalling between the input types
        // to the dispatcher and the actual function types (which may be different)
        //

        auto witnessTableSet = cast<IRWitnessTableSet>(dispatcher->getOperand(0));
        auto key = cast<IRStructKey>(dispatcher->getOperand(1));

        IRBuilder builder(dispatcher->getModule());

        Dictionary<IRInst*, IRInst*> elements;
        forEachInSet(
            witnessTableSet,
            [&](IRInst* table)
            {
                auto tag = builder.emitGetTagOfElementInSet(
                    builder.getSetTagType(witnessTableSet),
                    table,
                    witnessTableSet);
                elements.add(
                    tag,
                    cast<IRFunc>(findWitnessTableEntry(cast<IRWitnessTable>(table), key)));
            });

        if (dispatcher->hasUses() && dispatcher->getDataType() != nullptr)
        {
            auto dispatchFunc =
                createDispatchFunc(cast<IRFuncType>(dispatcher->getDataType()), elements);
            traverseUses(
                dispatcher,
                [&](IRUse* use)
                {
                    if (auto callInst = as<IRCall>(use->getUser()))
                    {
                        // Replace callee with the generated dispatchFunc.
                        if (callInst->getCallee() == dispatcher)
                        {
                            IRBuilder callBuilder(callInst);
                            callBuilder.setInsertBefore(callInst);
                            callBuilder.replaceOperand(callInst->getCalleeUse(), dispatchFunc);
                        }
                    }
                });
        }
    }


    void lowerGetSpecializedDispatcher(IRGetSpecializedDispatcher* dispatcher)
    {
        // Replace the `IRGetSpecializedDispatcher` with a dispatch function,
        // which takes an extra first parameter for the tag (i.e. ID)
        //
        // We'll also replace the callee in all 'call' insts.
        //
        // The logic here is very similar to `lowerGetDispatcher`, except that we need to
        // account for the specialization arguments when creating the dispatch function.
        // We construct an `IRSpecialize` inst around each generic function before dispatching
        // to it.
        //

        auto witnessTableSet = cast<IRWitnessTableSet>(dispatcher->getOperand(0));
        auto key = cast<IRStructKey>(dispatcher->getOperand(1));

        List<IRInst*> specArgs;
        for (UIndex i = 2; i < dispatcher->getOperandCount(); i++)
        {
            specArgs.add(dispatcher->getOperand(i));
        }

        Dictionary<IRInst*, IRInst*> elements;
        IRBuilder builder(dispatcher->getModule());
        forEachInSet(
            witnessTableSet,
            [&](IRInst* table)
            {
                auto generic =
                    cast<IRGeneric>(findWitnessTableEntry(cast<IRWitnessTable>(table), key));

                auto specializedFuncType =
                    (IRType*)specializeGeneric(cast<IRSpecialize>(builder.emitSpecializeInst(
                        builder.getTypeKind(),
                        generic->getDataType(),
                        specArgs.getCount(),
                        specArgs.getBuffer())));

                auto specializedFunc = builder.emitSpecializeInst(
                    specializedFuncType,
                    generic,
                    specArgs.getCount(),
                    specArgs.getBuffer());

                auto singletonTag = builder.emitGetTagOfElementInSet(
                    builder.getSetTagType(witnessTableSet),
                    table,
                    witnessTableSet);

                elements.add(singletonTag, specializedFunc);
            });

        if (dispatcher->hasUses() && dispatcher->getDataType() != nullptr)
        {
            auto dispatchFunc =
                createDispatchFunc(cast<IRFuncType>(dispatcher->getDataType()), elements);
            traverseUses(
                dispatcher,
                [&](IRUse* use)
                {
                    if (auto callInst = as<IRCall>(use->getUser()))
                    {
                        // Replace callee with the generated dispatchFunc.
                        if (callInst->getCallee() == dispatcher)
                        {
                            IRBuilder callBuilder(callInst);
                            callBuilder.setInsertBefore(callInst);
                            callBuilder.replaceOperand(callInst->getCalleeUse(), dispatchFunc);
                        }
                    }
                });
        }
    }

    void processModule()
    {
        processInstsOfType<IRGetDispatcher>(
            kIROp_GetDispatcher,
            [&](IRGetDispatcher* inst) { return lowerGetDispatcher(inst); });

        processInstsOfType<IRGetSpecializedDispatcher>(
            kIROp_GetSpecializedDispatcher,
            [&](IRGetSpecializedDispatcher* inst) { return lowerGetSpecializedDispatcher(inst); });
    }
};

bool lowerDispatchers(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    DispatcherLoweringContext context(module);
    context.processModule();
    return true;
}

// This context lowers `TypeSet` instructions.
struct SetLoweringContext : public InstPassBase
{
    SetLoweringContext(
        IRModule* module,
        TargetProgram* targetProgram,
        DiagnosticSink* sink = nullptr)
        : InstPassBase(module), targetProgram(targetProgram), sink(sink)
    {
    }

    SlangInt tryCalculateAnyValueSize(const HashSet<IRType*>& types)
    {
        SlangInt maxSize = 0;
        for (auto type : types)
        {
            auto size = getAnyValueSize(type);
            if (size > maxSize)
                maxSize = size;

            if (sink && !canTypeBeStored(type))
            {
                sink->diagnose(
                    type->sourceLoc,
                    Slang::Diagnostics::typeCannotBePackedIntoAnyValue,
                    type);
            }
        }

        // Defaults to 0 if any type could not be sized.
        return maxSize;
    }

    IRAnyValueType* createAnyValueType(IRBuilder* builder, const HashSet<IRType*>& types)
    {
        auto size = tryCalculateAnyValueSize(types);
        return builder->getAnyValueType(size);
    }

    bool canTypeBeStored(IRType* concreteType)
    {
        if (!areResourceTypesBindlessOnTarget(targetProgram->getTargetReq()))
        {
            IRType* opaqueType = nullptr;
            if (isOpaqueType(concreteType, &opaqueType))
            {
                return false;
            }
        }

        IRSizeAndAlignment sizeAndAlignment;
        Result result = getNaturalSizeAndAlignment(
            targetProgram->getOptionSet(),
            concreteType,
            &sizeAndAlignment);

        if (SLANG_FAILED(result))
            return false;

        return true;
    }

    void lowerUntaggedUnionType(IRUntaggedUnionType* valueOfSetType)
    {
        // Type collections are replaced with `AnyValueType` large enough to hold
        // any of the types in the collection.
        //

        HashSet<IRType*> types;
        for (UInt i = 0; i < valueOfSetType->getSet()->getCount(); i++)
        {
            if (auto type = as<IRType>(valueOfSetType->getSet()->getElement(i)))
            {
                types.add(type);
            }
        }

        IRBuilder builder(module);
        auto anyValueType = createAnyValueType(&builder, types);
        valueOfSetType->replaceUsesWith(anyValueType);
    }

    void processModule()
    {
        processInstsOfType<IRUntaggedUnionType>(
            kIROp_UntaggedUnionType,
            [&](IRUntaggedUnionType* inst) { return lowerUntaggedUnionType(inst); });
    }

private:
    DiagnosticSink* sink;
    TargetProgram* targetProgram;
};

// Lower `UntaggedUnionType(TypeSet(...))` instructions by replacing them with
// appropriate `AnyValueType` instructions.
//
void lowerUntaggedUnionTypes(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    SetLoweringContext context(module, targetProgram, sink);
    context.processModule();
}

// This context lowers `IRGetTagFromSequentialID` and `IRGetSequentialIDFromTag` instructions.
// Note: This pass requires that sequential ID decorations have been created for all witness
// tables.
//
struct SequentialIDTagLoweringContext : public InstPassBase
{
    SequentialIDTagLoweringContext(Linkage* linkage, IRModule* module)
        : InstPassBase(module), m_linkage(linkage)
    {
    }
    void lowerGetTagFromSequentialID(IRGetTagFromSequentialID* inst)
    {
        // We use the result type to figure out the destination collection
        // for which we need to generate the tag.
        //
        // We then replace this with call into an integer mapping function,
        // which takes the sequential ID and returns the local ID (i.e. tag).
        //
        // To construct, the mapping, we lookup the sequential ID decorator on
        // each element of the destination collection, and map it to the table's
        // operand index in the collection.
        //

        // We use the result type and the type of the operand
        auto srcSeqID = inst->getOperand(1);

        Dictionary<UInt, UInt> mapping;

        // Map from sequential ID to unique ID
        auto destSet = cast<IRSetTagType>(inst->getDataType())->getSet();

        IRBuilder builder(inst);
        builder.setInsertAfter(inst);

        forEachInSet(
            destSet,
            [&](IRInst* table)
            {
                // Get unique ID for the witness table
                auto outputId = builder.getUniqueID(table);
                auto seqDecoration = table->findDecoration<IRSequentialIDDecoration>();
                if (seqDecoration)
                {
                    auto inputId = seqDecoration->getSequentialID();
                    mapping[inputId] = outputId; // Map ID to itself for now
                }
            });

        // By default, use the tag for the largest available sequential ID.
        UInt defaultSeqID = 0;
        for (auto [inputId, outputId] : mapping)
        {
            if (inputId > defaultSeqID)
                defaultSeqID = inputId;
        }

        auto translatedID = builder.emitCallInst(
            inst->getDataType(),
            createIntegerMappingFunc(builder.getModule(), mapping, mapping[defaultSeqID]),
            List<IRInst*>({srcSeqID}));

        inst->replaceUsesWith(translatedID);
        inst->removeAndDeallocate();
    }


    void lowerGetSequentialIDFromTag(IRGetSequentialIDFromTag* inst)
    {
        // Similar logic to the `GetTagFromSequentialID` case, except that
        // we reverse the mapping.
        //

        SLANG_UNUSED(cast<IRInterfaceType>(inst->getOperand(0)));
        auto srcTagInst = inst->getOperand(1);

        Dictionary<UInt, UInt> mapping;

        // Map from sequential ID to unique ID
        auto destSet = cast<IRSetTagType>(srcTagInst->getDataType())->getSet();

        IRBuilder builder(inst);
        builder.setInsertAfter(inst);

        forEachInSet(
            destSet,
            [&](IRInst* table)
            {
                // Get unique ID for the witness table
                SLANG_UNUSED(cast<IRWitnessTable>(table));
                auto outputId = builder.getUniqueID(table);
                auto seqDecoration = table->findDecoration<IRSequentialIDDecoration>();
                if (seqDecoration)
                {
                    auto inputId = seqDecoration->getSequentialID();
                    mapping.add({outputId, inputId});
                }
            });

        auto translatedID = builder.emitCallInst(
            inst->getDataType(),
            createIntegerMappingFunc(builder.getModule(), mapping, 0),
            List<IRInst*>({srcTagInst}));

        inst->replaceUsesWith(translatedID);
        inst->removeAndDeallocate();
    }


    // Ensures every witness table object has been assigned a sequential ID.
    // All witness tables will have a SequentialID decoration after this function is run.
    // The sequantial ID in the decoration will be the same as the one specified in the Linkage.
    // Otherwise, a new ID will be generated and assigned to the witness table object, and
    // the sequantial ID map in the Linkage will be updated to include the new ID, so they
    // can be looked up by the user via future Slang API calls.
    void ensureWitnessTableSequentialIDs()
    {
        StringBuilder generatedMangledName;

        auto linkage = getLinkage();
        for (auto inst : module->getGlobalInsts())
        {
            if (inst->getOp() == kIROp_WitnessTable)
            {
                UnownedStringSlice witnessTableMangledName;
                if (auto instLinkage = inst->findDecoration<IRLinkageDecoration>())
                {
                    witnessTableMangledName = instLinkage->getMangledName();
                }
                else
                {
                    auto witnessTableType = as<IRWitnessTableType>(inst->getDataType());

                    if (witnessTableType && witnessTableType->getConformanceType() == nullptr)
                    {
                        // Ignore witness tables that represent 'none' for optional witness table
                        // types.
                        continue;
                    }

                    if (witnessTableType && witnessTableType->getConformanceType()
                                                ->findDecoration<IRSpecializeDecoration>())
                    {
                        // The interface is for specialization only, it would be an error if dynamic
                        // dispatch is used through the interface. Skip assigning ID for the witness
                        // table.
                        continue;
                    }

                    // generate a unique linkage for it.
                    static int32_t uniqueId = 0;
                    uniqueId++;
                    if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
                    {
                        generatedMangledName << nameHint->getName();
                    }
                    generatedMangledName << "_generated_witness_uuid_" << uniqueId;
                    witnessTableMangledName = generatedMangledName.getUnownedSlice();
                }

                // If the inst already has a SequentialIDDecoration, stop now.
                if (inst->findDecoration<IRSequentialIDDecoration>())
                    continue;

                // Get a sequential ID for the witness table using the map from the Linkage.
                uint32_t seqID = 0;
                if (!linkage->mapMangledNameToRTTIObjectIndex.tryGetValue(
                        witnessTableMangledName,
                        seqID))
                {
                    auto interfaceType =
                        cast<IRWitnessTableType>(inst->getDataType())->getConformanceType();
                    if (as<IRInterfaceType>(interfaceType))
                    {
                        auto interfaceLinkage =
                            interfaceType->findDecoration<IRLinkageDecoration>();
                        SLANG_ASSERT(
                            interfaceLinkage && "An interface type does not have a linkage,"
                                                "but a witness table associated with it has one.");
                        auto interfaceName = interfaceLinkage->getMangledName();
                        auto idAllocator =
                            linkage->mapInterfaceMangledNameToSequentialIDCounters.tryGetValue(
                                interfaceName);
                        if (!idAllocator)
                        {
                            linkage->mapInterfaceMangledNameToSequentialIDCounters[interfaceName] =
                                0;
                            idAllocator =
                                linkage->mapInterfaceMangledNameToSequentialIDCounters.tryGetValue(
                                    interfaceName);
                        }
                        seqID = *idAllocator;
                        ++(*idAllocator);
                    }
                    else
                    {
                        // NoneWitness, has special ID of -1.
                        seqID = uint32_t(-1);
                    }
                    linkage->mapMangledNameToRTTIObjectIndex[witnessTableMangledName] = seqID;
                }

                // Add a decoration to the inst.
                IRBuilder builder(module);
                builder.setInsertBefore(inst);
                builder.addSequentialIDDecoration(inst, seqID);
            }
        }
    }

    void processModule()
    {
        ensureWitnessTableSequentialIDs();

        processInstsOfType<IRGetTagFromSequentialID>(
            kIROp_GetTagFromSequentialID,
            [&](IRGetTagFromSequentialID* inst) { return lowerGetTagFromSequentialID(inst); });

        processInstsOfType<IRGetSequentialIDFromTag>(
            kIROp_GetSequentialIDFromTag,
            [&](IRGetSequentialIDFromTag* inst) { return lowerGetSequentialIDFromTag(inst); });
    }

    Linkage* getLinkage() { return m_linkage; }

private:
    Linkage* m_linkage;
};

void lowerSequentialIDTagCasts(IRModule* module, Linkage* linkage, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    SequentialIDTagLoweringContext context(linkage, module);
    context.processModule();
}

void lowerTagInsts(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    TagOpsLoweringContext tagContext(module);
    tagContext.processModule();
}

// This context lowers `IRSetTagType` instructions, by replacing
// them with a suitable integer type.
struct TagTypeLoweringContext : public InstPassBase
{
    TagTypeLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void processModule()
    {
        processInstsOfType<IRSetTagType>(
            kIROp_SetTagType,
            [&](IRSetTagType* inst)
            {
                IRBuilder builder(inst->getModule());
                inst->replaceUsesWith(builder.getUIntType());
            });
    }
};

void lowerTagTypes(IRModule* module)
{
    TagTypeLoweringContext context(module);
    context.processModule();
}

bool isEffectivelyComPtrType(IRType* type)
{
    if (!type)
        return false;
    if (type->findDecoration<IRComInterfaceDecoration>() || type->getOp() == kIROp_ComPtrType)
    {
        return true;
    }
    if (auto witnessTableType = as<IRWitnessTableTypeBase>(type))
    {
        return isComInterfaceType((IRType*)witnessTableType->getConformanceType());
    }
    if (auto ptrType = as<IRNativePtrType>(type))
    {
        auto valueType = ptrType->getValueType();
        return valueType->findDecoration<IRComInterfaceDecoration>() != nullptr;
    }

    return false;
}

// This context lowers `CastInterfaceToTaggedUnionPtr` and
// `CastTaggedUnionToInterfacePtr` by finding all `IRLoad` and
// `IRStore` uses of these insts, and upcasting the tagged-union
// tuple to the the interface-based tuple (of the loaded inst or before
// storing the val, as necessary)
//
struct TaggedUnionLoweringContext : public InstPassBase
{
    TaggedUnionLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    IRInst* convertToTaggedUnion(
        IRBuilder* builder,
        IRInst* val,
        IRInst* interfaceType,
        IRInst* targetType)
    {
        auto baseInterfaceValue = val;
        auto witnessTable = builder->emitExtractExistentialWitnessTable(baseInterfaceValue);
        auto tableID = builder->emitGetSequentialIDInst(witnessTable);

        auto taggedUnionTupleType = cast<IRTupleType>(targetType);

        List<IRInst*> getTagOperands;
        getTagOperands.add(interfaceType);
        getTagOperands.add(tableID);
        auto tableTag = builder->emitIntrinsicInst(
            (IRType*)taggedUnionTupleType->getOperand(0),
            kIROp_GetTagFromSequentialID,
            getTagOperands.getCount(),
            getTagOperands.getBuffer());

        return builder->emitMakeTuple(
            {tableTag,
             builder->emitReinterpret(
                 (IRType*)taggedUnionTupleType->getOperand(1),
                 builder->emitExtractExistentialValue(
                     (IRType*)builder->emitExtractExistentialType(baseInterfaceValue),
                     baseInterfaceValue))});
    }

    void lowerCastInterfaceToTaggedUnionPtr(IRCastInterfaceToTaggedUnionPtr* inst)
    {
        // `CastInterfaceToTaggedUnionPtr` is used to 'reinterpret' a pointer to an interface-typed
        // location into a tagged union type. Usually this is to avoid changing the type of the
        // base location because it is externally visible, and to avoid touching the external layout
        // of the interface type.
        //
        // To lower this, we won't actually change the pointer or the base location, but instead
        // rewrite all loads and stores out of this pointer by converting the existential into a
        // tagged union tuple.
        //
        // e.g.
        //
        //   let basePtr : PtrType(InterfaceType(I)) = /* ... */;
        //   let tuPtr : PtrType(TaggedUnionType(types, tables)) =
        //       CastInterfaceToTaggedUnionPtr(basePtr);
        //   let loadedVal : TaggedUnionType(...) = Load(tuPtr);
        //
        // becomes
        //
        //   let basePtr : PtrType(InterfaceType(I)) = /* ... */;
        //   let intermediateVal : InterfaceType(I) = Load(basePtr);
        //   let loadedTableID : TagType(tables) =
        //      GetTagFromSequentialID(
        //         InterfaceType(I),
        //         GetSequentialID(
        //           ExtractExistentialWitnessTable(intermediateVal)));
        //   let loadedVal : types = ExtractExistentialValue(intermediateVal);
        //   let loadedTuple : TupleType(TagType(tables), types) =
        //      MakeTuple(loadedTableID, loadedVal);
        //
        // The logic is similar for StructuredBufferLoad and RWStructuredBufferLoad,
        // but the operands structure is slightly different.
        //

        traverseUses(
            inst,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                switch (user->getOp())
                {
                case kIROp_Load:
                    {
                        auto baseInterfacePtr = inst->getOperand(0);
                        auto baseInterfaceType = as<IRInterfaceType>(
                            as<IRPtrTypeBase>(baseInterfacePtr->getDataType())->getValueType());

                        // Rewrite the load to use the original ptr and load
                        // an interface-typed object.
                        //
                        IRBuilder builder(module);
                        builder.setInsertAfter(user);
                        builder.replaceOperand(user->getOperands() + 0, baseInterfacePtr);
                        builder.replaceOperand(&user->typeUse, baseInterfaceType);

                        // Then, we'll rewrite it.
                        List<IRUse*> oldUses;
                        traverseUses(user, [&](IRUse* oldUse) { oldUses.add(oldUse); });

                        auto newVal = convertToTaggedUnion(
                            &builder,
                            user,
                            baseInterfaceType,
                            as<IRPtrTypeBase>(inst->getDataType())->getValueType());
                        for (auto oldUse : oldUses)
                        {
                            builder.replaceOperand(oldUse, newVal);
                        }
                        break;
                    }
                case kIROp_StructuredBufferLoad:
                case kIROp_RWStructuredBufferLoad:
                    {
                        auto baseInterfacePtr = inst->getOperand(0);
                        auto baseInterfaceType =
                            as<IRInterfaceType>((baseInterfacePtr->getDataType())->getOperand(0));

                        IRBuilder builder(module);
                        builder.setInsertAfter(user);
                        builder.replaceOperand(user->getOperands() + 0, baseInterfacePtr);
                        builder.replaceOperand(&user->typeUse, baseInterfaceType);

                        // Then, we'll rewrite it.
                        List<IRUse*> oldUses;
                        traverseUses(user, [&](IRUse* oldUse) { oldUses.add(oldUse); });

                        auto newVal = convertToTaggedUnion(
                            &builder,
                            user,
                            baseInterfaceType,
                            as<IRPtrTypeBase>(inst->getDataType())->getValueType());
                        for (auto oldUse : oldUses)
                        {
                            builder.replaceOperand(oldUse, newVal);
                        }
                        break;
                    }
                default:
                    SLANG_UNEXPECTED("Unexpected user of CastInterfaceToTaggedUnionPtr");
                }
            });

        SLANG_ASSERT(!inst->hasUses());
        inst->removeAndDeallocate();
    }

    IRType* lowerTaggedUnionType(IRTaggedUnionType* taggedUnion)
    {
        // Replace `TaggedUnionType(typeSet, tableSet)` with
        // `TupleType(SetTagType(tableSet), typeSet)`
        //
        // Unless the collection has a single element, in which case we
        // replace it with `TupleType(SetTagType(tableSet), elementType)`
        //
        // We still maintain a tuple type (even though it's not really necesssary) to avoid
        // breaking any operations that assumed this is a tuple.
        // In the single element case, the tuple should be optimized away.
        //

        IRBuilder builder(module);
        builder.setInsertInto(module);

        auto typeSet = builder.getUntaggedUnionType(taggedUnion->getTypeSet());
        auto tableSet = taggedUnion->getWitnessTableSet();

        if (taggedUnion->getTypeSet()->isSingleton())
            return builder.getTupleType(List<IRType*>(
                {(IRType*)builder.getSetTagType(tableSet),
                 (IRType*)taggedUnion->getTypeSet()->getElement(0)}));

        return builder.getTupleType(
            List<IRType*>({(IRType*)builder.getSetTagType(tableSet), (IRType*)typeSet}));
    }

    bool lowerGetValueFromTaggedUnion(IRGetValueFromTaggedUnion* inst)
    {
        // We replace `GetValueFromTaggedUnion(taggedUnionVal)` with
        // `GetTupleElement(taggedUnionVal, 1)`
        //

        IRBuilder builder(module);
        builder.setInsertAfter(inst);

        auto tupleVal = inst->getOperand(0);
        inst->replaceUsesWith(builder.emitGetTupleElement(
            (IRType*)as<IRTupleType>(tupleVal->getDataType())->getOperand(1),
            tupleVal,
            1));
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerGetTagFromTaggedUnion(IRGetTagFromTaggedUnion* inst)
    {
        // We replace `GetTagFromTaggedUnion(taggedUnionVal)` with
        // `GetTupleElement(taggedUnionVal, 0)`
        //

        IRBuilder builder(module);
        builder.setInsertAfter(inst);

        auto tupleVal = inst->getOperand(0);
        inst->replaceUsesWith(builder.emitGetTupleElement(
            (IRType*)as<IRTupleType>(tupleVal->getDataType())->getOperand(0),
            tupleVal,
            0));
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerGetTypeTagFromTaggedUnion(IRGetTypeTagFromTaggedUnion* inst)
    {
        IRBuilder builder(module);
        builder.setInsertAfter(inst);
        inst->replaceUsesWith(builder.emitPoison(inst->getDataType()));
        return true;
    }


    bool lowerMakeTaggedUnion(IRMakeTaggedUnion* inst)
    {
        // We replace `MakeTaggedUnion(typeTag, witnessTableTag, val)` with `MakeTuple(tag, val)`
        //

        IRBuilder builder(module);
        builder.setInsertAfter(inst);

        auto tuTupleType = cast<IRTupleType>(inst->getDataType());

        // The current lowering logic is only for bounded tagged unions (finite sets)
        SLANG_ASSERT(!as<IRSetTagType>(tuTupleType->getOperand(0))->getSet()->isUnbounded());

        auto typeTag = inst->getOperand(0);
        // We'll ignore the type tag, since the table is the only thing we need.
        // for the bounded case.
        SLANG_UNUSED(typeTag);

        auto witnessTableTag = inst->getOperand(1);
        auto val = inst->getOperand(2);
        inst->replaceUsesWith(
            builder.emitMakeTuple((IRType*)inst->getDataType(), {witnessTableTag, val}));
        inst->removeAndDeallocate();
        return true;
    }

    bool processModule()
    {
        // First, we'll lower all TaggedUnionType insts
        // into tuples.
        //
        processInstsOfType<IRTaggedUnionType>(
            kIROp_TaggedUnionType,
            [&](IRTaggedUnionType* inst)
            {
                inst->replaceUsesWith(lowerTaggedUnionType(inst));
                inst->removeAndDeallocate();
            });

        // TODO: Is this repeated scanning of the module inefficient?
        // It feels like this form could be very efficient if it's automatically
        // 'fused' together.
        //
        processInstsOfType<IRGetTagFromTaggedUnion>(
            kIROp_GetTagFromTaggedUnion,
            [&](IRGetTagFromTaggedUnion* inst) { return lowerGetTagFromTaggedUnion(inst); });

        processInstsOfType<IRGetTypeTagFromTaggedUnion>(
            kIROp_GetTypeTagFromTaggedUnion,
            [&](IRGetTypeTagFromTaggedUnion* inst)
            { return lowerGetTypeTagFromTaggedUnion(inst); });

        processInstsOfType<IRGetValueFromTaggedUnion>(
            kIROp_GetValueFromTaggedUnion,
            [&](IRGetValueFromTaggedUnion* inst) { return lowerGetValueFromTaggedUnion(inst); });

        processInstsOfType<IRMakeTaggedUnion>(
            kIROp_MakeTaggedUnion,
            [&](IRMakeTaggedUnion* inst) { return lowerMakeTaggedUnion(inst); });

        // Then, convert any loads/stores from reinterpreted pointers.
        bool hasCastInsts = false;
        processInstsOfType<IRCastInterfaceToTaggedUnionPtr>(
            kIROp_CastInterfaceToTaggedUnionPtr,
            [&](IRCastInterfaceToTaggedUnionPtr* inst)
            {
                hasCastInsts = true;
                return lowerCastInterfaceToTaggedUnionPtr(inst);
            });

        return hasCastInsts;
    }
};

bool lowerTaggedUnionTypes(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);

    TaggedUnionLoweringContext context(module);
    return context.processModule();
}

void lowerIsTypeInsts(IRModule* module)
{
    InstPassBase pass(module);
    pass.processInstsOfType<IRIsType>(
        kIROp_IsType,
        [&](IRIsType* inst)
        {
            auto witnessTableType =
                as<IRWitnessTableTypeBase>(inst->getValueWitness()->getDataType());
            if (witnessTableType &&
                isComInterfaceType((IRType*)witnessTableType->getConformanceType()))
                return;
            IRBuilder builder(module);
            builder.setInsertBefore(inst);
            auto eqlInst = builder.emitEql(
                builder.emitGetSequentialIDInst(inst->getValueWitness()),
                builder.emitGetSequentialIDInst(inst->getTargetWitness()));
            inst->replaceUsesWith(eqlInst);
            inst->removeAndDeallocate();
        });
}

struct ExistentialLoweringContext : public InstPassBase
{
    TargetProgram* targetProgram;

    ExistentialLoweringContext(IRModule* module, TargetProgram* targetProgram)
        : InstPassBase(module), targetProgram(targetProgram)
    {
    }

    bool _canReplace(IRUse* use)
    {
        switch (use->getUser()->getOp())
        {
        case kIROp_WitnessTableIDType:
        case kIROp_WitnessTableType:
        case kIROp_RTTIPointerType:
        case kIROp_RTTIHandleType:
        case kIROp_ComPtrType:
        case kIROp_NativePtrType:
            {
                // Don't replace
                return false;
            }
        case kIROp_ThisType:
            {
                // Appears replacable.
                break;
            }
        case kIROp_PtrType:
            {
                // We can have ** and ComPtr<T>*.
                // If it's a pointer type it could be because it is a global.
                break;
            }
        default:
            break;
        }
        return true;
    }

    // Replace all WitnessTableID type or RTTIHandleType with `uint2`.
    void lowerHandleTypes()
    {
        List<IRInst*> instsToRemove;
        for (auto inst : module->getGlobalInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_WitnessTableIDType:
                if (isComInterfaceType((IRType*)inst->getOperand(0)))
                    continue;
                // fall through
            case kIROp_RTTIHandleType:
                {
                    IRBuilder builder(module);
                    builder.setInsertBefore(inst);
                    auto uint2Type = builder.getVectorType(
                        builder.getUIntType(),
                        builder.getIntValue(builder.getIntType(), 2));
                    inst->replaceUsesWith(uint2Type);
                    instsToRemove.add(inst);
                }
                break;
            }
        }
        for (auto inst : instsToRemove)
            inst->removeAndDeallocate();
    }

    IRInst* lowerInterfaceType(IRInst* interfaceType)
    {
        if (isComInterfaceType((IRType*)interfaceType))
            return (IRType*)interfaceType;

        IRBuilder builder(module);
        if (isBuiltin(interfaceType))
            return (IRType*)builder.getIntValue(builder.getIntType(), 0);

        IRIntegerValue anyValueSize = 0;
        if (auto decor = interfaceType->findDecoration<IRAnyValueSizeDecoration>())
        {
            anyValueSize = decor->getSize();
        }

        auto anyValueType = builder.getAnyValueType(anyValueSize);
        auto witnessTableType = builder.getWitnessTableIDType((IRType*)interfaceType);
        auto rttiType = builder.getRTTIHandleType();

        return builder.getTupleType(rttiType, witnessTableType, anyValueType);
    }

    IRInst* lowerBoundInterfaceType(IRBoundInterfaceType* boundInterfaceType)
    {
        IRBuilder builder(module);

        auto payloadType = boundInterfaceType->getConcreteType();
        auto witnessTableType = builder.getWitnessTableIDType(
            (IRType*)as<IRWitnessTable>(boundInterfaceType->getWitnessTable())
                ->getConformanceType());
        auto rttiType = builder.getRTTIHandleType();
        auto interfaceType = boundInterfaceType->getInterfaceType();

        IRIntegerValue anyValueSize = 16;
        if (auto decor = interfaceType->findDecoration<IRAnyValueSizeDecoration>())
        {
            anyValueSize = decor->getSize();
        }

        auto anyValueType = builder.getAnyValueType(anyValueSize);

        IRSizeAndAlignment sizeAndAlignment;
        Result result = getNaturalSizeAndAlignment(
            targetProgram->getOptionSet(),
            payloadType,
            &sizeAndAlignment);
        if (SLANG_FAILED(result) || sizeAndAlignment.size > anyValueSize)
        {
            return builder.getTupleType(
                rttiType,
                witnessTableType,
                builder.getPseudoPtrType(payloadType),
                anyValueType);
        }
        else
        {
            // Regular case (lower in the same way as unbound interface types)
            return builder.getTupleType(rttiType, witnessTableType, anyValueType);
        }
    }

    bool lowerExtractExistentialType(IRExtractExistentialType* inst)
    {
        // Replace with extraction of the value type from the tagged-union tuple.
        //

        IRBuilder builder(module);
        builder.setInsertAfter(inst);

        if (auto tupleType = as<IRTupleType>(inst->getOperand(0)->getDataType()))
        {
            inst->replaceUsesWith(builder.emitGetTupleElement(
                (IRType*)tupleType->getOperand(0),
                inst->getOperand(0),
                0));
            inst->removeAndDeallocate();
        }
        else if (isEffectivelyComPtrType((IRType*)inst->getOperand(0)->getDataType()))
        {
            inst->replaceUsesWith(inst->getOperand(0));
            inst->removeAndDeallocate();
        }
        return true;
    }

    bool lowerExtractExistentialWitnessTable(IRExtractExistentialWitnessTable* inst)
    {
        // Replace with extraction of the value from the tagged-union tuple.
        //

        IRBuilder builder(module);
        builder.setInsertAfter(inst);

        if (auto tupleType = as<IRTupleType>(inst->getOperand(0)->getDataType()))
        {
            inst->replaceUsesWith(builder.emitGetTupleElement(
                (IRType*)tupleType->getOperand(1),
                inst->getOperand(0),
                1));
            inst->removeAndDeallocate();
            return true;
        }
        else if (isEffectivelyComPtrType((IRType*)inst->getOperand(0)->getDataType()))
        {
            inst->replaceUsesWith(inst->getOperand(0));
            inst->removeAndDeallocate();
            return true;
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected type for ExtractExistentialWitnessTable operand");
        }
    }

    bool lowerGetValueFromBoundInterface(IRGetValueFromBoundInterface* inst)
    {
        // Replace with extraction of the value from the tagged-union tuple.
        //

        IRBuilder builder(module);
        builder.setInsertAfter(inst);

        auto tupleType = as<IRTupleType>(inst->getOperand(0)->getDataType());

        if (as<IRPseudoPtrType>(tupleType->getOperand(2)))
        {
            inst->replaceUsesWith(builder.emitGetTupleElement(
                (IRType*)tupleType->getOperand(2),
                inst->getOperand(0),
                2));
            inst->removeAndDeallocate();
            return true;
        }
        else
        {
            inst->replaceUsesWith(builder.emitUnpackAnyValue(
                inst->getDataType(),
                builder.emitGetTupleElement(
                    (IRType*)tupleType->getOperand(2),
                    inst->getOperand(0),
                    2)));
            inst->removeAndDeallocate();
            return true;
        }
    }

    bool lowerExtractExistentialValue(IRExtractExistentialValue* inst)
    {
        // Replace with extraction of the value from the tagged-union tuple.
        //

        IRBuilder builder(module);
        builder.setInsertAfter(inst);

        if (auto tupleType = as<IRTupleType>(inst->getOperand(0)->getDataType()))
        {
            inst->replaceUsesWith(builder.emitGetTupleElement(
                (IRType*)tupleType->getOperand(2),
                inst->getOperand(0),
                2));
            inst->removeAndDeallocate();
            return true;
        }
        else if (isEffectivelyComPtrType((IRType*)inst->getOperand(0)->getDataType()))
        {
            inst->replaceUsesWith(inst->getOperand(0));
            inst->removeAndDeallocate();
            return true;
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected type for ExtractExistentialValue operand");
        }
    }

    bool processGetSequentialIDInst(IRGetSequentialID* inst)
    {
        // If the operand is a witness table, it is already replaced with a uint2
        // at this point, where the first element in the uint2 is the id of the
        // witness table.
        IRBuilder builder(module);
        builder.setInsertBefore(inst);

        if (auto table = as<IRWitnessTable>(inst->getRTTIOperand()))
        {
            auto seqDecoration = table->findDecoration<IRSequentialIDDecoration>();
            SLANG_ASSERT(seqDecoration && "Witness table missing SequentialID decoration");
            auto id = builder.getIntValue(builder.getUIntType(), seqDecoration->getSequentialID());
            inst->replaceUsesWith(id);
            inst->removeAndDeallocate();
            return true;
        }


        UInt index = 0;
        auto id = builder.emitSwizzle(builder.getUIntType(), inst->getRTTIOperand(), 1, &index);
        inst->replaceUsesWith(id);
        inst->removeAndDeallocate();
        return true;
    }

    void processModule()
    {
        // Then, start lowering the remaining non-COM/non-Builtin interface types
        // At this point, we should only bea dealing with public facing uses of
        // interface types (which must lower into a 3-tuple of RTTI, witness table ID, AnyValue)
        //
        processInstsOfType<IRInterfaceType>(
            kIROp_InterfaceType,
            [&](IRInterfaceType* inst)
            {
                IRBuilder builder(module);
                builder.setInsertInto(module);
                if (auto loweredInterfaceType = lowerInterfaceType(inst))
                {
                    if (loweredInterfaceType != inst)
                    {

                        traverseUses(
                            inst,
                            [&](IRUse* use)
                            {
                                if (_canReplace(use))
                                    builder.replaceOperand(use, loweredInterfaceType);
                            });
                    }
                }
            });

        processInstsOfType<IRBoundInterfaceType>(
            kIROp_BoundInterfaceType,
            [&](IRBoundInterfaceType* inst)
            {
                IRBuilder builder(module);
                builder.setInsertInto(module);
                if (auto loweredBoundInterfaceType = lowerBoundInterfaceType(inst))
                {
                    if (loweredBoundInterfaceType != inst)
                    {
                        traverseUses(
                            inst,
                            [&](IRUse* use)
                            {
                                if (_canReplace(use))
                                    builder.replaceOperand(use, loweredBoundInterfaceType);
                            });
                    }
                }
            });

        // Replace any other uses with dummy value 0.
        // TODO: Ideally, we should replace it with IRPoison..
        {
            IRBuilder builder(module);
            builder.setInsertInto(module);
            auto dummyInterfaceObj = builder.getIntValue(builder.getIntType(), 0);
            processInstsOfType<IRInterfaceType>(
                kIROp_InterfaceType,
                [&](IRInterfaceType* inst)
                {
                    if (!isComInterfaceType((IRType*)inst))
                    {
                        inst->replaceUsesWith(dummyInterfaceObj);
                        inst->removeAndDeallocate();
                    }
                });
        }

        processAllInsts(
            [&](IRInst* inst)
            {
                switch (inst->getOp())
                {
                case kIROp_ExtractExistentialType:
                    lowerExtractExistentialType(cast<IRExtractExistentialType>(inst));
                    break;
                case kIROp_ExtractExistentialValue:
                    lowerExtractExistentialValue(cast<IRExtractExistentialValue>(inst));
                    break;
                case kIROp_ExtractExistentialWitnessTable:
                    lowerExtractExistentialWitnessTable(
                        cast<IRExtractExistentialWitnessTable>(inst));
                    break;
                case kIROp_GetValueFromBoundInterface:
                    lowerGetValueFromBoundInterface(cast<IRGetValueFromBoundInterface>(inst));
                    break;
                }
            });

        lowerIsTypeInsts(module);

        processInstsOfType<IRGetSequentialID>(
            kIROp_GetSequentialID,
            [&](IRGetSequentialID* inst) { return processGetSequentialIDInst(inst); });

        lowerHandleTypes();
    }
};

bool lowerExistentials(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    ExistentialLoweringContext context(module, targetProgram);
    context.processModule();
    return true;
};

}; // namespace Slang