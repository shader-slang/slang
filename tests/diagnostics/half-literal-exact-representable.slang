//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression for https://github.com/shader-slang/slang/issues/12979.
//
// An integer literal that is EXACTLY representable in `half` (IEEE binary16)
// must not trigger the E30081 "implicit conversion not recommended" warning.
// half has an 11-bit significand, so every integer up to 2048 is exact, but so
// are many larger integers (4096 = 2^12, 8192 = 2^13, 65504 = the largest
// finite half, and their negatives). Only literals that actually lose precision
// when rounded to half (e.g. 4097 -> 4096) or that exceed half's finite range
// should warn.
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
    half f = 4096u;  // uint literal, exact (covers the uint fold path)

    // Not exactly representable -> WARN.
    half g = 4097;
//CHECK:     ^^^^ implicit conversion not recommended
//CHECK:     ^^^^ implicit conversion from 'int' to 'half' is not recommended
    half h = 65505;
//CHECK:     ^^^^^ implicit conversion not recommended
//CHECK:     ^^^^^ implicit conversion from 'int' to 'half' is not recommended
    half i = 65505u;
//CHECK:     ^^^^^^ implicit conversion not recommended
//CHECK:     ^^^^^^ implicit conversion from 'uint' to 'half' is not recommended

    output[0] = a + b + c + d + e + f + g + h + i;
}
