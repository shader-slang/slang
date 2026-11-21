// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "core/slang-math.h"
#include "unit-test/slang-unit-test.h"

#include <math.h>

using namespace Slang;

// Computes exact values on the positive Half rounding grid, independently of FloatToHalf.
// Code 0x7c00 denotes the hypothetical next finite value, 65536, so the last midpoint
// (65520) also tests the transition from the maximum finite Half to infinity.
static float getHalfRoundingGridValue(unsigned int code)
{
    unsigned int exponent = code / 1024;
    unsigned int fraction = code % 1024;
    return exponent == 0 ? ldexpf(float(fraction), -24)
                         : ldexpf(float(1024 + fraction), int(exponent) - 25);
}

SLANG_UNIT_TEST(mathHalfRoundsEveryFiniteBoundaryToNearestEven, NoRenderApi)
{
    // Test all adjacent finite Half pairs, plus the overflow endpoint at 65536. Their
    // midpoint and its Float32 neighbors are exact dyadic values, independent of the
    // narrowing implementation. Negative values have the same magnitude rounding rule.
    for (unsigned int code = 0; code < 0x7c00; ++code)
    {
        const float lower = getHalfRoundingGridValue(code);
        const float upper = getHalfRoundingGridValue(code + 1);
        const float midpoint = (lower + upper) * 0.5f;
        const unsigned int midpointBits = unsigned(FloatAsInt(midpoint));
        for (unsigned int sign : {0u, 0x80000000u})
        {
            const unsigned int halfSign = sign >> 16;
            SLANG_CHECK(
                FloatToHalf(IntAsFloat(unsigned(FloatAsInt(lower)) | sign)) == (code | halfSign));
            SLANG_CHECK(FloatToHalf(IntAsFloat((midpointBits - 1) | sign)) == (code | halfSign));
            SLANG_CHECK(
                FloatToHalf(IntAsFloat(midpointBits | sign)) == ((code + (code & 1)) | halfSign));
            SLANG_CHECK(
                FloatToHalf(IntAsFloat((midpointBits + 1) | sign)) == ((code + 1) | halfSign));
        }
    }
}

SLANG_UNIT_TEST(mathHalfPreservesSpecialValueClassification, NoRenderApi)
{
    for (unsigned int sign : {0u, 0x80000000u})
    {
        const unsigned int halfSign = sign >> 16;
        for (unsigned int bits : {0u, 1u, 0x007fffffu, 0x00800000u})
            SLANG_CHECK(FloatToHalf(IntAsFloat(bits | sign)) == halfSign);
        for (unsigned int bits : {0x47800000u, 0x7f7fffffu, 0x7f800000u})
            SLANG_CHECK(FloatToHalf(IntAsFloat(bits | sign)) == (halfSign | 0x7c00u));
        // The narrowed encoding is exactly sign | 0x7c00 | 0x0200 | (fraction >> 13): the
        // half quiet bit (0x0200) is forced on, the top payload bits carry over, and the low
        // 13 fraction bits are discarded. Pin the exact encodings — including the carried
        // payload bits — so a wrong shift or a dropped payload cannot pass silently.
        struct NarrowedNaNCase
        {
            unsigned int bits;
            unsigned short expected;
        };
        const NarrowedNaNCase cases[] = {
            {0x7f800001u, 0x7e00u},
            {0x7fa12345u, 0x7f09u},
            {0x7fc12345u, 0x7e09u},
            {0x7fc00000u, 0x7e00u},
            {0x7fffffffu, 0x7fffu},
        };
        for (const auto& nanCase : cases)
        {
            SLANG_CHECK(
                FloatToHalf(IntAsFloat(nanCase.bits | sign)) == (halfSign | nanCase.expected));
        }
    }
}
