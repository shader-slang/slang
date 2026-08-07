//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=c00647cfad21101d2bd2d831fcc3ac751988a6256cea5bd14c17a5aeb428a89e
//META: purpose=Entry-point validation rejects a return type that contains a resource type (E38011), because the stage has no way to return one.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "return type compatibility with the stage" part of entry-point
// validation. The function body is otherwise well-formed; only the
// entry-point-specific rule about what a stage may hand back rejects it, so
// the diagnostic has to come from the shader-specific pass rather than
// ordinary function checking. CHECK pins E38011 on the entry-point name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl

[shader("compute")]
[numthreads(1, 1, 1)]
RWStructuredBuffer<int> main()
//CHECK: E38011
{
    RWStructuredBuffer<int> b;
    return b;
}
