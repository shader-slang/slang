//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute
//
// Verifies that integer literal overflows trigger diagnostics. See
// docs/language-reference/expressions-literal.md for details.
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
    //
    // Warnings: decimal value in [INT64_MAX + 1, UINT64_MAX] with signed-style
    // suffix falls back to uint64_t per the (*)-marked rows of the spec table.
    //

    static_assert(IntegerLiteralType.UINT64  == getType(9223372036854775808),               "dec, no suffix: uint64_t low bound (warning)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615),              "dec, no suffix: uint64_t high bound (warning)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.UINT64  == getType(9223372036854775808L),              "dec, 'L' suffix: uint64_t low bound (warning)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615L),             "dec, 'L' suffix: uint64_t high bound (warning)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.UINT64  == getType(9223372036854775808LL),             "dec, 'LL' suffix: uint64_t low bound (warning)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615LL),            "dec, 'LL' suffix: uint64_t high bound (warning)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    //
    // Errors: decimal value > UINT64_MAX (18446744073709551616 = UINT64_MAX + 1)
    // for no/L/LL/U/UL/ULL suffix.
    //

    getType(18446744073709551616);        // dec, no suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(18446744073709551616L);       // dec, 'L' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(18446744073709551616LL);      // dec, 'LL' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(18446744073709551616U);       // dec, 'U' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(18446744073709551616UL);      // dec, 'UL' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(18446744073709551616ULL);     // dec, 'ULL' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    //
    // Errors: decimal value in [INTPTR_MAX + 1, UINT64_MAX] for 'Z' suffix.
    // The spec defines 'Z' dec only for [0, INTPTR_MAX]; no row falls back to
    // uintptr_t/uint64_t for a 'Z' (signed) suffix, so this is an error.
    //

    static_assert(IntegerLiteralType.UINTPTR == getType(9223372036854775808Z), "dec, 'Z' suffix: intptr_t overflow (error)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.UINTPTR == getType(18446744073709551615Z), "dec, 'Z' suffix: intptr_t overflow (error)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    //
    // Errors: decimal value > UINTPTR_MAX for 'Z'/'UZ'/'ZU' suffix.
    //

    getType(18446744073709551616Z);       // dec, 'Z' suffix: uintptr_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(18446744073709551616UZ);      // dec, 'UZ' suffix: uintptr_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(18446744073709551616ZU);      // dec, 'ZU' suffix: uintptr_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    //
    // Errors: hex value > UINT64_MAX (0x10000000000000000 = 2^64) for any suffix.
    //

    getType(0x10000000000000000);         // hex, no suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(0x10000000000000000L);        // hex, 'L' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(0x10000000000000000LL);       // hex, 'LL' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(0x10000000000000000U);        // hex, 'U' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(0x10000000000000000UL);       // hex, 'UL' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(0x10000000000000000ULL);      // hex, 'ULL' suffix: uint64_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(0x10000000000000000Z);        // hex, 'Z' suffix: uintptr_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(0x10000000000000000UZ);       // hex, 'UZ' suffix: uintptr_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    getType(0x10000000000000000ZU);       // hex, 'ZU' suffix: uintptr_t overflow (error)
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type

    // negated overflow
    static_assert(IntegerLiteralType.INT64   == getType(-9223372036854775809),   "unary-minus(INT64_MAX+2), no suffix");
//CHECK:                                                 ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    // check that not/plus prefixes still trigger the overflow warning (ignored with minus)
    static_assert(IntegerLiteralType.INT64   == getType(~9223372036854775808),   "bit-wise-not(INT64_MAX+1), no suffix");
//CHECK:                                                 ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.INT64   == getType(+9223372036854775808),   "unary-plus(INT64_MAX+1), no suffix");
//CHECK:                                                 ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned
}
