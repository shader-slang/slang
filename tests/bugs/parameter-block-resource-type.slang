//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry main -stage compute

// Test that ParameterBlock directly wrapping a resource type generates proper
// binding and group/set attributes.
//
// Regression test for:
//   https://github.com/shader-slang/slang/issues/8358 (WGSL)
//   https://github.com/shader-slang/slang/issues/6991 (SPIR-V)
//
// ParameterBlocks containing structs are flattened, so they do not have the same issue.

ParameterBlock<RWStructuredBuffer<int>> rwBuffer;
ParameterBlock<ConstantBuffer<int>> constBuffer;

// WGSL: @binding(0) @group(0) var<storage, read_write> rwBuffer
// WGSL: @binding(0) @group(1) var<uniform> constBuffer

// SPIRV-DAG: OpDecorate %rwBuffer{{.*}} Binding 0
// SPIRV-DAG: OpDecorate %rwBuffer{{.*}} DescriptorSet 0
// SPIRV-DAG: OpDecorate %constBuffer{{.*}} Binding 0
// SPIRV-DAG: OpDecorate %constBuffer{{.*}} DescriptorSet 1

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid: SV_DispatchThreadID) {
    rwBuffer[tid.x] = constBuffer; 
}
