//TEST_CATEGORY(wave, compute)
// Test WavePrefixMin and WavePrefixMax - exclusive prefix min/max operations
// These are equivalent to GLSL subgroupExclusiveMin/Max

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -capability cuda_sm_7_0 -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(8, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    // Test WavePrefixMin with increasing values
    // Lane 0 gets identity (max value), lane i gets min(val[0], ..., val[i-1])
    // Values: lane 0=10, lane 1=11, lane 2=12, ...
    // PrefixMin results: lane 0=max, lane 1=10, lane 2=10, lane 3=10, ...
    int valueMin = int(idx + 10);
    int prefixMinResult = WavePrefixMin(valueMin);
    
    // Test WavePrefixMax with increasing values
    // Lane 0 gets identity (min value), lane i gets max(val[0], ..., val[i-1])
    // Values: lane 0=10, lane 1=11, lane 2=12, ...
    // PrefixMax results: lane 0=min, lane 1=10, lane 2=11, lane 3=12, ...
    int valueMax = int(idx + 10);
    int prefixMaxResult = WavePrefixMax(valueMax);
    
    // For lane 0, the prefix operations return identity values which are implementation-defined
    // For lanes > 0, we can verify the results
    bool minOk = true;
    bool maxOk = true;
    
    if (idx > 0)
    {
        // PrefixMin should return 10 (the minimum of all previous lanes)
        minOk = (prefixMinResult == 10);
        // PrefixMax should return idx + 9 (the maximum of all previous lanes)
        maxOk = (prefixMaxResult == int(idx + 9));
    }
    
    // Test vector versions
    float2 vecValue = float2(float(idx + 5), float(20 - idx));
    float2 vecMinResult = WavePrefixMin(vecValue);
    float2 vecMaxResult = WavePrefixMax(vecValue);
    
    bool vecMinOk = true;
    bool vecMaxOk = true;
    if (idx > 0)
    {
        // For x component (increasing): prefixMin should be 5
        vecMinOk = (vecMinResult.x == 5.0f);
        // For y component (decreasing from 20): prefixMax should be 20
        vecMaxOk = (vecMaxResult.y == 20.0f);
    }
    
    uint result = (minOk ? 1 : 0) | (maxOk ? 2 : 0) | (vecMinOk ? 4 : 0) | (vecMaxOk ? 8 : 0);
    outputBuffer[idx] = result;
    
    // All lanes should have all bits set (0xF = 15)
    // CHECK-COUNT-8: F
}

