//TEST:COMPARE_COMPUTE:-cpu -output-using-type -xslang -experimental-feature

// Inferred targets also work with IDifferentiable alone and a vector specialization.
// Leave id without [Differentiable] so the caller must use the registered extensions.
T id<T : IDifferentiable>(T x)
{
    return x;
}

__func_extension<T : IDifferentiable>
fwd_diff(id)(DifferentialPair<T> dpx) -> DifferentialPair<T>
{
    return dpx;
}

__func_extension<T : IDifferentiable>
bwd_diff(id)(inout DifferentialPair<T> dpx, T.Differential dOut) -> void
{
    dpx = diffPair(dpx.p, dOut);
}

// Differentiating a caller checks that the synthesized conformances reach autodiff.
[Differentiable]
float2 scaledId(float2 x)
{
    return 3.0 * id<float2>(x);
}

//TEST_INPUT:ubuffer(data=[4.0 -2.0 2.0 -3.0 0.5 2.0], stride=4):name=inputBuffer
StructuredBuffer<float> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let x = float2(inputBuffer[0], inputBuffer[1]);
    let dIn = float2(inputBuffer[2], inputBuffer[3]);
    let result = fwd_diff(scaledId)(diffPair(x, dIn));
    outputBuffer[0] = result.p.x;
    outputBuffer[1] = result.p.y;
    outputBuffer[2] = result.d.x;
    outputBuffer[3] = result.d.y;

    var dpx = diffPair(x, float2(0.0));
    bwd_diff(scaledId)(dpx, float2(inputBuffer[4], inputBuffer[5]));
    outputBuffer[4] = dpx.d.x;
    outputBuffer[5] = dpx.d.y;
}
