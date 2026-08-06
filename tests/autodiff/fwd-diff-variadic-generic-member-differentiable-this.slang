//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Regression test for forward-differentiating a variadic generic member function. A specialized
// type pack must contribute one parameter per element to the forward derivative function.

interface IVariadic<each TArgs : IDifferentiable> : IDifferentiable
{
    [Differentiable]
    float operator()(expand each TArgs args);
}

float trigger<F : IVariadic<float>>(
    DifferentialPair<F> dFunction,
    DifferentialPair<float> dValue)
{
    return fwd_diff(dFunction.p.operator())(dFunction, dValue).d;
}

float triggerBinary<F : IVariadic<float, float>>(
    DifferentialPair<F> dFunction,
    DifferentialPair<float> dLeft,
    DifferentialPair<float> dRight)
{
    return fwd_diff(dFunction.p.operator())(dFunction, dLeft, dRight).d;
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
    outputBuffer[0] = trigger(
        diffPair(function, Scale.Differential(1.0)),
        diffPair(3.0, 1.0));
    // CHECK: 5.0

    Affine binaryFunction = {2.0};
    outputBuffer[1] = triggerBinary(
        diffPair(binaryFunction, Affine.Differential(1.0)),
        diffPair(3.0, 1.0),
        diffPair(4.0, 1.0));
    // CHECK: 6.0
}
