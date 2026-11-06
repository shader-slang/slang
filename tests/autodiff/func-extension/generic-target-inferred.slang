//TEST:COMPARE_COMPUTE:-cpu -output-using-type -xslang -experimental-feature

// Differentiability on the primal allows target inference to match DifferentialPair<T>.
// The separate IComparable constraint must also survive specialization of the target.
T id<T : IDifferentiable & IComparable>(T x)
{
    return x;
}

__func_extension<T : IDifferentiable & IComparable>
fwd_diff(id)(DifferentialPair<T> dpx) -> DifferentialPair<T>
{
    return dpx;
}

__func_extension<T : IDifferentiable & IComparable>
bwd_diff(id)(inout DifferentialPair<T> dpx, T.Differential dOut) -> void
{
    dpx = diffPair(dpx.p, dOut);
}

//TEST_INPUT:ubuffer(data=[7.0 -2.0 1.5 0.5], stride=4):name=inputBuffer
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
