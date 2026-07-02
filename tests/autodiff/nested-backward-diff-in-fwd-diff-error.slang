//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// The result of `bwd_diff` is not differentiable in *either* mode: forward-differentiating a
// function that calls `bwd_diff` must be rejected too (it previously produced an internal error).
// See https://github.com/shader-slang/slang/issues/10745.

[Differentiable]
float fff(float x, float y)
{
    return x + y;
}

[Differentiable]
float ggg(float x, float y)
{
    var xp = diffPair(x, 1.0);
    var yp = diffPair(y, 1.0);
    bwd_diff(fff)(xp, yp, 1.0);
//CHECK:         ^ cannot differentiate the result of a backward-derivative call
//CHECK:         ^ the code produced by 'bwd_diff' is not itself differentiable
    return fff(x, y) + xp.d + yp.d;
}

[shader("fragment")]
float4 fs_main(float4 pos: SV_Position) : SV_TARGET
{
    var g = fwd_diff(ggg)(diffPair(pos.x, 1.0), diffPair(pos.y, 0.0));
    return float4(g.d, 0, 0, 0);
}
