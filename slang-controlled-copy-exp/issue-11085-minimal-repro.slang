RWStructuredBuffer<float> outputBuffer;

struct BigVec : IDifferentiable
{
    float data[128];
}

[BackwardDerivative(innerBwd)]
[Differentiable]
[ForceInline]
BigVec inner(BigVec input)
{
    BigVec result;
    [ForceUnroll]
    for (int i = 0; i < 128; i++)
        result.data[i] = input.data[i] * input.data[i];
    return result;
}

[ForceInline]
void innerBwd(
    inout DifferentialPair<BigVec> input,
    BigVec.Differential dResult)
{
    BigVec dInput;

    [ForceUnroll]
    for (int i = 0; i < 128; i++)
        dInput.data[i] = 2.0f * input.p.data[i] * dResult.data[i];

    input = DifferentialPair<BigVec>(input.p, dInput);
}

[Differentiable]
[ForceInline]
BigVec outer(BigVec input)
{
    return inner(input);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    BigVec input;
    BigVec dOutput;
    [ForceUnroll]
    for (int i = 0; i < 128; i++)
    {
        input.data[i] = float(i + 1);
        dOutput.data[i] = 1.0f;
    }

    var inputPair = diffPair(input);
    bwd_diff(outer)(inputPair, dOutput);

    outputBuffer[0] = inputPair.d.data[0];
    outputBuffer[1] = inputPair.d.data[127];
}
