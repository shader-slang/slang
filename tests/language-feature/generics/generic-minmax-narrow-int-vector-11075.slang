//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry test -stage compute

// A generic `min`/`max` on a vector of a narrow integer (`int8/16`, `uint8/16`) is a clean language
// error (E55216), not an unresolvable `I16_min(...)` call: the narrow ints have no scalar
// `<prefix>_min`/`<prefix>_max` helper in the preludes, so no element-wise vector helper can be
// built for them, and the `$P` expander diagnoses the vector instead. `int16_t` here is
// representative of the narrow-int class; `uint8_t`/`int8_t`/`uint16_t` reach the same diagnostic.
// Regression test for shader-slang/slang#11075; covers the narrow-int side of the E55216 boundary.

T genMin<T : IComparable>(T a, T b)
{
    return min(a, b);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void test()
{
    vector<int16_t, 2> a = {int16_t(3), int16_t(9)};
    vector<int16_t, 2> b = {int16_t(5), int16_t(2)};
    vector<int16_t, 2> r = genMin(a, b);
    outputBuffer[0] = float(r.x);
    //CHECK: E55216
    //CHECK: does not support operand type
}
