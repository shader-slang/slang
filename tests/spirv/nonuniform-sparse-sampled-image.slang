//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly
// Second pass uses a separate prefix so NODECOR-NOT scans the entire output
// independently of the positive CHECK patterns (FileCheck NOT-directives only
// apply between surrounding positive matches of the same prefix).
//TEST:SIMPLE(filecheck=NODECOR):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression test for NonUniform propagation through a compiler-generated
// sparse-sampling spirv_asm block.
//
// The sparse `SampleLevel` overload (with an `out uint status`) lowers to a
// spirv_asm block whose result type is a locally-declared %id:
//     %sparseResultType = OpTypeStruct ...
//     %r:%sparseResultType = OpImageSparseSampleExplicitLod %sampledImage ...
// The NonUniform propagation must decorate the resource operand consumed in
// the block (the OpSampledImage result), and must NOT mistake the result-type
// struct for the result <id> and decorate that instead (NonUniform on a type
// is invalid SPIR-V).

// The sampled image actually consumed by the sampling op must be NonUniform.
// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing
// CHECK-DAG: %[[SI:[A-Za-z0-9_]+]] = OpSampledImage
// CHECK-DAG: OpDecorate %[[SI]] NonUniform

// The sparse result-type struct is a type, not a value, so it must never be
// decorated NonUniform.
// NODECOR-NOT: OpDecorate %sparseResultType NonUniform

Texture2D<float> textures[8];
SamplerState samp;
RWTexture2D<float4> outputTex;

cbuffer Params { uint idx; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    uint status;
    float v =
        textures[NonUniformResourceIndex(i)].SampleLevel(samp, float2(0.0, 0.0), 0.0, int2(0, 0), status);
    outputTex[uint2(0, 0)] = float4(v, 0.0, 0.0, 0.0);
}
