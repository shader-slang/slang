#include "slang-ir-typeflow-collection.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

IRCollectionTagType* makeTagType(IRCollectionBase* collection)
{
    IRInst* collectionInst = collection;
    // Create the tag type from the collection
    IRBuilder builder(collection->getModule());
    return as<IRCollectionTagType>(
        builder.emitIntrinsicInst(nullptr, kIROp_CollectionTagType, 1, &collectionInst));
}

UCount getCollectionCount(IRCollectionBase* collection)
{
    if (!collection)
        return 0;
    return collection->getOperandCount();
}

UCount getCollectionCount(IRCollectionTaggedUnionType* taggedUnion)
{
    auto typeCollection = taggedUnion->getTypeCollection();
    return getCollectionCount(as<IRCollectionBase>(typeCollection));
}

UCount getCollectionCount(IRCollectionTagType* tagType)
{
    auto collection = tagType->getCollection();
    return getCollectionCount(as<IRCollectionBase>(collection));
}

IRInst* getCollectionElement(IRCollectionBase* collection, UInt index)
{
    if (!collection || index >= collection->getOperandCount())
        return nullptr;
    return collection->getOperand(index);
}

IRInst* getCollectionElement(IRCollectionTagType* collectionTagType, UInt index)
{
    auto collection = collectionTagType->getCollection();
    return getCollectionElement(as<IRCollectionBase>(collection), index);
}

CollectionBuilder::CollectionBuilder(IRModule* module)
    : module(module)
{
    this->uniqueIds = module->getUniqueIdMap();
}

UInt CollectionBuilder::getUniqueID(IRInst* inst)
{
    auto existingId = uniqueIds->tryGetValue(inst);
    if (existingId)
        return *existingId;

    auto id = uniqueIds->getCount();
    uniqueIds->add(inst, id);
    return id;
}

// Helper method for creating canonical collections
IRCollectionBase* CollectionBuilder::createCollection(IROp op, const HashSet<IRInst*>& elements)
{
    SLANG_ASSERT(
        op == kIROp_TypeCollection || op == kIROp_FuncCollection || op == kIROp_TableCollection ||
        op == kIROp_GenericCollection);

    if (elements.getCount() == 0)
        return nullptr;

    // Verify that all operands are global instructions
    for (auto element : elements)
        if (element->getParent()->getOp() != kIROp_ModuleInst)
            SLANG_ASSERT_FAILURE("createCollection called with non-global operands");

    List<IRInst*> sortedElements;
    for (auto element : elements)
        sortedElements.add(element);

    // Sort elements by their unique IDs to ensure canonical ordering
    sortedElements.sort(
        [&](IRInst* a, IRInst* b) -> bool { return getUniqueID(a) < getUniqueID(b); });

    // Create the collection instruction
    IRBuilder builder(module);
    builder.setInsertInto(module);

    return as<IRCollectionBase>(builder.emitIntrinsicInst(
        nullptr,
        op,
        sortedElements.getCount(),
        sortedElements.getBuffer()));
}

IROp CollectionBuilder::getCollectionTypeForInst(IRInst* inst)
{
    if (as<IRGeneric>(inst))
        return kIROp_GenericCollection;

    if (as<IRTypeKind>(inst->getDataType()))
        return kIROp_TypeCollection;
    else if (as<IRFuncType>(inst->getDataType()))
        return kIROp_FuncCollection;
    else if (as<IRType>(inst) && !as<IRInterfaceType>(inst))
        return kIROp_TypeCollection;
    else if (as<IRWitnessTableType>(inst->getDataType()))
        return kIROp_TableCollection;
    else
        return kIROp_Invalid; // Return invalid IROp when not supported
}

// Factory methods for PropagationInfo
IRCollectionBase* CollectionBuilder::makeSingletonSet(IRInst* value)
{
    HashSet<IRInst*> singleSet;
    singleSet.add(value);
    return createCollection(getCollectionTypeForInst(value), singleSet);
}

IRCollectionBase* CollectionBuilder::makeSet(const HashSet<IRInst*>& values)
{
    SLANG_ASSERT(values.getCount() > 0);
    return createCollection(getCollectionTypeForInst(*values.begin()), values);
}

} // namespace Slang
