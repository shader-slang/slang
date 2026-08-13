//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A builtin `a - b` (a checker fast-path BuiltinOperatorExpr) lowers to a `sub` IR opcode in the doc's "BuiltinOperatorExpr -> a single pure value inst" row; the IRSub mirror of IRAdd.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: arithmetic-operator axis sibling to `add`. The doc names
// IRAdd / IRMul / IREq as examples; IRSub is the documented `-`
// counterpart. Both operands are `uniform` to defeat constant folding.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a - b;
}

// CHECK-LABEL: func %main
// CHECK: sub(%{{.*}}, %{{.*}})
