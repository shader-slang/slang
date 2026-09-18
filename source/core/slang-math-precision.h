#ifndef SLANG_CORE_MATH_PRECISION_H
#define SLANG_CORE_MATH_PRECISION_H

#include <bit>
#include <limits>
#include <type_traits>

namespace Slang
{

/// @brief Precision helper for integer to floating point conversion
///
/// @tparam fpMaxExponent    Maximum exponent for a floating point number in 0.111.. * 2^exp format
///                          (same as std::numeric_limits<FloatType>::max_exponent)
/// @tparam fpNumDigits      Number of mantissa bits (same as
///                          std::numeric_limits<FloatType>::digits)
/// @tparam IntType          Integer type
template<int fpMaxExponent, int fpNumDigits, typename IntType>
struct IntToFloatPrecisionHelper
{
    static_assert(fpMaxExponent >= 1);
    static_assert(fpNumDigits >= 1);

    /// @brief Computes a bit mask of contiguously set bits. The range is capped
    /// to the integer type
    ///
    /// @tparam lb     Lowest bit set (inclusive)
    /// @tparam hb     Highest bit set (inclusive)
    template<int lb, int hb>
    static constexpr IntType computeBitMask()
    {
        static_assert(hb >= lb);

        using UnsignedIntType = std::make_unsigned_t<IntType>;

        // bits 0 (inclusive) to hb (inclusive)
        UnsignedIntType bits0ToHB;

        if (hb >= (std::numeric_limits<UnsignedIntType>::digits - 1))
            bits0ToHB = std::numeric_limits<UnsignedIntType>::max();
        else
            bits0ToHB = (UnsignedIntType{1U} << (static_cast<unsigned>(hb) + 1U)) - 1U;

        if constexpr (lb <= 0)
        {
            return static_cast<IntType>(bits0ToHB);
        }
        else
        {
            // bits 0 (inclusive) to lb (exclusive)
            UnsignedIntType bits0ToLBExcl =
                ((UnsignedIntType{1U} << static_cast<unsigned>(lb)) - 1U);

            return static_cast<IntType>(bits0ToHB - bits0ToLBExcl);
        }
    }

    /// @brief Returns the maximum integer value that is representable by a floating-point type
    static constexpr IntType getMaximumRepresentableValue()
    {
        constexpr int intDigits{std::numeric_limits<IntType>::digits};

        // Float max > int max? (only matters for std::float16_t)
        if constexpr (fpMaxExponent <= intDigits)
        {
            // Maximum representable value is
            //
            //          bit[fpMaxExponent-1]                        bit[0]
            //          |                                           |
            //          |                                           |
            // maxInt = 1   1   1   ...     1   1   0   0   ... 0   0
            //          |                       |
            //          |                       |
            //          +----- fpNumDigits -----+
            return computeBitMask<fpMaxExponent - fpNumDigits, fpMaxExponent - 1>();
        }
        else
        {
            // Maximum representable value is
            //
            //          bit[intDigits-1]                            bit[0]
            //          |                                           |
            //          |                                           |
            // maxInt = 1   1   1   ...     1   1   0   0   ... 0   0
            //          |                       |
            //          |                       |
            //          +----- fpNumDigits -----+
            return computeBitMask<intDigits - fpNumDigits, intDigits - 1>();
        }
    }

    // Returns the minimum integer value that is representable by a floating-point type
    static constexpr IntType getMinimumRepresentableValue()
    {
        if constexpr (std::is_unsigned_v<IntType>)
        {
            // 0 is always representable
            return 0U;
        }
        else
        {
            constexpr int intDigits{std::numeric_limits<IntType>::digits};

            if (fpMaxExponent > intDigits)
            {
                // Float minimum is less than or equal to int minimum, so we'll
                // return the int minimum. Note that the int minimum is a power of
                // two, so it's always representable by a float in this case.
                return std::numeric_limits<IntType>::min();
            }
            else
            {
                // Int minimum is less than float minimum, so we'll return the
                // negated float maximum.
                return -getMaximumRepresentableValue();
            }
        }
    }

    // Tests whether an integer value is precisely representable by a floating point
    // type.
    //
    // Given that the integer value is within the range of the floating point type,
    // the test is based on checking whether the span of set bits of the integer
    // value fit in the floating-point mantissa.
    //
    // Example:
    //
    //  267366384 = 0b0000'1111'1110'1111'1010'1111'1111'0000
    //                     |                           |
    //                     +--------- 24 bits ---------+
    //
    // This will fit in a float, since float has 24 bits of mantissa.
    //
    // For negative integer values, the absolute value is tested. Floating point
    // types represent the same range of positive and negative numbers.
    //
    static constexpr bool isPreciselyRepresentable(IntType v)
    {
        // Upper bounds check
        if (v > getMaximumRepresentableValue())
            return false;

        // Lower bound check is needed only for signed types. Skip it for unsigned
        // types to avoid potential complaints about always-false comparisons
        if constexpr (std::is_signed_v<IntType>)
        {
            if (v < getMinimumRepresentableValue())
                return false;
        }

        // normalize 'v' for the digit span test by taking the absolute value
        if constexpr (std::is_signed_v<IntType>)
        {
            // Handle the special case to make negation always well-defined. Signed
            // int minimum is a power-of-two, so it'll always be representable by a
            // float after passing the bounds test
            if (v == std::numeric_limits<IntType>::min())
                return true;

            // make 'v' non-negative
            if (v < 0)
                v = -v;
        }

        // do the digit span test
        using UnsignedIntType = std::make_unsigned_t<IntType>;
        const UnsignedIntType u{static_cast<UnsignedIntType>(v)};

        // handle 0 - always representable by a float
        if (u == 0U)
            return true;

        // highest and lowest set bit indexes
        const int hb{std::bit_width(u) - 1};
        const int lb{std::countr_zero(u)};

        // check if the span of set bits fits in the float mantissa
        const int spanBits{hb - lb + 1};
        return spanBits <= fpNumDigits;
    }
};

template<typename FloatType, typename IntType>
constexpr bool isPreciselyRepresentableByFloatingPointType(IntType v)
{
    return IntToFloatPrecisionHelper<
        std::numeric_limits<FloatType>::max_exponent,
        std::numeric_limits<FloatType>::digits,
        IntType>::isPreciselyRepresentable(v);
}

} // namespace Slang

#endif
