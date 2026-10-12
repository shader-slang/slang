//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type -g0

// CHECK: type: float
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 3.000000

// Resource legalization hoists `Consumer.gradient`, so the retained `Consumer` context contains
// only the empty `Identity` value. Empty-type legalization replaces that value with `Void`; the
// late void cleanup must remove the placeholder before SPIR-V emission.

float customTransform(no_diff RWStructuredBuffer<float> gradient, float input)
{
    return input;
}

[BackwardDerivativeOf(customTransform)]
void customTransformBackward(
    RWStructuredBuffer<float> gradient,
    inout DifferentialPair<float> input,
    float resultGradient)
{
    gradient[0] += resultGradient * input.p;
    input = diffPair(input.p, resultGradient);
}

struct Identity
{
    [Differentiable]
    float eval(float input)
    {
        return input;
    }
}

struct Consumer
{
    RWStructuredBuffer<float> gradient;
    Identity activation;

    [Differentiable]
    [NoDiffThis]
    float eval(float input)
    {
        return activation.eval(customTransform(gradient, input));
    }
}

struct Model
{
    uint value;
    Consumer consumer;
}

//TEST_INPUT: set model.value = 3
//TEST_INPUT: set model.consumer.gradient = ubuffer(data=[0.0], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0.0 0.0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float objective(no_diff Model model, float scale)
{
    return model.consumer.eval(float(model.value)) * scale;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform Model model)
{
    var scale = diffPair(1.0);
    bwd_diff(objective)(model, scale, 1.0);
    outputBuffer[0] = model.consumer.gradient[0];
    outputBuffer[1] = scale.d;
}
