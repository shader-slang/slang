//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -validate-ir -g
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NUMERIC): -cuda -compute -shaderobj -output-using-type -Xslang -DTEST_COMPUTE=1

// Capturing a large immutable input must preserve its value identity through
// rematerialization. Both automatic derivatives and custom derivative adapters
// should forward the input instead of materializing a context-field copy.
// Every Settings occurrence in generated helper bodies should be a pointer,
// not a by-value context field or temporary. Check the whole generated module.
// CUDA: struct Settings_{{[0-9]+}}
// CUDA-NOT: Settings_{{[0-9]+}} {{[[:alnum:]_]+}}
// CUDA: __device__ float automaticGradient(Settings_{{[0-9]+}} *
// CUDA-NOT: Settings_{{[0-9]+}} {{[[:alnum:]_]+}}
// CUDA: __device__ float customGradient(Settings_{{[0-9]+}} *
// CUDA-NOT: Settings_{{[0-9]+}} {{[[:alnum:]_]+}}
struct Settings
{
    float weights[256];
    uint index;
}

[Differentiable]
[PreferRecompute]
float square(Settings settings, float x)
{
    return settings.weights[settings.index] * x * x;
}

[Differentiable]
float automaticObjective(Settings settings, float x)
{
    return sin(square(settings, x));
}

[BackwardDerivative(cubeBackward)]
[PreferRecompute]
float cube(Settings settings, float x)
{
    return settings.weights[settings.index] * x * x * x;
}

void cubeBackward(Settings settings, inout DifferentialPair<float> x, float seed)
{
    x = diffPair(x.p, 3.0f * settings.weights[settings.index] * x.p * x.p * seed);
}

[Differentiable]
float customObjective(Settings settings, float x)
{
    return sin(cube(settings, x));
}

[CudaDeviceExport]
float automaticGradient(__constref Settings settings, float x)
{
    var pair = diffPair(x);
    bwd_diff(automaticObjective)(settings, pair, 1.0f);
    return pair.d;
}

[CudaDeviceExport]
float customGradient(__constref Settings settings, float x)
{
    var pair = diffPair(x);
    bwd_diff(customObjective)(settings, pair, 1.0f);
    return pair.d;
}

#if TEST_COMPUTE
// Each case supplies a weight, x, and the two analytic gradients at runtime.
// The objectives are sin(w*x*x) and sin(w*x*x*x), respectively.
//TEST_INPUT:ubuffer(data=[2.0 1.0 -1.66458735 -2.49688102 -1.0 2.0 2.61457448 1.74600041 3.0 0.0 0.0 0.0 0.0 -2.0 0.0 0.0], stride=4):name=inputs
StructuredBuffer<float> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<float> output;

// NUMERIC: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint i = 0; i < 4; i++)
    {
        Settings settings = {};
        settings.index = 17;
        settings.weights[settings.index] = inputs[4 * i];
        float x = inputs[4 * i + 1];
        output[2 * i] =
            abs(automaticGradient(settings, x) - inputs[4 * i + 2]) < 1e-5f ? 1.0f : 0.0f;
        output[2 * i + 1] =
            abs(customGradient(settings, x) - inputs[4 * i + 3]) < 1e-5f ? 1.0f : 0.0f;
    }
}
#endif
