//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression: a combined Sampler2DShadow array indexed with
// NonUniformResourceIndex, exercising the OpImageSampleDrefExplicitLod path
// from the combined-sampler spirv_asm blocks.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing

// The access chain and loaded combined shadow sampler must be NonUniform.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %shadowSamplers %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform

Sampler2DShadow shadowSamplers[8];
RWStructuredBuffer<float> outputBuffer;

cbuffer Params { uint idx; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    float v = shadowSamplers[NonUniformResourceIndex(i)].SampleCmpLevelZero(float2(0.5, 0.5), 0.5);
    outputBuffer[0] = v;
}
