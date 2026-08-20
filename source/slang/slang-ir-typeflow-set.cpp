#include "slang-ir-typeflow-set.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Finds the witness-table-set member that describes `concreteType`. For example, when adapting a
// LinearValueHolder into the tagged-union representation of IValueHolder, this returns the
// LinearValueHolder:IValueHolder table. Type and witness-table sets are logically keyed data, so
// the match is made by concrete type rather than by assuming their operand orders correspond.
static IRWitnessTable* findWitnessTableForConcreteType(
    IRModule* module,
    IRWitnessTableSet* witnessTableSet,
    IRType* concreteType)
{
    IRWitnessTable* result = nullptr;
    forEachInSet(
        module,
        witnessTableSet,
        [&](IRInst* element)
        {
            if (auto witnessTable = as<IRWitnessTable>(element))
            {
                if (witnessTable->getConcreteType() == concreteType)
                {
                    SLANG_RELEASE_ASSERT(!result || result == witnessTable);
                    result = witnessTable;
                }
            }
        });
    return result;
}

template<typename F>
IRInst* openOptional(IRModule* module, IRInst* arg, F innerFunc)
{
    auto argOptionalType = as<IROptionalType>(arg->getDataType());
    SLANG_ASSERT(argOptionalType);

    // Create a helper function that performs the reinterpretation
    IRBuilder builder(module);

    auto func = builder.createFunc();
    builder.addNameHintDecoration(func, UnownedStringSlice("openOptional"));

    builder.setInsertInto(func);

    // Entry block
    auto entryBlock = builder.emitBlock();
    auto param = builder.emitParam(argOptionalType);

    // Check if the source optional has a value
    auto hasValue = builder.emitOptionalHasValue(param);

    // Create the if-else control flow blocks
    auto trueBlock = builder.emitBlock();
    auto falseBlock = builder.emitBlock();
    auto unreachableBlock = builder.emitBlock();

    // Go back to entry block to emit the branch
    builder.setInsertInto(entryBlock);
    builder.emitIfElse(hasValue, trueBlock, falseBlock, unreachableBlock);

    // True branch: extract, apply F, and wrap
    builder.setInsertInto(trueBlock);
    auto extractedValue = builder.emitGetOptionalValue(param);

    // Call the template function F.
    IRInst* resultInst = innerFunc(&builder, extractedValue);

    auto destOptionalType = builder.getOptionalType(resultInst->getDataType());

    IRType* funcParamTypes[] = {argOptionalType};
    auto funcType = builder.getFuncType(1, funcParamTypes, destOptionalType);
    func->setFullType(funcType);

    auto wrappedValue = builder.emitMakeOptionalValue(destOptionalType, resultInst);
    builder.emitReturn(wrappedValue);

    // False branch: create none and return
    builder.setInsertInto(falseBlock);
    auto noneValue = builder.emitMakeOptionalNone(destOptionalType);
    builder.emitReturn(noneValue);

    // Unreachable block (both branches return, so this is never reached)
    builder.setInsertInto(unreachableBlock);
    builder.emitUnreachable();

    // Replace the ReinterpretOptional instruction with a call to the helper function
    builder.setInsertAfter(arg);
    auto callResult = builder.emitCallInst(destOptionalType, func, 1, &arg);

    return callResult;
}

