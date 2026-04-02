//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -g0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -g0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -g0
//TEST:SIMPLE(filecheck=CHK):-target glsl -stage compute -entry computeMain -report-checkpoint-intermediates -g0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// This test verifies that recomputation occurs even if a function is accessed through multiple call sites, potentially
// with different form (i.e. it may be untranslated at one call-site, and translated by the time it is encountered at another call-site).
//

typedef DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;

public enum TonemapMode 
{
    Reinhard,
    Karis,
};

[Differentiable]
public float3 tonemap_forward(float3 x, TonemapMode mode, float max_val = 1.f)
{
    float3 tmp_x = max(x, float3(0.0f));
    float3 colour;

    switch (mode) {
        case TonemapMode.Reinhard:
            colour = tmp_x * rcp(1.f + tmp_x);
            break;
        case TonemapMode.Karis:
            colour = tmp_x * rcp(1.f + max(max(tmp_x.r, tmp_x.g), tmp_x.b));
            break;
        default:
            colour = tmp_x;
    }

    return clamp(colour, 0.0, max_val);
}


[BackwardDifferentiable]
float g(float x)
{
    return tonemap_forward(x, TonemapMode::Karis).r;
}

[BackwardDifferentiable]
float f(int p, float x)
{
    float y = g(x);
    if (p == 0)
        // Test that complex functions get recomputed
        y = tonemap_forward(x, TonemapMode::Reinhard).r;

    return y * y;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    dpfloat dpa = dpfloat(0.5, 0.0);

    bwd_diff(g)(dpa, 1.0f);
    outputBuffer[0] = dpa.d; // CHECK: 0.444444

    dpfloat dpb = dpfloat(0.5, 0.0);

    bwd_diff(f)(0, dpb, 1.0f);
    outputBuffer[1] = dpb.d; // CHECK: 0.296296
}

// CHK: note: no checkpoint contexts to report