#include "slang-math-int-ops.h"

#include <bit>
#include <cmath>
#include <type_traits>

namespace Slang
{

const MathIntType MathIntType::NullType = MathIntType{.width = 0, .isSigned = false};

const MathIntType MathIntType::BoolType = MathIntType{.width = 1, .isSigned = false};

const MathIntType MathIntType::Int8Type = MathIntType{.width = 8, .isSigned = true};
const MathIntType MathIntType::Int16Type = MathIntType{.width = 16, .isSigned = true};
const MathIntType MathIntType::Int32Type = MathIntType{.width = 32, .isSigned = true};
const MathIntType MathIntType::Int64Type = MathIntType{.width = 64, .isSigned = true};

const MathIntType MathIntType::UInt8Type = MathIntType{.width = 8, .isSigned = false};
const MathIntType MathIntType::UInt16Type = MathIntType{.width = 16, .isSigned = false};
const MathIntType MathIntType::UInt32Type = MathIntType{.width = 32, .isSigned = false};
const MathIntType MathIntType::UInt64Type = MathIntType{.width = 64, .isSigned = false};


namespace
{

bool isNegative(const MathIntValue& v)
{
    return v.getType().isSigned && (v.getSignedValue() < 0);
}

int64_t getMinForType(const MathIntType& type)
{
    SLANG_RELEASE_ASSERT(type.width <= 64U);

    if (type.width > 0U && type.isSigned)
        return std::numeric_limits<int64_t>::min() >> int(64 - type.width);
    else
        return 0;
}

uint64_t getMaxForType(const MathIntType& type)
{
    SLANG_RELEASE_ASSERT(type.width <= 64U);

    if (type.width > 0U && type.isSigned)
        return (uint64_t{1} << (type.width - 1U)) - 1U;
    else
    {
        if (type.width == 64U)
            return uint64_t{0} - uint64_t{1};
        else
            return (uint64_t{1} << (type.width & 63U)) - 1U;
    }
}

// Return the two's-complement negation of `a`, i.e. `-a` modulo 2^64. We negate unsigned
// magnitudes because negating a signed value such as INT64_MIN overflows, which is undefined
// behavior. Writing `-a` on a `uint64_t` is well defined but triggers MSVC warning C4146, so we
// spell the negation out here. The template parameter with `static_assert` rejects any argument
// that would otherwise convert implicitly to `uint64_t`.
template<typename T>
constexpr uint64_t negU64(T a)
{
    static_assert(std::is_same_v<T, uint64_t>);
    return uint64_t{0} - a;
}

struct Int128
{
    uint64_t hi;
    uint64_t lo;
};

template<typename T>
Int128 extendToI128(T a)
{
    Int128 ret{};

    if constexpr (std::is_signed_v<T>)
    {
        ret.hi = a < 0 ? std::numeric_limits<uint64_t>::max() : 0U;
        ret.lo = static_cast<uint64_t>(int64_t{a});
    }
    else
    {
        ret.hi = 0U;
        ret.lo = uint64_t{a};
    }

    return ret;
}

Int128 extendToI128(MathIntValue a)
{
    if (a.getType().isSigned)
        return extendToI128(a.getSignedValue());
    else
        return extendToI128(a.getUnsignedValue());
}

Int128 addI128(Int128 a, Int128 b)
{
    Int128 ret;

    ret.hi = a.hi + b.hi;
    ret.lo = a.lo + b.lo;

    // lo overflow check
    if (ret.lo < a.lo)
        ++ret.hi;

    return ret;
}

Int128 subI128(Int128 a, Int128 b)
{
    Int128 ret;

    ret.hi = a.hi - b.hi;
    ret.lo = a.lo - b.lo;

    // lo overflow check
    if (a.lo < b.lo)
        --ret.hi;

    return ret;
}

Int128 negI128(Int128 a)
{
    return subI128(Int128{}, a);
}

enum class ArgumentOrder
{
    LhsLessThanRhs,
    LhsEqualToRhs,
    LhsGreaterThanRhs,
};

ArgumentOrder determineOrder(const MathIntValue& a, const MathIntValue& b)
{
    // First: sign-based ordering
    if (isNegative(a) && !isNegative(b))
        return ArgumentOrder::LhsLessThanRhs;

    if (!isNegative(a) && isNegative(b))
        return ArgumentOrder::LhsGreaterThanRhs;

    // Second: same sign, compare unsigned raw values. This is correct for both
    // negative and non-negative values.
    // - Non-negative values: obvious (unsigned raw value)
    // - Negative values: The order is maintained from [INT64_MIN, -1] which maps to
    //   [0x8000000000000000, 0xFFFFFFFFFFFFFFFF] by adding 2^64 to the negative values,
    //   which cast from int64_t to uint64_t does.
    if (a.getUnsignedValue() < b.getUnsignedValue())
        return ArgumentOrder::LhsLessThanRhs;

    if (a.getUnsignedValue() > b.getUnsignedValue())
        return ArgumentOrder::LhsGreaterThanRhs;

    return ArgumentOrder::LhsEqualToRhs;
}

ArgumentOrder compareI128(Int128 a, Int128 b)
{
    Int128 tmp = subI128(a, b);

    if (static_cast<int64_t>(tmp.hi) < 0)
        return ArgumentOrder::LhsLessThanRhs;

    if (tmp.hi == 0U && tmp.lo == 0U)
        return ArgumentOrder::LhsEqualToRhs;

    return ArgumentOrder::LhsGreaterThanRhs;
}

bool valueInRange(const MathIntType& type, const Int128& a)
{
    Int128 minForType = extendToI128(getMinForType(type));
    if (compareI128(a, minForType) == ArgumentOrder::LhsLessThanRhs)
        return false;

    Int128 maxForType = extendToI128(getMaxForType(type));
    if (compareI128(a, maxForType) == ArgumentOrder::LhsGreaterThanRhs)
        return false;

    return true;
}

} // namespace

void MathIntValue::normalize()
{
    SLANG_RELEASE_ASSERT(type.width <= 64);

    if (type.width == 0)
    {
        rawValue = 0;
    }
    else if (type.width <= 63)
    {
        if (type.isSigned)
        {
            // sign-extend leading bits
            const int shift = 64 - type.width;
            int64_t tmpValue = static_cast<int64_t>(rawValue);
            tmpValue <<= shift;
            tmpValue >>= shift;
            rawValue = static_cast<uint64_t>(tmpValue);
        }
        else
        {
            // zero-extend leading bits
            const unsigned int shift = 64 - type.width;
            rawValue <<= shift;
            rawValue >>= shift;
        }
    }

    // note: the raw value of 64-bit integers is already normalized
}

String MathIntValue::toString() const
{
    if (!type.isSigned && type.width == 1U)
    {
        if (getUnsignedValue())
            return String("true");
        else
            return String("false");
    }

    String num;
    if (type.isSigned)
        num.append(getSignedValue());
    else
        num.append(getUnsignedValue());

    if (type.width != 32 && type.width != 64)
    {
        String ret;
        if (type.isSigned)
            ret = "int";
        else
            ret = "uint";

        ret.append(unsigned{type.width});
        ret.append("_t(");
        ret.append(num);
        ret.append(")");
        return ret;
    }
    else
    {
        if (!type.isSigned)
            num.append("U");

        if (type.width == 64)
            num.append("LL");

        return num;
    }
}

MathIntResult MathIntOps::convertFloatValue(const MathIntType& resultType, double value)
{
    SLANG_RELEASE_ASSERT(resultType.width > 0U && resultType.width <= 64U);

    if (std::isnan(value))
        return constructResult(resultType, 0U, MathIntResult::TargetDefined);

    if (resultType.width == MathIntType::BoolType.width)
        return constructBoolResult(value != 0.0);

    // We compare the truncated value against the bounds of `resultType` in `double`, where
    // every bound is exact: -2^(width-1) and 2^(width-1) for signed types, and 2^width for
    // unsigned types. A value that passes the check converts to int64_t or uint64_t with
    // defined behavior. An infinity fails the check, because it compares beyond every bound.
    //
    // Note: the lower bound is inclusive and the upper bound is exclusive to
    // avoid rounding of boundary values. For example, UINT64_MAX is not exactly
    // representable as a double, but INT64_MIN is.
    const double truncated = std::trunc(value);

    if (resultType.isSigned)
    {
        const double minValue = static_cast<double>(getMinForType(resultType));
        const double maxExclusive = -minValue;
        if (!(truncated >= minValue && truncated < maxExclusive))
            return constructResult(resultType, 0U, MathIntResult::TargetDefined);

        return constructResult(resultType, static_cast<int64_t>(truncated), 0U);
    }

    const double maxExclusive = std::ldexp(1.0, resultType.width);
    if (!(truncated >= 0.0 && truncated < maxExclusive))
        return constructResult(resultType, 0U, MathIntResult::TargetDefined);

    return constructResult(resultType, static_cast<uint64_t>(truncated), 0U);
}

MathIntResult MathIntOps::cast(const MathIntType& resultType, const MathIntValue& a)
{
    const bool overflow = !valueInRange(resultType, extendToI128(a));
    return constructResult(
        resultType,
        a.getUnsignedValue(),
        overflow ? unsigned(MathIntResult::Overflow) : 0);
}

MathIntResult MathIntOps::castToBool(const MathIntValue& a)
{
    return constructBoolResult(a.getUnsignedValue() != 0U);
}

MathIntResult MathIntOps::compEql(const MathIntValue& a, const MathIntValue& b)
{
    return constructBoolResult(determineOrder(a, b) == ArgumentOrder::LhsEqualToRhs);
}

MathIntResult MathIntOps::compNeq(const MathIntValue& a, const MathIntValue& b)
{
    return constructBoolResult(determineOrder(a, b) != ArgumentOrder::LhsEqualToRhs);
}

MathIntResult MathIntOps::compGeq(const MathIntValue& a, const MathIntValue& b)
{
    return constructBoolResult(determineOrder(a, b) != ArgumentOrder::LhsLessThanRhs);
}

MathIntResult MathIntOps::compLeq(const MathIntValue& a, const MathIntValue& b)
{
    return constructBoolResult(determineOrder(a, b) != ArgumentOrder::LhsGreaterThanRhs);
}

MathIntResult MathIntOps::compGreater(const MathIntValue& a, const MathIntValue& b)
{
    return constructBoolResult(determineOrder(a, b) == ArgumentOrder::LhsGreaterThanRhs);
}

MathIntResult MathIntOps::compLess(const MathIntValue& a, const MathIntValue& b)
{
    return constructBoolResult(determineOrder(a, b) == ArgumentOrder::LhsLessThanRhs);
}

MathIntResult MathIntOps::add(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    Int128 res = addI128(extendToI128(a), extendToI128(b));
    const bool overflow = !valueInRange(resultType, res);
    return MathIntOps::constructResult(
        resultType,
        res.lo,
        overflow ? unsigned(MathIntResult::Overflow) : 0);
}

MathIntResult MathIntOps::sub(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    Int128 res = subI128(extendToI128(a), extendToI128(b));
    const bool overflow = !valueInRange(resultType, res);
    return MathIntOps::constructResult(
        resultType,
        res.lo,
        overflow ? unsigned(MathIntResult::Overflow) : 0);
}

// Return the magnitude of `v` as an unsigned 64-bit value. The magnitude of every value fits,
// including that of INT64_MIN, which is 2^63.
static uint64_t getMagnitude(const MathIntValue& v)
{
    return isNegative(v) ? negU64(v.getUnsignedValue()) : v.getUnsignedValue();
}

MathIntResult MathIntOps::mul(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    // multiply by 0?
    if ((a.getUnsignedValue() == 0) || (b.getUnsignedValue() == 0))
        return MathIntOps::constructResult(resultType, 0, 0);

    // The product overflows when its magnitude exceeds the largest magnitude that the result
    // type can hold for the product's sign. For example, an int8_t product may be at most 127
    // when positive and at most 128 when negative, and an unsigned product may not be negative.
    const uint64_t lhsMagnitude = getMagnitude(a);
    const uint64_t rhsMagnitude = getMagnitude(b);
    const bool isResultNegative = isNegative(a) != isNegative(b);
    const uint64_t maxResultMagnitude =
        isResultNegative ? negU64(static_cast<uint64_t>(getMinForType(resultType)))
                         : getMaxForType(resultType);
    const bool overflow = lhsMagnitude > maxResultMagnitude / rhsMagnitude;

    return MathIntOps::constructResult(
        resultType,
        a.getUnsignedValue() * b.getUnsignedValue(),
        overflow ? unsigned(MathIntResult::Overflow) : 0);
}

MathIntResult MathIntOps::div(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    // division by 0?
    if (b.getUnsignedValue() == 0)
    {
        return MathIntOps::constructResult(
            resultType,
            getMaxForType(resultType),
            MathIntResult::Undefined);
    }

    uint64_t res;
    bool overflow;

    if (isNegative(a) && isNegative(b))
    {
        res = negU64(a.getUnsignedValue()) / negU64(b.getUnsignedValue());
        overflow = !valueInRange(resultType, extendToI128(res));
    }
    else if (isNegative(a) && !isNegative(b))
    {
        Int128 res128 = negI128(extendToI128(negU64(a.getUnsignedValue()) / b.getUnsignedValue()));
        overflow = !valueInRange(resultType, res128);
        res = res128.lo;
    }
    else if (!isNegative(a) && isNegative(b))
    {
        // a is in range [0, UINT64_MAX] and b is in range [INT64_MIN, -1].
        //
        // If b <= -2, the uint64_t division a / -b has no overflow. The critical test is
        // UINT64_MAX / -2 == -INT64_MAX, which fits in int64_t.
        Int128 res128 = negI128(extendToI128(a.getUnsignedValue() / negU64(b.getUnsignedValue())));
        overflow = !valueInRange(resultType, res128);
        res = res128.lo;
    }
    else
    {
        res = a.getUnsignedValue() / b.getUnsignedValue();
        overflow = !valueInRange(resultType, extendToI128(res));
    }

    unsigned flags{};
    if (overflow)
    {
        flags |= MathIntResult::Overflow;
        flags |= MathIntResult::TargetDefined;
    }

    return MathIntOps::constructResult(resultType, res, flags);
}

MathIntResult MathIntOps::rem(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    if (b.getUnsignedValue() == 0)
    {
        return MathIntOps::constructResult(resultType, 0, MathIntResult::Undefined);
    }

    // we'll calculate the remainder using uint64_t and post-fix the sign
    bool signedRem;
    uint64_t dividend;
    uint64_t divisor;

    if (isNegative(a))
    {
        dividend = negU64(a.getUnsignedValue());
        signedRem = true;
    }
    else
    {
        dividend = a.getUnsignedValue();
        signedRem = false;
    }

    if (isNegative(b))
        divisor = negU64(b.getUnsignedValue());
    else
        divisor = b.getUnsignedValue();

    Int128 res128 = extendToI128(dividend % divisor);

    if (signedRem)
        res128 = negI128(res128);

    unsigned flags{};
    if (!valueInRange(resultType, res128))
        flags |= MathIntResult::Overflow;

    // Finally, set TargetDefined if division with the same arguments set
    // TargetDefined. This catches issues such as INT_MIN % -1.
    flags |= div(resultType, a, b).flags & MathIntResult::TargetDefined;

    return MathIntOps::constructResult(resultType, res128.lo, flags);
}

MathIntResult MathIntOps::neg(const MathIntType& resultType, const MathIntValue& a)
{
    Int128 res = negI128(extendToI128(a));
    const bool overflow = !valueInRange(resultType, res);
    return MathIntOps::constructResult(
        resultType,
        res.lo,
        overflow ? unsigned(MathIntResult::Overflow) : 0);
}

MathIntResult MathIntOps::bitwiseAnd(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    return MathIntOps::constructResult(
        resultType,
        (a.getUnsignedValue() & b.getUnsignedValue()),
        0);
}

MathIntResult MathIntOps::bitwiseOr(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    return MathIntOps::constructResult(resultType, a.getUnsignedValue() | b.getUnsignedValue(), 0);
}

MathIntResult MathIntOps::bitwiseXor(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    return MathIntOps::constructResult(resultType, a.getUnsignedValue() ^ b.getUnsignedValue(), 0);
}

MathIntResult MathIntOps::bitwiseNot(const MathIntType& resultType, const MathIntValue& a)
{
    return MathIntOps::constructResult(resultType, ~a.getUnsignedValue(), 0);
}

MathIntResult MathIntOps::lsh(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    const bool targetDefined{b.getUnsignedValue() >= resultType.width};
    const unsigned shiftAmount =
        resultType.width > 0U ? b.getUnsignedValue() % resultType.width : 0U;

    return MathIntOps::constructResult(
        resultType,
        a.getUnsignedValue() << shiftAmount,
        targetDefined ? unsigned(MathIntResult::TargetDefined) : 0);
}

MathIntResult MathIntOps::rsh(
    const MathIntType& resultType,
    const MathIntValue& a,
    const MathIntValue& b)
{
    const bool targetDefined{b.getUnsignedValue() >= resultType.width};
    const unsigned shiftAmount =
        resultType.width > 0U ? b.getUnsignedValue() % resultType.width : 0U;
    uint64_t value;

    if (a.getType().isSigned)
        value = static_cast<uint64_t>(a.getSignedValue() >> static_cast<int>(shiftAmount));
    else
        value = a.getUnsignedValue() >> shiftAmount;

    return MathIntOps::constructResult(
        resultType,
        value,
        targetDefined ? unsigned(MathIntResult::TargetDefined) : 0);
}

MathIntResult MathIntOps::logicalNot(const MathIntValue& a)
{
    return constructBoolResult(!a.getUnsignedValue());
}

MathIntResult MathIntOps::logicalOr(const MathIntValue& a, const MathIntValue& b)
{
    return constructBoolResult(a.getUnsignedValue() || b.getUnsignedValue());
}

MathIntResult MathIntOps::logicalAnd(const MathIntValue& a, const MathIntValue& b)
{
    return constructBoolResult(a.getUnsignedValue() && b.getUnsignedValue());
}

MathIntResult MathIntOps::ternaryConditional(
    const MathIntValue& a,
    const MathIntValue& b,
    const MathIntValue& c)
{
    MathIntResult res{};
    if (a.getUnsignedValue())
        res.value = b;
    else
        res.value = c;
    return res;
}

} // namespace Slang
