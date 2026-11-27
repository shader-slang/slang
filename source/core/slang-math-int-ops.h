#ifndef SLANG_CORE_MATH_INT_OPS_H
#define SLANG_CORE_MATH_INT_OPS_H

// Integer operations for compile-time evaluation

#include "slang-string.h"

#include <cstdint>
#include <limits>
#include <type_traits>

namespace Slang
{

struct MathIntType
{
    // Bit width of the integer type
    uint8_t width{};

    // Whether the integer type is signed
    bool isSigned{};

    // width: 0, unsigned -- null/invalid type
    static const MathIntType NullType;

    // width: 1, unsigned
    static const MathIntType BoolType;

    // width: 8, signed
    static const MathIntType Int8Type;

    // width: 16, signed
    static const MathIntType Int16Type;

    // width: 32, signed
    static const MathIntType Int32Type;

    // width: 64, signed
    static const MathIntType Int64Type;

    // width: 8, unsigned
    static const MathIntType UInt8Type;

    // width: 16, unsigned
    static const MathIntType UInt16Type;

    // width: 32, unsigned
    static const MathIntType UInt32Type;

    // width: 64, unsigned
    static const MathIntType UInt64Type;
};

/// @brief Typed integer value using two's complement representations
class MathIntValue
{
private:
    // The raw bit pattern
    //
    // The raw value is always zero/sign-extended, such that it fits in the
    // integer type as depicted by `type`.
    uint64_t rawValue{};

    // The integer type parameters
    MathIntType type{};

    // Normalize value by discarding all extra high bits and extending the sign
    // for signed types.
    void normalize();

public:
    /// @brief Sets the type and the value. If _value does not fit in _type, it
    /// is truncated.
    ///
    /// If T is a signed type, _value is sign-extended to 64-bits before
    /// truncation. If T is an unsigned type, it is zero-extended.
    ///
    /// Examples:
    ///
    /// - setValue(MathIntType::Int16Type, int8_t{-123}) -- sets:
    ///     type: 16-bit int, value -123 == 0xFFFFFFFFFFFFFF85
    /// - setValue(MathIntType::Int8Type, uint8_t{0xFE}) -- sets:
    ///     type: 8-bit int, value -2 = 0xFFFFFFFFFFFFFFFE
    /// - setValue(MathIntType::UInt8Type, int16_t{0x1234}) -- sets:
    ///     type: 8-bit unsigned int, value 0x34 = 0x0000000000000034
    template<typename T>
    void setValue(const MathIntType& _type, T _value)
    {
        static_assert(std::is_integral_v<T>);

        if constexpr (std::is_unsigned_v<T>)
            rawValue = _value;
        else
            rawValue = static_cast<uint64_t>(int64_t{_value});

        type = _type;

        normalize();
    }

    /// @brief Constructs a MathIntValue object
    template<typename T>
    static MathIntValue constructFromValue(const MathIntType& _type, T _value)
    {
        MathIntValue ret{};
        ret.setValue(_type, _value);
        return ret;
    }

    /// @brief Returns the stored value as an unsigned 64-bit integer.
    uint64_t getUnsignedValue() const { return rawValue; }

    /// @brief Returns the stored value as a signed 64-bit integer.
    int64_t getSignedValue() const { return static_cast<int64_t>(rawValue); }

    const MathIntType& getType() const { return type; }

    bool hasValue() const { return type.width > 0U; }

    bool isZero() const { return getUnsignedValue() == 0U; }
    bool isNegative() const { return type.isSigned && (getSignedValue() < 0); }
    bool isPositive() const { return !(isZero() || isNegative()); }

    /// @brief Produces a typed value
    ///
    /// The format is as follows:
    /// - int32_t         Plain number
    /// - uint32_t        'U' suffix
    /// - int64_t         'LL'  suffix
    /// - uint64_t        'ULL' suffix
    /// - boolean         true/false
    /// - anything else:  [u]intNN_t(number), e.g., int16_t(123) or uint8_t(255)
    String toString() const;
};

struct MathIntResult
{
    MathIntValue value{};

