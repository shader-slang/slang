// unit-test-math-int-ops.cpp

#include "core/slang-math-int-ops.h"
#include "unit-test/slang-unit-test.h"

#include <cinttypes>
#include <cstdio>

using namespace Slang;

// Unit tests for the public operations of MathIntOps. Each operation has its
// own test so that a failure points directly at the operation in question.

namespace
{

bool checkEquals(const MathIntResult& a, const MathIntResult& b)
{
    bool ret{true};

    if (a.value.getUnsignedValue() != b.value.getUnsignedValue())
    {
        char msgbuf[128];
        snprintf(
            msgbuf,
            sizeof msgbuf,
            "a.value.rawValue (0x%016" PRIx64 ") != b.value.rawValue (0x%016" PRIx64 ")",
            a.value.getUnsignedValue(),
            b.value.getUnsignedValue());
        getTestReporter()->message(TestMessageType::TestFailure, msgbuf);
        ret = false;
    }

    if (a.value.getType().width != b.value.getType().width)
    {
        getTestReporter()->message(
            TestMessageType::TestFailure,
            "a.value.type.width != b.value.type.width");
        ret = false;
    }

    if (a.value.getType().isSigned != b.value.getType().isSigned)
    {
        getTestReporter()->message(
            TestMessageType::TestFailure,
            "a.value.type.isSigned != b.value.type.isSigned");
        ret = false;
    }

    if (a.flags != b.flags)
    {
        char msgbuf[128];
        snprintf(msgbuf, sizeof msgbuf, "a.flags (0x%08x) != b.flags (0x%08x)", a.flags, b.flags);
        getTestReporter()->message(TestMessageType::TestFailure, msgbuf);
        ret = false;
    }

    return ret;
}

} // namespace

// Casts
SLANG_UNIT_TEST(mathIntOpsCast)
{
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 0xFEU, 0),
        MathIntOps::cast(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::UInt8Type, 0xFE))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 0xFEU, 0),
        MathIntOps::cast(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::UInt16Type, 0xFE))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 0xFEU, MathIntResult::Overflow),
        MathIntOps::cast(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::UInt16Type, 0x1FE))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -2, 0),
        MathIntOps::cast(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -2))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -2, 0),
        MathIntOps::cast(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -2))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -2, MathIntResult::Overflow),
        MathIntOps::cast(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -258))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 254, 0),
        MathIntOps::cast(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 254))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 254, MathIntResult::Overflow),
        MathIntOps::cast(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -2))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, UINT64_MAX, MathIntResult::Overflow),
        MathIntOps::cast(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, -1, MathIntResult::Overflow),
        MathIntOps::cast(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX))));

    // narrow negative to 64-bit unsigned: the bit pattern is unchanged by
    // sign-extension, but the value changes
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, UINT64_MAX, MathIntResult::Overflow),
        MathIntOps::cast(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));

    // widening casts preserve the value and must not report overflow
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, -1, 0),
        MathIntOps::cast(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 255, 0),
        MathIntOps::cast(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt8Type, 255))));
}

SLANG_UNIT_TEST(mathIntOpsCastToBool)
{
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::BoolType, 0, 0),
        MathIntOps::castToBool(MathIntValue::constructFromValue(MathIntType::Int16Type, 0))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::BoolType, 1, 0),
        MathIntOps::castToBool(MathIntValue::constructFromValue(MathIntType::Int16Type, 1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::BoolType, 1, 0),
        MathIntOps::castToBool(MathIntValue::constructFromValue(MathIntType::Int16Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::BoolType, 1, 0),
        MathIntOps::castToBool(MathIntValue::constructFromValue(MathIntType::Int16Type, 11000))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::BoolType, 0, 0),
        MathIntOps::castToBool(MathIntValue::constructFromValue(MathIntType::UInt64Type, 0))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::BoolType, 1, 0),
        MathIntOps::castToBool(MathIntValue::constructFromValue(MathIntType::UInt64Type, 1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::BoolType, 1, 0),
        MathIntOps::castToBool(MathIntValue::constructFromValue(MathIntType::UInt64Type, 11000))));
}


