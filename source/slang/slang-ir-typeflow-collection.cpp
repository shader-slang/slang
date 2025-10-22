#include "slang-ir-typeflow-collection.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Upcast the value in 'arg' to match the destInfo type. This method inserts
// any necessary reinterprets or tag translation instructions.
//
IRInst* upcastSet(IRBuilder* builder, IRInst* arg, IRType* destInfo)
{
    // The upcasting process inserts the appropriate instructions
    // to make arg's type match the type provided by destInfo.
    //
    // This process depends on the structure of arg and destInfo.
    //
    // We only deal with the type-flow data-types that are created in
    // our pass (SetBase/TaggedUnionType/SetTagType/any other
    // composites of these insts)
    //

    auto argInfo = arg->getDataType();
    if (!argInfo || !destInfo)
        return arg;

    if (as<IRTaggedUnionType>(argInfo) && as<IRTaggedUnionType>(destInfo))
    {
        // A collection tagged union is essentially a tuple(TagType(tableSet),
        // typeSet) We simply extract the two components, upcast each one, and put it
        // back together.
        //

        auto argTUType = as<IRTaggedUnionType>(argInfo);
        auto destTUType = as<IRTaggedUnionType>(destInfo);

        if (argTUType != destTUType)
        {
            // Technically, IRTaggedUnionType is not a TupleType,
            // but in practice it works the same way so we'll re-use Slang's
            // tuple accessors & constructors
            //
            auto argTableTag = builder->emitGetTagFromTaggedUnion(arg);
            auto reinterpretedTag = upcastSet(
                builder,
                argTableTag,
                builder->getSetTagType(destTUType->getWitnessTableSet()));

            auto argVal = builder->emitGetValueFromTaggedUnion(arg);
            auto reinterpretedVal =
                upcastSet(builder, argVal, builder->getUntaggedUnionType(destTUType->getTypeSet()));
            return builder->emitMakeTaggedUnion(destTUType, reinterpretedTag, reinterpretedVal);
        }
    }
    else if (as<IRSetTagType>(argInfo) && as<IRSetTagType>(destInfo))
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
            return builder->emitIntrinsicInst((IRType*)destInfo, kIROp_GetTagForSuperSet, 1, &arg);
        }
    }
    else if (as<IRUntaggedUnionType>(argInfo) && as<IRUntaggedUnionType>(destInfo))
    {
        // If the arg has a collection type, but the dest is a _different_ collection,
        // we need to perform a reinterpret.
        //
        // e.g. TypeSet({T1, T2}) may lower to AnyValueType(N), while
        // TypeSet({T1, T2, T3}) may lower to AnyValueType(M). Since the target
        // is necessarily a super-set, the target any-value-type is always larger (M >= N),
        // so we only need a simple reinterpret.
        //
        if (argInfo != destInfo)
        {
            auto argSet = as<IRUntaggedUnionType>(argInfo)->getSet();
            if (argSet->isSingleton() && as<IRVoidType>(argSet->getElement(0)))
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
    else if (!as<IRUntaggedUnionType>(argInfo) && as<IRUntaggedUnionType>(destInfo))
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
