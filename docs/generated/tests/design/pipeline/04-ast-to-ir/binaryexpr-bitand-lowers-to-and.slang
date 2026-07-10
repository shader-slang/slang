//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=19eb09c072c556dd0799b5002c2f1b24781da93f2cebc122ca027062c24e0b84
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
