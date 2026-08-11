//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute -warnings-as-errors all
//
// Verifies correct minimum integer decimal value handling as
// specified in docs/language-reference/expressions-literal.md.
//
// This test amends tests/front-end/integer-literal-types.slang.

enum IntegerLiteralType
{
    INT32 = 1,
    UINT32,
    INT64,
    UINT64,
    INTPTR,
    UINTPTR
}

[ForceInline] IntegerLiteralType getType(int       i) { return IntegerLiteralType.INT32;   }
[ForceInline] IntegerLiteralType getType(uint      i) { return IntegerLiteralType.UINT32;  }
[ForceInline] IntegerLiteralType getType(int64_t   i) { return IntegerLiteralType.INT64;   }
[ForceInline] IntegerLiteralType getType(uint64_t  i) { return IntegerLiteralType.UINT64;  }
[ForceInline] IntegerLiteralType getType(intptr_t  i) { return IntegerLiteralType.INTPTR;  }
[ForceInline] IntegerLiteralType getType(uintptr_t i) { return IntegerLiteralType.UINTPTR; }

[numthreads(1,1,1)]
void main()
{
    //CHECK: %main = OpFunction

    static_assert(IntegerLiteralType.INT32   == getType(-2147483648),            "minimum value of int32_t, no suffix");
    static_assert(IntegerLiteralType.INT64   == getType(-9223372036854775808),   "minimum value of int64_t, no suffix");

    static_assert(IntegerLiteralType.INT32   == getType(-2147483648L),           "minimum value of int32_t, 'L'");
    static_assert(IntegerLiteralType.INT64   == getType(-9223372036854775808L),  "minimum value of int64_t 'L'");

    static_assert(IntegerLiteralType.INT64   == getType(-2147483648LL),          "minimum value of int32_t 'LL'");
    static_assert(IntegerLiteralType.INT64   == getType(-9223372036854775808LL), "minimum value of int64_t 'LL'");

    static_assert(IntegerLiteralType.INTPTR  == getType(-9223372036854775808Z),  "minimum value of intptr_t 'Z'");

    // Special values related to folding prefix + (INT_MAX+1)
    static_assert(IntegerLiteralType.INT64   == getType(2147483648),                        "dec, no suffix: int64_t low bound");
    static_assert(IntegerLiteralType.INT32   == getType(-2147483648),                       "dec, no suffix: neg + int64_t low bound");
    static_assert(IntegerLiteralType.INT64   == getType(+2147483648),                       "dec, no suffix: plus + int64_t low bound");
    static_assert(IntegerLiteralType.INT64   == getType(~2147483648),                       "dec, no suffix: not + int64_t low bound");
}
