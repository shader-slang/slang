//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=5defec5bd482a82bdf1c8601f2d3ae1514d0fd25fa9d73a9ad98b6c32be3f36c
//META: purpose=Phase D's checkForRecursiveTypes (D1) rejects a self-referential struct with the cyclic-reference diagnostic.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// D1 disallows recursive type definitions. A `struct Bad`
// containing itself as a field triggers the cyclic-reference
// error. The diagnostic is attached to the struct name token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

struct Bad { Bad next; };
/*CHECK:
       ^^^ type 'Bad' contains cyclic reference to itself.
*/
// CHECK: E41001

[numthreads(1, 1, 1)]
void main() {}