// Comparisons
namespace
{

enum class ComparisonType
{
    Equal,
    NotEqual,
    Less,
    LessEqual,
    GreaterEqual,
    Greater,
};

enum class OrderType
{
    LhsLessThanRhs,
    LhsEqualToRhs,
    LhsGreaterThanRhs,
};

MathIntResult getCompareExpectancy(ComparisonType c, OrderType o)
{
    bool expect;
    switch (c)
    {
    case ComparisonType::Equal:
        expect = (o == OrderType::LhsEqualToRhs);
        break;

    case ComparisonType::NotEqual:
        expect = (o != OrderType::LhsEqualToRhs);
        break;

    case ComparisonType::Less:
        expect = (o == OrderType::LhsLessThanRhs);
        break;

    case ComparisonType::LessEqual:
        expect = (o != OrderType::LhsGreaterThanRhs);
        break;

    case ComparisonType::GreaterEqual:
        expect = (o != OrderType::LhsLessThanRhs);
        break;

    case ComparisonType::Greater:
        expect = (o == OrderType::LhsGreaterThanRhs);
        break;

    default:
        SLANG_RELEASE_ASSERT(!"Bad comparison type");
        break;
    }

    return MathIntOps::constructResult(MathIntType::BoolType, static_cast<unsigned>(expect), 0);
}

void checkComparisonPattern(
    MathIntResult (&fn)(const MathIntValue& a, const MathIntValue& b),
    ComparisonType compType)
{
    std::size_t i{};

    // equality
    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsEqualToRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int64Type, -1),
           MathIntValue::constructFromValue(MathIntType::Int64Type, -1))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsEqualToRhs),
        fn(MathIntValue::constructFromValue(MathIntType::UInt64Type, 0U),
           MathIntValue::constructFromValue(MathIntType::UInt64Type, 0U))));
    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsEqualToRhs),
        fn(MathIntValue::constructFromValue(MathIntType::UInt64Type, 0U),
           MathIntValue::constructFromValue(MathIntType::Int64Type, 0U))));
    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsEqualToRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int64Type, 0xFFFFFFFFFFFFFFFF),
           MathIntValue::constructFromValue(MathIntType::Int64Type, 0xFFFFFFFFFFFFFFFF))));
    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsEqualToRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int16Type, 0xFFFF),
           MathIntValue::constructFromValue(MathIntType::Int64Type, 0xFFFFFFFFFFFFFFFF))));
    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsEqualToRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int16Type, 0xFFFF),
           MathIntValue::constructFromValue(MathIntType::Int8Type, 0xFF))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsEqualToRhs),
        fn(MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xFFFF),
           MathIntValue::constructFromValue(MathIntType::UInt16Type, 0xFFFFFFFFFFFFFFFF))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsEqualToRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int16Type, 0x7FFF),
           MathIntValue::constructFromValue(MathIntType::UInt32Type, 0x00007FFF))));

    // non-equality
    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsLessThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int64Type, 0xFFFFFFFFFFFFFFFF),
           MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xFFFFFFFFFFFFFFFF))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsGreaterThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xFFFFFFFFFFFFFFFF),
           MathIntValue::constructFromValue(MathIntType::Int64Type, 0xFFFFFFFFFFFFFFFF))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsLessThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int64Type, 0x8000000000000000),
           MathIntValue::constructFromValue(MathIntType::Int64Type, 0x7FFFFFFFFFFFFFFF))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsGreaterThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int64Type, 0x7FFFFFFFFFFFFFFF),
           MathIntValue::constructFromValue(MathIntType::Int64Type, 0x8000000000000000))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsGreaterThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::UInt16Type, 0xFFFF),
           MathIntValue::constructFromValue(MathIntType::Int16Type, 0xFFFF))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsLessThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int16Type, 0xFFFF),
           MathIntValue::constructFromValue(MathIntType::UInt16Type, 0xFFFF))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsGreaterThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int16Type, 0x7FFF),
           MathIntValue::constructFromValue(MathIntType::Int16Type, 0x8000))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsLessThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int16Type, 0x8000),
           MathIntValue::constructFromValue(MathIntType::Int16Type, 0x7FFF))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsLessThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int16Type, 0x7FFF),
           MathIntValue::constructFromValue(MathIntType::UInt16Type, 0x8000))));

    SLANG_CHECK(checkEquals(
        getCompareExpectancy(compType, OrderType::LhsLessThanRhs),
        fn(MathIntValue::constructFromValue(MathIntType::Int32Type, 0x80000000U),
           MathIntValue::constructFromValue(MathIntType::UInt16Type, 0x7FFF))));
}

} // namespace

