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

ParameterBlock<RWStructuredBuffer<int>> buffer;

// WGSL: @binding(0) @group(0) var<storage, read_write> buffer

// SPIRV-DAG: OpDecorate %buffer{{.*}} Binding 0
// SPIRV-DAG: OpDecorate %buffer{{.*}} DescriptorSet 0

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) { 
    buffer[tid.x] = 1; 
}
