//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Test that NonUniform propagates through the GLSL image-atomic path
// (imageAtomicAdd -> __imageTexelPointer -> OpImageTexelPointer). This path
// differs from the HLSL InterlockedAdd(tex[i][coord], ...) path, which lowers
// through ImageSubscript. Here the wrapper must float onto the
// ImageTexelPointer and decorate the index, access chain, and texel pointer.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability StorageImageArrayNonUniformIndexing
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %images %[[IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: OpDecorate %[[IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[PTR:[A-Za-z0-9_]+]] = OpImageTexelPointer %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[PTR]] NonUniform

import glsl;

[[vk::binding(0, 0)]] RWTexture2D<uint> images[];

cbuffer Params { uint idx; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    imageAtomicAdd(images[NonUniformResourceIndex(i)], int2(0, 0), 1u);
}
