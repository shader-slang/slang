//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Regression test for forward-differentiating a variadic generic member function. A specialized
// type pack must contribute one parameter per element to the forward derivative function.

interface IVariadic<each TArgs : IDifferentiable> : IDifferentiable
{
    [Differentiable]
    float operator()(expand each TArgs args);
}

void trigger<F : IVariadic<float>>(F function, float value)
{
    let result = fwd_diff(function.operator())(
        diffPair(function),
        diffPair(value, 1.0));

    outputBuffer[0] = result.d; // CHECK: 2.0
}

void triggerBinary<F : IVariadic<float, float>>(F function, float left, float right)
{
    let result = fwd_diff(function.operator())(
        diffPair(function),
        diffPair(left, 1.0),
        diffPair(right, 1.0));

    outputBuffer[1] = result.d; // CHECK: 3.0
}

struct Scale : IVariadic<float>
{
    float factor;

    [Differentiable]
    float operator()(float value)
    {
        return factor * value;
    }
}

struct Affine : IVariadic<float, float>
{
    float factor;

    [Differentiable]
    float operator()(float left, float right)
    {
        return factor * left + right;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Scale function = {2.0};
    trigger(function, 3.0);

    Affine binaryFunction = {2.0};
    triggerBinary(binaryFunction, 3.0, 4.0);
}
