//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int doSomething(int a)
{
    // No warning, literal will be interpreted as 64 bit.
    uint64_t c0 = 0x800000000;

    // No warning as top bits are just ignored
    int c1 = -1ll;

    int c2 = int(-1u);

    // Should sign extend
    int c3 = 0x80000000;

    // No warning, hex literal will be interpreted as an unsigned 64 integer then signed with negative operator.
    int64_t c4 = -0xfffffffff;

    a += (int)c0 + c1 + c2;

    int64_t b = 0;

    // Ok
    b += 0x800000000ll;

    uint64_t c5 = -2ull;

    // Warning, integer literal is too large for signed 64 bit, must be interpreted as unsigned.
    uint64_t d0 = 18446744073709551615;
//CHECK:          ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    // Warning, integer literal is too small for signed 64 bit, must be interpreted as unsigned.
    uint64_t d1 = -9223372036854775809;
//CHECK:           ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    // This is INT64_MIN and valid negative signed integer, no warning will be emitted.
    int64_t d2 = -9223372036854775808;

    // No warning: the type ladder for non-decimal integer literals admits
    // `uint64_t`, so `0xFFFFFFFFFFFFFFFF` is silently typed as `uint64_t`
    // (#11216). Reinterpreting that bit pattern as `int64_t` is `-1`, so
    // there is no overflowing-assignment warning either.
    int64_t x4 = 0xFFFFFFFFFFFFFFFF;

    // No warning: hex literal in the (INT64_MAX, UINT64_MAX] range is `uint64_t`
    // per the documented type ladder for non-decimal literals (#11216).
    uint64_t x5 = 0x8000000000000000;

    // No warning: octal/binary literals follow the same non-decimal ladder.
    uint64_t x6 = 0b1000000000000000000000000000000000000000000000000000000000000000;

    return a + int(b);
}
