//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile cs_6_3 -use-dxil
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute 

//TEST_INPUT: Texture1D(size=4, content = one):name t1D
Texture1D<float> t1D;
//TEST_INPUT: Texture2D(size=4, content = one):name t2D
Texture2D<float> t2D;

//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    float u = idx * (1.0f / 4);
    
    float val = 0.0f;
    val += t1D.Sample(samplerState, u);
    val += t2D.Sample(samplerState, float2(u, u));
 
    outputBuffer[idx] = val;
}
