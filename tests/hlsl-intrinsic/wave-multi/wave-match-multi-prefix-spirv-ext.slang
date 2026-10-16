// The partitioned subgroup operations (WaveMatch, WaveMaskMatch and the WaveMulti* reductions and
// scans) declare SPV_NV_shader_subgroup_partitioned by default and
// SPV_EXT_shader_subgroup_partitioned when the target requests spvGroupNonUniformPartitionedEXT.
// The two extensions define the same opcodes, operands and capability.

//TEST:SIMPLE(filecheck=NV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=NV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly -capability spvGroupNonUniformPartitionedNV
//TEST:SIMPLE(filecheck=EXT): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly -capability spvGroupNonUniformPartitionedEXT

// NV: OpCapability GroupNonUniformPartitionedEXT
// NV: OpExtension "SPV_NV_shader_subgroup_partitioned"
// NV-NOT: SPV_EXT_shader_subgroup_partitioned
// NV: OpGroupNonUniformPartitionEXT
// NV: OpGroupNonUniformIAdd {{.*}} PartitionedReduceEXT
// NV: OpGroupNonUniformIAdd {{.*}} PartitionedExclusiveScanEXT

// EXT: OpCapability GroupNonUniformPartitionedEXT
// EXT: OpExtension "SPV_EXT_shader_subgroup_partitioned"
// EXT-NOT: SPV_NV_shader_subgroup_partitioned
// EXT: OpGroupNonUniformPartitionEXT
// EXT: OpGroupNonUniformIAdd {{.*}} PartitionedReduceEXT
// EXT: OpGroupNonUniformIAdd {{.*}} PartitionedExclusiveScanEXT

RWStructuredBuffer<uint4> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint index = dispatchThreadID.x;
    const uint4 partition = WaveMatch(index & 3);
    const uint sum = WaveMultiSum(index, partition);
    const uint prefix = WaveMultiPrefixSum(index, partition);
    const uint4 vectorPartition = WaveMatch(float2(index & 1, 1.0));
    outputBuffer[index] = partition + vectorPartition + uint4(sum, prefix, WaveMaskMatch(0, index), 0);
}
