//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
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

        [Differentiable]
        set
        {
            value = newValue;
        }
    }
}

[Differentiable]
ValueHolder scaleAndStore(ValueHolder valueHolder, float factor)
{
    valueHolder.Value = valueHolder.Value * factor;
    return valueHolder;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ValueHolder valueHolder = {3.0};
    ValueHolder.Differential valueHolderDifferential = {1.0};
    // Forward mode differentiates the getter, multiplication, and setter together:
    // (3 + 1d) * (4 + 2d) = 12 + 10d.
    let result = fwd_diff(scaleAndStore)(
        DifferentialPair<ValueHolder>(valueHolder, valueHolderDifferential),
        diffPair(4.0, 2.0));

    // With an output differential of 5, reverse mode sends 4 * 5 to the original stored
    // value and 3 * 5 to the factor that supplied the value written through the setter.
    var valueHolderPair = diffPair(valueHolder);
    var factorPair = diffPair(4.0);
    ValueHolder.Differential outputDifferential = {5.0};
    bwd_diff(scaleAndStore)(valueHolderPair, factorPair, outputDifferential);

    outputBuffer[0] = result.p.value; // CHECK: 12.0
    outputBuffer[1] = result.d.value; // CHECK: 10.0
    outputBuffer[2] = valueHolderPair.d.value; // CHECK: 20.0
    outputBuffer[3] = factorPair.d; // CHECK: 15.0
}
