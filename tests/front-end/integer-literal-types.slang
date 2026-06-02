//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute
//
// Verifies that integer literals have correct types as specified in the
// summary table at docs/language-reference/expressions-literal.md.
//
// This test covers, for every row of that table, the lowest and highest
// value in the row and verifies that the resulting literal selects the
// expected type (resolved through overload resolution on `getType()`).
//
// Pointer-sized rows: `intptr_t`/`uintptr_t` are sized as the host's
// `intptr_t` during compilation, which is 64 bits on every supported
// development host. Only the 64-bit `Z`/`UZ` rows are exercised here;
// the 32-bit rows from the summary table cannot be tested in this file.
//
// Negation special cases (the (**) marker in the table, e.g.
// `-2147483648`) are not part of the summary table itself and are
// covered separately.
//
// Some rows in the summary table are marked (*) and intentionally trigger
// a warning (e.g. decimal value > INT64_MAX selecting `uint64_t`). The
// test directive therefore does not pass `-warnings-as-errors`.
//
// This test is complemented by
// - tests/diagnostics/integer-literal-overflow.slang for overflow diagnostics.
// - tests/front-end/integer-literal-types-intmin.slang for minimum
//   negative integer values.
//
// Test enablement is tracked by https://github.com/shader-slang/slang/issues/11216

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

    //
    // No suffix, decimal base
    //
    // Row: dec [0, 2147483647] -> int
    static_assert(IntegerLiteralType.INT32   == getType(0),                                 "dec, no suffix: int low bound");
    static_assert(IntegerLiteralType.INT32   == getType(2147483647),                        "dec, no suffix: int high bound");
    // Row: dec [2147483648, 9223372036854775807] -> int64_t
    static_assert(IntegerLiteralType.INT64   == getType(2147483648),                        "dec, no suffix: int64_t low bound");
    static_assert(IntegerLiteralType.INT64   == getType(9223372036854775807),               "dec, no suffix: int64_t high bound");
    // Row: dec [9223372036854775808, 18446744073709551615] -> uint64_t (warning)
    static_assert(IntegerLiteralType.UINT64  == getType(9223372036854775808),               "dec, no suffix: uint64_t low bound (warning)");
    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615),              "dec, no suffix: uint64_t high bound (warning)");

    //
    // No suffix, hexadecimal base
    //
    // Row: hex [0x0, 0x7FFFFFFF] -> int
    static_assert(IntegerLiteralType.INT32   == getType(0x0),                               "hex, no suffix: int low bound");
    static_assert(IntegerLiteralType.INT32   == getType(0x7FFFFFFF),                        "hex, no suffix: int high bound");
    // Row: hex [0x80000000, 0xFFFFFFFF] -> uint
    static_assert(IntegerLiteralType.UINT32  == getType(0x80000000),                        "hex, no suffix: uint low bound");
    static_assert(IntegerLiteralType.UINT32  == getType(0xFFFFFFFF),                        "hex, no suffix: uint high bound");
    // Row: hex [0x100000000, 0x7FFFFFFFFFFFFFFF] -> int64_t
    static_assert(IntegerLiteralType.INT64   == getType(0x100000000),                       "hex, no suffix: int64_t low bound");
    static_assert(IntegerLiteralType.INT64   == getType(0x7FFFFFFFFFFFFFFF),                "hex, no suffix: int64_t high bound");
    // Row: hex [0x8000000000000000, 0xFFFFFFFFFFFFFFFF] -> uint64_t
    static_assert(IntegerLiteralType.UINT64  == getType(0x8000000000000000),                "hex, no suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(0xFFFFFFFFFFFFFFFF),                "hex, no suffix: uint64_t high bound");

    //
    // No suffix, binary base (shares hex/bin/oct row with hex)
    //
    static_assert(IntegerLiteralType.INT32   == getType(0b0),                                                                "bin, no suffix: int low bound");
    static_assert(IntegerLiteralType.INT32   == getType(0b1111111111111111111111111111111),                                  "bin, no suffix: int high bound (0x7FFFFFFF)");
    static_assert(IntegerLiteralType.UINT32  == getType(0b10000000000000000000000000000000),                                 "bin, no suffix: uint low bound (0x80000000)");
    static_assert(IntegerLiteralType.UINT64  == getType(0b1000000000000000000000000000000000000000000000000000000000000000), "bin, no suffix: uint64_t low bound (0x8000000000000000)");

    //
    // No suffix, octal base (shares hex/bin/oct row with hex; octal literals are deprecated and warn)
    //
    static_assert(IntegerLiteralType.INT32   == getType(017777777777),            "oct, no suffix: int high bound (0x7FFFFFFF)");
    static_assert(IntegerLiteralType.UINT32  == getType(020000000000),            "oct, no suffix: uint low bound (0x80000000)");
    static_assert(IntegerLiteralType.INT64   == getType(040000000000),            "oct, no suffix: int64_t low bound (0x100000000)");
    static_assert(IntegerLiteralType.UINT64  == getType(01000000000000000000000), "oct, no suffix: uint64_t low bound (0x8000000000000000)");

    //
    // 'L' suffix (same rows as no-suffix)
    //
    // dec [0, 2147483647] -> int
    static_assert(IntegerLiteralType.INT32   == getType(0L),                                "dec, 'L' suffix: int low bound");
    static_assert(IntegerLiteralType.INT32   == getType(2147483647L),                       "dec, 'L' suffix: int high bound");
    // dec [2147483648, 9223372036854775807] -> int64_t
    static_assert(IntegerLiteralType.INT64   == getType(2147483648L),                       "dec, 'L' suffix: int64_t low bound");
    static_assert(IntegerLiteralType.INT64   == getType(9223372036854775807L),              "dec, 'L' suffix: int64_t high bound");
    // dec [9223372036854775808, 18446744073709551615] -> uint64_t (warning)
    static_assert(IntegerLiteralType.UINT64  == getType(9223372036854775808L),              "dec, 'L' suffix: uint64_t low bound (warning)");
    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615L),             "dec, 'L' suffix: uint64_t high bound (warning)");
    // hex [0x0, 0x7FFFFFFF] -> int
    static_assert(IntegerLiteralType.INT32   == getType(0x0L),                              "hex, 'L' suffix: int low bound");
    static_assert(IntegerLiteralType.INT32   == getType(0x7FFFFFFFL),                       "hex, 'L' suffix: int high bound");
    // hex [0x80000000, 0xFFFFFFFF] -> uint
    static_assert(IntegerLiteralType.UINT32  == getType(0x80000000L),                       "hex, 'L' suffix: uint low bound");
    static_assert(IntegerLiteralType.UINT32  == getType(0xFFFFFFFFL),                       "hex, 'L' suffix: uint high bound");
    // hex [0x100000000, 0x7FFFFFFFFFFFFFFF] -> int64_t
    static_assert(IntegerLiteralType.INT64   == getType(0x100000000L),                      "hex, 'L' suffix: int64_t low bound");
    static_assert(IntegerLiteralType.INT64   == getType(0x7FFFFFFFFFFFFFFFL),               "hex, 'L' suffix: int64_t high bound");
    // hex [0x8000000000000000, 0xFFFFFFFFFFFFFFFF] -> uint64_t
    static_assert(IntegerLiteralType.UINT64  == getType(0x8000000000000000L),               "hex, 'L' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(0xFFFFFFFFFFFFFFFFL),               "hex, 'L' suffix: uint64_t high bound");

    //
    // 'LL' suffix
    //
    // dec [0, 9223372036854775807] -> int64_t
    static_assert(IntegerLiteralType.INT64   == getType(0LL),                               "dec, 'LL' suffix: int64_t low bound");
    static_assert(IntegerLiteralType.INT64   == getType(9223372036854775807LL),             "dec, 'LL' suffix: int64_t high bound");
    // dec [9223372036854775808, 18446744073709551615] -> uint64_t (warning)
    static_assert(IntegerLiteralType.UINT64  == getType(9223372036854775808LL),             "dec, 'LL' suffix: uint64_t low bound (warning)");
    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615LL),            "dec, 'LL' suffix: uint64_t high bound (warning)");
    // hex [0x0, 0x7FFFFFFFFFFFFFFF] -> int64_t
    static_assert(IntegerLiteralType.INT64   == getType(0x0LL),                             "hex, 'LL' suffix: int64_t low bound");
    static_assert(IntegerLiteralType.INT64   == getType(0x7FFFFFFFFFFFFFFFLL),              "hex, 'LL' suffix: int64_t high bound");
    // hex [0x8000000000000000, 0xFFFFFFFFFFFFFFFF] -> uint64_t
    static_assert(IntegerLiteralType.UINT64  == getType(0x8000000000000000LL),              "hex, 'LL' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(0xFFFFFFFFFFFFFFFFLL),              "hex, 'LL' suffix: uint64_t high bound");

    //
    // 'U' / 'UL' / 'LU' suffix (decimal and non-decimal share the same rows)
    //
    // [0, 4294967295] -> uint
    static_assert(IntegerLiteralType.UINT32  == getType(0U),                                "dec, 'U' suffix: uint low bound");
    static_assert(IntegerLiteralType.UINT32  == getType(4294967295U),                       "dec, 'U' suffix: uint high bound");
    static_assert(IntegerLiteralType.UINT32  == getType(0UL),                               "dec, 'UL' suffix: uint low bound");
    static_assert(IntegerLiteralType.UINT32  == getType(4294967295UL),                      "dec, 'UL' suffix: uint high bound");
    static_assert(IntegerLiteralType.UINT32  == getType(0LU),                               "dec, 'LU' suffix: uint low bound");
    static_assert(IntegerLiteralType.UINT32  == getType(4294967295LU),                      "dec, 'LU' suffix: uint high bound");
    static_assert(IntegerLiteralType.UINT32  == getType(0x0U),                              "hex, 'U' suffix: uint low bound");
    static_assert(IntegerLiteralType.UINT32  == getType(0xFFFFFFFFU),                       "hex, 'U' suffix: uint high bound");
    // [4294967296, 18446744073709551615] -> uint64_t
    static_assert(IntegerLiteralType.UINT64  == getType(4294967296U),                       "dec, 'U' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615U),             "dec, 'U' suffix: uint64_t high bound");
    static_assert(IntegerLiteralType.UINT64  == getType(4294967296UL),                      "dec, 'UL' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615UL),            "dec, 'UL' suffix: uint64_t high bound");
    static_assert(IntegerLiteralType.UINT64  == getType(4294967296LU),                      "dec, 'LU' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615LU),            "dec, 'LU' suffix: uint64_t high bound");
    static_assert(IntegerLiteralType.UINT64  == getType(0x100000000U),                      "hex, 'U' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(0xFFFFFFFFFFFFFFFFU),               "hex, 'U' suffix: uint64_t high bound");

    //
    // 'ULL' / 'LLU' suffix (any base)
    //
    // [0, 18446744073709551615] -> uint64_t
    static_assert(IntegerLiteralType.UINT64  == getType(0ULL),                              "dec, 'ULL' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615ULL),           "dec, 'ULL' suffix: uint64_t high bound");
    static_assert(IntegerLiteralType.UINT64  == getType(0LLU),                              "dec, 'LLU' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(18446744073709551615LLU),           "dec, 'LLU' suffix: uint64_t high bound");
    static_assert(IntegerLiteralType.UINT64  == getType(0x0ULL),                            "hex, 'ULL' suffix: uint64_t low bound");
    static_assert(IntegerLiteralType.UINT64  == getType(0xFFFFFFFFFFFFFFFFULL),             "hex, 'ULL' suffix: uint64_t high bound");

    //
    // 'Z' suffix (64-bit pointers; intptr_t == int64_t on host)
    //
    // dec [0, 9223372036854775807] -> intptr_t
    static_assert(IntegerLiteralType.INTPTR  == getType(0Z),                                "dec, 'Z' suffix (64-bit): intptr_t low bound");
    static_assert(IntegerLiteralType.INTPTR  == getType(9223372036854775807Z),              "dec, 'Z' suffix (64-bit): intptr_t high bound");
    // hex [0x0, 0xFFFFFFFFFFFFFFFF] -> intptr_t
    static_assert(IntegerLiteralType.INTPTR  == getType(0x0Z),                              "hex, 'Z' suffix (64-bit): intptr_t low bound");
    static_assert(IntegerLiteralType.INTPTR  == getType(0xFFFFFFFFFFFFFFFFZ),               "hex, 'Z' suffix (64-bit): intptr_t high bound");

    //
    // 'UZ' / 'ZU' suffix (64-bit pointers; uintptr_t == uint64_t on host)
    //
    // dec [0, 18446744073709551615] -> uintptr_t
    static_assert(IntegerLiteralType.UINTPTR == getType(0UZ),                               "dec, 'UZ' suffix (64-bit): uintptr_t low bound");
    static_assert(IntegerLiteralType.UINTPTR == getType(18446744073709551615UZ),            "dec, 'UZ' suffix (64-bit): uintptr_t high bound");
    static_assert(IntegerLiteralType.UINTPTR == getType(0ZU),                               "dec, 'ZU' suffix (64-bit): uintptr_t low bound");
    static_assert(IntegerLiteralType.UINTPTR == getType(18446744073709551615ZU),            "dec, 'ZU' suffix (64-bit): uintptr_t high bound");
    // hex [0x0, 0xFFFFFFFFFFFFFFFF] -> uintptr_t
    static_assert(IntegerLiteralType.UINTPTR == getType(0x0UZ),                             "hex, 'UZ' suffix (64-bit): uintptr_t low bound");
    static_assert(IntegerLiteralType.UINTPTR == getType(0xFFFFFFFFFFFFFFFFUZ),              "hex, 'UZ' suffix (64-bit): uintptr_t high bound");
}
