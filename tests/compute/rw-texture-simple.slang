//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
// Doesn't work on DX11 currently - locks up on binding
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile cs_6_0 -use-dxil
// TODO(JS): Doesn't work on vk currently, because createTextureView not implemented on vk renderer
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute 

//TEST_INPUT: RWTexture1D(format=R_Float32, size=4, content = one):name rwt1D
RWTexture1D<float> rwt1D;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    float u = idx * (1.0f / 4);
    
    float val = 0.0f;
 
    val += rwt1D.Load(idx);
 
    outputBuffer[idx] = val;
}
