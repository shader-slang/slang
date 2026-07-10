//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// A differentiable method is registered through a synthesized extension that copies the generic
// constraints of its enclosing type. The copied constraint uses fresh generic parameters, so
// matching the extension must treat the two declared witnesses as inference hints and leave the
// final proof to generic argument solving.

interface IModel<T>
{}

struct Model : IModel<float[2]>
{}

interface IArrayModel<TInput, TOutput>
{}

struct ReportShapeModel : IArrayModel<float[5], float[15]>
{}

struct DefaultShapeModel : IModel<float[7]>
{}

struct MixedShapeModel : IModel<float[17]>
{}

interface IFunction
{
    [BackwardDifferentiable]
    float forward(float input);
}

struct Wrapper<int N, T : IModel<float[1 + N]>> : IFunction
{
    [BackwardDifferentiable]
    float forward(float input)
    {
        return input * float(1 + N);
    }
}

// This matches the shape from the original report: the constraint contains multiple arithmetic
// expressions and one of them depends on a value parameter with an arithmetic default.
struct Sampler<
    int LatentCount,
    int LobeCount,
    int SpecularCount = LobeCount - 1,
    Network : IArrayModel<float[3 + LatentCount], float[3 + 6 * SpecularCount]>> : IFunction
{
    [BackwardDifferentiable]
    float forward(float input)
    {
        return input * float((3 + LatentCount) + (3 + 6 * SpecularCount));
    }
}

// Put the default last so the specialization below can omit it. This verifies that a dependent
// subtraction default can flow into a lifted constraint that also contains multiplication.
struct DefaultedWrapper<int Size, T, int TrimmedSize = Size - 1> : IFunction
    where T : IModel<float[2 * TrimmedSize + 1]>
{
    [BackwardDifferentiable]
    float forward(float input)
    {
        return input * float(2 * TrimmedSize + 1);
    }
}

// Division and remainder use BuiltinOperationIntVal rather than PolynomialIntVal. Parentheses and
// unary negation should also survive lifting and fold after specialization.
struct MixedArithmeticWrapper<
    int N,
    T : IModel<float[10 + (-N) + ((N * 6) / 2) + (N % 2)]>> : IFunction
{
    [BackwardDifferentiable]
    float forward(float input)
    {
        return input * float(10 + (-N) + ((N * 6) / 2) + (N % 2));
    }
}

[BackwardDifferentiable]
float evaluate(Wrapper<1, Model> wrapper, float input)
{
    return wrapper.forward(input);
}

[BackwardDifferentiable]
float evaluateSampler(Sampler<2, 3, 2, ReportShapeModel> sampler, float input)
{
    return sampler.forward(input);
}

[BackwardDifferentiable]
float evaluateDefaulted(DefaultedWrapper<4, DefaultShapeModel> wrapper, float input)
{
    return wrapper.forward(input);
}

[BackwardDifferentiable]
float evaluateMixed(MixedArithmeticWrapper<3, MixedShapeModel> wrapper, float input)
{
    return wrapper.forward(input);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Wrapper<1, Model> wrapper;
    outputBuffer[0] = evaluate(wrapper, 3.0f); // CHECK: 6.0

    var input = diffPair(3.0f);
    bwd_diff(evaluate)(wrapper, input, 1.0f);
    outputBuffer[1] = input.d; // CHECK: 2.0

    Sampler<2, 3, 2, ReportShapeModel> sampler;
    outputBuffer[2] = evaluateSampler(sampler, 2.0f); // CHECK: 40.0

    var samplerInput = diffPair(2.0f);
    bwd_diff(evaluateSampler)(sampler, samplerInput, 1.0f);
    outputBuffer[3] = samplerInput.d; // CHECK: 20.0

    DefaultedWrapper<4, DefaultShapeModel> defaultedWrapper;
    outputBuffer[4] = evaluateDefaulted(defaultedWrapper, 2.0f); // CHECK: 14.0

    var defaultedInput = diffPair(2.0f);
    bwd_diff(evaluateDefaulted)(defaultedWrapper, defaultedInput, 1.0f);
    outputBuffer[5] = defaultedInput.d; // CHECK: 7.0

    MixedArithmeticWrapper<3, MixedShapeModel> mixedWrapper;
    outputBuffer[6] = evaluateMixed(mixedWrapper, 2.0f); // CHECK: 34.0

    var mixedInput = diffPair(2.0f);
    bwd_diff(evaluateMixed)(mixedWrapper, mixedInput, 1.0f);
    outputBuffer[7] = mixedInput.d; // CHECK: 17.0
}
