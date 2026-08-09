//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A builtin comparison site (`a > 0`, a checker fast-path BuiltinOperatorExpr) lowers to a comparison IR opcode (`cmpGT`) — the doc's BuiltinOperatorExpr row.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `a > 0` with a non-constant `uniform a` lowers to `cmpGT(%a, 0 :
// Int)` in the IR. The doc lists `IREq` as the example for the row;
// `cmpGT` is the comparison opcode for the `>` operator.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (a > 0)
        buf[0] = 1;
    else
        buf[0] = 2;
}

// CHECK-LABEL: func %main
// CHECK: cmpGT(%{{.*}}, 0 : Int)
