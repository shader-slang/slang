#include "slang-ir-typeflow-collection.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
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
        op == kIROp_TypeCollection || op == kIROp_FuncCollection ||
        op == kIROp_WitnessTableCollection || op == kIROp_GenericCollection);

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
        return kIROp_WitnessTableCollection;
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

// Upcast the value in 'arg' to match the destInfo type. This method inserts
// any necessary reinterprets or tag translation instructions.
//
IRInst* upcastCollection(IRBuilder* builder, IRInst* arg, IRType* destInfo)
{
    // The upcasting process inserts the appropriate instructions
    // to make arg's type match the type provided by destInfo.
    //
    // This process depends on the structure of arg and destInfo.
    //
    // We only deal with the type-flow data-types that are created in
    // our pass (CollectionBase/CollectionTaggedUnionType/CollectionTagType/any other
    // composites of these insts)
    //

    auto argInfo = arg->getDataType();
    if (!argInfo || !destInfo)
        return arg;

    if (as<IRCollectionTaggedUnionType>(argInfo) && as<IRCollectionTaggedUnionType>(destInfo))
    {
        // A collection tagged union is essentially a tuple(TagType(tableCollection),
        // typeCollection) We simply extract the two components, upcast each one, and put it
        // back together.
        //

        auto argTUType = as<IRCollectionTaggedUnionType>(argInfo);
        auto destTUType = as<IRCollectionTaggedUnionType>(destInfo);

        if (getCollectionCount(argTUType) != getCollectionCount(destTUType))
        {
            // Technically, IRCollectionTaggedUnionType is not a TupleType,
            // but in practice it works the same way so we'll re-use Slang's
            // tuple accessors & constructors
            //
            // IRBuilder builder(module);
            // setInsertAfterOrdinaryInst(&builder, arg);
            auto argTableTag = builder->emitGetTagFromTaggedUnion(arg);
            auto reinterpretedTag = upcastCollection(
                builder,
                argTableTag,
                makeTagType(destTUType->getWitnessTableCollection()));

            auto argVal = builder->emitGetValueFromTaggedUnion(arg);
            auto reinterpretedVal = upcastCollection(
                builder,
                argVal,
                builder->getValueOfCollectionType(destTUType->getTypeCollection()));
            return builder->emitMakeTaggedUnion(destTUType, reinterpretedTag, reinterpretedVal);
        }
    }
    else if (as<IRCollectionTagType>(argInfo) && as<IRCollectionTagType>(destInfo))
    {
        // If the arg represents a tag of a colleciton, but the dest is a _different_
        // collection, then we need to emit a tag operation to reinterpret the
        // tag.
        //
        // Note that, by the invariant provided by the typeflow analysis, the target
        // collection must necessarily be a super-set.
        //
        if (getCollectionCount(as<IRCollectionTagType>(argInfo)) !=
            getCollectionCount(as<IRCollectionTagType>(destInfo)))
        {
            return builder
                ->emitIntrinsicInst((IRType*)destInfo, kIROp_GetTagForSuperCollection, 1, &arg);
        }
    }
    else if (as<IRValueOfCollectionType>(argInfo) && as<IRValueOfCollectionType>(destInfo))
    {
        // If the arg has a collection type, but the dest is a _different_ collection,
        // we need to perform a reinterpret.
        //
        // e.g. TypeCollection({T1, T2}) may lower to AnyValueType(N), while
        // TypeCollection({T1, T2, T3}) may lower to AnyValueType(M). Since the target
        // is necessarily a super-set, the target any-value-type is always larger (M >= N),
        // so we only need a simple reinterpret.
        //
        if (getCollectionCount(as<IRValueOfCollectionType>(argInfo)->getCollection()) !=
            getCollectionCount(as<IRValueOfCollectionType>(destInfo)->getCollection()))
        {
            auto argCollection = as<IRValueOfCollectionType>(argInfo)->getCollection();
            if (argCollection->isSingleton() && as<IRVoidType>(argCollection->getElement(0)))
            {
                // There's a specific case where we're trying to reinterpret a value of 'void'
                // type. We'll avoid emitting a reinterpret in this case, and emit a
                // default-construct instead.
                //
                // IRBuilder builder(module);
                // setInsertAfterOrdinaryInst(&builder, arg);
                return builder->emitDefaultConstruct((IRType*)destInfo);
            }

            // General case:
            //
            // If the sets of witness tables are not equal, reinterpret to the
            // parameter type
            //
            // IRBuilder builder(module);
            // setInsertAfterOrdinaryInst(&builder, arg);
            return builder->emitReinterpret((IRType*)destInfo, arg);
        }
    }
    else if (!as<IRValueOfCollectionType>(argInfo) && as<IRValueOfCollectionType>(destInfo))
    {
        // If the arg is not a collection-type, but the dest is a collection,
        // we need to perform a pack operation.
        //
        // This case only arises when passing a value of type T to a parameter
        // of a type-collection that contains T.
        //
        return builder->emitPackAnyValue((IRType*)destInfo, arg);
    }

    return arg; // Can use as-is.
}

} // namespace Slang
