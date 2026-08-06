//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// A derivative method has an implicit receiver, while explicitly referencing its target as
// `Type::method` makes the differentiated callable expose that receiver as parameter zero.
// Overload resolution must use the derivative method's receiver before selecting by the declared
// parameters.

struct Scale
{
    float factor;

    [Differentiable]
    [NoDiffThis]
    float apply(float value)
    {
        return factor * value;
    }

    [Differentiable]
    static float apply(float left, float right)
    {
        return left + right;
    }

    [BackwardDerivativeOf(Scale::apply)]
    [NoDiffThis]
    void applyBackward(inout DifferentialPair<float> value, float resultGradient)
    {
        value = diffPair(value.p, factor * resultGradient);
    }

    [BackwardDerivativeOf(Scale::apply)]
    static void applyStaticBackward(
        inout DifferentialPair<float> left,
        inout DifferentialPair<float> right,
        float resultGradient)
    {
        left = diffPair(left.p, resultGradient);
        right = diffPair(right.p, resultGradient);
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Scale instance = {2.0};
    var value = diffPair(3.0);
    bwd_diff(instance.apply)(instance, value, 1.0);
    outputBuffer[0] = value.d;
    // CHECK: 2.0

    var left = diffPair(3.0);
    var right = diffPair(4.0);
    bwd_diff(instance.apply)(left, right, 1.0);
    outputBuffer[1] = left.d;
    outputBuffer[2] = right.d;
    // CHECK: 1.0
    // CHECK: 1.0
}
