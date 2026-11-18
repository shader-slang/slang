//TEST:COMPARE_COMPUTE:-cpu -output-using-type -xslang -experimental-feature

// The primal does not establish differentiability. Explicitly specializing the target lets
// the extension supply its stronger constraints without inferring T from DifferentialPair<T>.
T id<T : IComparable>(T x)
{
    return x;
}

__func_extension<T : IDifferentiable & IComparable>
fwd_diff(id<T>)(DifferentialPair<T> dpx) -> DifferentialPair<T>
{
    return dpx;
}

__func_extension<T : IDifferentiable & IComparable>
bwd_diff(id<T>)(inout DifferentialPair<T> dpx, T.Differential dOut) -> void
{
    dpx = diffPair(dpx.p, dOut);
}

//TEST_INPUT:ubuffer(data=[3.0 2.0 -4.0 -3.0], stride=4):name=inputBuffer
StructuredBuffer<float> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let result = fwd_diff(id<float>)(diffPair(inputBuffer[0], inputBuffer[1]));
    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;

    var dpx = diffPair(inputBuffer[2], 0.0);
    bwd_diff(id<float>)(dpx, inputBuffer[3]);
    outputBuffer[2] = dpx.p;
    outputBuffer[3] = dpx.d;
}
