//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set primalBuffer = ubuffer(data=[3.0], stride=4)
//TEST_INPUT: set differentialBuffer = ubuffer(data=[0.0], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
StructuredBuffer<float> primalBuffer;
RWStructuredBuffer<float> differentialBuffer;
RWStructuredBuffer<uint> outputBuffer;

struct DifferentiableBuffer
{
    StructuredBuffer<float> primal;
    RWStructuredBuffer<float> differential;

    [BackwardDifferentiable]
    [NoDiffThis]
    float load(uint index)
    {
        return primal[index];
    }

    [BackwardDerivativeOf(load)]
    [NoDiffThis]
    void loadBackward(uint index, float differentialValue)
    {
        differential[index] += differentialValue;
    }
}

[Differentiable]
float apply<TFunc : IDifferentiableFuncWithDiffThis<float, float>>(TFunc function, float value)
{
    return function(value);
}

[Differentiable]
float evaluate(no_diff float scale, float value)
{
    return apply((float innerValue)~ => scale * innerValue * innerValue, value);
}

[Differentiable]
float evaluateBuffer(
    no_diff DifferentiableBuffer values,
    no_diff uint index,
    float value)
{
    return apply((float innerValue)~ => values.load(index) * innerValue, value);
}

void requireDifferentiableMutating<TFunc : IDifferentiableMutatingFunc<float, float>>(
    inout TFunc function)
{}

[numthreads(1, 1, 1)]
void computeMain()
{
    var differentiableMutating = (float value)!~ => value;
    requireDifferentiableMutating(differentiableMutating);

    // CHECK: 12
    outputBuffer[0] = uint(fwd_diff(evaluate)(3.0, diffPair(2.0, 1.0)).d);

    var value = diffPair(2.0);
    bwd_diff(evaluate)(3.0, value, 1.0);

    // CHECK: 12
    outputBuffer[1] = uint(value.d);
    // CHECK: 12
    outputBuffer[2] = uint(evaluate(3.0, 2.0));

    DifferentiableBuffer values = {primalBuffer, differentialBuffer};
    var bufferValue = diffPair(2.0);
    bwd_diff(evaluateBuffer)(values, 0, bufferValue, 1.0);

    // CHECK: 6
    outputBuffer[3] = uint(evaluateBuffer(values, 0, 2.0));
    // CHECK: 3
    outputBuffer[4] = uint(bufferValue.d);
    // CHECK: 2
    outputBuffer[5] = uint(differentialBuffer[0]);
}
