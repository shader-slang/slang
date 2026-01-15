//TEST_CATEGORY(wave-mask, compute)
// Test WaveMaskPrefixMin and WaveMaskPrefixMax - masked exclusive prefix min/max operations

// CUDA supports these via WaveMultiPrefixExclusiveMin/Max
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0 -shaderobj

// Vulkan/SPIRV with VK_NV_shader_subgroup_partitioned
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(8, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    // Use a mask that includes all 8 lanes
    WaveMask mask = 0xFF;
    
    // Test WaveMaskPrefixMin with increasing values
    // Values: lane 0=10, lane 1=11, lane 2=12, ...
    // PrefixMin: lane 0=identity, lane i=min(10, 11, ..., i+9) = 10 for i>0
    int minValue = int(idx + 10);
    int prefixMinResult = WaveMaskPrefixMin(mask, minValue);
    
    // Test WaveMaskPrefixMax with increasing values
    // Values: lane 0=10, lane 1=11, lane 2=12, ...
    // PrefixMax: lane 0=identity, lane i=max(10, 11, ..., i+9) = i+9 for i>0
    int maxValue = int(idx + 10);
    int prefixMaxResult = WaveMaskPrefixMax(mask, maxValue);
    
    bool minOk = true;
    bool maxOk = true;
    
    if (idx > 0)
    {
        // PrefixMin should return 10 (minimum of all previous lanes)
        minOk = (prefixMinResult == 10);
        // PrefixMax should return idx + 9 (maximum of all previous lanes)
        maxOk = (prefixMaxResult == int(idx + 9));
    }
    
    // Test with a partial mask (only even lanes)
    WaveMask evenMask = 0x55; // bits 0, 2, 4, 6
    int evenMinValue = int(idx * 2 + 5); // 5, 7, 9, 11, 13, 15, 17, 19
    int evenPrefixMinResult = WaveMaskPrefixMin(evenMask, evenMinValue);
    
    bool evenMinOk = true;
    // For lanes in the mask (0, 2, 4, 6), check the result
    if ((idx == 2) || (idx == 4) || (idx == 6))
    {
        // Even lanes after lane 0 should get the min of previous even lanes
        // Lane 2: min of lane 0's value = 5
        // Lane 4: min of lane 0, 2's values = min(5, 9) = 5
        // Lane 6: min of lane 0, 2, 4's values = min(5, 9, 13) = 5
        evenMinOk = (evenPrefixMinResult == 5);
    }
    
    // Test vector version
    float2 vecValue = float2(float(idx + 5), float(20 - idx));
    float2 vecMinResult = WaveMaskPrefixMin(mask, vecValue);
    float2 vecMaxResult = WaveMaskPrefixMax(mask, vecValue);
    
    bool vecOk = true;
    if (idx > 0)
    {
        // For x component (increasing from 5): prefixMin should be 5
        // For y component (decreasing from 20): prefixMax should be 20
        vecOk = (vecMinResult.x == 5.0f) && (vecMaxResult.y == 20.0f);
    }
    
    uint result = (minOk ? 1u : 0u) | (maxOk ? 2u : 0u) | (evenMinOk ? 4u : 0u) | (vecOk ? 8u : 0u);
    outputBuffer[idx] = result;
    
    // All lanes should have all bits set (0xF = 15)
    // CHECK-COUNT-8: F
}

