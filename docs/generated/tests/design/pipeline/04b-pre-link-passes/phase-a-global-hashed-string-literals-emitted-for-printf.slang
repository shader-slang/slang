//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission
//META: doc_section_digest=66b9b0ad1e435e5d64e2372e5eedf6745238e43195f17a70dd348156a613b908
//META: purpose=Phase A emits the single kIROp_GlobalHashedStringLiterals aggregate (A8) holding every hashed string literal observed during the declaration walk.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that when the module contains a hashed string literal, Phase A
// closes the declaration walk by emitting one module-scope aggregate that
// collects them. `printf` routes its format string through that table, so
// the CHECK looks for `global_hashed_string_literals` carrying the literal
// text in the post-Phase-D snapshot.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("hello!\n");
    buf[0] = 1;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK: global_hashed_string_literals("hello!\n")
