//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -vk
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type -cuda

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
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
            return value * T(Scale);
        }
    }
}

[Differentiable]
T square<T : IDifferentiable & IFloat, let Scale : int>(ValueHolder<T, Scale> valueHolder)
    where T.Differential == T
{
    return valueHolder.Value * valueHolder.Value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ValueHolder<float, 2> valueHolder = {3.0};
    ValueHolder<float, 2>.Differential valueHolderDifferential = {1.0};
    let result = fwd_diff(square<float, 2>)(
        DifferentialPair<ValueHolder<float, 2>>(valueHolder, valueHolderDifferential));

    var valueHolderPair = diffPair(valueHolder);
    bwd_diff(square<float, 2>)(valueHolderPair, 1.0);

    outputBuffer[0] = result.p; // CHECK: 36.0
    outputBuffer[1] = result.d; // CHECK: 24.0
    outputBuffer[2] = valueHolderPair.d.value; // CHECK: 24.0
}
