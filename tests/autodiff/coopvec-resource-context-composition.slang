//DISABLE_TEST:SIMPLE: -target spirv -entry computeMain -stage compute

// Composing these independently valid custom reverse functions aborts in legalizeEmptyTypes with
// `unexpected: non-simple operand(s)!`. The unhandled instruction is kIROp_CopyLogical. Removing
// `Identity activation`, removing `Producer.resolution`, or replacing CoopVec with vector makes
// the test compile, so the failure requires the generated reverse context's complete shape.

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

[Differentiable]
half objective(no_diff Model model, float scale)
{
    let encoded = encode(model.producer);
    let output = model.consumer.eval(encoded);
    return output[0] * half(scale);
}

[shader("compute")]
[numthreads(1, 1, 1)]
[require(spvCooperativeVectorNV)]
void computeMain(uniform Model model)
{
    var scale = diffPair(1.0);
    bwd_diff(objective)(model, scale, half(1.0));
}
