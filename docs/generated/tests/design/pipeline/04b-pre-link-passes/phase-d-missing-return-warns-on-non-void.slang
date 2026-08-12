//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=d5cc2044547103fad1def230cd84b372ec52583c6c1c9461da97d1ff06c46f52
//META: purpose=Phase D's checkForMissingReturns (D5) warns when a non-void function has a control-flow path that reaches the end of the body without returning.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the target-agnostic missing-return validator. `helper` returns
// only on the taken side of the `if`, leaving one path to the closing
// brace. The annotation is anchored on the function's declaration line
// because that is where the diagnostic is reported, and the code CHECK
// pins the warning identity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int x) { if (x > 0) return 1; }
/*CHECK:
    ^^^^^^ non-void function does not return in all cases
*/
// CHECK: W41010

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(a);
}