SLANG_UNIT_TEST(mathIntOpsCompEql)
{
    checkComparisonPattern(MathIntOps::compEql, ComparisonType::Equal);
}

SLANG_UNIT_TEST(mathIntOpsCompNeq)
{
    checkComparisonPattern(MathIntOps::compNeq, ComparisonType::NotEqual);
}

SLANG_UNIT_TEST(mathIntOpsCompGeq)
{
    checkComparisonPattern(MathIntOps::compGeq, ComparisonType::GreaterEqual);
}

SLANG_UNIT_TEST(mathIntOpsCompLeq)
{
    checkComparisonPattern(MathIntOps::compLeq, ComparisonType::LessEqual);
}

SLANG_UNIT_TEST(mathIntOpsCompGreater)
{
    checkComparisonPattern(MathIntOps::compGreater, ComparisonType::Greater);
}

SLANG_UNIT_TEST(mathIntOpsCompLess)
{
    checkComparisonPattern(MathIntOps::compLess, ComparisonType::Less);
}

// Arithmetic

SLANG_UNIT_TEST(mathIntOpsAdd)
{
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, 0, 0),
        MathIntOps::add(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 123),
            MathIntValue::constructFromValue(MathIntType::Int16Type, -123))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, 32767, 0),
        MathIntOps::add(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 32000),
            MathIntValue::constructFromValue(MathIntType::Int16Type, 767))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, -32768, MathIntResult::Overflow),
        MathIntOps::add(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 32000),
            MathIntValue::constructFromValue(MathIntType::Int16Type, 768))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, -32768, 0),
        MathIntOps::add(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 32000),
            MathIntValue::constructFromValue(MathIntType::Int32Type, -64768))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, 32767, MathIntResult::Overflow),
        MathIntOps::add(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 32000),
            MathIntValue::constructFromValue(MathIntType::Int32Type, -64769))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, -32768, 0),
        MathIntOps::add(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -10000),
            MathIntValue::constructFromValue(MathIntType::Int16Type, -22768))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, 32767, MathIntResult::Overflow),
        MathIntOps::add(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -10000),
            MathIntValue::constructFromValue(MathIntType::Int16Type, -22769))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 0, MathIntResult::Overflow),
        MathIntOps::add(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MAX, MathIntResult::Overflow),
        MathIntOps::add(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::Int64Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, -1, 0),
        MathIntOps::add(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MAX, 0),
        MathIntOps::add(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, INT64_MAX, 0),
        MathIntOps::add(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0, MathIntResult::Overflow),
        MathIntOps::add(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 1))));
}

SLANG_UNIT_TEST(mathIntOpsSub)
{
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, 0, 0),
        MathIntOps::sub(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 123),
            MathIntValue::constructFromValue(MathIntType::Int16Type, 123))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, 32767, 0),
        MathIntOps::sub(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 32000),
            MathIntValue::constructFromValue(MathIntType::Int16Type, -767))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, -32768, MathIntResult::Overflow),
        MathIntOps::sub(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 32000),
            MathIntValue::constructFromValue(MathIntType::Int16Type, -768))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, -32768, 0),
        MathIntOps::sub(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 32000),
            MathIntValue::constructFromValue(MathIntType::Int32Type, 64768))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, 32767, MathIntResult::Overflow),
        MathIntOps::sub(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 32000),
            MathIntValue::constructFromValue(MathIntType::Int32Type, 64769))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, -32768, 0),
        MathIntOps::sub(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -10000),
            MathIntValue::constructFromValue(MathIntType::Int16Type, 22768))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, 32767, MathIntResult::Overflow),
        MathIntOps::sub(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -10000),
            MathIntValue::constructFromValue(MathIntType::Int16Type, 22769))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 0, 0),
        MathIntOps::sub(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0, 0),
        MathIntOps::sub(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, -1, 0),
        MathIntOps::sub(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::Int64Type, -INT64_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN, 0),
        MathIntOps::sub(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MAX, MathIntResult::Overflow),
        MathIntOps::sub(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, UINT64_MAX, 0),
        MathIntOps::sub(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX),
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0, MathIntResult::Overflow),
        MathIntOps::sub(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX),
            MathIntValue::constructFromValue(MathIntType::Int64Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, UINT64_MAX - 1U, 0),
        MathIntOps::sub(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 1))));
}

