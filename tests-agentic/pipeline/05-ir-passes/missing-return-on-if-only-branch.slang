//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=15ffe04c81dead304d9773538e8bca993b707b3393574380bcd38d0a23da4044
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
