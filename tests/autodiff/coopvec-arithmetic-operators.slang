//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_10 -entry main -stage compute -DSCALAR=half
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_10 -entry main -stage compute -DSCALAR=float
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile sm_6_10 -entry main -stage compute -DSCALAR=half
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile sm_6_10 -entry main -stage compute -DSCALAR=float

// HLSL: void main(
// SPIRV: OpEntryPoint GLCompute

// Arithmetic operators dispatch through IDifferentiableArithmetic. They must use the
// original CoopVec methods and their custom derivatives, just like named method calls.
// In particular, autodiff must not try to differentiate the HLSL intrinsic bodies.
typealias V = CoopVec<SCALAR, 4>;

RWStructuredBuffer<float> outputBuffer;

[Differentiable]
V arithmetic(V x, V y)
{
    return (x * y + x - y) / (-y);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    let x = V(SCALAR(outputBuffer[0]));
    let y = V(SCALAR(outputBuffer[1]));
    let seed = V(SCALAR(1));
    let forward = fwd_diff(arithmetic)(diffPair(x, seed), diffPair(y, seed));
    outputBuffer[2] = float(forward.d[0]);

    var dx = diffPair(x);
    var dy = diffPair(y);
    bwd_diff(arithmetic)(dx, dy, seed);
    outputBuffer[3] = float(dx.d[0]);
    outputBuffer[4] = float(dy.d[0]);
}
