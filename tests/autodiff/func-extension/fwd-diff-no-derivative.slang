//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):

// Test: calling fwd_diff on a specialization that lacks a derivative extension

T sqr<T : IArithmetic>(T x) { return x * x; }

// Only provide fwd_diff for float specialization
__func_extension fwd_diff(sqr<float>)(DifferentialPair<float> dpx) -> DifferentialPair<float>
{
    return diffPair(sqr(dpx.p), 2.0f * dpx.p * dpx.d);
}

void main()
{
    // sqr<int> has no derivative — should produce an error.
    let x = fwd_diff(sqr<int>)(diffPair(3, 1));
//CHECK:                               ^ type mismatch
}
