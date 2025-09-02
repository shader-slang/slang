#include "slang-ir-lower-typeflow-insts.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
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

IRFunc* createDispatchFunc(IRFuncCollection* collection)
{
    // An effective func type should have been set during the dynamic-inst-lowering
    // pass.
    //
    IRFuncType* dispatchFuncType = cast<IRFuncType>(collection->getFullType());

    // Create a dispatch function with switch-case for each function
    IRBuilder builder(collection->getModule());

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

    UIndex funcSeqID = 0;
    forEachInCollection(
        collection,
        [&](IRInst* funcInst)
        {
            auto funcId = funcSeqID++;
            auto wrapperFunc =
                emitWitnessTableWrapper(funcInst->getModule(), funcInst, innerFuncType);

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

            caseValues.add(builder.getIntValue(builder.getUIntType(), funcId));
            caseBlocks.add(caseBlock);
        });

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


IRFunc* createIntegerMappingFunc(IRModule* module, Dictionary<UInt, UInt>& mapping, UInt defaultVal)
{
    // Create a function that maps input IDs to output IDs
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

// This context lowers `IRGetTagFromSequentialID`,
// `IRGetTagForSuperCollection`, and `IRGetTagForMappedCollection` instructions,
//

struct TagOpsLoweringContext : public InstPassBase
{
    TagOpsLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerGetTagForSuperCollection(IRGetTagForSuperCollection* inst)
    {
        auto srcCollection = cast<IRCollectionBase>(
            cast<IRCollectionTagType>(inst->getOperand(0)->getDataType())->getOperand(0));
        auto destCollection =
            cast<IRCollectionBase>(cast<IRCollectionTagType>(inst->getDataType())->getOperand(0));

        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        List<IRInst*> indices;
        for (UInt i = 0; i < srcCollection->getOperandCount(); i++)
        {
            // Find in destCollection
            auto srcElement = srcCollection->getOperand(i);

            bool found = false;
            for (UInt j = 0; j < destCollection->getOperandCount(); j++)
            {
                auto destElement = destCollection->getOperand(j);
                if (srcElement == destElement)
                {
                    found = true;
                    indices.add(builder.getIntValue(builder.getUIntType(), j));
                    break; // Found the index
                }
            }

            if (!found)
            {
                // destCollection must be a super-set
                SLANG_UNEXPECTED("Element not found in destination collection");
            }
        }

        // Create an array for the lookup
        auto lookupArrayType = builder.getArrayType(
            builder.getUIntType(),
            builder.getIntValue(builder.getUIntType(), indices.getCount()));
        auto lookupArray =
            builder.emitMakeArray(lookupArrayType, indices.getCount(), indices.getBuffer());
        auto resultID =
            builder.emitElementExtract(inst->getDataType(), lookupArray, inst->getOperand(0));
        inst->replaceUsesWith(resultID);
        inst->removeAndDeallocate();
    }

    void lowerGetTagForMappedCollection(IRGetTagForMappedCollection* inst)
    {
        auto srcCollection = cast<IRTableCollection>(
            cast<IRCollectionTagType>(inst->getOperand(0)->getDataType())->getOperand(0));
        auto destCollection =
            cast<IRCollectionBase>(cast<IRCollectionTagType>(inst->getDataType())->getOperand(0));
        auto key = cast<IRStructKey>(inst->getOperand(1));

        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        List<IRInst*> indices;
        for (UInt i = 0; i < srcCollection->getOperandCount(); i++)
        {
            // Find in destCollection
            bool found = false;
            auto srcElement =
                findWitnessTableEntry(cast<IRWitnessTable>(srcCollection->getOperand(i)), key);
            for (UInt j = 0; j < destCollection->getOperandCount(); j++)
            {
                auto destElement = destCollection->getOperand(j);
                if (srcElement == destElement)
                {
                    found = true;
                    indices.add(builder.getIntValue(builder.getUIntType(), j));
                    break; // Found the index
                }
            }

            if (!found)
            {
                // destCollection must be a super-set
                SLANG_UNEXPECTED("Element not found in destination collection");
            }
        }

        // Create an array for the lookup
        auto lookupArrayType = builder.getArrayType(
            builder.getUIntType(),
            builder.getIntValue(builder.getUIntType(), indices.getCount()));
        auto lookupArray =
            builder.emitMakeArray(lookupArrayType, indices.getCount(), indices.getBuffer());
        auto resultID =
            builder.emitElementExtract(inst->getDataType(), lookupArray, inst->getOperand(0));
        inst->replaceUsesWith(resultID);
        inst->removeAndDeallocate();
    }

    void lowerGetTagForSpecializedCollection(IRGetTagForSpecializedCollection* inst)
    {
        auto srcCollection = cast<IRCollectionBase>(
            cast<IRCollectionTagType>(inst->getOperand(0)->getDataType())->getOperand(0));
        auto destCollection =
            cast<IRCollectionBase>(cast<IRCollectionTagType>(inst->getDataType())->getOperand(0));
        Dictionary<IRInst*, IRInst*> mapping;

        for (UInt i = 1; i < inst->getOperandCount(); i += 2)
        {
            auto srcElement = inst->getOperand(i);
            auto destElement = inst->getOperand(i + 1);
            mapping[srcElement] = destElement;
        }

        IRBuilder builder(inst->getModule());
        builder.setInsertAfter(inst);

        List<IRInst*> indices;
        for (UInt i = 0; i < srcCollection->getOperandCount(); i++)
        {
            // Find in destCollection
            bool found = false;
            auto mappedElement = mapping[srcCollection->getOperand(i)];
            for (UInt j = 0; j < destCollection->getOperandCount(); j++)
            {
                auto destElement = destCollection->getOperand(j);
                if (mappedElement == destElement)
                {
                    found = true;
                    indices.add(builder.getIntValue(builder.getUIntType(), j));
                    break; // Found the index
                }
            }

            if (!found)
            {
                SLANG_UNEXPECTED("Element not found in specialized collection");
            }
        }

        // Create an array for the lookup
        auto lookupArrayType = builder.getArrayType(
            builder.getUIntType(),
            builder.getIntValue(builder.getUIntType(), indices.getCount()));
        auto lookupArray =
            builder.emitMakeArray(lookupArrayType, indices.getCount(), indices.getBuffer());
        auto resultID =
            builder.emitElementExtract(inst->getDataType(), lookupArray, inst->getOperand(0));
        inst->replaceUsesWith(resultID);
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
        case kIROp_GetTagForSpecializedCollection:
            lowerGetTagForSpecializedCollection(as<IRGetTagForSpecializedCollection>(inst));
            break;
        default:
            break;
        }
    }

    void lowerFuncCollection(IRFuncCollection* collection)
    {
        IRBuilder builder(collection->getModule());
        if (collection->hasUses() && collection->getDataType() != nullptr)
        {
            auto dispatchFunc = createDispatchFunc(collection);
            traverseUses(
                collection,
                [&](IRUse* use)
                {
                    if (auto callInst = as<IRCall>(use->getUser()))
                    {
                        // If the call is a collection call, replace it with the dispatch function
                        if (callInst->getCallee() == collection)
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
        processInstsOfType<IRFuncCollection>(
            kIROp_FuncCollection,
            [&](IRFuncCollection* inst) { return lowerFuncCollection(inst); });

        processAllInsts([&](IRInst* inst) { return processInst(inst); });
    }
};

// This context lowers `IRTypeCollection` and `IRFuncCollection` instructions
struct CollectionLoweringContext : public InstPassBase
{
    CollectionLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerTypeCollection(IRTypeCollection* collection)
    {
        HashSet<IRType*> types;
        for (UInt i = 0; i < collection->getOperandCount(); i++)
        {
            if (auto type = as<IRType>(collection->getOperand(i)))
            {
                types.add(type);
            }
        }

        IRBuilder builder(collection->getModule());
        auto anyValueType = createAnyValueType(&builder, types);
        collection->replaceUsesWith(anyValueType);
    }

    void processModule()
    {
        processInstsOfType<IRTypeCollection>(
            kIROp_TypeCollection,
            [&](IRTypeCollection* inst) { return lowerTypeCollection(inst); });
    }
};

void lowerTypeCollections(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    CollectionLoweringContext context(module);
    context.processModule();
}

struct SequentialIDTagLoweringContext : public InstPassBase
{
    SequentialIDTagLoweringContext(IRModule* module)
        : InstPassBase(module)
    {
    }

    void lowerGetTagFromSequentialID(IRGetTagFromSequentialID* inst)
    {
        SLANG_UNUSED(cast<IRInterfaceType>(inst->getOperand(0)));
        auto srcSeqID = inst->getOperand(1);

        Dictionary<UInt, UInt> mapping;

        // Map from sequential ID to unique ID
        auto destCollection =
            cast<IRCollectionBase>(cast<IRCollectionTagType>(inst->getDataType())->getOperand(0));

        UIndex dstSeqID = 0;
        forEachInCollection(
            destCollection,
            [&](IRInst* table)
            {
                // Get unique ID for the witness table
                SLANG_UNUSED(cast<IRWitnessTable>(table));
                auto outputId = dstSeqID++;
                auto seqDecoration = table->findDecoration<IRSequentialIDDecoration>();
                if (seqDecoration)
                {
                    auto inputId = seqDecoration->getSequentialID();
                    mapping[inputId] = outputId; // Map ID to itself for now
                }
            });

        IRBuilder builder(inst);
        builder.setInsertAfter(inst);

        // Default to largest available sequential ID.
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
        SLANG_UNUSED(cast<IRInterfaceType>(inst->getOperand(0)));
        auto srcTagInst = inst->getOperand(1);

        Dictionary<UInt, UInt> mapping;

        // Map from sequential ID to unique ID
        auto destCollection = cast<IRCollectionBase>(
            cast<IRCollectionTagType>(srcTagInst->getDataType())->getOperand(0));

        UIndex dstSeqID = 0;
        forEachInCollection(
            destCollection,
            [&](IRInst* table)
            {
                // Get unique ID for the witness table
                SLANG_UNUSED(cast<IRWitnessTable>(table));
                auto outputId = dstSeqID++;
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
        // Find all uses of the inst
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

    void lowerCastTaggedUnionToInterfacePtr(IRCastTaggedUnionToInterfacePtr* inst)
    {
        SLANG_UNUSED(inst);
        SLANG_UNEXPECTED("Unexpected inst of CastTaggedUnionToInterfacePtr");
    }

    IRType* convertToTupleType(IRCollectionTaggedUnionType* taggedUnion)
    {
        // Replace type with Tuple<CollectionTagType(collection), TypeCollection>
        IRBuilder builder(module);
        builder.setInsertInto(module);

        auto typeCollection = cast<IRTypeCollection>(taggedUnion->getOperand(0));
        auto tableCollection = cast<IRTableCollection>(taggedUnion->getOperand(1));

        if (getCollectionCount(typeCollection) == 1)
            return builder.getTupleType(List<IRType*>(
                {(IRType*)makeTagType(tableCollection),
                 (IRType*)getCollectionElement(typeCollection, 0)}));

        return builder.getTupleType(
            List<IRType*>({(IRType*)makeTagType(tableCollection), (IRType*)typeCollection}));
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

        bool hasCastInsts = false;
        processInstsOfType<IRCastInterfaceToTaggedUnionPtr>(
            kIROp_CastInterfaceToTaggedUnionPtr,
            [&](IRCastInterfaceToTaggedUnionPtr* inst)
            {
                hasCastInsts = true;
                return lowerCastInterfaceToTaggedUnionPtr(inst);
            });

        processInstsOfType<IRCastTaggedUnionToInterfacePtr>(
            kIROp_CastTaggedUnionToInterfacePtr,
            [&](IRCastTaggedUnionToInterfacePtr* inst)
            {
                hasCastInsts = true;
                return lowerCastTaggedUnionToInterfacePtr(inst);
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