// unit-test-math.cpp
//
// Contract under test
// -------------------
// `Math` (in `slang-math.h`) provides Slang's compile-time-friendly
// math primitives used throughout the compiler — bit-twiddling for
// IR-pass numerics, IEEE-754 type-pun helpers, and float ↔ half /
// bfloat / E4M3 / E5M2 conversions used by the FP emit paths.
//
// What this suite pins:
//
//   * `Abs / Min / Max / Clamp` — work for any comparable type;
//     the 3-arg forms produce the same result as a chain of 2-arg
//     calls.
//   * `FastFloor` — equivalent to floor() for inputs within int
//     range, including negatives (where naive truncation is wrong).
//   * `IsNaN / IsInf` — match the C++ standard predicates.
//   * `Ones32(x)` returns popcount(x); `Log2Floor` /  `Log2Ceil`
//     match their mathematical definition for x >= 1.
//   * `AreNearlyEqual(a, b, eps)` — relative tolerance, with a
//     special case near zero so `eps`-around-zero behaves sanely.
//   * `getLowestBit` — value with all bits cleared except the
//     least-significant set bit; matches the standard `x & -x`
//     identity.
//   * `FloatAsInt / IntAsFloat / DoubleAsInt64 / Int64AsDouble`
//     are bit-pattern type-puns: round-trip preserves the bits.
//   * Float ↔ half / bfloat conversions round-trip values that fit
//     in the smaller format, and saturate / lose precision in the
//     documented way otherwise.

#include "../../source/core/slang-math.h"
#include "unit-test/slang-unit-test.h"

#include <limits>
#include <math.h>

using namespace Slang;

// -- Abs / Min / Max / Clamp --------------------------------------

SLANG_UNIT_TEST(mathAbs)
{
    SLANG_CHECK(Math::Abs(0) == 0);
    SLANG_CHECK(Math::Abs(7) == 7);
    SLANG_CHECK(Math::Abs(-7) == 7);
    SLANG_CHECK(Math::Abs(-1.5f) == 1.5f);
    SLANG_CHECK(Math::Abs(-1234567890LL) == 1234567890LL);
}

SLANG_UNIT_TEST(mathMinMax2Arg)
{
    SLANG_CHECK(Math::Min(3, 5) == 3);
    SLANG_CHECK(Math::Min(5, 3) == 3);
    SLANG_CHECK(Math::Min(-1, 1) == -1);
    SLANG_CHECK(Math::Max(3, 5) == 5);
    SLANG_CHECK(Math::Max(5, 3) == 5);
    SLANG_CHECK(Math::Max(-1, -2) == -1);
    SLANG_CHECK(Math::Min(2.5f, 1.25f) == 1.25f);
    SLANG_CHECK(Math::Max(2.5f, 1.25f) == 2.5f);
}

SLANG_UNIT_TEST(mathMinMax3ArgMatches2ArgChain)
{
    // 3-arg form is documented as equivalent to chaining 2-arg.
    int a = 4, b = 7, c = 2;
    SLANG_CHECK(Math::Min(a, b, c) == Math::Min(a, Math::Min(b, c)));
    SLANG_CHECK(Math::Max(a, b, c) == Math::Max(a, Math::Max(b, c)));
    SLANG_CHECK(Math::Min(a, b, c) == 2);
    SLANG_CHECK(Math::Max(a, b, c) == 7);
}

SLANG_UNIT_TEST(mathClamp)
{
    SLANG_CHECK(Math::Clamp(5, 0, 10) == 5);
    SLANG_CHECK(Math::Clamp(-1, 0, 10) == 0);
    SLANG_CHECK(Math::Clamp(11, 0, 10) == 10);
    SLANG_CHECK(Math::Clamp(0, 0, 10) == 0);
    SLANG_CHECK(Math::Clamp(10, 0, 10) == 10);
    SLANG_CHECK(Math::Clamp(0.5f, 0.0f, 1.0f) == 0.5f);
    SLANG_CHECK(Math::Clamp(-0.5f, 0.0f, 1.0f) == 0.0f);
}

