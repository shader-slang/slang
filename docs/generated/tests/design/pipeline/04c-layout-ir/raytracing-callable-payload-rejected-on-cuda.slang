//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#diagnosed-failures-during-binding
//META: doc_section_digest=210845004e453a7583de469b0de24d48d58f75a96ce4c3d08e88229e3bb5eb0d
//META: purpose=CUDA lacks only the callable-payload layout rules, so a callable entry-point payload is rejected with E39032 naming the callable stage.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-accessor carve-out in the ray-tracing rules table: the
// CUDA layout-rules family is documented as returning null from the
// callable-payload accessor alone, so the missing-rules diagnostic must be
// reachable on CUDA through a `callable` entry point specifically. The
// caret pins E39032 to the payload parameter, and the annotated message
// names the 'callable' stage, which is what distinguishes this branch from
// the families that lack all three accessors.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry main -stage callable

struct CallData
{
    float4 color;
}

[shader("callable")]
void main(inout CallData p)
//CHECK:                 ^ target does not support ray tracing entry point parameters
//CHECK:                 ^ the current compilation target does not support ray tracing entry point parameters for the 'callable' stage
//CHECK:                 ^ target does not support ray tracing entry point parameters
//CHECK:                 ^ the current compilation target does not support ray tracing entry point parameters for the 'callable' stage
{
    p.color = float4(1, 0, 0, 1);
}
