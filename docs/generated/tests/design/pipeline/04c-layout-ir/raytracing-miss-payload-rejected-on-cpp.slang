//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#diagnosed-failures-during-binding
//META: doc_section_digest=210845004e453a7583de469b0de24d48d58f75a96ce4c3d08e88229e3bb5eb0d
//META: purpose=The CPU layout-rules family supplies no ray-payload rules either, so the same miss payload that compiles for SPIR-V is rejected with E39032 for -target cpp.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second half of the worked example for the missing-rules
// diagnostic: the CPU family is documented alongside Metal and LLVM as
// returning null from all three ray-tracing accessors, so the payload
// parameter must draw the same E39032 there rather than reaching the
// null-dereferencing layout construction. The caret pins the diagnostic to
// the payload parameter and the message names the 'miss' stage.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry main -stage miss

struct Payload
{
    float4 color;
}

[shader("miss")]
void main(inout Payload payload)
//CHECK:                ^^^^^^^ target does not support ray tracing entry point parameters
//CHECK:                ^^^^^^^ the current compilation target does not support ray tracing entry point parameters for the 'miss' stage
//CHECK:                ^^^^^^^ target does not support ray tracing entry point parameters
//CHECK:                ^^^^^^^ the current compilation target does not support ray tracing entry point parameters for the 'miss' stage
{
    payload.color = float4(1, 0, 0, 1);
}
