//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// A property requirement declares its accessor without a body, so the
// differentiability attribute must survive interface-requirement synthesis:
// conformance checking rejects an implementation whose getter drops
// [Differentiable], and the property still lowers normally when read through a
// generic. Differentiating *through* an interface-declared property accessor is a
// separate, currently unsupported case tracked by #12232.
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

float readValue<T : IValueHolder>(T valueHolder)
{
    return valueHolder.Value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ValueHolder valueHolder = {3.0};
    outputBuffer[0] = readValue(valueHolder); // CHECK: 9.0

    ValueHolder other = {4.0};
    outputBuffer[1] = readValue(other); // CHECK: 16.0
}
