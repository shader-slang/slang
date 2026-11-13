//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -experimental-feature
// The second run passes `-disable-non-essential-validations`, which skips the front-end
// `checkAutoDiffUsages` check; this exercises the backstop copy of the check inside the auto-diff
// translation pass.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -experimental-feature -disable-non-essential-validations

// The experimental `__apply(fwd_diff(fff))` looks up the `apply_bwd` requirement of the
// `IBackwardDifferentiable` conformance synthesized for `fwd_diff(fff)`, the primal half of its
// backward derivative. As with `bwd_diff(fwd_diff(fff))` in
// `nested-backward-diff-composed-error.slang`, a function that calls it cannot be differentiated.
// See https://github.com/shader-slang/slang/issues/13321.

[Differentiable]
float fff(float x)
{
    return x * x;
}

[Differentiable]
float ggg(float x)
{
    __apply(fwd_diff(fff))(diffPair(x, 1.0));
//CHECK:                  ^ cannot differentiate the result of a backward-derivative call
//CHECK:                  ^ the code produced by 'bwd_diff' is not itself differentiable
    return x * x;
}

[shader("fragment")]
float4 fs_main(float4 in: SV_Position) : SV_TARGET
{
    var p = diffPair(in.x, 1.0);
    bwd_diff(ggg)(p, 1.0);
    return float4(p.d, 0, 0, 0);
}
