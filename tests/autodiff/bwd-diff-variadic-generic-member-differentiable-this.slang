//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Regression test for differentiating a variadic generic member function with differentiable
// `this`.

interface IVariadic<each TArgs : IDifferentiable> : IDifferentiable
{
    [Differentiable]
    float operator()(expand each TArgs args);
}

void trigger<F : IVariadic<float>>(
    inout DifferentialPair<F> dFunction,
    inout DifferentialPair<float> dValue)
{
    bwd_diff(dFunction.p.operator())(dFunction, dValue, 1.0);
}

void triggerBinary<F : IVariadic<float, float>>(
    inout DifferentialPair<F> dFunction,
    inout DifferentialPair<float> dLeft,
    inout DifferentialPair<float> dRight)
{
    bwd_diff(dFunction.p.operator())(dFunction, dLeft, dRight, 1.0);
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
    var dFunction = diffPair(function);
    var dValue = diffPair(3.0);
    trigger(dFunction, dValue);

    outputBuffer[0] = dFunction.d.factor; // CHECK: 3.0
    outputBuffer[1] = dValue.d; // CHECK: 2.0

    Affine binaryFunction = {2.0};
    var dBinaryFunction = diffPair(binaryFunction);
    var dLeft = diffPair(3.0);
    var dRight = diffPair(4.0);
    triggerBinary(dBinaryFunction, dLeft, dRight);

    outputBuffer[2] = dBinaryFunction.d.factor; // CHECK: 3.0
    outputBuffer[3] = dLeft.d; // CHECK: 2.0
    outputBuffer[4] = dRight.d; // CHECK: 1.0
}
