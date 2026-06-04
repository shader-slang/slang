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
    // Warnings: decimal value in [INTPTR_MAX + 1, UINT64_MAX] with 'Z' suffix
    // falls back to uintptr_t per the (*)-marked rows of the spec table, in the
    // same way as the 'LL' cases above.
    //
    static_assert(IntegerLiteralType.UINTPTR == getType(9223372036854775808Z),              "dec, 'Z' suffix: uintptr_t low bound (warning)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.UINTPTR == getType(18446744073709551615Z),             "dec, 'Z' suffix: uintptr_t high bound (warning)");
//CHECK:                                                ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned


    // negated overflow
    static_assert(IntegerLiteralType.UINT64  == getType(-9223372036854775809),   "unary-minus(INT64_MAX+2), no suffix");
//CHECK:                                                 ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    // check that not/plus prefixes still trigger the overflow warning (ignored with minus)
    static_assert(IntegerLiteralType.UINT64  == getType(~9223372036854775808),   "bit-wise-not(INT64_MAX+1), no suffix");
//CHECK:                                                 ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    static_assert(IntegerLiteralType.UINT64  == getType(+9223372036854775808),   "unary-plus(INT64_MAX+1), no suffix");
//CHECK:                                                 ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned
}
