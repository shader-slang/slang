//TEST_CATEGORY(wave, compute)
// Test WaveGetNumWaves - returns the number of waves (subgroups) in current workgroup
// Equivalent to GLSL gl_NumSubgroups

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_0 -shaderobj -render-feature hardware-device
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    // Get the number of waves in the workgroup
    uint numWaves = WaveGetNumWaves();
    
    // Get the wave size for reference
    uint waveSize = WaveGetLaneCount();
    
    // With 4 threads in the workgroup:
    // - If wave size >= 4: numWaves should be 1
    // - If wave size == 2: numWaves should be 2
    // - If wave size == 1: numWaves should be 4
    // The formula is: numWaves = ceil(totalThreads / waveSize)
    
    // We can't predict the exact value since it depends on hardware,
    // but we can verify that numWaves > 0 and is consistent with the wave size
    bool validNumWaves = (numWaves > 0);
    
    // Verify consistency: numWaves * waveSize >= 4 (total threads)
    // and (numWaves - 1) * waveSize < 4
    bool consistent = (numWaves * waveSize >= 4u);
    if (numWaves > 1)
    {
        consistent = consistent && ((numWaves - 1u) * waveSize < 4u);
    }
    
    // Output: 
    // - Bits 0-7: numWaves (for debugging)
    // - Bit 8: validNumWaves
    // - Bit 9: consistent
    uint result = (numWaves & 0xFFu) | (validNumWaves ? 0x100u : 0u) | (consistent ? 0x200u : 0u);
    outputBuffer[idx] = result;
    
    // All lanes should have bits 8 and 9 set (0x300 = 768 minimum, plus numWaves in lower bits)
    // We just check that the result is non-zero and has the validation bits set
    // CHECK-COUNT-4: {{[0-9A-Fa-f]*[3-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]}}
}

