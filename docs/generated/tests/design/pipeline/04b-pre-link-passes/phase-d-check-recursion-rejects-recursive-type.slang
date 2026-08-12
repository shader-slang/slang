//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=d5cc2044547103fad1def230cd84b372ec52583c6c1c9461da97d1ff06c46f52
//META: purpose=Phase D's checkForRecursiveTypes (D1) disallows recursive type definitions and reports the cycle on the offending struct.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the first of Phase D's validators. A struct holding itself by
// value is the minimal cycle; the annotation pins the error to the struct
// name token so a future misattribution to another line would fail the
// test, and the code CHECK pins the diagnostic identity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

struct Bad { Bad next; };
/*CHECK:
       ^^^ type 'Bad' contains cyclic reference to itself.
*/
// CHECK: E41001

[numthreads(1, 1, 1)]
void main() {}
