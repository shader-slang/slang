//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=babb40e44ba0eb7addfafeddc37185c8c3bbed4c27b3b7fc8a2469d95b33aca1
//META: purpose=Boundary - WaveSize(128) at the large-lane edge produces waveSize(128 : Int) unchanged.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upper end of the accepted lane counts. 128 is the largest
// power-of-two wave size the attribute accepts; the CHECK pins the exact
// operand so a clamp to a smaller hardware wave size would fail here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
[WaveSize(128)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [waveSize(128 : Int)]
// CHECK: func %main
