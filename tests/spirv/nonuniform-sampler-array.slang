//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression: a separate SamplerState array indexed with NonUniformResourceIndex
// must emit SampledImageArrayNonUniformIndexing (Vulkan has no separate sampler
// capability; samplers are consumed via OpSampledImage). This exercises the
// IRSamplerStateTypeBase branch in requireNonUniformIndexingCapabilityForInst.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing

// The access chain into the sampler array and the loaded sampler must be NonUniform.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %samplers %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform

Texture2D<float> tex;
SamplerState samplers[8];
RWStructuredBuffer<float> outputBuffer;

cbuffer Params { uint idx; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    float v = tex.SampleLevel(samplers[NonUniformResourceIndex(i)], float2(0.5, 0.5), 0.0);
    outputBuffer[0] = v;
}
