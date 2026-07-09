//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=ad77551965f16f59589941dbe50f81bd8509b595aa5ee08277212d611c118e05
//META: purpose=Phase D's checkForMissingReturns (D5) warns when a non-void function lacks a return on every control-flow path (target=None, target-agnostic pre-link).
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// D5 runs with `target=None` (target-agnostic) and emits the
// missing-return warning for a non-void function whose body
// has a path that reaches end-of-function without a return.

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
