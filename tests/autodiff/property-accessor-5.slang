//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cpu

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IValueHolder : IDifferentiable
{
    property Value : float
    {
        [Differentiable]
        get;
    }
}

struct ValueHolder : IValueHolder
{
    float value;

    property Value : float
    {
        [Differentiable]
        get
        {
            return value * value;
        }
    }
}

[Differentiable]
float readValue<T : IValueHolder>(T valueHolder)
{
    return valueHolder.Value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ValueHolder valueHolder = {3.0};
    ValueHolder.Differential valueHolderDifferential = {1.0};
    let result = fwd_diff(readValue<ValueHolder>)(
        DifferentialPair<ValueHolder>(valueHolder, valueHolderDifferential));

    var valueHolderPair = diffPair(valueHolder);
    bwd_diff(readValue<ValueHolder>)(valueHolderPair, 5.0);

    outputBuffer[0] = result.p; // CHECK: 9.0
    outputBuffer[1] = result.d; // CHECK: 6.0
    outputBuffer[2] = valueHolderPair.d.value; // CHECK: 30.0
}