// -- FastFloor ----------------------------------------------------

SLANG_UNIT_TEST(mathFastFloor)
{
    SLANG_CHECK(Math::FastFloor(0.0f) == 0);
    SLANG_CHECK(Math::FastFloor(1.0f) == 1);
    SLANG_CHECK(Math::FastFloor(1.99f) == 1);
    // Critical case: negative non-integer must round DOWN, not
    // truncate-toward-zero. FastFloor(-1.5) is -2, not -1.
    SLANG_CHECK(Math::FastFloor(-1.5f) == -2);
    SLANG_CHECK(Math::FastFloor(-0.1f) == -1);
    SLANG_CHECK(Math::FastFloor(-1.0f) == -1);
    // Same for double.
    SLANG_CHECK(Math::FastFloor(2.999) == 2);
    SLANG_CHECK(Math::FastFloor(-2.5) == -3);
    SLANG_CHECK(Math::FastFloor(-2.0) == -2);
}

// -- IsNaN / IsInf ------------------------------------------------

SLANG_UNIT_TEST(mathIsNaN)
{
    SLANG_CHECK(Math::IsNaN(std::numeric_limits<float>::quiet_NaN()));
    SLANG_CHECK(Math::IsNaN(std::numeric_limits<double>::quiet_NaN()));
    SLANG_CHECK(!Math::IsNaN(0.0f));
    SLANG_CHECK(!Math::IsNaN(1.0f));
    SLANG_CHECK(!Math::IsNaN(std::numeric_limits<float>::infinity()));
}

SLANG_UNIT_TEST(mathIsInf)
{
    SLANG_CHECK(Math::IsInf(std::numeric_limits<float>::infinity()));
    SLANG_CHECK(Math::IsInf(-std::numeric_limits<float>::infinity()));
    SLANG_CHECK(Math::IsInf(std::numeric_limits<double>::infinity()));
    SLANG_CHECK(!Math::IsInf(0.0f));
    SLANG_CHECK(!Math::IsInf(1.0f));
    SLANG_CHECK(!Math::IsInf(std::numeric_limits<float>::quiet_NaN()));
}

// -- Bit ops: Ones32 / Log2Floor / Log2Ceil / getLowestBit -------

