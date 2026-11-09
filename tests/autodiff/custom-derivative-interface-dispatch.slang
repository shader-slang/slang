//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// The generic call to IAccessor.load computes the ordinary scale derivative but does not invoke
// BufferAccessor.loadBackward. Changing evaluate's first parameter from generic `T : IAccessor`
// to concrete `BufferAccessor` produces the expected parameter gradient.

//TEST_INPUT: set primal = ubuffer(data=[3.0], stride=4)
//TEST_INPUT: set gradient = ubuffer(data=[0.0], stride=4)
//TEST_INPUT: set output = out ubuffer(data=[0 0], stride=4)
StructuredBuffer<float> primal;
RWStructuredBuffer<float> gradient;
RWStructuredBuffer<uint> output;

interface IAccessor
{
    [BackwardDifferentiable]
    [NoDiffThis]
    float load(uint index);
}

struct BufferAccessor : IAccessor
{
    StructuredBuffer<float> primal;
    RWStructuredBuffer<float> gradient;

    [BackwardDifferentiable]
    [NoDiffThis]
    float load(uint index)
    {
        return primal[index];
    }

    [BackwardDerivativeOf(load)]
    [NoDiffThis]
    void loadBackward(uint index, float resultGradient)
    {
        gradient[index] += resultGradient;
    }
}

[Differentiable]
float evaluate<T : IAccessor>(no_diff T accessor, float scale)
{
    return accessor.load(0) * scale;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    BufferAccessor accessor = {primal, gradient};
    var scale = diffPair(2.0);
    bwd_diff(evaluate)(accessor, scale, 1.0);

    // CHECK: type: uint32_t
    // CHECK-NEXT: 3
    output[0] = uint(scale.d);
    // CHECK-NEXT: 2
    output[1] = uint(gradient[0]);
}
