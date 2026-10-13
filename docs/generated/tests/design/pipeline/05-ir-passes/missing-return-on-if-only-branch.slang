//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
//META: purpose=Expansion (negative): missing-return diagnoses an `if`-only function where the false branch implicitly falls through — alternative control-flow shape to the empty-body case.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Control-flow variation on missing-return: the function has an
// `if (cond) return ...;` but no return on the fall-through path.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int firstPos(int x)
/*CHECK:
    ^^^^^^^^ non-void function does not return in all cases
*/
// CHECK: W41010
{
    if (x > 0) return x;
    // Falls off the end on the negative path.
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = firstPos(a);
}
