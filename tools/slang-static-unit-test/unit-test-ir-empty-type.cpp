// Pin the distinct emptiness policies used by DCE and ray-tracing payload boundaries.
// Hand-built IR exposes void fields, decorated empty types, and opaque wrappers directly,
// without depending on which of those shapes survive frontend lowering and optimization.

#include "slang/slang-ir-dce.h"
#include "slang/slang-ir-util.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(irStructEmptyPreservesDceSemantics)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    builder.setInsertInto(fixture.getModule());

    auto empty = builder.createStructType();
    auto nested = builder.createStructType();
    builder.createStructField(nested, builder.createStructKey(), builder.getVoidType());
    builder.createStructField(nested, builder.createStructKey(), empty);
    SLANG_CHECK(isStructEmpty(empty));
    SLANG_CHECK(isStructEmpty(nested));
    SLANG_CHECK(!isStructEmpty(builder.getVoidType()));
    SLANG_CHECK(!isStructEmpty(builder.getIntType()));
    SLANG_CHECK(!isStructEmpty(builder.getPtrType(empty)));

    auto emptyArray = builder.getArrayType(empty, builder.getIntValue(builder.getIntType(), 2));
    auto containingArray = builder.createStructType();
    builder.createStructField(containingArray, builder.createStructKey(), emptyArray);
    SLANG_CHECK(!isStructEmpty(emptyArray));
    SLANG_CHECK(!isStructEmpty(containingArray));

    // DCE has historically ignored decorations in this structural query. The boundary query
    // deliberately differs, including when the decorated empty struct is nested inside another.
    builder.addTargetIntrinsicDecoration(empty, CapabilitySet(), UnownedStringSlice("opaque"));
    SLANG_CHECK(isStructEmpty(empty));
    SLANG_CHECK(isStructEmpty(nested));
    SLANG_CHECK(!isEmptyType(empty));
    SLANG_CHECK(!isEmptyType(nested));
}

SLANG_UNIT_TEST(irEmptyTypeRecursesThroughEmptyAggregates)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    builder.setInsertInto(fixture.getModule());

    auto empty = builder.createStructType();
    auto nested = builder.createStructType();
    builder.createStructField(nested, builder.createStructKey(), empty);
    builder.createStructField(nested, builder.createStructKey(), builder.getVoidType());
    auto count = builder.getIntValue(builder.getIntType(), 2);
    auto emptyArray = builder.getArrayType(nested, count);
    auto nestedArray = builder.getArrayType(emptyArray, count);
    auto containingArray = builder.createStructType();
    builder.createStructField(containingArray, builder.createStructKey(), nestedArray);

    SLANG_CHECK(isEmptyType(builder.getVoidType()));
    SLANG_CHECK(isEmptyType(empty));
    SLANG_CHECK(isEmptyType(nested));
    SLANG_CHECK(isEmptyType(emptyArray));
    SLANG_CHECK(isEmptyType(nestedArray));
    SLANG_CHECK(isEmptyType(containingArray));

    // One data-bearing field makes the entire aggregate nonempty. Array length alone does not
    // establish erasure: even a zero-length array of integers is not an array of empty elements.
    auto mixed = builder.createStructType();
    builder.createStructField(mixed, builder.createStructKey(), emptyArray);
    builder.createStructField(mixed, builder.createStructKey(), builder.getIntType());
    SLANG_CHECK(!isEmptyType(mixed));
    SLANG_CHECK(!isEmptyType(builder.getArrayType(mixed, count)));
    SLANG_CHECK(!isEmptyType(
        builder.getArrayType(builder.getIntType(), builder.getIntValue(builder.getIntType(), 0))));
}

SLANG_UNIT_TEST(irEmptyTypePreservesOpaqueRepresentations)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    builder.setInsertInto(fixture.getModule());

    auto empty = builder.createStructType();
    auto intrinsic = builder.createStructType();
    builder.addTargetIntrinsicDecoration(intrinsic, CapabilitySet(), UnownedStringSlice("opaque"));
    IRInst* bufferOperands[] = {empty, builder.getDefaultBufferLayoutType()};
    IRType* nonemptyTypes[] = {
        builder.getPtrType(empty),
        builder.getType(kIROp_SamplerStateType),
        builder.getType(kIROp_HLSLStructuredBufferType, 2, bufferOperands),
        builder.getType(kIROp_EmptyNodeInputType),
        builder.getType(kIROp_DispatchNodeInputRecordType, empty),
        intrinsic,
    };

    for (auto type : nonemptyTypes)
    {
        SLANG_CHECK(!isEmptyType(type));
        auto wrapped = builder.createStructType();
        builder.createStructField(wrapped, builder.createStructKey(), type);
        SLANG_CHECK(!isEmptyType(wrapped));
        SLANG_CHECK(!isEmptyType(
            builder.getArrayType(wrapped, builder.getIntValue(builder.getIntType(), 2))));
    }
}

SLANG_UNIT_TEST(irTrimOptimizableTypesKeepsHistoricalEmptyFields)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    builder.setInsertInto(fixture.getModule());

    auto empty = builder.createStructType();
    auto nestedArray = builder.createStructType();
    builder.createStructField(
        nestedArray,
        builder.createStructKey(),
        builder.getArrayType(empty, builder.getIntValue(builder.getIntType(), 2)));
    auto outer = builder.createStructType();
    builder.addDecoration(outer, kIROp_OptimizableTypeDecoration);
    auto retained = builder.createStructKey();
    builder.createStructField(outer, retained, empty);
    builder.createStructField(outer, builder.createStructKey(), nestedArray);

    // DCE leaves the empty-struct field for type legalization, but trims the unused array-bearing
    // field. Calling the broader isEmptyType predicate from DCE would incorrectly retain both.
    SLANG_CHECK(trimOptimizableTypes(fixture.getModule()));
    auto field = outer->getFields().getFirst();
    SLANG_CHECK_ABORT(field != nullptr);
    SLANG_CHECK(field->getKey() == retained);
    SLANG_CHECK(field->getNextInst() == nullptr);
}
