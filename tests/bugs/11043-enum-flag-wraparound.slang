//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry main -target spirv

[Flags]
enum TestEnumS8 : int8_t
{
    Bit6 = 0x40,
    Bit7,
    Bit0_Wraparound,
/*CHECK:
    ^^^^^^^^^^^^^^^ implicit enum case value overflows underlying tag type
    ^^^^^^^^^^^^^^^ implicit value for enum case 'Bit0_Wraparound' overflows tag type 'int8_t' and wraps around
*/
    Bit1_Wraparound,
}

[Flags]
enum TestEnumS8_NoWrapAround : int8_t
{
    Bit6 = 0x40,
    Bit7,
    Bit0_Reset = 0, // no wraparound diagnostics here
}

[Flags]
enum TestEnumS16 : int16_t
{
    Bit14 = 0x4000,
    Bit15,
    Bit0_Wraparound,
/*CHECK:
    ^^^^^^^^^^^^^^^ implicit enum case value overflows underlying tag type
    ^^^^^^^^^^^^^^^ implicit value for enum case 'Bit0_Wraparound' overflows tag type 'int16_t' and wraps around
*/
    Bit1_Wraparound,
}

[Flags]
enum TestEnumS32 : int32_t
{
    Bit30 = 0x40000000,
    Bit31,
    Bit0_Wraparound,
/*CHECK:
    ^^^^^^^^^^^^^^^ implicit enum case value overflows underlying tag type
    ^^^^^^^^^^^^^^^ implicit value for enum case 'Bit0_Wraparound' overflows tag type 'int' and wraps around
*/
    Bit1_Wraparound,
}

[Flags]
enum TestEnumS64 : int64_t
{
    Bit62 = 0x4000000000000000,
    Bit63,
    Bit0_Wraparound,
/*CHECK:
    ^^^^^^^^^^^^^^^ implicit enum case value overflows underlying tag type
    ^^^^^^^^^^^^^^^ implicit value for enum case 'Bit0_Wraparound' overflows tag type 'int64_t' and wraps around
*/
    Bit1_Wraparound,
}

[Flags]
enum TestEnumU8 : uint8_t
{
    Bit6 = 0x40,
    Bit7,
    Bit0_Wraparound,
/*CHECK:
    ^^^^^^^^^^^^^^^ implicit enum case value overflows underlying tag type
    ^^^^^^^^^^^^^^^ implicit value for enum case 'Bit0_Wraparound' overflows tag type 'uint8_t' and wraps around
*/
    Bit1_Wraparound,
}

[Flags]
enum TestEnumU16 : uint16_t
{
    Bit14 = 0x4000,
    Bit15,
    Bit0_Wraparound,
/*CHECK:
    ^^^^^^^^^^^^^^^ implicit enum case value overflows underlying tag type
    ^^^^^^^^^^^^^^^ implicit value for enum case 'Bit0_Wraparound' overflows tag type 'uint16_t' and wraps around
*/
    Bit1_Wraparound,
}

[Flags]
enum TestEnumU32 : uint32_t
{
    Bit30 = 0x40000000,
    Bit31,
    Bit0_Wraparound,
/*CHECK:
    ^^^^^^^^^^^^^^^ implicit enum case value overflows underlying tag type
    ^^^^^^^^^^^^^^^ implicit value for enum case 'Bit0_Wraparound' overflows tag type 'uint' and wraps around
*/
    Bit1_Wraparound,
}

[Flags]
enum TestEnumU64 : uint64_t
{
    Bit62 = 0x4000000000000000,
    Bit63,
    Bit0_Wraparound,
/*CHECK:
    ^^^^^^^^^^^^^^^ implicit enum case value overflows underlying tag type
    ^^^^^^^^^^^^^^^ implicit value for enum case 'Bit0_Wraparound' overflows tag type 'uint64_t' and wraps around
*/
    Bit1_Wraparound,
}

[numthreads(1,1,1)]
void main()
{
    // TestEnumS8 verification
    static_assert(TestEnumS8.Bit7 == 0x80, "Flag before wraparound");
    static_assert(TestEnumS8.Bit0_Wraparound == 0x01, "Flag after wraparound");
    static_assert(TestEnumS8.Bit1_Wraparound == 0x02, "Flag after wraparound + 1");

    // TestEnumS16 verification
    static_assert(TestEnumS16.Bit15 == 0x8000, "Flag before wraparound");
    static_assert(TestEnumS16.Bit0_Wraparound == 0x01, "Flag after wraparound");
    static_assert(TestEnumS16.Bit1_Wraparound == 0x02, "Flag after wraparound + 1");

    // TestEnumS32 verification
    static_assert(TestEnumS32.Bit31 == 0x80000000, "Flag before wraparound");
    static_assert(TestEnumS32.Bit0_Wraparound == 0x01, "Flag after wraparound");
    static_assert(TestEnumS32.Bit1_Wraparound == 0x02, "Flag after wraparound + 1");

    // TestEnumS64 verification
    static_assert(TestEnumS64.Bit63 == 0x8000000000000000LL, "Flag before wraparound");
    static_assert(TestEnumS64.Bit0_Wraparound == 0x01, "Flag after wraparound");
    static_assert(TestEnumS64.Bit1_Wraparound == 0x02, "Flag after wraparound + 1");

    // TestEnumU8 verification
    static_assert(TestEnumU8.Bit7 == 0x80, "Flag before wraparound");
    static_assert(TestEnumU8.Bit0_Wraparound == 0x01, "Flag after wraparound");
    static_assert(TestEnumU8.Bit1_Wraparound == 0x02, "Flag after wraparound + 1");

    // TestEnumU16 verification
    static_assert(TestEnumU16.Bit15 == 0x8000, "Flag before wraparound");
    static_assert(TestEnumU16.Bit0_Wraparound == 0x01, "Flag after wraparound");
    static_assert(TestEnumU16.Bit1_Wraparound == 0x02, "Flag after wraparound + 1");

    // TestEnumU32 verification
    static_assert(TestEnumU32.Bit31 == 0x80000000, "Flag before wraparound");
    static_assert(TestEnumU32.Bit0_Wraparound == 0x01, "Flag after wraparound");
    static_assert(TestEnumU32.Bit1_Wraparound == 0x02, "Flag after wraparound + 1");

    // TestEnumU64 verification
    static_assert(TestEnumU64.Bit63 == 0x8000000000000000LL, "Flag before wraparound");
    static_assert(TestEnumU64.Bit0_Wraparound == 0x01, "Flag after wraparound");
    static_assert(TestEnumU64.Bit1_Wraparound == 0x02, "Flag after wraparound + 1");
}
