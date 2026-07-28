//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry test -stage compute

// A generic `min`/`max` on a vector whose element type has a `$P` prefix but no vector-arity
// prelude helper (`intptr_t` → `IPTR`, scalar helper only) must fail as a clean language error,
// not emit an unresolvable `IPTR_min(...)` call. Regression test for shader-slang/slang#11075.

T genMin<T : IComparable>(T a, T b)
{
    return min(a, b);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void test()
{
    vector<intptr_t, 2> a = {intptr_t(3), intptr_t(9)};
    vector<intptr_t, 2> b = {intptr_t(5), intptr_t(2)};
    vector<intptr_t, 2> r = genMin(a, b);
    outputBuffer[0] = float(r.x);
    //CHECK: E55215
    //CHECK: does not support operand type
}
