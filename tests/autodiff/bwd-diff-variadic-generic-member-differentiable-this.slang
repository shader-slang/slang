//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Regression test for differentiating a variadic generic member function with differentiable
// `this`.

interface IVariadic<each TArgs : IDifferentiable> : IDifferentiable
{
    [Differentiable]
    float operator()(expand each TArgs args);
}

void trigger<F : IVariadic<float>>(F function, float value)
{
    var dFunction = diffPair(function);
    var dValue = diffPair(value);

    bwd_diff(dFunction.p.operator())(dFunction, dValue, 1.0);

    outputBuffer[0] = dValue.d; // CHECK: 2.0
}

void triggerBinary<F : IVariadic<float, float>>(F function, float left, float right)
{
    var dFunction = diffPair(function);
    var dLeft = diffPair(left);
    var dRight = diffPair(right);

    bwd_diff(dFunction.p.operator())(dFunction, dLeft, dRight, 1.0);

    outputBuffer[1] = dLeft.d; // CHECK: 2.0
    outputBuffer[2] = dRight.d; // CHECK: 1.0
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