    enum ResultFlag : unsigned
    {
        // Result is undefined and it should not be used
        //
        // Example: 1/0  (division by zero)
        Undefined = 1U << 0,

        // Result has an overflow, but all targets are expected to compute it similarly.
        //
        // Note that shifts do not trigger an overflow when shifted bits get out
        // of range.
        //
        // Examples:
        // - uint(65536) * uint(65536) (integer overflow)
        Overflow = 1U << 1,

        // Set if the value is target-defined
        //
        // Examples:
        // - uint32_t(1) << 32   (out-of-range shift)
        // - (-2147483648) / -1
        TargetDefined = 1U << 2,
    };

    unsigned flags{};
};

/// @brief Integer math ops
///
/// The integer math ops are checked for undefined behavior, target-defined
/// behavior, and overflows. The intent is that anything that might result in
/// differences between compile-time and runtime behavior gets flagged, allowing
/// the compiler to diagnose things accordingly.
///
/// The operands may have types that differ from each other and from the result
/// type. Similarly to `__builtin_add_overflow()`, each operation computes its
/// result as if with infinite precision and then assigns it to the result
/// type. If the value does not fit, the stored value is truncated and
/// MathIntResult::Overflow is set. Slang AST/IR folding converts the operands
/// to the result type before invoking an operation, so the mixed-type
/// generality is not relied on there.
///
/// When the operand types match the result type, the results and the
/// Undefined/TargetDefined flags agree with the usual C/C++ and SPIR-V rules:
///
/// - Division or remainder by zero sets MathIntResult::Undefined.
/// - Signed `MIN / -1` and `MIN % -1` set MathIntResult::TargetDefined.
/// - A shift amount that is negative or not less than the bit width sets
///   MathIntResult::TargetDefined.
///
/// Signed overflow in add, sub, mul, and neg follows SPIR-V rather than
/// C/C++: the result wraps around and only MathIntResult::Overflow is set, so
/// the result remains foldable.
class MathIntOps
{
public:
    template<typename T>
    static MathIntResult constructResult(MathIntType type, T rawValue, unsigned flags)
    {
        MathIntResult res{};
        res.value.setValue(type, rawValue);
        res.flags = flags;

        return res;
    }

    static MathIntResult constructBoolResult(bool value)
    {
        return constructResult(MathIntType::BoolType, value ? 1U : 0U, 0U);
    }

    /// @brief Converts the floating-point value `value` to the integer type `resultType`,
    /// truncating toward zero.
    ///
    /// If the truncated value does not fit in `resultType`, or `value` is NaN or infinite, the
    /// result is target-defined.
    ///
    /// For MathIntType::BoolType, the result is 1 if `value` is nonzero and 0 otherwise, as in
    /// C. A NaN is target-defined for BoolType too, because targets disagree on whether NaN
    /// compares unequal to zero.
    ///
    /// Examples:
    /// - convertFloatValue(Int8Type, -128.9) == -128
    /// - convertFloatValue(Int8Type, 128.0) sets TargetDefined
    /// - convertFloatValue(UInt8Type, -0.5) == 0 (truncates to zero, which is in range)
    /// - convertFloatValue(UInt64Type, 9223372036854775808.0) == 0x8000000000000000
    /// - convertFloatValue(UInt64Type, 18446744073709551616.0) sets TargetDefined
    static MathIntResult convertFloatValue(const MathIntType& resultType, double value);

    MathIntOps() = delete;
    ~MathIntOps() = delete;

    /// @brief Converts the value to the specified type.
    ///
    /// If the value changes in conversion, MathIntResult::Overflow is set.
    static MathIntResult cast(const MathIntType& resultType, const MathIntValue& a);

    /// @brief Converts the value to a Boolean type.
    ///
    /// If the integer value is non-zero, the result is 1. Otherwise, the result
    /// is 0. No flags are set.
    static MathIntResult castToBool(const MathIntValue& a);

