#include "slang-ir-typeflow-collection.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

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

        if (argTUType != destTUType)
        {
            // Technically, IRCollectionTaggedUnionType is not a TupleType,
            // but in practice it works the same way so we'll re-use Slang's
            // tuple accessors & constructors
            //
            auto argTableTag = builder->emitGetTagFromTaggedUnion(arg);
            auto reinterpretedTag = upcastCollection(
                builder,
                argTableTag,
                builder->getCollectionTagType(destTUType->getWitnessTableCollection()));

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
        if (argInfo != destInfo)
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
        if (argInfo != destInfo)
        {
            auto argCollection = as<IRValueOfCollectionType>(argInfo)->getCollection();
            if (argCollection->isSingleton() && as<IRVoidType>(argCollection->getElement(0)))
            {
                // There's a specific case where we're trying to reinterpret a value of 'void'
                // type. We'll avoid emitting a reinterpret in this case, and emit a
                // default-construct instead.
                //
                return builder->emitDefaultConstruct((IRType*)destInfo);
            }

            // General case:
            //
            // If the sets of witness tables are not equal, reinterpret to the
            // parameter type
            //
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
