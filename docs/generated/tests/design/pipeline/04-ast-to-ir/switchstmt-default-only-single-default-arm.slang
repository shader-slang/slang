//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A SwitchStmt with a default and a single explicit case lowers to a `switch(scrutinee, merge, default, k, block)` — minimum non-trivial case count.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: switch-case-count axis at one explicit case. The doc's
// row says `SwitchStmt` lowers to a `switch` terminator with
// "case-value/case-block operand pairs and a merge operand". With
// one case, exactly one `(k, block)` pair appears after the default.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    switch (sel)
    {
        case 0: buf[0] = 100; break;
        default: buf[0] = 999; break;
    }
}

// CHECK-LABEL: func %main
// CHECK: switch(%{{.*}}, %{{.*}}, %{{.*}}, 0 : Int, %{{.*}})
