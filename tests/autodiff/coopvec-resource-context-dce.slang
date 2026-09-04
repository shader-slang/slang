//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -render-feature cooperative-vector -output-using-type -g0

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 3.000000

// Reverse-mode AD temporarily rematerializes `Consumer.eval` while differentiating the CoopVec
// subscript in `objective`. That rematerialization is unused and read-none, so DCE must recognize
// its CoopVec argument as a value and remove the call before resource and buffer legalization.

CoopVec<half, 1> customTransform(
    no_diff RWStructuredBuffer<float> gradient,
    CoopVec<half, 1> input)
{
    return input;
}

[BackwardDerivativeOf(customTransform)]
void customTransformBackward(
    RWStructuredBuffer<float> gradient,
    inout DifferentialPair<CoopVec<half, 1>> input,
    CoopVec<half, 1> resultGradient)
{
    gradient[0] += float(resultGradient[0]);
    input = diffPair(input.p, resultGradient);
}

struct Identity
{
    [Differentiable]
    CoopVec<half, 1> eval(CoopVec<half, 1> input)
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
    CoopVec<half, 1> eval(CoopVec<half, 1> input)
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
half objective(no_diff Model model, float scale)
{
    CoopVec<half, 1> input;
    input[0] = half(model.value);
    return model.consumer.eval(input)[0] * half(scale);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform Model model)
{
    var scale = diffPair(1.0);
    bwd_diff(objective)(model, scale, half(1.0));
    outputBuffer[0] = model.consumer.gradient[0];
    outputBuffer[1] = scale.d;
}
