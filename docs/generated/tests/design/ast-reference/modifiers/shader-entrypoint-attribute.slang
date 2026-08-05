//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#stage-specific-entry-point-attributes
//META: doc_section_digest=9976dfbcc03a512351f63ab9251ef58411b90efdfa2f7539d1c820164c50911b
//META: purpose=The `[shader("compute")]` attribute marks a function as a compute entry point selectable without `-entry`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the stage string in the attribute is what makes the function
// discoverable as an entry point: the compile names neither `-entry` nor
// `-stage`, so nothing else identifies `csMain`. The CHECK-DAG lines assert the
// function and its compute workgroup attribute reach the emitted HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void csMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = 7;
}

//CHECK-DAG: numthreads
//CHECK-DAG: csMain
