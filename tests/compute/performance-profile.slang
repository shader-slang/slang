//TEST(compute):PERFORMANCE_PROFILE:-cpu -compute -compile-arg -O3 -compute-dispatch 1024,1,1
//TEST(compute):PERFORMANCE_PROFILE:-slang -compute -compute-dispatch 1024,1,1
//TEST(compute):PERFORMANCE_PROFILE:-slang -compute -dx12 -compute-dispatch 1024,1,1
//TEST(compute, vulkan):PERFORMANCE_PROFILE:-vk -compute -compute-dispatch 1024,1,1

//TEST_INPUT:ubuffer(random(float, 4096, -1, 1), stride=4):dxbinding(0),glbinding(0),out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    float v = outputBuffer[i];
    v = v < 0.0f ? (v * v) : (v + v + v);
	outputBuffer[i] = v;
}