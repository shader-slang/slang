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
    // Exactly representable in half -> NO warning.
    half a = 2048;   // contiguous-exact-range boundary
    half b = 4096;   // 2^12
    half c = 8192;   // 2^13
    half d = 65504;  // largest finite half
    half e = -4096;  // negative, exact
    half f = 4096u;  // exact, unsigned
    half k = 3072;   // non-power-of-two, exact (locks the (2048, 4096) band)

    // Lose precision when rounded to half -> WARN.
    half g = 4097;
//CHECK:     ^^^^ implicit conversion not recommended: precision loss
//CHECK:     ^^^^ precision lost in conversion from 'int' to 'half'
    half h = 65505;
//CHECK:     ^^^^^ implicit conversion not recommended: out of range
//CHECK:     ^^^^^ value out of range in conversion from 'int' to 'half'
    half i = 4097u;
//CHECK:     ^^^^^ implicit conversion not recommended: precision loss
//CHECK:     ^^^^^ precision lost in conversion from 'uint' to 'half'
    half m = -4097;  // negative, lossy (rounds to -4096)
//CHECK:      ^^^^ implicit conversion not recommended: precision loss
//CHECK:      ^^^^ precision lost in conversion from 'int' to 'half'

    // Exceed half's finite range (overflow to +inf) -> WARN.
    half j = 131072u;
//CHECK:     ^^^^^^^ implicit conversion not recommended: out of range
//CHECK:     ^^^^^^^ value out of range in conversion from 'uint' to 'half'

    // Special case: uint64_t out-of-range value that is in range if
    // incorrectly interpreted as int64_t
    half n = 18446744073709551615u; // -1 as int64_t

    output[0] = a + b + c + d + e + f + g + h + i + j + k + m + n;
}
