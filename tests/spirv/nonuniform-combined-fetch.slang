//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Test that NonUniform propagates to OpImage when extracting the image
// from a non-uniformly indexed combined sampler array for OpImageFetch.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %combinedSamplers %[[IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: OpDecorate %[[IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform
// CHECK-DAG: %[[IMG:[A-Za-z0-9_]+]] = OpImage %{{[A-Za-z0-9_]+}} %[[LOAD]]
// CHECK-DAG: OpDecorate %[[IMG]] NonUniform

Sampler2D combinedSamplers[8];
RWTexture2D<float4> output;

cbuffer Params { uint idx; };

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    float4 v = combinedSamplers[NonUniformResourceIndex(i)].Load(int3(0, 0, 0));
    output[uint2(0, 0)] = v;
}
