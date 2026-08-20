//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -render-feature cooperative-vector -output-using-type -g0

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 3.000000

// Resource legalization leaves an empty differential context for `Consumer` after hoisting its
// resource field. Buffer-element lowering then copies between physical and logical versions of the
// empty struct. Empty-type legalization must remove that copy after both operands disappear.

CoopVec<half, 2> customTransform(
    no_diff RWStructuredBuffer<float> gradient,
    CoopVec<half, 2> input)
{
    return input;
}

[BackwardDerivativeOf(customTransform)]
void customTransformBackward(
    RWStructuredBuffer<float> gradient,
    inout DifferentialPair<CoopVec<half, 2>> input,
    CoopVec<half, 2> resultGradient)
{
    gradient[0] += float(resultGradient[0]) * float(input.p[0]);
    input = diffPair(input.p, resultGradient);
}

struct Identity
{
    [Differentiable]
    CoopVec<half, 2> eval(CoopVec<half, 2> input)
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
    CoopVec<half, 2> eval(CoopVec<half, 2> input)
    {
        return activation.eval(customTransform(gradient, input));
    }
}

struct Producer
{
    RWStructuredBuffer<float> gradient;
    uint resolution;
}

[BackwardDifferentiable]
CoopVec<half, 2> encode(no_diff Producer producer)
{
    CoopVec<half, 2> result;
    result[0] = half(producer.resolution);
    result[1] = half(producer.resolution + 1);
    return result;
}

[BackwardDerivativeOf(encode)]
void encodeBackward(Producer producer, CoopVec<half, 2> resultGradient)
{
    producer.gradient[0] += float(resultGradient[0]);
    producer.gradient[1] += float(resultGradient[1]);
}

struct Model
{
    Producer producer;
    Consumer consumer;
}

//TEST_INPUT: set model.producer.gradient = ubuffer(data=[0.0 0.0], stride=4)
//TEST_INPUT: set model.producer.resolution = 3
//TEST_INPUT: set model.consumer.gradient = ubuffer(data=[0.0], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0.0 0.0 0.0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
half objective(no_diff Model model, float scale)
{
    let encoded = encode(model.producer);
    let output = model.consumer.eval(encoded);
    return output[0] * half(scale);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform Model model)
{
    var scale = diffPair(1.0);
    bwd_diff(objective)(model, scale, half(1.0));
    outputBuffer[0] = model.producer.gradient[0];
    outputBuffer[1] = model.producer.gradient[1];
    outputBuffer[2] = model.consumer.gradient[0];
}
