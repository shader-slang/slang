#include "slang-ir-lower-typeflow-insts.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize.h"
#include "slang-ir-typeflow-collection.h"
#include "slang-ir-util.h"
#include "slang-ir-witness-table-wrapper.h"
#include "slang-ir.h"

namespace Slang
{
SlangInt calculateAnyValueSize(const HashSet<IRType*>& types)
{
    SlangInt maxSize = 0;
    for (auto type : types)
    {
        auto size = getAnyValueSize(type);
        if (size > maxSize)
            maxSize = size;
    }
    return maxSize;
}

IRAnyValueType* createAnyValueType(IRBuilder* builder, const HashSet<IRType*>& types)
{
    auto size = calculateAnyValueSize(types);
    return builder->getAnyValueType(size);
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

// This context lowers `GetTagOfElementInCollection`,
// `GetTagForSuperCollection`, and `GetTagForMappedCollection` instructions,
//
struct TagOpsLoweringContext : public InstPassBase
{
    TagOpsLoweringContext(IRModule* module)
        : InstPassBase(module), cBuilder(module)
    {
    }

    void lowerGetTagForSuperCollection(IRGetTagForSuperCollection* inst)
    {
        inst->replaceUsesWith(inst->getOperand(0));
        inst->removeAndDeallocate();
    }

    void lowerGetTagForMappedCollection(IRGetTagForMappedCollection* inst)
    {
        auto srcCollection = cast<IRWitnessTableCollection>(
            cast<IRCollectionTagType>(inst->getOperand(0)->getDataType())->getOperand(0));
        auto destCollection =
            cast<IRCollectionBase>(cast<IRCollectionTagType>(inst->getDataType())->getOperand(0));
        auto key = cast<IRStructKey>(inst->getOperand(1));

        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        Dictionary<UInt, UInt> mapping;
        for (UInt i = 0; i < srcCollection->getCount(); i++)
        {
            // Find in destCollection
            bool found = false;
            auto srcMappedElement =
                findWitnessTableEntry(cast<IRWitnessTable>(srcCollection->getElement(i)), key);
            for (UInt j = 0; j < destCollection->getCount(); j++)
            {
                auto destElement = destCollection->getElement(j);
                if (srcMappedElement == destElement)
                {
                    found = true;
                    // We rely on the fact that if the element ever appeared in a collection,
                    // it must have been assigned a unique ID.
                    //
                    mapping.add(
                        cBuilder.getUniqueID(srcCollection->getElement(i)),
                        cBuilder.getUniqueID(destElement));
                    break; // Found the index
                }
            }

            if (!found)
            {
                // destCollection must be a super-set
                SLANG_UNEXPECTED("Element not found in destination collection");
            }
        }

        // Create an index mapping func and call that
        auto mappingFunc = createIntegerMappingFunc(inst->getModule(), mapping, 0);

        auto resultID = builder.emitCallInst(
            inst->getDataType(),
            mappingFunc,
            List<IRInst*>({inst->getOperand(0)}));
        inst->replaceUsesWith(resultID);
        inst->removeAndDeallocate();
    }

    void lowerGetTagOfElementInCollection(IRGetTagOfElementInCollection* inst)
    {
        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        auto uniqueId = cBuilder.getUniqueID(inst->getOperand(0));
        auto resultValue = builder.getIntValue(inst->getDataType(), uniqueId);
        inst->replaceUsesWith(resultValue);
        inst->removeAndDeallocate();
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_GetTagForSuperCollection:
            lowerGetTagForSuperCollection(as<IRGetTagForSuperCollection>(inst));
            break;
        case kIROp_GetTagForMappedCollection:
            lowerGetTagForMappedCollection(as<IRGetTagForMappedCollection>(inst));
            break;
        case kIROp_GetTagOfElementInCollection:
            lowerGetTagOfElementInCollection(as<IRGetTagOfElementInCollection>(inst));
            break;
        default:
            break;
        }
    }

    void processModule()
    {
        processAllInsts([&](IRInst* inst) { return processInst(inst); });
    }

    CollectionBuilder cBuilder;
};

struct DispatcherLoweringContext : public InstPassBase
{
    DispatcherLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerGetSpecializedDispatcher(IRGetSpecializedDispatcher* dispatcher)
    {
        // Replace the `IRGetSpecializedDispatcher` with a dispatch function,
        // which takes an extra first parameter for the tag (i.e. ID)
        //
        // We'll also replace the callee in all 'call' insts.
        //

        auto witnessTableCollection = cast<IRWitnessTableCollection>(dispatcher->getOperand(0));
        auto key = cast<IRStructKey>(dispatcher->getOperand(1));

        List<IRInst*> specArgs;
        for (UIndex i = 2; i < dispatcher->getOperandCount(); i++)
        {
            specArgs.add(dispatcher->getOperand(i));
        }

        Dictionary<IRInst*, IRInst*> elements;
        IRBuilder builder(dispatcher->getModule());
        forEachInCollection(
            witnessTableCollection,
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

                auto singletonTag = builder.emitGetTagOfElementInCollection(
                    builder.getCollectionTagType(witnessTableCollection),
                    table,
                    witnessTableCollection);

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

    void lowerGetDispatcher(IRGetDispatcher* dispatcher)
    {
        // Replace the `IRGetDispatcher` with a dispatch function,
        // which takes an extra first parameter for the tag (i.e. ID)
        //
        // We'll also replace the callee in all 'call' insts.
        //

        auto witnessTableCollection = cast<IRWitnessTableCollection>(dispatcher->getOperand(0));
        auto key = cast<IRStructKey>(dispatcher->getOperand(1));

        IRBuilder builder(dispatcher->getModule());

        Dictionary<IRInst*, IRInst*> elements;
        forEachInCollection(
            witnessTableCollection,
            [&](IRInst* table)
            {
                auto tag = builder.emitGetTagOfElementInCollection(
                    builder.getCollectionTagType(witnessTableCollection),
                    table,
                    witnessTableCollection);
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

// This context lowers `TypeCollection` instructions.
struct CollectionLoweringContext : public InstPassBase
{
    CollectionLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerValueOfCollectionType(IRValueOfCollectionType* valueOfCollectionType)
    {
        // Type collections are replaced with `AnyValueType` large enough to hold
        // any of the types in the collection.
        //

        HashSet<IRType*> types;
        for (UInt i = 0; i < valueOfCollectionType->getCollection()->getCount(); i++)
        {
            if (auto type = as<IRType>(valueOfCollectionType->getCollection()->getElement(i)))
            {
                types.add(type);
            }
        }

        IRBuilder builder(module);
        auto anyValueType = createAnyValueType(&builder, types);
        valueOfCollectionType->replaceUsesWith(anyValueType);
    }

    void processModule()
    {
        processInstsOfType<IRValueOfCollectionType>(
            kIROp_ValueOfCollectionType,
            [&](IRValueOfCollectionType* inst) { return lowerValueOfCollectionType(inst); });
    }
};

// Lower `ValueOfCollectionType(TypeCollection(...))` instructions by replacing them with
// appropriate `AnyValueType` instructions.
//
void lowerTypeCollections(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    CollectionLoweringContext context(module);
    context.processModule();
}

// This context lowers `IRGetTagFromSequentialID` and `IRGetSequentialIDFromTag` instructions.
// Note: This pass requires that sequential ID decorations have been created for all witness
// tables.
//
struct SequentialIDTagLoweringContext : public InstPassBase
{
    SequentialIDTagLoweringContext(IRModule* module)
        : InstPassBase(module), cBuilder(module)
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
        auto destCollection = cast<IRCollectionTagType>(inst->getDataType())->getCollection();

        forEachInCollection(
            destCollection,
            [&](IRInst* table)
            {
                // Get unique ID for the witness table
                auto outputId = cBuilder.getUniqueID(table);
                auto seqDecoration = table->findDecoration<IRSequentialIDDecoration>();
                if (seqDecoration)
                {
                    auto inputId = seqDecoration->getSequentialID();
                    mapping[inputId] = outputId; // Map ID to itself for now
                }
            });

        IRBuilder builder(inst);
        builder.setInsertAfter(inst);

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
        auto destCollection = cast<IRCollectionTagType>(srcTagInst->getDataType())->getCollection();

        forEachInCollection(
            destCollection,
            [&](IRInst* table)
            {
                // Get unique ID for the witness table
                SLANG_UNUSED(cast<IRWitnessTable>(table));
                auto outputId = cBuilder.getUniqueID(table);
                auto seqDecoration = table->findDecoration<IRSequentialIDDecoration>();
                if (seqDecoration)
                {
                    auto inputId = seqDecoration->getSequentialID();
                    mapping.add({outputId, inputId});
                }
            });

        IRBuilder builder(inst);
        builder.setInsertAfter(inst);
        auto translatedID = builder.emitCallInst(
            inst->getDataType(),
            createIntegerMappingFunc(builder.getModule(), mapping, 0),
            List<IRInst*>({srcTagInst}));

        inst->replaceUsesWith(translatedID);
        inst->removeAndDeallocate();
    }

    void processModule()
    {
        processInstsOfType<IRGetTagFromSequentialID>(
            kIROp_GetTagFromSequentialID,
            [&](IRGetTagFromSequentialID* inst) { return lowerGetTagFromSequentialID(inst); });

        processInstsOfType<IRGetSequentialIDFromTag>(
            kIROp_GetSequentialIDFromTag,
            [&](IRGetSequentialIDFromTag* inst) { return lowerGetSequentialIDFromTag(inst); });
    }

    CollectionBuilder cBuilder;
};

void lowerSequentialIDTagCasts(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    SequentialIDTagLoweringContext context(module);
    context.processModule();
}

void lowerTagInsts(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    TagOpsLoweringContext tagContext(module);
    tagContext.processModule();
}

// This context lowers `IRCollectionTagType` instructions, by replacing
// them with a suitable integer type.
struct TagTypeLoweringContext : public InstPassBase
{
    TagTypeLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void processModule()
    {
        processInstsOfType<IRCollectionTagType>(
            kIROp_CollectionTagType,
            [&](IRCollectionTagType* inst)
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
        //   let tuPtr : PtrType(CollectionTaggedUnionType(types, tables)) =
        //       CastInterfaceToTaggedUnionPtr(basePtr);
        //   let loadedVal : CollectionTaggedUnionType(...) = Load(tuPtr);
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

    IRType* convertToTupleType(IRCollectionTaggedUnionType* taggedUnion)
    {
        // Replace `CollectionTaggedUnionType(typeCollection, tableCollection)` with
        // `TupleType(CollectionTagType(tableCollection), typeCollection)`
        //
        // Unless the collection has a single element, in which case we
        // replace it with `TupleType(CollectionTagType(tableCollection), elementType)`
        //
        // We still maintain a tuple type (even though it's not really necesssary) to avoid
        // breaking any operations that assumed this is a tuple.
        // In the single element case, the tuple should be optimized away.
        //

        IRBuilder builder(module);
        builder.setInsertInto(module);

        auto typeCollection = builder.getValueOfCollectionType(taggedUnion->getTypeCollection());
        auto tableCollection = taggedUnion->getWitnessTableCollection();

        if (taggedUnion->getTypeCollection()->isSingleton())
            return builder.getTupleType(List<IRType*>(
                {(IRType*)makeTagType(tableCollection),
                 (IRType*)taggedUnion->getTypeCollection()->getElement(0)}));

        return builder.getTupleType(
            List<IRType*>({(IRType*)makeTagType(tableCollection), (IRType*)typeCollection}));
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
        // We don't use type tags anywhere, so this instruction should have no
        // uses.
        //
        SLANG_ASSERT(inst->hasUses() == false);
        inst->removeAndDeallocate();
        return true;
    }

    bool lowerMakeTaggedUnion(IRMakeTaggedUnion* inst)
    {
        // We replace `MakeTaggedUnion(tag, val)` with `MakeTuple(tag, val)`
        //

        IRBuilder builder(module);
        builder.setInsertAfter(inst);

        auto tag = inst->getOperand(0);
        auto val = inst->getOperand(1);
        inst->replaceUsesWith(builder.emitMakeTuple((IRType*)inst->getDataType(), {tag, val}));
        inst->removeAndDeallocate();
        return true;
    }

    bool processModule()
    {
        // First, we'll lower all CollectionTaggedUnionType insts
        // into tuples.
        //
        processInstsOfType<IRCollectionTaggedUnionType>(
            kIROp_CollectionTaggedUnionType,
            [&](IRCollectionTaggedUnionType* inst)
            {
                inst->replaceUsesWith(convertToTupleType(inst));
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
}; // namespace Slang