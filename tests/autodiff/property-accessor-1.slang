//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct ValueHolder : IDifferentiable
{
    float value;

    property Value : float
    {
        [Differentiable]
        get
        {
            return value;
        }
    }
}

[Differentiable]
float square(ValueHolder valueHolder)
{
    return valueHolder.Value * valueHolder.Value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ValueHolder valueHolder = {3.0};
    ValueHolder.Differential valueHolderDifferential = {1.0};
    let result = fwd_diff(square)(
        DifferentialPair<ValueHolder>(valueHolder, valueHolderDifferential));

    var valueHolderPair = diffPair(valueHolder);
    bwd_diff(square)(valueHolderPair, 1.0);

    outputBuffer[0] = result.p; // CHECK: 9.0
    outputBuffer[1] = result.d; // CHECK: 6.0
    outputBuffer[2] = valueHolderPair.d.value; // CHECK: 6.0
}
