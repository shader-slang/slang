//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#diagnosed-failures-during-binding
//META: doc_section_digest=7be614a0462c62207f70cd7d5a1d37fc1ba9d18f1198a952e2b8d3e90274ac95
//META: purpose=A ray-tracing entry-point parameter on a target with no layout rules for it is diagnosed with error E39032 instead of crashing.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the missing ray-tracing layout rules are reported rather
// than dereferenced. Compiling a `miss` entry point for Metal, which has no
// ray-payload parameter rules, must produce E39032 against the payload
// parameter; the caret annotation pins the diagnostic to that parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry main -stage miss

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
