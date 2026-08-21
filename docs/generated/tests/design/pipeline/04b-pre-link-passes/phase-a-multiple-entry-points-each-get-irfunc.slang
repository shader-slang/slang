//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission
//META: doc_section_digest=66b9b0ad1e435e5d64e2372e5eedf6745238e43195f17a70dd348156a613b908
//META: purpose=Phase A iterates lowerFrontEndEntryPointToIR (A6) over every entry point of the translation unit, so a second [shader("compute")] entry also survives to the LOWER-TO-IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that A6 is a per-element iteration over the translation unit's
// entry-point list rather than a single-entry step. The module declares two
// compute entry points but only `main` is requested on the command line;
// both `func %other` and `func %main` still appear in the post-Phase-D
// snapshot, which is only possible if each entry point was lowered.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void other(uint3 tid : SV_DispatchThreadID)
{
    buf[1] = int(tid.x);
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK: func %other
// CHECK: func %main
