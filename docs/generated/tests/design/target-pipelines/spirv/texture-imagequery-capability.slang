//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=adbc9f62b2b56295371ba0ea74f7f7385925be80ae5d4b3005d64a60b8337c56
//META: purpose=Using OpImageQuerySize* triggers the ImageQuery SPIR-V capability, emitted only when a size-query is actually present.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: capability gating boundary — a Texture2D that is
// only sampled (no GetDimensions / size query) should NOT emit
// OpCapability ImageQuery. Adding a GetDimensions call does.
// This test pins the positive side.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2D<float4> tex;
RWStructuredBuffer<uint2> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint w, h;
    tex.GetDimensions(w, h);
    outBuf[tid.x] = uint2(w, h);
}

// CHECK: OpCapability ImageQuery
