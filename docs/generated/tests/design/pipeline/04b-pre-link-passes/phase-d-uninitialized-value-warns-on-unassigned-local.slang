//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=de3af7e62fdf97582267b3c674b87776ce5d49cf79a3d54ae3a770a3c679cad0
//META: purpose=Phase D's checkForUsingUninitializedValues (D4) is a dataflow check over the SSA form Phase C produced, and warns when a local is read before any assignment.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the dataflow validator that depends on Phase C having run
// first: only after SSA construction can the checker see that the read of
// `x` has no reaching definition. The annotation is placed on the use, and
// the code CHECK pins the warning identity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x;
    buf[0] = x;
//CHECK:   ^ use of uninitialized variable
//CHECK:   ^ use of uninitialized variable 'x'
// CHECK: W41016
}