    static MathIntResult compEql(const MathIntValue& a, const MathIntValue& b);
    static MathIntResult compNeq(const MathIntValue& a, const MathIntValue& b);
    static MathIntResult compGeq(const MathIntValue& a, const MathIntValue& b);
    static MathIntResult compLeq(const MathIntValue& a, const MathIntValue& b);
    static MathIntResult compGreater(const MathIntValue& a, const MathIntValue& b);
    static MathIntResult compLess(const MathIntValue& a, const MathIntValue& b);

    /// @brief Adds two values together. If the result does not fit in the
    /// result type, the value is truncated and MathIntResult::Overflow is set.
    static MathIntResult add(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Subtracts `b` from `a`. If the result does not fit in the result
    /// type, the value is truncated and MathIntResult::Overflow is set.
    static MathIntResult sub(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Multiplies two values. If the result does not fit in the result
    /// type, the value is truncated and MathIntResult::Overflow is set.
    static MathIntResult mul(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Computes integer division a / b. If the result does not fit in
    /// the result type, the value is truncated and both MathIntResult::Overflow
    /// and MathIntResult::TargetDefined are set. Division by 0 returns the
    /// maximum value that fits in resultType and sets MathIntResult::Undefined.
    static MathIntResult div(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Computes remainder of a / b. If the result does not fit in the
    /// result type, the value is truncated and MathIntResult::Overflow is
    /// set. Division by 0 returns 0 and sets MathIntResult::Undefined.
    ///
    /// We treat the remainder as a byproduct of the division, so
    /// MathIntResult::TargetDefined is set whenever div() with the same
    /// arguments sets it. For example, `INT32_MIN % -1` is target-defined even
    /// though the remainder 0 fits in the result type.
    ///
    /// Matching the Slang specification, the remainder operator returns the
    /// remainder of `a` by `b` division such that `rem = a - n * b` where `n`
    /// is an integer and `abs(rem) < abs(b)`. The sign of the remainder matches
    /// the sign of `a`. That is,
    ///
    ///     `a % b == a - trunc(a / b) * b`
    static MathIntResult rem(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Negates a value. If the result does not fit in the result type,
    /// the value is truncated and MathIntResult::Overflow is set.
    static MathIntResult neg(const MathIntType& resultType, const MathIntValue& a);

    /// @brief Computes `a` bitwise-and `b`. No flags are set.
    static MathIntResult bitwiseAnd(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Computes `a` bitwise-or `b`. No flags are set.
    static MathIntResult bitwiseOr(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Computes `a` bitwise-xor `b`. No flags are set.
    static MathIntResult bitwiseXor(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Computes bitwise-not `a`. No flags are set.
    static MathIntResult bitwiseNot(const MathIntType& resultType, const MathIntValue& a);

    /// @brief Shifts bit pattern of `a` left by `b` bit positions. If `b` is
    /// negative or larger than the width of the result type, the shift amount
    /// is `unsigned(b) % resultType.width`.
    static MathIntResult lsh(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Shifts bit pattern of `a` right by `b` bit positions. If `b` is
    /// negative or larger than the width of the result type, the shift amount
    /// is `unsigned(b) % resultType.width`.
    ///
    /// When shifting right, sign-extension is used if `a` has a signed
    /// type. Otherwise, zero-extension is used.
    static MathIntResult rsh(
        const MathIntType& resultType,
        const MathIntValue& a,
        const MathIntValue& b);

    /// @brief Returns MathIntTypes::BoolType(1) if a == 0
    static MathIntResult logicalNot(const MathIntValue& a);

    /// @brief Returns MathIntTypes::BoolType(1) if a != 0 or b != 0
    static MathIntResult logicalOr(const MathIntValue& a, const MathIntValue& b);

    /// @brief Returns MathIntTypes::BoolType(1) if a != 0 and b != 0
    static MathIntResult logicalAnd(const MathIntValue& a, const MathIntValue& b);

    /// @brief Returns a != 0 ? b : c
    static MathIntResult ternaryConditional(
        const MathIntValue& a,
        const MathIntValue& b,
        const MathIntValue& c);
};

} // namespace Slang

#endif
