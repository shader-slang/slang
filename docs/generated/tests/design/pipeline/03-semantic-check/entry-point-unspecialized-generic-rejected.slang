//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=c00647cfad21101d2bd2d831fcc3ac751988a6256cea5bd14c17a5aeb428a89e
//META: purpose=A generic entry point left genuinely unspecialized is diagnosed (E38014) rather than compiled into an unusable generic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unspecialized-generic-entry-point check. No specialization
// arguments are supplied on the command line, so the entry point has no
// concrete type to lower to; the check turns that into a diagnostic at the
// entry-point declaration. CHECK pins E38014 on the entry-point name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main<T : IArithmetic>(uniform T v)
{
    outputBuffer[0] = 1;
}
//CHECK: E38014