// Adapt the value in `arg` to `destInfo`. Most existing callers perform an upcast into a larger
// type-flow set, but witness-table wrappers also use the same structural recursion in reverse to
// recover the concrete parameter type expected by an implementation.
//
IRInst* adaptTypeFlowValue(IRBuilder* builder, IRInst* arg, IRType* destInfo)
{
    // The adaptation process inserts the appropriate instructions
    // to make the argument's type match the type provided by `destInfo`.
    //
    // This process depends on the structure of arg and destInfo.
    //
    // We only deal with the type-flow data-types that are created in
    // our pass (SetBase/TaggedUnionType/SetTagType/any other
    // composites of these insts)
    //

    // If either side has attributes, we drop them for now.
    if (as<IRAttributedType>(destInfo))
    {
        // Unwrap and upcast.
        auto destBase = (IRType*)unwrapAttributedType(destInfo);
        return adaptTypeFlowValue(builder, arg, destBase);
    }

    auto argInfo = arg->getDataType();
    if (!argInfo || !destInfo)
        return arg;
    if (argInfo == destInfo)
        return arg;

    // If we are upcasting a default-constructed value and the destination type differs,
    // we should materialize a default value of the destination type instead of trying to
    // reinterpret/cast the old default value.
    //
    // This is important when earlier specialization/lowering changes the effective type of
    // a phi/block-parameter, but a predecessor edge still passes a `defaultConstruct` of the
    // pre-specialization type.
    if (argInfo != destInfo)
    {
        if (as<IRDefaultConstruct>(arg))
        {
            if (auto newDefault =
                    builder->emitDefaultConstruct((IRType*)destInfo, /*fallback*/ true))
                return newDefault;
        }
    }

    if (auto argPairInfo = as<IRDifferentialPairInfoType>(argInfo))
    {
        if (auto destPairType = as<IRDifferentialPairType>(destInfo))
        {
            // A witness-table implementation receives its nominal pair type. Project the two
            // semantic components and adapt each independently; this remains correct when either
            // component is itself a composite type-flow value.
            auto primalArg = builder->emitDifferentialValuePairGetPrimal(
                argPairInfo->getPrimalInfo(),
                arg);
            auto differentialArg = builder->emitDifferentialValuePairGetDifferential(
                argPairInfo->getDifferentialInfo(),
                arg);
            auto primal = adaptTypeFlowValue(
                builder,
                primalArg,
                destPairType->getValueType());
            auto differentialType = getConcreteDifferentialType(builder, destPairType);
            auto differential = adaptTypeFlowValue(builder, differentialArg, differentialType);
            return builder->emitMakeDifferentialPair(destPairType, primal, differential);
        }

        if (auto destPairInfo = as<IRDifferentialPairInfoType>(destInfo))
        {
            auto primalArg = builder->emitDifferentialValuePairGetPrimal(
                argPairInfo->getPrimalInfo(),
                arg);
            auto differentialArg = builder->emitDifferentialValuePairGetDifferential(
                argPairInfo->getDifferentialInfo(),
                arg);
            auto primal = adaptTypeFlowValue(
                builder,
                primalArg,
                destPairInfo->getPrimalInfo());
            auto differential = adaptTypeFlowValue(
                builder,
                differentialArg,
                destPairInfo->getDifferentialInfo());
            return builder->emitMakeDifferentialValuePair(destPairInfo, primal, differential);
        }
    }

    if (auto argPairType = as<IRDifferentialPairType>(argInfo))
    {
        if (auto destPairInfo = as<IRDifferentialPairInfoType>(destInfo))
        {
            // Preserve pair identity until the specialization fixed point is complete. The later
            // pair-info lowering pass chooses the ordinary tuple layout in one place.
            auto primal = builder->emitDifferentialValuePairGetPrimal(
                argPairType->getValueType(),
                arg);
            auto differentialType = getConcreteDifferentialType(builder, argPairType);
            auto differential = builder->emitDifferentialValuePairGetDifferential(
                differentialType,
                arg);
            auto adaptedPrimal = adaptTypeFlowValue(
                builder,
                primal,
                destPairInfo->getPrimalInfo());
            auto adaptedDifferential = adaptTypeFlowValue(
                builder,
                differential,
                destPairInfo->getDifferentialInfo());
            return builder->emitMakeDifferentialValuePair(
                destPairInfo,
                adaptedPrimal,
                adaptedDifferential);
        }
    }

    if (as<IRTaggedUnionType>(argInfo) && as<IRTaggedUnionType>(destInfo))
    {
        // A tagged union is essentially a tuple(TagType(tableSet), typeSet). Extract its
        // components, adapt each one, and put it back together.
        //

        auto argTUType = as<IRTaggedUnionType>(argInfo);
        auto destTUType = as<IRTaggedUnionType>(destInfo);

        if (argTUType != destTUType)
        {
            auto argTableTag = builder->emitGetTagFromTaggedUnion(arg);
            auto reinterpretedTableTag = adaptTypeFlowValue(
                builder,
                argTableTag,
                builder->getSetTagType(destTUType->getWitnessTableSet()));

            auto argTypeTag = builder->emitGetTypeTagFromTaggedUnion(arg);
            auto reinterpretedTypeTag =
                adaptTypeFlowValue(
                    builder,
                    argTypeTag,
                    builder->getSetTagType(destTUType->getTypeSet()));

            auto argVal = builder->emitGetValueFromTaggedUnion(arg);
            auto reinterpretedVal =
                adaptTypeFlowValue(
                    builder,
                    argVal,
                    builder->getUntaggedUnionType(destTUType->getTypeSet()));
            return builder->emitMakeTaggedUnion(
                destTUType,
                reinterpretedTypeTag,
                reinterpretedTableTag,
                reinterpretedVal);
        }
    }
    else if (auto destTaggedUnionType = as<IRTaggedUnionType>(destInfo))
    {
        // A composite constructor can merge nominal values into a tagged-union element type. For
        // example, Array<DifferentialPair<IValueHolder>, 2> may receive one nominal
        // DifferentialPair<LinearValueHolder> and one nominal DifferentialPair<SquaredValueHolder>.
        // Pair adaptation reaches this branch for each concrete primal and differential component.
        auto witnessTable = findWitnessTableForConcreteType(
            builder->getModule(),
            destTaggedUnionType->getWitnessTableSet(),
            argInfo);
        SLANG_RELEASE_ASSERT(witnessTable);

        auto typeSet = destTaggedUnionType->getTypeSet();
        auto witnessTableSet = destTaggedUnionType->getWitnessTableSet();
        auto typeTag = builder->emitGetTagOfElementInSet(
            builder->getSetTagType(typeSet),
            argInfo,
            typeSet);
        auto witnessTableTag = builder->emitGetTagOfElementInSet(
            builder->getSetTagType(witnessTableSet),
            witnessTable,
            witnessTableSet);

        IRType* payloadType = typeSet->isSingleton()
                                  ? (IRType*)typeSet->getElement(0)
                                  : builder->getUntaggedUnionType(typeSet);
        auto payload = adaptTypeFlowValue(builder, arg, payloadType);
        return builder->emitMakeTaggedUnion(
            destTaggedUnionType,
            typeTag,
            witnessTableTag,
            payload);
    }
    else if (as<IRTaggedUnionType>(argInfo))
    {
        // A concrete witness-table wrapper does not need the existential tag after dispatch has
        // selected the implementation. Extract the payload first, then continue adapting it.
        return adaptTypeFlowValue(builder, builder->emitGetValueFromTaggedUnion(arg), destInfo);
    }
    else if (as<IRSetTagType>(argInfo) && as<IRSetTagType>(destInfo))
    {
        // If the arg represents a tag of a set, but the dest is a _different_
        // set, then we need to emit a tag operation to reinterpret the
        // tag.
        //
        // Note that, by the invariant provided by the typeflow analysis, the target
        // set must necessarily be a super-set.
        //
        if (argInfo != destInfo)
        {
            return builder->emitIntrinsicInst((IRType*)destInfo, kIROp_GetTagForSuperSet, 1, &arg);
        }
    }
    else if (as<IRUntaggedUnionType>(argInfo) && as<IRUntaggedUnionType>(destInfo))
    {
        // If the arg has a untagged union type, but the dest is a _different_ untagged union,
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
            if (argSet->isSingleton() && as<IRNoneTypeElement>(argSet->getElement(0)))
            {
                // There's a specific case where we're trying to reinterpret a value of 'none'
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
    else if (as<IRUntaggedUnionType>(argInfo) && !as<IRUntaggedUnionType>(destInfo))
    {
        return builder->emitUnpackAnyValue(destInfo, arg);
    }
    else if (as<IRAnyValueType>(argInfo) && !as<IRAnyValueType>(destInfo))
    {
        return builder->emitUnpackAnyValue(destInfo, arg);
    }
    else if (!as<IRAnyValueType>(argInfo) && as<IRAnyValueType>(destInfo))
    {
        return builder->emitPackAnyValue(destInfo, arg);
    }
    else if (!as<IRUntaggedUnionType>(argInfo) && as<IRUntaggedUnionType>(destInfo))
    {
        // If the arg is not a collection-type, but the dest is a collection,
        // we need to perform a pack operation.
        //
        // This case only arises when passing a value of type T to a parameter
        // of a type-set that contains T.
        //
        return builder->emitPackAnyValue((IRType*)destInfo, arg);
    }
    else if (as<IRArrayType>(argInfo) && as<IRArrayType>(destInfo))
    {
        // If both arg and dest are arrays, adapt each element.
        //
        auto argArrayType = as<IRArrayType>(argInfo);
        auto destArrayType = as<IRArrayType>(destInfo);
        auto argElementType = argArrayType->getElementType();
        auto destElementType = destArrayType->getElementType();

        if (argElementType != destElementType)
        {
            auto arraySize = getIntVal(argArrayType->getElementCount());
            SLANG_RELEASE_ASSERT(arraySize == getIntVal(destArrayType->getElementCount()));

            List<IRInst*> adaptedElements;
            adaptedElements.setCount((Index)arraySize);
            for (IRIntegerValue i = 0; i < arraySize; i++)
            {
                auto argElement = builder->emitGetElement(argElementType, arg, i);
                auto adaptedElement = adaptTypeFlowValue(builder, argElement, destElementType);
                adaptedElements[(Index)i] = adaptedElement;
            }

            return builder->emitMakeArray(
                destArrayType,
                adaptedElements.getCount(),
                adaptedElements.getBuffer());
        }
    }
    else if (as<IRTupleType>(argInfo) && as<IRTupleType>(destInfo))
    {
        // If both arg and dest are tuples, adapt each element.
        //
        auto argTupleType = as<IRTupleType>(argInfo);
        auto destTupleType = as<IRTupleType>(destInfo);

        if (argTupleType != destTupleType)
        {
            UInt argElementCount = argTupleType->getOperandCount();
            SLANG_RELEASE_ASSERT(argElementCount == destTupleType->getOperandCount());

            List<IRInst*> adaptedElements;
            adaptedElements.setCount((Index)argElementCount);
            for (UInt i = 0; i < argElementCount; i++)
            {
                auto argElementType = (IRType*)argTupleType->getOperand(i);
                auto destElementType = (IRType*)destTupleType->getOperand(i);
                auto argElement = builder->emitGetTupleElement(argElementType, arg, i);
                auto adaptedElement = adaptTypeFlowValue(builder, argElement, destElementType);
                adaptedElements[(Index)i] = adaptedElement;
            }

            return builder->emitMakeTuple(destTupleType, adaptedElements);
        }
    }
    else if (as<IROptionalType>(argInfo) && as<IROptionalType>(destInfo))
    {
        // If both arg and dest are optionals, we need to upcast the value type.
        //
        auto argOptionalType = as<IROptionalType>(argInfo);
        auto destOptionalType = as<IROptionalType>(destInfo);
        auto argValueType = (IRType*)argOptionalType->getValueType();
        auto destValueType = (IRType*)destOptionalType->getValueType();

        if (argValueType != destValueType)
        {
            // We emit a ReinterpretOptional instruction that will be lowered
            // later in lowerReinterpret to an if-else block with proper control flow.
            //
            return openOptional(
                builder->getModule(),
                arg,
                [destValueType](IRBuilder* b, IRInst* extractedValue)
                { return (IRInst*)adaptTypeFlowValue(b, extractedValue, destValueType); });
        }
    }
    else if (as<IROptionalNoneType>(argInfo) && as<IROptionalType>(destInfo))
    {
        // Special case: upcasting from `none_t` to `optional<T>` means
        // creating a `none` value.
        return builder->emitMakeOptionalNone((IRType*)destInfo);
    }

    return arg; // Can use as-is.
}

} // namespace Slang
