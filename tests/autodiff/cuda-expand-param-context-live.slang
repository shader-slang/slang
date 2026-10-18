//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -validate-ir -g
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none -validate-ir -g
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NUMERIC): -cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NUMERIC): -cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NUMERIC): -metal -compute -shaderobj -output-using-type

// Exercise live nondifferentiable fields, nested call contexts, a differentiable
// struct, and a custom reverse derivative after parameter contexts are expanded.
// CUDA: static __device__ void s_bwdProp_cubic_{{[0-9]+}}(float
// CUDA-SAME: , float {{[[:alnum:]_]+}}, float * {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}})
// CUDA: static __device__ void s_bwdProp_inner_{{[0-9]+}}(Payload_{{[0-9]+}} *
// CUDA-SAME: , Coefficients_{{[0-9]+}} * {{[[:alnum:]_]+}}, float
// CUDA: static __device__ void s_bwdProp_objective_{{[0-9]+}}(Payload_{{[0-9]+}} *
// CUDA-SAME: , Coefficients_{{[0-9]+}} * {{[[:alnum:]_]+}}, Inputs_{{[0-9]+}} *
// CUDA: __global__ void computeMain(

// Metal must expose the same independent arguments, retaining thread address spaces
// when aggregate arguments become references later in lowering.
// METAL: void s_bwdProp_cubic_{{[0-9]+}}(float
// METAL-SAME: , float {{[[:alnum:]_]+}}, float thread* {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}})
// METAL: void s_bwdProp_inner_{{[0-9]+}}(const Payload_{{[0-9]+}} thread*
// METAL-SAME: , const Coefficients_{{[0-9]+}} thread* {{[[:alnum:]_]+}}, float
// METAL: void s_bwdProp_objective_{{[0-9]+}}(const Payload_{{[0-9]+}} thread*
// METAL-SAME: , const Coefficients_{{[0-9]+}} thread* {{[[:alnum:]_]+}}, const Inputs_{{[0-9]+}} thread*
// METAL: {{\[\[kernel\]\]}} void computeMain(

// Each case supplies x, y, scale, bias, and the reverse-mode seed at runtime.
//TEST_INPUT:ubuffer(data=[1.0 3.0 2.0 0.5 0.5 -1.0 -0.5 -2.0 -0.5 -2.0 0.0 2.0 3.0 -1.0 1.0 1.0 2.0 3.0 1.0 0.0], stride=4):name=testInputs
StructuredBuffer<float> testInputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Payload
{
    float unusedData[256];

    [Differentiable]
    [PreferRecompute]
    float square(float x)
    {
        return x * x;
    }
}

struct Coefficients
{
    float scale;
    float bias;
}

struct Inputs : IDifferentiable
{
    float x;
    float y;
}

[BackwardDerivative(cubicBackward)]
[PreferRecompute]
float cubic(float x, no_diff float scale)
{
    return scale * x * x * x;
}

void cubicBackward(inout DifferentialPair<float> x, float scale, float derivative)
{
    x = diffPair(x.p, 3.0f * scale * x.p * x.p * derivative);
}

[Differentiable]
[PreferRecompute]
void addBias(Coefficients coefficients, inout float value)
{
    value += coefficients.bias;
}

[Differentiable]
[PreferRecompute]
float inner(Payload payload, Coefficients coefficients, float x)
{
    float value = payload.square(x);
    addBias(coefficients, value);
    return value;
}

[Differentiable]
float objective(Payload payload, Coefficients coefficients, Inputs input)
{
    float a = inner(payload, coefficients, input.x);
    return cubic(a, coefficients.scale) + input.x * input.y + input.y * input.y;
}

[Differentiable]
void affineUpdate(Coefficients coefficients, inout float x, float y)
{
    x = coefficients.scale * x + y * y + coefficients.bias;
}

void checkParameterContext(float x, float y, float scale, float bias, float seed, out float output[9])
{
    Payload payload;
    Coefficients coefficients = {scale, bias};
    Inputs input = {x, y};
    var differentiatedInput = diffPair(input);
    bwd_diff(objective)(payload, coefficients, differentiatedInput, seed);
    output[0] = objective(payload, coefficients, input);
    output[1] = differentiatedInput.d.x;
    output[2] = differentiatedInput.d.y;
    output[3] = differentiatedInput.p.x;
    output[4] = differentiatedInput.p.y;

    var updatedX = diffPair(x, seed);
    var differentiatedY = diffPair(y);
    bwd_diff(affineUpdate)(coefficients, updatedX, differentiatedY);
    float forwardX = x;
    affineUpdate(coefficients, forwardX, y);
    output[5] = forwardX;
    output[6] = updatedX.d;
    output[7] = differentiatedY.d;
    // Reverse differentiation preserves the input primal of an inout argument.
    output[8] = updatedX.p;
}

// Let a = x*x + bias. The objective is scale*a^3 + x*y + y*y, so its seeded
// gradient is seed*(6*scale*x*a*a + y, x + 2*y). The affine update is
// scale*x + y*y + bias, with seeded gradient seed*(scale, 2*y).
// Each case checks both primals, both gradients, and preserved input primals.

// Positive seed and distinct, nonzero gradient components.
// NUMERIC: {{^}}18.750000{{$}}
// NUMERIC-NEXT: {{^}}15.000000{{$}}
// NUMERIC-NEXT: {{^}}3.500000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^}}11.500000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// Negative inputs, coefficients, and seed.
// NUMERIC-NEXT: {{^}}0.500000{{$}}
// NUMERIC-NEXT: {{^}}-5.000000{{$}}
// NUMERIC-NEXT: {{^}}4.000000{{$}}
// NUMERIC-NEXT: {{^}}-1.000000{{$}}
// NUMERIC-NEXT: {{^}}-0.500000{{$}}
// NUMERIC-NEXT: {{^}}1.750000{{$}}
// NUMERIC-NEXT: {{^}}4.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}-1.000000{{$}}
// Zero x still has a nonzero derivative through x*y.
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}4.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^}}4.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// A zero seed clears all derivatives without changing the primals.
// NUMERIC-NEXT: {{^}}30.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}8.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}1.000000{{$}}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint caseIndex = 0; caseIndex < 4; caseIndex++)
    {
        uint inputOffset = 5 * caseIndex;
        float values[9];
        checkParameterContext(
            testInputs[inputOffset],
            testInputs[inputOffset + 1],
            testInputs[inputOffset + 2],
            testInputs[inputOffset + 3],
            testInputs[inputOffset + 4],
            values);
        for (uint i = 0; i < 9; i++)
            outputBuffer[9 * caseIndex + i] = values[i];
    }
}
