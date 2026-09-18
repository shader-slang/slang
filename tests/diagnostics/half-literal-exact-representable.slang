//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/12979
//
// Check that precision loss in integer literal -> half conversion is diagnosed.
//
// diag=CHECK is an exhaustive match: a source line with no //CHECK asserts that
// no diagnostic is emitted for it.

RWStructuredBuffer<half> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    half sum = 0;

    // Exactly representable in half -> NO warning.
    sum   += 2048;   // contiguous-exact-range boundary
    sum   += 4096;   // 2^12
    sum   += 8192;   // 2^13
    sum   += 65504;  // largest finite half
    sum   += -65504; // smallest finite half
    sum   += 65472;  // largest finite half - ulp
    sum   += -65472; // smallest finite half + ulp
    sum   += -4096;  // negative, exact
    sum   += 4096u;  // exact, unsigned
    sum   += -4100;  // negative, exact
    sum   += 4100;   // non-power-of-two, exact
    sum   += 4100u;  // non-power-of-two, exact, unsigned
    sum   += 3072;   // non-power-of-two, exact (locks the (2048, 4096) band)

    // Lose precision when rounded to half -> WARN.
    sum   += 2049;
//CHECK:     ^^^^ implicit conversion not recommended: precision loss
//CHECK:     ^^^^ precision loss in converting value '2049' to 'half'
    sum   += 4097;
//CHECK:     ^^^^ implicit conversion not recommended: precision loss
//CHECK:     ^^^^ precision loss in converting value '4097' to 'half'
    sum   += 4098;
//CHECK:     ^^^^ implicit conversion not recommended: precision loss
//CHECK:     ^^^^ precision loss in converting value '4098' to 'half'
    sum   += 4099;
//CHECK:     ^^^^ implicit conversion not recommended: precision loss
//CHECK:     ^^^^ precision loss in converting value '4099' to 'half'
    sum   += 65503;
//CHECK:     ^^^^^ implicit conversion not recommended: precision loss
//CHECK:     ^^^^^ precision loss in converting value '65503' to 'half'
    sum   += 4097u;
//CHECK:     ^^^^^ implicit conversion not recommended: precision loss
//CHECK:     ^^^^^ precision loss in converting value '4097' to 'half'
    sum   += -2049; // negative, lossy
//CHECK:      ^^^^ implicit conversion not recommended: precision loss
//CHECK:      ^^^^ precision loss in converting value '-2049' to 'half'
    sum   += -4097; // negative, lossy (rounds to -4096)
//CHECK:      ^^^^ implicit conversion not recommended: precision loss
//CHECK:      ^^^^ precision loss in converting value '-4097' to 'half'

    // Exceed half's finite range (overflow to +inf) -> WARN.
    sum   += 65505;
//CHECK:     ^^^^^ implicit conversion not recommended: value out of range
//CHECK:     ^^^^^ value '65505' out of range in conversion to 'half'
    sum   += -65505;
//CHECK:      ^^^^^ implicit conversion not recommended: value out of range
//CHECK:      ^^^^^ value '-65505' out of range in conversion to 'half'
    sum   += 131072u;
//CHECK:     ^^^^^^^ implicit conversion not recommended: value out of range
//CHECK:     ^^^^^^^ value '131072' out of range in conversion to 'half'

    // Special case: uint64_t out-of-range value that is in range if
    // incorrectly interpreted as int64_t
    sum   += 18446744073709551615u; // -1 as int64_t
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: value out of range
//CHECK:     ^^^^^^^^^^^^^^^^^^^^^ value '18446744073709551615' out of range in conversion to 'half'

    output[0] = sum;
}