SLANG_UNIT_TEST(mathIntOpsMul)
{
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 24, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 6),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -24, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -6),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -24, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 6),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 24, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -6),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -4))));

    // overflow checks
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 124, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 31),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -128, MathIntResult::Overflow),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 32),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 124, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -31),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -128, MathIntResult::Overflow),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -32),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -124, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -31),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -128, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -32),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 124, MathIntResult::Overflow),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -33),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -128, 0),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 32),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -4))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 124, MathIntResult::Overflow),
        MathIntOps::mul(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 33),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -4))));

    // Limits
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN, MathIntResult::Overflow),
        MathIntOps::mul(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MAX, 0),
        MathIntOps::mul(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, -INT64_MAX),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0xFFFFFFFF00000000U, 0),
        MathIntOps::mul(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT32_MAX + 1ULL),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT32_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0xFFFFFFFFFFFFFFFFU, 0),
        MathIntOps::mul(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT32_MAX + 2ULL),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT32_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0, MathIntResult::Overflow),
        MathIntOps::mul(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT32_MAX + 1ULL),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT32_MAX + 1ULL))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, UINT64_MAX, MathIntResult::Overflow),
        MathIntOps::mul(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 1),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));
}


SLANG_UNIT_TEST(mathIntOpsDiv)
{
    // basics
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 1, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 2),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 2))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 1, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 3),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 2))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 2, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 4),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 2))));

    // rounding toward zero
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -3, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -7),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 2))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -3, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 7),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -2))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 3, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -7),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -2))));

    // 8-bit int limits

    // pos / pos
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 127, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 12799),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 100))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::Int8Type,
            -128,
            MathIntResult::Overflow | MathIntResult::TargetDefined),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 12800),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 100))));

    // pos / neg
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -127, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 12700),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -100))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::Int8Type,
            127,
            MathIntResult::Overflow | MathIntResult::TargetDefined),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 12900),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -100))));

    // neg / neg
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 127, 0),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -12799),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -100))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::Int8Type,
            -128,
            MathIntResult::Overflow | MathIntResult::TargetDefined),
        MathIntOps::div(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -12800),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -100))));

    // limits
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN, 0),
        MathIntOps::div(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, uint64_t{INT64_MAX} + 1ULL),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::Int64Type,
            INT64_MAX,
            MathIntResult::Overflow | MathIntResult::TargetDefined),
        MathIntOps::div(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, uint64_t{INT64_MAX} + 2ULL),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN + 1, 0),
        MathIntOps::div(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, -INT64_MAX),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0x8000000000000000U, 0),
        MathIntOps::div(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));

    // special values
    // 1/0
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, UINT64_MAX, MathIntResult::Undefined),
        MathIntOps::div(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 1),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 0))));

    // 0/0
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MAX, MathIntResult::Undefined),
        MathIntOps::div(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 0))));

    // division by -1
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::Int32Type,
            INT32_MIN,
            MathIntResult::Overflow | MathIntResult::TargetDefined),
        MathIntOps::div(
            MathIntType::Int32Type,
            MathIntValue::constructFromValue(MathIntType::Int32Type, INT32_MIN),
            MathIntValue::constructFromValue(MathIntType::Int32Type, -1))));
}

SLANG_UNIT_TEST(mathIntOpsRem)
{
    // From the spec:
    //
    // The remainder operator returns the remainder of lhs by rhs division such
    // that rem = lhs - n * rhs where n is an integer and abs(rem) < abs(rhs).
    // The sign of remainder matches the sign of lhs. That is,
    //
    //   lhs % rhs == lhs - trunc(lhs / rhs) * rhs

    // Basics:
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 3, 0),
        MathIntOps::rem(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 23),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 10))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 3, 0),
        MathIntOps::rem(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, 23),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -10))));


    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -3, 0),
        MathIntOps::rem(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -23),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 10))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -3, 0),
        MathIntOps::rem(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -23),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -10))));

    // limits
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, UINT64_MAX - 1U, 0),
        MathIntOps::rem(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX - 1U),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::UInt64Type,
            9223372036854775807U,
            MathIntResult::TargetDefined),
        MathIntOps::rem(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX),
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN, 0),
        MathIntOps::rem(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN + 1, 0),
        MathIntOps::rem(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN + 1),
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0, 0),
        MathIntOps::rem(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 1U))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0, MathIntResult::TargetDefined),
        MathIntOps::rem(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, UINT64_MAX),
            MathIntValue::constructFromValue(MathIntType::Int64Type, -1))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 0, 0),
        MathIntOps::rem(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 1U))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 0, MathIntResult::TargetDefined),
        MathIntOps::rem(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN),
            MathIntValue::constructFromValue(MathIntType::Int64Type, -1))));

    // undefined
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 0, MathIntResult::Undefined),
        MathIntOps::rem(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN + 1),
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 0, MathIntResult::Undefined),
        MathIntOps::rem(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0),
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0))));

    // overflow check
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -1, MathIntResult::Overflow),
        MathIntOps::rem(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 32123),
            MathIntValue::constructFromValue(MathIntType::Int64Type, 257))));
}

