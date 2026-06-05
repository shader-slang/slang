//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Regression test for the old crash/ICE path. `bwd_diff(s.distance)` resolves
// to a static derivative function, so omitting `s` should be diagnosed at this
// call site instead of reaching IR lowering with a malformed argument list.

struct Sphere
{
    float r;

    [Differentiable]
    [NoDiffThis]
    float distance(float3 p)
    {
        return length(p) - r;
    }
}

void test(float3 p)
{
    Sphere s = {1.0};
    var dp = diffPair(p);

    // Missing explicit `this` argument: expected spelling is
    // `__bwd_diff(s.distance)(s, dp, 1.0)`.
    __bwd_diff(s.distance)(dp, 1.0);
// CHECK: not enough arguments to call
// CHECK: not enough arguments to call (got 2, expected 3)
}
