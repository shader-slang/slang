//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A builtin `a & b` (bitwise AND, a checker fast-path BuiltinOperatorExpr) lowers to an `and` IR opcode — the doc's BuiltinOperatorExpr row explicitly names bitwise operators (`IRBitAnd`).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: bitwise-operator axis. The doc's pure-value-inst row
// uses "arithmetic / comparison Expr" as its catch-all; bitwise
// integer operators (`&`, `|`, `^`, `<<`, `>>`) belong to the same
// family. Verifies `and` opcode for `&`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a & b;
}

// CHECK-LABEL: func %main
// CHECK: and(%{{.*}}, %{{.*}})
