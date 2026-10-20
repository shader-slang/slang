//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry rayGen -stage raygeneration -line-directive-mode none -validate-ir

// OptiX entry points use the same CUDA lowering pipeline as compute kernels.
// Keep the captured aggregate separate from the scalar in the propagation helper.
// CUDA: static __device__ void s_bwdProp_evaluate_{{[0-9]+}}(Coefficients_{{[0-9]+}} *
// CUDA-SAME: , float {{[[:alnum:]_]+}}, float * {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}})
// CUDA: __raygen__rayGen()

struct Coefficients
{
    float unused[16];
}

[Differentiable]
[PreferRecompute]
float square(Coefficients coefficients, float x)
{
    return x * x;
}

[Differentiable]
float evaluate(Coefficients coefficients, float x)
{
    return sin(square(coefficients, x));
}

[shader("raygeneration")]
void rayGen(RWStructuredBuffer<float> output, uniform Coefficients coefficients, uniform float x)
{
    var differentiated = diffPair(x);
    bwd_diff(evaluate)(coefficients, differentiated, 1.0f);
    output[0] = differentiated.d;
}
