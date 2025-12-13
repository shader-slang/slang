//TEST_CATEGORY(wave, compute)
// Test WaveGetLaneEqMask, WaveGetLaneGeMask, WaveGetLaneGtMask, WaveGetLaneLeMask, WaveGetLaneLtMask
// These are equivalent to GLSL gl_SubgroupEqMask, gl_SubgroupGeMask, etc.

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_0 -shaderobj -render-feature hardware-device
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(8, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    uint laneIndex = WaveGetLaneIndex();
    
    // Get all the lane masks
    uint4 eqMask = WaveGetLaneEqMask();
    uint4 geMask = WaveGetLaneGeMask();
    uint4 gtMask = WaveGetLaneGtMask();
    uint4 leMask = WaveGetLaneLeMask();
    uint4 ltMask = WaveGetLaneLtMask();
    
    // For simplicity, we'll only check the first component of the masks
    // since our workgroup size is 8, all masks fit in the first uint
    
    // Expected values for lane with index laneIndex:
    // EqMask: only bit 'laneIndex' is set: 1 << laneIndex
    uint expectedEq = 1u << laneIndex;
    
    // GeMask: bits laneIndex and above are set: ~0 << laneIndex
    uint expectedGe = ~0u << laneIndex;
    
    // GtMask: bits above laneIndex are set: ~0 << (laneIndex + 1), or 0 if laneIndex >= 31
    uint expectedGt = (laneIndex >= 31u) ? 0u : (~0u << (laneIndex + 1u));
    
    // LeMask: bits laneIndex and below are set: (1 << (laneIndex + 1)) - 1, or ~0 if laneIndex >= 31
    uint expectedLe = (laneIndex >= 31u) ? ~0u : ((1u << (laneIndex + 1u)) - 1u);
    
    // LtMask: bits below laneIndex are set: (1 << laneIndex) - 1
    uint expectedLt = (1u << laneIndex) - 1u;
    
    // Verify each mask
    bool eqOk = (eqMask.x == expectedEq);
    bool geOk = (geMask.x == expectedGe);
    bool gtOk = (gtMask.x == expectedGt);
    bool leOk = (leMask.x == expectedLe);
    bool ltOk = (ltMask.x == expectedLt);
    
    // Check that components y, z, w are 0 for our small workgroup
    bool otherComponentsOk = (eqMask.y == 0u) && (eqMask.z == 0u) && (eqMask.w == 0u);
    
    // Combine results
    uint result = 0u;
    result |= eqOk ? 1u : 0u;
    result |= geOk ? 2u : 0u;
    result |= gtOk ? 4u : 0u;
    result |= leOk ? 8u : 0u;
    result |= ltOk ? 0x10u : 0u;
    result |= otherComponentsOk ? 0x20u : 0u;
    
    outputBuffer[idx] = result;
    
    // All lanes should have all bits set (0x3F = 63)
    // CHECK-COUNT-8: 3F
}

