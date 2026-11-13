//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
// The second run passes `-disable-non-essential-validations`, which skips the front-end
// `checkAutoDiffUsages` check; this exercises the backstop copy of the check inside the auto-diff
// translation pass.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -disable-non-essential-validations

// A function that calls the composed `bwd_diff(fwd_diff(fff))` cannot itself be differentiated,
// for the same reason as in `nested-backward-diff-error.slang`: the result of `bwd_diff` is not
// differentiable. The composed callee is a requirement of the `IBackwardDifferentiable`
// conformance synthesized for `fwd_diff(fff)` rather than a `bwd_diff` of a declared function, and
// it must be rejected all the same instead of being differentiated as if it were constant.
// See https://github.com/shader-slang/slang/issues/13321.

[Differentiable]
float fff(float x, float y)
{
    return x * y;
}

[Differentiable]
float ggg(float x, float y)
{
    var a = diffPair(diffPair(x, 1.0));
    var b = diffPair(diffPair(y, 1.0));
    bwd_diff(fwd_diff(fff))(a, b, diffPair(0.0, 1.0));
//CHECK:                   ^ cannot differentiate the result of a backward-derivative call
//CHECK:                   ^ the code produced by 'bwd_diff' is not itself differentiable
    return fff(x, y) + a.d.p + b.d.d;
}

[shader("fragment")]
float4 fs_main(float4 in: SV_Position) : SV_TARGET
{
    var xp2 = diffPair(in.x, 1.0);
    var yp2 = diffPair(in.y, 1.0);
    bwd_diff(ggg)(xp2, yp2, 1.0);
    return float4(xp2.d, yp2.d, 0, 0);
}
