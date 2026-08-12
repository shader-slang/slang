//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=babb40e44ba0eb7addfafeddc37185c8c3bbed4c27b3b7fc8a2469d95b33aca1
//META: purpose=Boundary - WaveSize(4) at the small-lane edge produces waveSize(4 : Int) in the dump.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower end of the accepted lane counts. The attribute
// requires a power of two and 4 is the smallest value it accepts, so this
// pins the bottom of the range whose top the `WaveSize(128)` sibling
// pins.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
[WaveSize(4)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [waveSize(4 : Int)]
// CHECK: func %main
