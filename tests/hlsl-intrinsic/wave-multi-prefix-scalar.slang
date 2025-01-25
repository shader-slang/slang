//TEST_CATEGORY(wave, compute)
//DISABLE_TEST:COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//DISABLE_TEST:COMPARE_COMPUTE_EX:-slang -compute -shaderobj

//TEST:COMPARE_COMPUTE_EX:-slang -compute -dx12 -use-dxil -profile sm_6_5 -shaderobj
// We cannot test via GLSL as our glslang does not emit the required SPIRV version.
//TEST:COMPARE_COMPUTE_EX:-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX:-cuda -compute -render-features cuda_sm_7_0 -shaderobj

//TEST:SIMPLE(filecheck=CHECK_SPIRV): -stage compute -entry computeMain -target spirv
//TEST:SIMPLE(filecheck=CHECK_GLSL): -stage compute -entry computeMain -target glsl

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// CHECK_SPIRV: OpCapability GroupNonUniformPartitionedNV
// CHECK_SPIRV: OpExtension "SPV_NV_shader_subgroup_partitioned"

// CHECK_GLSL: GL_NV_shader_subgroup_partitioned

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = int(dispatchThreadID.x);

    // Split into two groups.
    uint4 mask = 0b00001111;
    if (index >=  4)
    {
        mask = 0b11110000;
    }

    //
    // WaveMultiPrefixSum.
    // Results in hex: [0 1 3 7], [0 10 30 70]
    //
    uint sumValue = WaveMultiPrefixSum(1 << index, mask);
    const uint sumBaseIndex = 0;
    outputBuffer[sumBaseIndex + index] = sumValue;
    // CHECK_GLSL: subgroupPartitionedExclusiveAddNV

    //
    // WaveMultiPrefixProduct.
    // Results in hex: [1 1 2 8], [1 10 200 8000]
    //
    uint productValue = WaveMultiPrefixProduct(1 << index, mask);
    const uint productBaseIndex = 8;
    outputBuffer[productBaseIndex + index] = productValue;
    // CHECK_GLSL: subgroupPartitionedExclusiveMulNV

    //
    // WaveMultiPrefixBitAnd.
    // This prefix operation starts with all bits set.
    // Results in hex: [FFFFFFFF 1 1 1], [FFFFFFFF F F F]
    //
    uint andBits = 0b1;
    if (index >= 4)
    {
        andBits = 0b1111;
    }
    uint andValue = WaveMultiPrefixBitAnd(andBits, mask);
    const uint andBaseIndex = 16;
    outputBuffer[andBaseIndex + index] = andValue;
    // CHECK_GLSL: subgroupPartitionedExclusiveAndNV

    //
    // WaveMultiPrefixBitOr.
    // Results in hex: [0 1 3 7], [0 10 30 70]
    //
    uint orValue = WaveMultiPrefixBitOr(1 << index, mask);
    const uint orBaseIndex = 24;
    outputBuffer[orBaseIndex + index] = orValue;
    // CHECK_GLSL: subgroupPartitionedExclusiveOrNV

    //
    // WaveMultiPrefixBitXor.
    // Results in hex: [0 1 3 7], [0 F 0 F]
    //
    uint xorBits = (1 << index);
    if (index >= 4)
    {
        xorBits = 0b1111;
    }
    uint xorValue = WaveMultiPrefixBitXor(xorBits, mask);
    const uint xorBaseIndex = 32;
    outputBuffer[xorBaseIndex + index] = xorValue;
    // CHECK_GLSL: subgroupPartitionedExclusiveXorNV
}
