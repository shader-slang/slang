//TEST(compute):COMPARE_COMPUTE:-cpu -compute 
//TEST(compute):COMPARE_COMPUTE:-cuda -compute 
//TEST(compute, nvvm):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute, nvvm):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer : register(u0);

//TEST_INPUT: Texture2D(size=4, content=one):name texture
Texture2D<float> texture;

//TEST_INPUT: Sampler:name sampler
SamplerState sampler;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int tid = int(dispatchThreadID.x);
    float u = tid * (1.0f / 3.0f);
    float v = 1.0f - u;
    float2 uv = float2(u, v);

    outputBuffer[tid] = texture.Sample(sampler, uv);
}
