//DISABLE_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature
// TODO: re-enable when func-extension visibility is implemented separately.
//CHECK: invalid private visibility
//CHECK: cannot have private visibility because it is not a member of a type.

// Test: visibility modifiers on generic `__func_extension` declarations go
// through normal visibility validation rather than being rejected as an
// unsupported modifier placement.

T sqr<T : IArithmetic>(T x) { return x * x; }

private __func_extension<T : IFloat> fwd_diff(sqr<T>)(DifferentialPair<T> dpx) -> DifferentialPair<T>
    where T.Differential == T
{
    return diffPair(sqr(dpx.p), T(2.0) * dpx.p * T(dpx.d));
}
