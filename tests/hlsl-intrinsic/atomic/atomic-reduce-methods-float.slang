// atomic-reduce-methods-float.slang
// Tests for Atomic<float>.reduceAdd method.
// Separated from atomic-reduce-methods.slang because HLSL does not support
// floating-point atomics. Only tested on CUDA and Vulkan.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-directly -compute -shaderobj -output-using-type -render-feature hardware-device

//TEST_INPUT:ubuffer(data=[0], stride=4):name=floatAddTarget
RWStructuredBuffer<Atomic<float> > floatAddTarget;

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const int NUM_THREADS = 64;

[numthreads(NUM_THREADS, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    // === Test Atomic<float>.reduceAdd ===
    // 64 threads each add 0.5f -> result 32.0f
    floatAddTarget[0].reduceAdd(0.5f);

    AllMemoryBarrierWithGroupSync();

    if (tid == 0)
    {
        outputBuffer[0] = int(floatAddTarget[0].load());   // Expected: 32 (from 32.0f)
    }
}

// BUF: {{^32$}}
