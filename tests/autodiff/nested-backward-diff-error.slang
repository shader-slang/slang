//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Nested `bwd_diff` (backward-differentiating a function that itself calls `bwd_diff`) used to
// crash the auto-diff passes with an internal error, because the result of `bwd_diff` is not
// itself differentiable. It must now be rejected with a human-readable diagnostic.
// See https://github.com/shader-slang/slang/issues/10745.

[Differentiable]
float fff(float x, float y)
{
    return x + y;
}

[Differentiable]
float ggg(float x, float y)
{
    var xp1 = diffPair(x, 1.0);
    var yp1 = diffPair(y, 1.0);
    bwd_diff(fff)(xp1, yp1, 1.0);
//CHECK:         ^ cannot differentiate the result of a backward-derivative call
//CHECK:         ^ the code produced by 'bwd_diff' is not itself differentiable
    return fff(x, y) + xp1.d + yp1.d;
}

[shader("fragment")]
float4 fs_main(float4 in: SV_Position) : SV_TARGET
{
    var xp2 = diffPair(in.x, 1.0);
    var yp2 = diffPair(in.y, 1.0);
    bwd_diff(ggg)(xp2, yp2, 1.0);
    return float4(xp2.d, yp2.d, 0, 0);
}
