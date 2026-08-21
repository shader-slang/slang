// unit-test-ast-types.cpp
//
// Tests for AST type construction and the deduplication invariant, exercised
// through `ASTBuilder` without compiling any source.
//
// Slang relies on structurally identical types being the *same* object. Much of
// the compiler compares types by pointer, and `Val::equals` and the various
// deduplication caches assume the same thing. If `ASTBuilder` ever returned two
// distinct `vector<float,3>` objects, comparisons would start reporting that a
// type differs from itself, in ways that surface far from the cause. These tests
// pin that invariant at its source.

#include "slang/slang-ast-builder.h"
#include "slang/slang-mangle.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Requesting the same vector type twice yields the same object, and `equals`
// agrees with pointer identity.
SLANG_UNIT_TEST(astVectorTypesAreDeduplicated)
{
    StaticUnitTestEnv env(unitTestContext);
    ASTBuilder* astBuilder = env.getASTBuilder();

    Type* floatType = astBuilder->getFloatType();
    Type* intType = astBuilder->getIntType();
    IntVal* three = astBuilder->getIntVal(intType, 3);

    Type* first = astBuilder->getVectorType(floatType, three);
    Type* second = astBuilder->getVectorType(floatType, three);

    SLANG_CHECK(first == second);
    SLANG_CHECK(first->equals(second));
}

// `getVectorType` canonicalizes a constant element count to `int` before consulting
// the type cache, so a count spelled `uint` 3 and one spelled `int` 3 name the same
// vector type.
//
// Note what this test deliberately does *not* do: calling `getIntVal(intType, 3)`
// twice would prove nothing, because `getIntVal` deduplicates on (type, value) and
// would hand back the same object both times. The assertion would then hold whether
// or not the canonicalization existed. Two different *types* are what produce two
// genuinely distinct `ConstantIntVal`s of equal value.
SLANG_UNIT_TEST(astVectorDeduplicationCanonicalizesElementCountType)
{
    StaticUnitTestEnv env(unitTestContext);
    ASTBuilder* astBuilder = env.getASTBuilder();

    Type* floatType = astBuilder->getFloatType();
    IntVal* intThree = astBuilder->getIntVal(astBuilder->getIntType(), 3);
    IntVal* uintThree = astBuilder->getIntVal(astBuilder->getUIntType(), 3);

    // Guards the premise. If these were the same object, the check below would pass
    // for a reason that has nothing to do with canonicalization.
    SLANG_CHECK_ABORT(intThree != uintThree);

    Type* first = astBuilder->getVectorType(floatType, intThree);
    Type* second = astBuilder->getVectorType(floatType, uintThree);

    SLANG_CHECK(first == second);
}

// Types that genuinely differ — in element count or element type — are distinct
// and compare unequal. Without this, a deduplication scheme that collapsed too
// much would pass the tests above.
SLANG_UNIT_TEST(astDistinctVectorTypesAreNotEqual)
{
    StaticUnitTestEnv env(unitTestContext);
    ASTBuilder* astBuilder = env.getASTBuilder();

    Type* floatType = astBuilder->getFloatType();
    Type* intType = astBuilder->getIntType();

    Type* float3 = astBuilder->getVectorType(floatType, astBuilder->getIntVal(intType, 3));
    Type* float4 = astBuilder->getVectorType(floatType, astBuilder->getIntVal(intType, 4));
    Type* int3 = astBuilder->getVectorType(intType, astBuilder->getIntVal(intType, 3));

    SLANG_CHECK(float3 != float4);
    SLANG_CHECK(!float3->equals(float4));

    SLANG_CHECK(float3 != int3);
    SLANG_CHECK(!float3->equals(int3));
}

// Mangled type names distinguish types that are not equal. Mangled names are how
// separately-compiled modules agree on symbols, so two different types sharing a
// mangled name would let one definition silently satisfy a reference to another.
SLANG_UNIT_TEST(astMangledTypeNamesDistinguishDistinctTypes)
{
    StaticUnitTestEnv env(unitTestContext);
    ASTBuilder* astBuilder = env.getASTBuilder();

    Type* floatType = astBuilder->getFloatType();
    Type* intType = astBuilder->getIntType();

    Type* float3 = astBuilder->getVectorType(floatType, astBuilder->getIntVal(intType, 3));
    Type* float4 = astBuilder->getVectorType(floatType, astBuilder->getIntVal(intType, 4));
    Type* int3 = astBuilder->getVectorType(intType, astBuilder->getIntVal(intType, 3));

    String float3Name = getMangledTypeName(astBuilder, float3);
    String float4Name = getMangledTypeName(astBuilder, float4);
    String int3Name = getMangledTypeName(astBuilder, int3);

    SLANG_CHECK(float3Name.getLength() > 0);
    SLANG_CHECK(float3Name != float4Name);
    SLANG_CHECK(float3Name != int3Name);

    // Mangling is deterministic: the same type mangles identically every time.
    SLANG_CHECK(getMangledTypeName(astBuilder, float3) == float3Name);
}
