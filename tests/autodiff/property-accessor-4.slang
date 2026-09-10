//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct ValueHolder<T : IDifferentiable & IFloat, let Scale : int> : IDifferentiable
    where T.Differential == T
{
    T value;

    property Value : T
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
ValueHolder<T, Scale> scaleAndStore<T : IDifferentiable & IFloat, let Scale : int>(
    ValueHolder<T, Scale> valueHolder,
    T factor)
    where T.Differential == T
{
    valueHolder.Value = valueHolder.Value * factor * T(Scale);
    return valueHolder;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ValueHolder<float, 2> valueHolder = {3.0};
    ValueHolder<float, 2>.Differential valueHolderDifferential = {1.0};
    // Forward mode differentiates the getter, multiplication, and setter together:
    // (3 + 1d) * (4 + 2d) * 2 = 24 + 20d.
    let result = fwd_diff(scaleAndStore<float, 2>)(
        DifferentialPair<ValueHolder<float, 2>>(valueHolder, valueHolderDifferential),
        diffPair(4.0, 2.0));

    // With an output differential of 5, reverse mode sends 4 * 2 * 5 to the stored
    // value and 3 * 2 * 5 to the factor that supplied the value written by the setter.
    var valueHolderPair = diffPair(valueHolder);
    var factorPair = diffPair(4.0);
    ValueHolder<float, 2>.Differential outputDifferential = {5.0};
    bwd_diff(scaleAndStore<float, 2>)(valueHolderPair, factorPair, outputDifferential);

    outputBuffer[0] = result.p.value; // CHECK: 24.0
    outputBuffer[1] = result.d.value; // CHECK: 20.0
    outputBuffer[2] = valueHolderPair.d.value; // CHECK: 40.0
    outputBuffer[3] = factorPair.d; // CHECK: 30.0
}
