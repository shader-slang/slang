//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute

// Regression test for https://github.com/shader-slang/slang/issues/10520
//
// When a ConstantBuffer<T> is a field inside a struct wrapped by ParameterBlock,
// the ConstantBuffer must still receive Binding and DescriptorSet decorations.
// Tests both scalar (uint) and struct element types.

struct MyData {
    float4 value;
    uint count;
}

struct Resources {
    ConstantBuffer<uint> constantBuf;
    ConstantBuffer<MyData> structBuf;
    RWStructuredBuffer<uint> writeBuf;
}

ParameterBlock<Resources> resources;

// SPIRV-DAG: OpDecorate %resources_constantBuf{{.*}} Binding 0
// SPIRV-DAG: OpDecorate %resources_constantBuf{{.*}} DescriptorSet 0
// SPIRV-DAG: OpDecorate %resources_structBuf{{.*}} Binding 1
// SPIRV-DAG: OpDecorate %resources_structBuf{{.*}} DescriptorSet 0
// SPIRV-DAG: OpDecorate %resources_writeBuf{{.*}} Binding 2
// SPIRV-DAG: OpDecorate %resources_writeBuf{{.*}} DescriptorSet 0

// WGSL-DAG: @binding(0) @group(0) var<uniform> resources_constantBuf
// WGSL-DAG: @binding(1) @group(0) var<uniform> resources_structBuf
// WGSL-DAG: @binding(2) @group(0) var<storage, read_write> resources_writeBuf

// HLSL-DAG: register(b0)
// HLSL-DAG: register(b1)
// HLSL-DAG: register(u0)

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    resources.writeBuf[0] = resources.constantBuf + resources.structBuf.count;
}
