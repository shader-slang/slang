//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A short-circuit `(a > b) && (c < d)` lowers to an `ifElse(...)` terminator whose true/false blocks unconditionally branch to a merge block carrying the joined Bool as a block parameter — the doc's phi-as-block-param encoding applied to short-circuit logic.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: phi-encoding axis — short-circuit boolean. The
// expression-level `&&` is lowered as a control-flow split (so the
// RHS is not evaluated when LHS is false); the join carries a Bool
// block parameter. The doc's claim "branches to a block carry the
// parameter values as arguments" applies here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform int c;
uniform int d;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if ((a > b) && (c < d))
        buf[0] = 1;
    else
        buf[0] = 0;
}

// CHECK-LABEL: func %main
// CHECK: ifElse(
// CHECK: unconditionalBranch(%{{.*}}, %{{.*}})
// CHECK: unconditionalBranch(%{{.*}}, false)
// CHECK-NOT: = phi(
