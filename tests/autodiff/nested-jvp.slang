//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[__custom_jvp(pow_jvp)]
float pow_(float x, float n)
{
    return pow<float>(x, n);
}


[__custom_jvp(max_jvp)]
float max_(float x, float y)
{
    return max<float>(x, y);
}


float pow_jvp(float x, float n, float dx, float dn)
{
    return dx * n * pow(x, n-1) + dn * pow(x, n) * log(x);
}


float max_jvp(float x, float y, float dx, float dy)
{
    return (x > y) ? dx : dy;
}


/* Fresnel Schlick example */
__differentiate_jvp float3 fresnel(float3 f0, float3 f90, float cosTheta)
{
    return f0 + (f90 - f0) * pow_(max_(1 - cosTheta, 0.0), 5);
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
     {
        float3 f0 = float3(0.2, 0.2, 0.2);
        float3 f90 = float3(0.7, 0.7, 0.7);
        float cosTheta = 0.5;

        float3 d_f0 = float3(0.1, 0.1, 0.1);
        float3 d_f90 = float3(0.9, 0.9, 0.9);
        float d_cosTheta = 1.0;

        outputBuffer[0] = __jvp(fresnel)(f0, f90, cosTheta, d_f0, d_f90, d_cosTheta).y; // Expect: -0.031250
    }
}