SLANG_UNIT_TEST(mathIntOpsNeg)
{
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int16Type, -123, 0),
        MathIntOps::neg(
            MathIntType::Int16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 123))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt16Type, 123, 0),
        MathIntOps::neg(
            MathIntType::UInt16Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -123))));

    // boundary value checks - unsigned target
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 255, MathIntResult::Overflow),
        MathIntOps::neg(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 1))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 0, 0),
        MathIntOps::neg(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 0))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 255, 0),
        MathIntOps::neg(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -255))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt8Type, 256, MathIntResult::Overflow),
        MathIntOps::neg(
            MathIntType::UInt8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -256))));

    // boundary value checks - signed target
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 127, MathIntResult::Overflow),
        MathIntOps::neg(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::UInt16Type, 129))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -128, 0),
        MathIntOps::neg(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::UInt16Type, 128))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, 127, 0),
        MathIntOps::neg(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -127))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int8Type, -128, MathIntResult::Overflow),
        MathIntOps::neg(
            MathIntType::Int8Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -128))));

    // boundary value checks - uint64_t target
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::UInt64Type,
            0xFFFFFFFFFFFFFFFFU,
            MathIntResult::Overflow),
        MathIntOps::neg(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt8Type, 1))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0, 0),
        MathIntOps::neg(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt16Type, 0))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 1U, 0),
        MathIntOps::neg(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int8Type, -1))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0x8000000000000000U, 0),
        MathIntOps::neg(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0x7FFFFFFFFFFFFFFFU, 0),
        MathIntOps::neg(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, INT64_MIN + 1))));

    // boundary value checks - int64_t target
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN + 1, 0),
        MathIntOps::neg(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x7FFFFFFFFFFFFFFFU))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN, 0),
        MathIntOps::neg(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x8000000000000000U))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN, MathIntResult::Overflow),
        MathIntOps::neg(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0x8000000000000000U))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MAX, MathIntResult::Overflow),
        MathIntOps::neg(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x8000000000000001U))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MAX - 1, 0),
        MathIntOps::neg(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0x8000000000000002))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MAX, 0),
        MathIntOps::neg(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0x8000000000000001))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, INT64_MIN, MathIntResult::Overflow),
        MathIntOps::neg(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0x8000000000000000))));
}

// Bitwise operations
SLANG_UNIT_TEST(mathIntOpsBitwiseAnd)
{
    // Note: zero/sign extension, then OP, then store. No overflow checks.
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0xF0F0F0F080808080, 0U),
        MathIntOps::bitwiseAnd(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xF0F0F0F0F0F0F0F0U),
            MathIntValue::constructFromValue(MathIntType::Int32Type, 0x88888888))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int32Type, 0xFFFFFFFF80808080, 0U),
        MathIntOps::bitwiseAnd(
            MathIntType::Int32Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xF0F0F0F0F0F0F0F0U),
            MathIntValue::constructFromValue(MathIntType::Int32Type, 0x88888888))));
}

SLANG_UNIT_TEST(mathIntOpsBitwiseOr)
{
    // Note: zero/sign extension, then OP, then store. No overflow checks.
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0xFFFFFFFFF8F8F8F8, 0U),
        MathIntOps::bitwiseOr(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xF0F0F0F0F0F0F0F0U),
            MathIntValue::constructFromValue(MathIntType::Int32Type, 0x88888888))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int32Type, 0xF0F0F0F0F8F8F8F8, 0U),
        MathIntOps::bitwiseOr(
            MathIntType::Int32Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xF0F0F0F0F0F0F0F0U),
            MathIntValue::constructFromValue(MathIntType::UInt32Type, 0x88888888))));
}

