//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-12T00:00:00+00:00
//META: source_commit=2094af503348384f42f81337791d59c3b5e5b6ee
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#diagnosed-failures-during-binding
//META: doc_section_digest=51100b7159110d5959dc04570edd5f4b8242b0c33e4d66e9cc5d44b0f1f049dd
//META: purpose=A target whose layout-rules family has no callable-payload rules rejects a callable entry-point payload with E39032 naming the callable stage, rather than dereferencing the null rules.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the null-rules diagnostic added by PR #12280: before it,
// `processEntryPointVaryingParameter` passed the null straight into
// `createTypeLayoutWith` and segfaulted with no diagnostic. Metal is used
// because its layout-rules family returns null from all three ray-tracing
// accessors, so the callable arm is reachable there.
//
// Note this test previously targeted CUDA, on the strength of CUDA returning
// null from the callable accessor alone. That stopped being true at
// `5ed83a468c` ("Add callable shader support to CUDA/OptiX backend", #12182),
// which gave the accessor real rules -- CUDA now accepts all three parameter
// kinds, so the diagnostic is correctly absent there and the test had to move
// to a family that still lacks the rules.
//
// The annotation pair appears twice because the diagnostic itself is
// emitted twice for the one parameter; exhaustive mode requires every
// emitted diagnostic to be annotated, so both copies are pinned.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry main -stage callable

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
