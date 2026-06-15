//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression: a combined Sampler2D array indexed with NonUniformResourceIndex
// must emit NonUniform on the consumed resource operand and the
// SampledImageArrayNonUniformIndexing capability. This exercises the path where
// the combined sampler enters the spirv_asm block as an IR operand (no
// intermediate OpSampledImage is constructed inside the block).

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing

// The access chain and loaded combined sampler must be NonUniform.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %samplers %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform

Sampler2D<float> samplers[8];
RWStructuredBuffer<float> outputBuffer;

cbuffer Params { uint idx; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    float v = samplers[NonUniformResourceIndex(i)].SampleLevel(float2(0.5, 0.5), 0.0);
    outputBuffer[0] = v;
}
