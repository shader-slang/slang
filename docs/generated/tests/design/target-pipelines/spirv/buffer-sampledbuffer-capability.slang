//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Read-only Buffer<float4> triggers the SampledBuffer SPIR-V capability, distinct from RWBuffer which triggers ImageBuffer.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: capability bookkeeping for texel-buffer types.
// SampledBuffer is required by Buffer<T>; ImageBuffer by RWBuffer<T>.
// Pin the read-only side here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Buffer<float4> buf;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = buf[tid.x];
}

// CHECK: OpCapability SampledBuffer
