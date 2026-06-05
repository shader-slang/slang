//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Test that NonUniform propagates through the image-atomic path.
// The index and access chain must be decorated NonUniform.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability StorageImageArrayNonUniformIndexing
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %images %[[IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: OpDecorate %[[IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[AC]] NonUniform

RWTexture2D<uint> images[8];

cbuffer Params { uint idx; };

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    InterlockedAdd(images[NonUniformResourceIndex(i)][uint2(0, 0)], 1);
}
