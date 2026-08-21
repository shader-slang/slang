//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=5fdd0ac6daa767e15b8f1dad12d2d7a2846fe891f83f100e2466f794797eb465
//META: purpose=Negative: the HLSL-spelled `NonUniformResourceIndex(index)` cannot feed the Metal pipeline because its capability set excludes Metal, so the entry point is rejected with E36107 at check time.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the pass which floats non-uniform resource indices runs for
// Metal only in the sense that its target predicate allows it -- no Metal
// shader can reach it through the HLSL spelling, because the declaration's
// capability set names the other targets. The failure is a check-time
// entry-point capability rejection, not an emit-time one, so it attaches to
// the entry-point signature line rather than to the use; a second note
// pointing at the use itself is left unannotated.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target metal -entry main -stage compute

Texture2D<float4> texs[8];
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
//CHECK:     ^^^^ E36107
{
    outBuf[0] = texs[NonUniformResourceIndex(tid.x)].SampleLevel(samp, float2(0, 0), 0);
}
