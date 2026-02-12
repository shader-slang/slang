// atomic-reduce-float.slang
// Tests for atomic reduction intrinsics with floating-point types (float, double).
// Separated from atomic-reduce-intrinsics.slang because HLSL does not support
// floating-point atomics. Only tested on CUDA and Vulkan.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-directly -compute -shaderobj -output-using-type -render-feature hardware-device

// Float add reduction target
//TEST_INPUT:ubuffer(data=[0.0], stride=4):name=floatAddTarget
RWStructuredBuffer<float> floatAddTarget;

// Double add reduction target
//TEST_INPUT:ubuffer(data=[0.0 0.0], stride=8):name=doubleAddTarget
RWStructuredBuffer<double> doubleAddTarget;

//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const int NUM_THREADS = 64;

[numthreads(NUM_THREADS, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    // === Test __atomic_reduce_add with float ===
    // Each thread adds 0.5f, result should be 32.0f
    __atomic_reduce_add(floatAddTarget[0], 0.5f);

    // === Test __atomic_reduce_add with double ===
    // Each thread adds 0.25, result should be 16.0
    __atomic_reduce_add(doubleAddTarget[0], 0.25);

    AllMemoryBarrierWithGroupSync();

    // Only thread 0 writes results
    if (tid == 0)
    {
        outputBuffer[0] = int(floatAddTarget[0]);     // Expected: 32 (from 32.0f)
        outputBuffer[1] = int(doubleAddTarget[0]);    // Expected: 16 (from 16.0)
    }
}

// BUF: {{^32$}}
// BUF-NEXT: 16
