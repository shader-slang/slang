//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Buffer<float4> (texel buffer) lowers to OpTypeImage with dim Buffer; the read dispatches via OpImageFetch and requires the SampledBuffer capability.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: Buffer<T> is a texel buffer, not a StructuredBuffer.
// SPIR-V models it as an OpTypeImage with dim=Buffer, sampled=1,
// and requires the SampledBuffer capability. Reads are OpImageFetch
// (no sampler involved).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Buffer<float4> buf;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = buf[tid.x];
}

// CHECK-DAG: OpCapability SampledBuffer
// CHECK-DAG: OpTypeImage %float Buffer
// CHECK-DAG: OpImageFetch
