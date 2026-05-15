//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry main -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_WARNING_DISABLED): -stage compute -entry main -target spirv -warnings-disable 32006

enum TestEnumBool : bool
{
    Value1 = true,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'bool' and wraps around
*/
    Value3,
}

enum TestEnumS8 : int8_t
{
    Value1 = 127,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'int8_t' and wraps around
*/
    Value3,
}

enum TestEnumS8_NoWrapAround : int8_t
{
    Value1 = 127,
    Value2 = -128, // no wraparound diagnostics here
    Value3,
}

enum TestEnumU8 : uint8_t
{
    Value1 = 255U,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'uint8_t' and wraps around
*/
    Value3,
}

enum TestEnumS16 : int16_t
{
    Value1 = 32767,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'int16_t' and wraps around
*/
    Value3,
}

enum TestEnumU16 : uint16_t
{
    Value1 = 65535U,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'uint16_t' and wraps around
*/
    Value3,
}

enum TestEnumDefault
{
    Value1 = 2147483647,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'int' and wraps around
*/
    Value3,
}

enum TestEnumS32 : int32_t
{
    Value1 = 2147483647,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'int' and wraps around
*/
    Value3,
}

enum TestEnumU32 : uint32_t
{
    Value1 = 4294967295U,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'uint' and wraps around
*/
    Value3,
}

enum TestEnumS64 : int64_t
{
    Value1 = 9223372036854775807,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'int64_t' and wraps around
*/
    Value3,
}

enum TestEnumU64 : uint64_t
{
    Value1 = 18446744073709551615ULL,
    Value2,
/*CHECK:
    ^^^^^^ warning E32006
    ^^^^^^ overflows tag type 'uint64_t' and wraps around
*/
    Value3,
}

[numthreads(1,1,1)]
void main()
{
    // TestEnumBool wraparound verification
    static_assert(TestEnumBool.Value1, "Value before wraparound");
    static_assert(!TestEnumBool.Value2, "Value after wraparound");
    static_assert(TestEnumBool.Value3, "Value after wraparound + 1");

    // TestEnumS8 wraparound verification
    static_assert(TestEnumS8.Value1 == 127, "Value before wraparound");
    static_assert(TestEnumS8.Value2 == -128, "Value after wraparound");
    static_assert(TestEnumS8.Value3 == -127, "Value after wraparound + 1");

    // TestEnumS16 wraparound verification
    static_assert(TestEnumS16.Value1 == 32767, "Value before wraparound");
    static_assert(TestEnumS16.Value2 == -32768, "Value after wraparound");
    static_assert(TestEnumS16.Value3 == -32767, "Value after wraparound + 1");

    // TestEnumS32 wraparound verification
    static_assert(TestEnumS32.Value1 == 2147483647, "Value before wraparound");
    static_assert(TestEnumS32.Value2 == -2147483648, "Value after wraparound");
    static_assert(TestEnumS32.Value3 == -2147483647, "Value after wraparound + 1");

    // TestEnumS64 wraparound verification
    static_assert(TestEnumS64.Value1 == 9223372036854775807LL, "Value before wraparound");
    static_assert(TestEnumS64.Value2 == -9223372036854775808LL, "Value after wraparound");
    static_assert(TestEnumS64.Value3 == -9223372036854775807LL, "Value after wraparound + 1");

    // TestEnumU8 wraparound verification
    static_assert(TestEnumU8.Value1 == 255, "Value before wraparound");
    static_assert(TestEnumU8.Value2 == 0, "Value after wraparound");
    static_assert(TestEnumU8.Value3 == 1, "Value after wraparound + 1");

    // TestEnumU16 wraparound verification
    static_assert(TestEnumU16.Value1 == 65535, "Value before wraparound");
    static_assert(TestEnumU16.Value2 == 0, "Value after wraparound");
    static_assert(TestEnumU16.Value3 == 1, "Value after wraparound + 1");

    // TestEnumU32 wraparound verification
    static_assert(TestEnumU32.Value1 == 4294967295, "Value before wraparound");
    static_assert(TestEnumU32.Value2 == 0, "Value after wraparound");
    static_assert(TestEnumU32.Value3 == 1, "Value after wraparound + 1");

    // TestEnumU64 wraparound verification
    static_assert(TestEnumU64.Value1 == 18446744073709551615ULL, "Value before wraparound");
    static_assert(TestEnumU64.Value2 == 0ULL, "Value after wraparound");
    static_assert(TestEnumU64.Value3 == 1ULL, "Value after wraparound + 1");
}
