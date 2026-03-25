//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Test that NonUniform decorations propagate through to storage image writes
// and that StorageImageArrayNonUniformIndexing capability is emitted.

// CHECK-DAG: OpCapability StorageImageArrayNonUniformIndexing

// The index, access chain, and loaded image (consumed by OpImageWrite) must
// all be decorated NonUniform per Vulkan VUID-RuntimeSpirv-None-10148.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %images %[[IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: OpDecorate %[[IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform

RWTexture2D<float4> images[8];

cbuffer Params { uint idx; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    images[NonUniformResourceIndex(i)][uint2(0, 0)] = float4(1, 0, 0, 1);
}
