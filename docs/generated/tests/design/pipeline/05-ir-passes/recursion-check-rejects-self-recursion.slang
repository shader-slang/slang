//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=667952fd5b9698b654c70c639e18194281145ed55e05baa506681d6bf7a7125d
//META: purpose=The check-recursion validation pass diagnoses unsupported recursion in the IR.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A directly self-recursive function triggers an E55201 error from
// the check-recursion validation pass listed in the "Linking and
// validation" category.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int fact(int n)
{
    return n <= 1 ? 1 : n * fact(n - 1);
//CHECK:                        ^ recursion not allowed
//CHECK:                        ^ recursion detected in call to 'fact', but the current code generation target does not allow recursion.
// CHECK: E55201
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = fact(5);
}
