//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#shape-of-the-emitted-file
//META: doc_section_digest=f7d5cbd1c3620eef32d32fe0c7e4e9df28ec40d641f947026a4c21d50fd6e988
//META: purpose=Both axes of the spliced `Gather<compare><component>` intrinsic family survive HLSL emit under their composed names — the compare axis as `GatherCmpRed` on a `SamplerComparisonState` and the component axis as `GatherGreen` on a `SamplerState`.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `Gather*` method spellings are carried through
// emit as declared rather than being normalised to a single
// `Gather` plus arguments. The bundle already pins `GatherRed`,
// which fixes neither axis of the splice; this file moves one axis
// at a time off that baseline — `GatherCmpRed` changes only the
// compare half, `GatherGreen` only the component half — so a
// regression that dropped either half would leave one CHECK
// unmatched.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float4> tex;
SamplerComparisonState sampCmp;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.GatherCmpRed(sampCmp, float2(0.5, 0.5), 0.25);
    outBuf[tid.x + 1] = tex.GatherGreen(samp, float2(0.5, 0.5));
}

// CHECK-DAG: {{^}}SamplerComparisonState sampCmp_{{[0-9]+}} : register(s{{[0-9]+}});
// CHECK-DAG: GatherCmpRed(sampCmp_{{[0-9]+}},
// CHECK-DAG: GatherGreen(samp_{{[0-9]+}},
