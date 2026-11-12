//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=73b0a54aa16472868adbc586d5469e6a6d226c42e5fd1a1ff6720178eede46b1
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
