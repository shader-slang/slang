//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct ValueHolder : IDifferentiable
{
    float value;

    property Value : float
    {
        [TreatAsDifferentiable]
        get
        {
            return value * value;
        }
    }
}

[Differentiable]
float square(float x)
{
    ValueHolder valueHolder = {x};
    return valueHolder.Value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let result = fwd_diff(square)(diffPair(3.0, 1.0));

    outputBuffer[0] = result.p; // CHECK: 9.0

    // [TreatAsDifferentiable] makes the getter callable from differentiable code
    // while supplying a zero derivative, so the propagated derivative is 0 rather
    // than the 6 that differentiating `value * value` would give.
    outputBuffer[1] = result.d; // CHECK: 0.0
}
