//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A BinaryExpr `a & b` (bitwise AND) lowers to an `and` IR opcode — a pure-value inst in the doc's arithmetic/comparison Expr row.
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
