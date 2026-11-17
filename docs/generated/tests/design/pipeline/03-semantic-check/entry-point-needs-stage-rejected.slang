//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=73b0a54aa16472868adbc586d5469e6a6d226c42e5fd1a1ff6720178eede46b1
//META: purpose=Entry-point validation requires a stage, so a function compiled as an entry point with neither a `[shader(...)]` attribute nor `-stage` is rejected with E38007.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "the function's stage attribute" part of entry-point
// validation. The command line names the entry point but supplies no stage,
// and the function carries no stage attribute, so the checker has nothing to
// validate the signature against. CHECK pins E38007 on the function name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry main

[numthreads(1, 1, 1)]
void main()
{
}
//CHECK: E38007
