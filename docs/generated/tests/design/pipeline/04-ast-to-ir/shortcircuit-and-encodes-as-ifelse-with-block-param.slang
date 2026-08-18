//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
