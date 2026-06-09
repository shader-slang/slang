//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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
