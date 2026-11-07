//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/12979
//
// An integer literal that is EXACTLY representable in `half` (IEEE binary16)
// must not trigger the E30081 "implicit conversion not recommended" warning.
// half has an 11-bit significand, so every integer up to 2048 is exact, but so
// are many larger integers (4096 = 2^12, 8192 = 2^13, 65504 = the largest
// finite half, and their negatives). Only literals that lose precision when
// rounded to half (e.g. 4097 -> 4096, 65505 -> 65504) or that exceed half's
// finite range (e.g. 131072 -> +inf) should warn. The same holds for unsigned
// literals, which fold through the same path.
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
//CHECK:     ^^^^ implicit conversion not recommended
//CHECK:     ^^^^ implicit conversion from 'int' to 'half' is not recommended
    half h = 65505;
//CHECK:     ^^^^^ implicit conversion not recommended
//CHECK:     ^^^^^ implicit conversion from 'int' to 'half' is not recommended
    half i = 4097u;
//CHECK:     ^^^^^ implicit conversion not recommended
//CHECK:     ^^^^^ implicit conversion from 'uint' to 'half' is not recommended
    half m = -4097;  // negative, lossy (rounds to -4096)
//CHECK:      ^^^^ implicit conversion not recommended
//CHECK:      ^^^^ implicit conversion from 'int' to 'half' is not recommended

    // Exceed half's finite range (overflow to +inf) -> WARN.
    half j = 131072u;
//CHECK:     ^^^^^^^ implicit conversion not recommended
//CHECK:     ^^^^^^^ implicit conversion from 'uint' to 'half' is not recommended

    output[0] = a + b + c + d + e + f + g + h + i + j + k + m;
}
