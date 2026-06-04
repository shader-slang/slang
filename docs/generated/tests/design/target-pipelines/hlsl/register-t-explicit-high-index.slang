//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=Upper-edge boundary: a high explicit register(t63) for a Texture2D is preserved literally on HLSL emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: a numerically high SRV slot — the emitter must
// not clamp or renumber large user-chosen `t` indices.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D tex : register(t63);
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(0.5, 0.5), 0);
}

// CHECK: Texture2D{{.*}} tex_{{[0-9]+}} : register(t63)
