//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/12929
//
// Check that precision loss in integer literal -> double conversion is diagnosed.
//
// diag=CHECK is an exhaustive match: a source line with no //CHECK asserts that
// no diagnostic is emitted for it.

RWStructuredBuffer<double> output;

static const int64_t kLossy = 9007199254740993LL;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Exactly representable in double -> NO warning.
    double sum = 0;
    sum += 1;
    sum += 2;

    // Every 32-bit value fits in double's 53-bit mantissa, so the whole int and
    // uint range is exact. Contrast with float, where 0xFFFFFFFF is lossy.
    sum += 2147483647;
    sum += -2147483648;
    sum += 4294967295u;

    // contiguous-exact-range boundary
    sum += 9007199254740991LL; // 2^53 - 1
    sum += 0x001FFFFFFFFFFFFFLLU;
    sum += 9007199254740992LL; // 2^53, last representable
    sum += 9007199254740994LL; // now every second is representable
    sum += 9007199254740996LL; // now every second is representable

    sum += -9007199254740991LL;
    sum += -9007199254740992LL; // last representable
    sum += -9007199254740994LL; // now every second is representable
    sum += -9007199254740996LL; // now every second is representable

    // largest representables
    sum += 18446744073709549568LLU;
    sum += 0xFFFFFFFFFFFFF800LLU;

    sum += -9223372036854774784LL;
    sum += -0x7FFFFFFFFFFFFC00LL;

    // Two's complement minimum is a power of two, so it is exact
    sum += -9223372036854775808;

    // 53-bit span at mid scale
    sum += 0x0FFFFFFFFFFFFF80LLU;

    // An explicit cast opts out of the diagnostic entirely
    sum += (double)9007199254740993LL;
    sum += double(9007199254740993LL);

    sum += 9007199254740993LL;
//CHECK:   ^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^ precision loss in converting value '9007199254740993' to 'double'

    sum += -9007199254740993LL;
//CHECK:    ^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:    ^^^^^^^^^^^^^^^^^^ precision loss in converting value '-9007199254740993' to 'double'

    // one mantissa bit too many at various scales
    sum += 18014398509481983LL;
//CHECK:   ^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^ precision loss in converting value '18014398509481983' to 'double'

    sum += 0x0FFFFFFFFFFFFFC0LLU;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '1152921504606846912' to 'double'

    sum += -0x0FFFFFFFFFFFFFC0LL;
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '-1152921504606846912' to 'double'

    sum += 0xFFFFFFFFFFFFFC00LLU;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '18446744073709550592' to 'double'

    sum += 0x7FFFFFFFFFFFFE00LLU;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '9223372036854775296' to 'double'

    sum += -0x7FFFFFFFFFFFFE00LL;
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '-9223372036854775296' to 'double'

    sum += 9223372036854775807LL;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '9223372036854775807' to 'double'

    sum += 18446744073709551615u;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '18446744073709551615' to 'double'

    // The check runs on the constant-folded value, not on the literal token.
    sum += kLossy;
//CHECK:   ^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^ precision loss in converting value '9007199254740993' to 'double'

    sum += 9007199254740992LL + 1;
//CHECK:                      ^ implicit conversion not recommended: precision loss
//CHECK:                      ^ precision loss in converting value '9007199254740993' to 'double'

    output[0] = sum;
}
