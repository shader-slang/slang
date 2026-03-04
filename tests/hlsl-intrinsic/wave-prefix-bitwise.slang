//TEST_CATEGORY(wave, compute)
// Test WavePrefixBitAnd, WavePrefixBitOr, WavePrefixBitXor - exclusive prefix bitwise operations
// These are equivalent to GLSL subgroupExclusiveAnd/Or/Xor
//
// Note: DX12/HLSL requires SM 6.5 because the implementation uses WaveMultiPrefixBit* intrinsics

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_5 -shaderobj -render-feature hardware-device
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
    
    // Test WavePrefixBitAnd
    // Each lane has value 0xFF (all bits set)
    // PrefixBitAnd: lane 0 gets identity (~0), lane i gets AND of all previous values
    // Since all values are 0xFF, all lanes > 0 should get 0xFF
    uint andValue = 0xFFu;
    uint prefixAndResult = WavePrefixBitAnd(andValue);
    
    // Test WavePrefixBitOr
    // Each lane has bit 'idx' set: lane 0=0x1, lane 1=0x2, lane 2=0x4, ...
    // PrefixBitOr: lane 0 gets identity (0), lane i gets OR of bits 0..i-1
    // lane 0: 0, lane 1: 0x1, lane 2: 0x3, lane 3: 0x7, ...
    uint orValue = 1u << idx;
    uint prefixOrResult = WavePrefixBitOr(orValue);
    
    // Test WavePrefixBitXor
    // Each lane has value 0x1
    // PrefixBitXor: lane 0 gets identity (0), lane i gets XOR of all previous values
    // XOR of odd count of 1s = 1, even count = 0
    // lane 0: 0, lane 1: 1, lane 2: 0, lane 3: 1, ...
    uint xorValue = 1u;
    uint prefixXorResult = WavePrefixBitXor(xorValue);
    
    bool andOk = true;
    bool orOk = true;
    bool xorOk = true;
    
    if (idx == 0)
    {
        // Lane 0 gets identity values
        // AND identity is ~0 (all bits set)
        andOk = (prefixAndResult == 0xFFFFFFFFu);
        // OR identity is 0
        orOk = (prefixOrResult == 0u);
        // XOR identity is 0
        xorOk = (prefixXorResult == 0u);
    }
    else
    {
        // AND: all previous lanes have 0xFF, so result should be 0xFF
        andOk = (prefixAndResult == 0xFFu);
        // OR: should be (1 << idx) - 1 (all bits below idx set)
        orOk = (prefixOrResult == (1u << idx) - 1u);
        // XOR: should be idx % 2 (alternating 0, 1)
        xorOk = (prefixXorResult == (idx % 2u));
    }
    
    // Test vector versions
    uint2 vecOrValue = uint2(1u << idx, 1u << (7u - idx));
    uint2 vecOrResult = WavePrefixBitOr(vecOrValue);
    
    bool vecOrOk = true;
    if (idx == 0)
    {
        vecOrOk = (vecOrResult.x == 0u) && (vecOrResult.y == 0u);
    }
    else
    {
        vecOrOk = (vecOrResult.x == (1u << idx) - 1u);
    }
    
    uint result = (andOk ? 1u : 0u) | (orOk ? 2u : 0u) | (xorOk ? 4u : 0u) | (vecOrOk ? 8u : 0u);
    outputBuffer[idx] = result;
    
    // All lanes should have all bits set (0xF = 15)
    // CHECK-COUNT-8: F
}

