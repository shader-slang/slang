//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A BlockStmt containing a single statement (no control flow) lowers to a single entry IRBlock; lowering does not introduce extra blocks just because of the brace nesting.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: BlockStmt lower edge — a brace-only nested block with
// one statement. Doc says "BlockStmt → A sequence of basic blocks";
// the lower bound of "sequence" is exactly one block when there is
// no branching. The function body has a single block ending in
// `return_val`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int x) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    { buf[0] = helper(a); }
}

// CHECK-LABEL: func %main
// CHECK: block %{{.*}}
// CHECK: call %helper
// CHECK: return_val(void_constant)
