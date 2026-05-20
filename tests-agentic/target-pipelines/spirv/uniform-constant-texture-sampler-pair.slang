//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=A separate Texture2D and SamplerState pair both lower to UniformConstant-storage variables (the SPIR-V mapping for opaque texture and sampler handles).
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the UniformConstant storage class. Both a separate
// Texture2D and a SamplerState lower to UniformConstant pointers,
// distinct from Uniform (constant buffers) and StorageBuffer
// (read/write structured buffers).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2D tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(0.5, 0.5), 0.0);
}

// CHECK-DAG: %tex = OpVariable %{{[a-zA-Z_0-9]+}} UniformConstant
// CHECK-DAG: %samp = OpVariable %{{[a-zA-Z_0-9]+}} UniformConstant
