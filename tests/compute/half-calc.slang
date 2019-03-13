//TEST(compute):COMPARE_COMPUTE:-dx12 -compute -use-dxil -profile cs_6_2
//TEST(compute):COMPARE_COMPUTE:-vk -compute -use-dxil -profile cs_6_2
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out

// Test for doing a calculation using half

RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    
    half v = tid;
    v *= half(1.0f / 3.0f);
    v += half(1.0f);
    
    outputBuffer[tid] = v;
}