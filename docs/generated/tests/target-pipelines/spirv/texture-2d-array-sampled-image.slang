//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture2DArray emits OpTypeImage with dim 2D and the Arrayed=1 flag — a fourth axis of the SPIR-V image-type table.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: Texture2DArray emits OpTypeImage %float 2D <depth> 1
// <ms> <sampled> <fmt>. The 4th operand (Arrayed) is the
// distinguishing bit from a plain Texture2D.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2DArray<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float3(0.5, 0.5, 0), 0.0);
}

// CHECK: OpTypeImage %float 2D 2 1