SLANG_UNIT_TEST(mathIntOpsBitwiseXor)
{
    // Note: zero/sign extension, then OP, then store. No overflow checks.
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0x0F0F0F0F78787878, 0U),
        MathIntOps::bitwiseXor(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xF0F0F0F0F0F0F0F0U),
            MathIntValue::constructFromValue(MathIntType::Int32Type, 0x88888888))));

    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0xF0F0F0F078787878, 0U),
        MathIntOps::bitwiseXor(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xF0F0F0F0F0F0F0F0U),
            MathIntValue::constructFromValue(MathIntType::UInt32Type, 0x88888888))));
}

SLANG_UNIT_TEST(mathIntOpsBitwiseNot)
{
    // Note: zero/sign extension, then OP, then store. No overflow checks.
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt32Type, 0x0000000012345678, 0U),
        MathIntOps::bitwiseNot(
            MathIntType::UInt32Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x12345678EDCBA987U))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int32Type, 0x0000000012345678, 0U),
        MathIntOps::bitwiseNot(
            MathIntType::Int32Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x12345678EDCBA987U))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int32Type, 0xFFFFFFFF82345678, 0U),
        MathIntOps::bitwiseNot(
            MathIntType::Int32Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x123456787DCBA987U))));
}

// Shifts

SLANG_UNIT_TEST(mathIntOpsLsh)
{
    // basics
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 1U, 0U),
        MathIntOps::lsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x0000000000000001U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 0))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 2U, 0U),
        MathIntOps::lsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x0000000000000001U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 1))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0x8000000000000000U, 0U),
        MathIntOps::lsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x0000000000000001U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 63))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0x1110000000000000U, 0U),
        MathIntOps::lsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x0000000000001111U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 52))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::UInt64Type,
            0x0000000000000001U,
            MathIntResult::TargetDefined),
        MathIntOps::lsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x0000000000000001U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 64))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::UInt64Type,
            0x4000000000000000U,
            MathIntResult::TargetDefined),
        MathIntOps::lsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xFFFFFFFFFFFFFFF1U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -2))));

    // mismatching types for 'a' and target. 'a' is zero-extended or sign-extended and then shifted.
    // Target-defined shift is per target type
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, -4398046511104, 0),
        MathIntOps::lsh(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, -1024),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 32))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 4398046511104, 0),
        MathIntOps::lsh(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 1024),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 32))));
}

SLANG_UNIT_TEST(mathIntOpsRsh)
{
    // basics
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 0x4444444444444444U, 0U),
        MathIntOps::rsh(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x8888888888888888U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 1))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0xC444444444444444U, 0U),
        MathIntOps::rsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0x8888888888888888U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 1))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0x0000000000000001U, 0U),
        MathIntOps::rsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0xFFFFFFFFFFFFFFFFU),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 63))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0xFFFFFFFFFFFFFFFFU, 0U),
        MathIntOps::rsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int64Type, 0x8000000000000000U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 63))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::UInt64Type,
            0x123456789ABCDEF0U,
            MathIntResult::TargetDefined),
        MathIntOps::rsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x123456789ABCDEF0U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 64))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(
            MathIntType::UInt64Type,
            0x00123456789ABCDE,
            MathIntResult::TargetDefined),
        MathIntOps::rsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::UInt64Type, 0x123456789ABCDEF0U),
            MathIntValue::constructFromValue(MathIntType::Int8Type, -56))));

    // mismatching types for 'a' and target. 'a' is zero-extended or sign-extended and then shifted.
    // Target-defined shift is per target type
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::UInt64Type, 0xFFFFFFFFFFFFC040U, 0),
        MathIntOps::rsh(
            MathIntType::UInt64Type,
            MathIntValue::constructFromValue(MathIntType::Int16Type, 0x8080),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 1))));
    SLANG_CHECK(checkEquals(
        MathIntOps::constructResult(MathIntType::Int64Type, 0x0000000000004040U, 0),
        MathIntOps::rsh(
            MathIntType::Int64Type,
            MathIntValue::constructFromValue(MathIntType::UInt16Type, 0x8080),
            MathIntValue::constructFromValue(MathIntType::Int8Type, 1))));
}
