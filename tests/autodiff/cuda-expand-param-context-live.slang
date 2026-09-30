//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -validate-ir -g

// Exercise live nondifferentiable fields, nested call contexts, a differentiable
// struct, and a custom reverse derivative after parameter contexts are expanded.
// CUDA: static __device__ void s_bwdProp_cubic_{{[0-9]+}}(float
// CUDA-SAME: , float {{[[:alnum:]_]+}}, float * {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}})
// CUDA: static __device__ void s_bwdProp_inner_{{[0-9]+}}(Payload_{{[0-9]+}} *
// CUDA-SAME: , Coefficients_{{[0-9]+}} * {{[[:alnum:]_]+}}, float
// CUDA: static __device__ void s_bwdProp_objective_{{[0-9]+}}(Payload_{{[0-9]+}} *
// CUDA-SAME: , Coefficients_{{[0-9]+}} * {{[[:alnum:]_]+}}, Inputs_{{[0-9]+}} *
// CUDA: __global__ void checkParameterContext(

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

[CudaKernel]
void checkParameterContext(float x, float y, float scale, float bias, float seed, float* output)
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
