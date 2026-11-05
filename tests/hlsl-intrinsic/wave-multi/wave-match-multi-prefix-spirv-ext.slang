// The partitioned subgroup operations (WaveMatch, WaveMaskMatch and the WaveMulti* reductions and
// scans) declare SPV_NV_shader_subgroup_partitioned by default and
// SPV_EXT_shader_subgroup_partitioned when the target requests spvGroupNonUniformPartitionedEXT.
// The two extensions define the same opcodes, operands and capability, and the disassembler prints
// the EXT spelling of each in both cases, so only the OpExtension line tells the runs apart.

//TEST:SIMPLE(filecheck=NV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=NV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly -capability spvGroupNonUniformPartitionedNV
//TEST:SIMPLE(filecheck=EXT): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly -capability spvGroupNonUniformPartitionedEXT
// Only the capability atom selects EXT. The extension atom alone keeps the NV declaration.
//TEST:SIMPLE(filecheck=NV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly -capability SPV_EXT_shader_subgroup_partitioned
// With -restrictive-capability-check, a target whose capabilities do not satisfy the intrinsics'
// requirements is an error (no implicit profile upgrade). These two runs check that
// spvGroupNonUniformPartitionedEXT can take the place of spvGroupNonUniformPartitionedNV in the
// intrinsics' requirements; the ballot and arithmetic capabilities are needed by both runs.
//TEST:SIMPLE(filecheck=NV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly -capability spvGroupNonUniformBallot -capability spvGroupNonUniformArithmetic -capability spvGroupNonUniformPartitionedNV -restrictive-capability-check
//TEST:SIMPLE(filecheck=EXT): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly -capability spvGroupNonUniformBallot -capability spvGroupNonUniformArithmetic -capability spvGroupNonUniformPartitionedEXT -restrictive-capability-check

// NV-NOT: SPV_EXT_shader_subgroup_partitioned
// NV: OpCapability GroupNonUniformPartitionedEXT
// NV-NOT: SPV_EXT_shader_subgroup_partitioned
// NV: OpExtension "SPV_NV_shader_subgroup_partitioned"
// NV-NOT: SPV_EXT_shader_subgroup_partitioned
// NV: OpGroupNonUniformPartitionEXT
// NV: OpGroupNonUniformIAdd {{.*}} PartitionedReduceEXT
// NV: OpGroupNonUniformIAdd {{.*}} PartitionedExclusiveScanEXT

// EXT-NOT: SPV_NV_shader_subgroup_partitioned
// EXT: OpCapability GroupNonUniformPartitionedEXT
// EXT-NOT: SPV_NV_shader_subgroup_partitioned
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
    outputBuffer[index] = partition + vectorPartition +
                          uint4(sum, prefix, WaveMaskMatch(0, index), WaveMaskMatch(0, float2(index, 1.0)));
}