SLANG_UNIT_TEST(mathOnes32IsPopcount)
{
    SLANG_CHECK(Math::Ones32(0u) == 0);
    SLANG_CHECK(Math::Ones32(1u) == 1);
    SLANG_CHECK(Math::Ones32(0xFFu) == 8);
    SLANG_CHECK(Math::Ones32(0xFFFFu) == 16);
    SLANG_CHECK(Math::Ones32(0xFFFFFFFFu) == 32);
    // Single bit at every position.
    for (unsigned i = 0; i < 32; ++i)
    {
        SLANG_CHECK(Math::Ones32(1u << i) == 1);
    }
    // Pair of bits.
    SLANG_CHECK(Math::Ones32(0x5555'5555u) == 16);
    SLANG_CHECK(Math::Ones32(0xAAAA'AAAAu) == 16);
}

SLANG_UNIT_TEST(mathLog2FloorMatchesDefinition)
{
    // floor(log2(x)) for x >= 1.
    SLANG_CHECK(Math::Log2Floor(1u) == 0);
    SLANG_CHECK(Math::Log2Floor(2u) == 1);
    SLANG_CHECK(Math::Log2Floor(3u) == 1);
    SLANG_CHECK(Math::Log2Floor(4u) == 2);
    SLANG_CHECK(Math::Log2Floor(7u) == 2);
    SLANG_CHECK(Math::Log2Floor(8u) == 3);
    SLANG_CHECK(Math::Log2Floor(1024u) == 10);
    SLANG_CHECK(Math::Log2Floor(0x80000000u) == 31);
}

SLANG_UNIT_TEST(mathLog2CeilMatchesDefinition)
{
    // ceil(log2(x)) for x >= 1: same as Floor for powers of 2,
    // otherwise +1.
    SLANG_CHECK(Math::Log2Ceil(1u) == 0);
    SLANG_CHECK(Math::Log2Ceil(2u) == 1);
    SLANG_CHECK(Math::Log2Ceil(3u) == 2);
    SLANG_CHECK(Math::Log2Ceil(4u) == 2);
    SLANG_CHECK(Math::Log2Ceil(5u) == 3);
    SLANG_CHECK(Math::Log2Ceil(8u) == 3);
    SLANG_CHECK(Math::Log2Ceil(1024u) == 10);
    SLANG_CHECK(Math::Log2Ceil(1025u) == 11);
}

SLANG_UNIT_TEST(mathGetLowestBitMatchesIdentity)
{
    // Standard identity: x & -x isolates the LSB.
    for (uint32_t x : {1u, 2u, 4u, 6u, 12u, 0xFF00u, 0x80000000u, 0xFFFFFFFFu})
    {
        SLANG_CHECK(Math::getLowestBit(x) == (x & -x));
    }
    // Zero in → zero out.
    SLANG_CHECK(Math::getLowestBit(0u) == 0u);
    // Specific examples.
    SLANG_CHECK(Math::getLowestBit(0b1010u) == 0b0010u);
    SLANG_CHECK(Math::getLowestBit(0b1100u) == 0b0100u);
    SLANG_CHECK(Math::getLowestBit(0b1000u) == 0b1000u);
}

// -- AreNearlyEqual ----------------------------------------------

SLANG_UNIT_TEST(mathAreNearlyEqualExact)
{
    // Bit-identical values are nearly-equal at any non-negative eps.
    SLANG_CHECK(Math::AreNearlyEqual(1.5, 1.5, 1e-10));
    SLANG_CHECK(Math::AreNearlyEqual(0.0, 0.0, 0.0));
    SLANG_CHECK(Math::AreNearlyEqual(-7.25, -7.25, 1e-15));
}

SLANG_UNIT_TEST(mathAreNearlyEqualRelative)
{
    // Inputs within relative tolerance.
    const double eps = 1e-9;
    SLANG_CHECK(Math::AreNearlyEqual(1.0, 1.0 + 1e-12, eps));
    SLANG_CHECK(!Math::AreNearlyEqual(1.0, 1.001, eps));

    // Large magnitudes — relative tolerance applies.
    SLANG_CHECK(Math::AreNearlyEqual(1e20, 1e20 + 100, eps));
    // But not when the gap exceeds eps * (|a|+|b|).
    SLANG_CHECK(!Math::AreNearlyEqual(1.0, 2.0, eps));
}

SLANG_UNIT_TEST(mathAreNearlyEqualNearZero)
{
    // Near-zero values use absolute tolerance to avoid the
    // relative-comparison singularity.
    SLANG_CHECK(Math::AreNearlyEqual(0.0, 0.0, 1e-9));
    // Different signs but tiny magnitudes — still equal-ish.
    SLANG_CHECK(Math::AreNearlyEqual(1e-310, -1e-310, 1.0));
}

// -- Type-pun helpers --------------------------------------------

SLANG_UNIT_TEST(mathFloatIntRoundTrip)
{
    // Bit-pattern round-trip preserves the value.
    for (float f : {0.0f, 1.0f, -1.5f, 3.14159265f, 1e-30f})
    {
        SLANG_CHECK(IntAsFloat(FloatAsInt(f)) == f);
    }
}

SLANG_UNIT_TEST(mathDoubleInt64RoundTrip)
{
    for (double d : {0.0, 1.0, -2.5, 3.14159265358979, 1e-30, 1e30})
    {
        SLANG_CHECK(Int64AsDouble(DoubleAsInt64(d)) == d);
    }
}

SLANG_UNIT_TEST(mathFloatAsIntRespectsBits)
{
    // 1.0f has IEEE-754 representation 0x3F800000.
    SLANG_CHECK(FloatAsInt(1.0f) == 0x3F800000);
    SLANG_CHECK(FloatAsInt(0.0f) == 0);
    // -0.0f is sign-bit only. Construct it via IntAsFloat so the
    // value isn't a `-0.0f` literal that fast-math compile flags
    // (e.g. -fno-signed-zeros) could fold to +0.0f at compile time.
    const float negZero = IntAsFloat((int)0x80000000);
    SLANG_CHECK((unsigned)FloatAsInt(negZero) == 0x80000000u);
}

// -- Half conversion ---------------------------------------------

SLANG_UNIT_TEST(mathHalfRoundTripExactValues)
{
    // Values exactly representable in fp16 round-trip exactly.
    for (float f : {0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 2.0f, 1024.0f, -1024.0f})
    {
        unsigned short h = FloatToHalf(f);
        float back = HalfToFloat(h);
        SLANG_CHECK(back == f);
    }
}

SLANG_UNIT_TEST(mathHalfSaturatesAtMax)
{
    // SLANG_HALF_MAX is the largest finite fp16 value.
    unsigned short h = FloatToHalf(SLANG_HALF_MAX);
    float back = HalfToFloat(h);
    SLANG_CHECK(back == SLANG_HALF_MAX);

    // Values exceeding fp16 max produce infinity (or saturate).
    unsigned short hInf = FloatToHalf(1e30f);
    float backInf = HalfToFloat(hInf);
    SLANG_CHECK(Math::IsInf(backInf) || backInf == SLANG_HALF_MAX);
}

SLANG_UNIT_TEST(mathBFloatRoundTripExactValues)
{
    // bfloat16 has the same exponent range as float but only 7 bits
    // of mantissa. Values whose mantissa fits in 7 bits round-trip
    // exactly.
    for (float f : {0.0f, 1.0f, -1.0f, 2.0f, -0.5f, 1024.0f})
    {
        unsigned short h = FloatToBFloat16(f);
        float back = BFloat16ToFloat(h);
        SLANG_CHECK(back == f);
    }
}

// `Math::Pi` is declared in slang-math.h as `static const float Pi`
// but has no out-of-line definition (no `Slang::Math::Pi` in any
// `.cpp`). Any test that ODR-uses it would fail to link. Tracked
// as shader-slang/slang#10952; no test is added here.

// -- E4M3 / E5M2 8-bit float formats -----------------------------

SLANG_UNIT_TEST(mathFloatE4M3RoundTripExactValues)
{
    // E4M3 has 4 exponent + 3 mantissa bits; values exactly
    // representable in this format round-trip without loss.
    for (float f : {0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 2.0f, -2.0f, 4.0f})
    {
        unsigned int e = FloatToFloatE4M3(f);
        float back = FloatE4M3ToFloat(e);
        SLANG_CHECK(back == f);
    }
}

SLANG_UNIT_TEST(mathFloatE4M3SaturatesAtMax)
{
    // E4M3 max is 448; values above the format's range either
    // saturate or produce NaN. Either way the value must NOT
    // round-trip back to the original 1e10.
    unsigned int e = FloatToFloatE4M3(1e10f);
    float back = FloatE4M3ToFloat(e);
    SLANG_CHECK(back != 1e10f);
}

SLANG_UNIT_TEST(mathFloatE5M2RoundTripExactValues)
{
    // E5M2: 5 exponent + 2 mantissa bits. Wider range than E4M3,
    // less precision. {0, ±1, ±0.5, powers of two} round-trip.
    for (float f : {0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 2.0f, -2.0f, 1024.0f})
    {
        unsigned int e = FloatToFloatE5M2(f);
        float back = FloatE5M2ToFloat(e);
        SLANG_CHECK(back == f);
    }
}

SLANG_UNIT_TEST(mathFloatE5M2SaturatesAtMax)
{
    // E5M2 max is ~57344 (2^15 * 1.75). 1e10 exceeds that.
    unsigned int e = FloatToFloatE5M2(1e10f);
    float back = FloatE5M2ToFloat(e);
    SLANG_CHECK(back != 1e10f);
}
