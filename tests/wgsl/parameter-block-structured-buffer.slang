//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry main -stage compute

// Test that ParameterBlock directly wrapping a resource type generates proper
// @binding and @group attributes in WGSL.
//
// This is a regression test for https://github.com/shader-slang/slang/issues/8358
// ParameterBlocks containing structs are flattened, so they do not have the same issue.

ParameterBlock<RWStructuredBuffer<int>> buffer;

// CHECK: @binding(0) @group(0) var<storage, read_write> buffer

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) { 
    buffer[tid.x] = 1; 
}
