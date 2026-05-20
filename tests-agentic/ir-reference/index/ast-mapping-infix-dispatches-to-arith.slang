//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:43Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ir-reference/index.md#how-ast-nodes-lower-to-ir
//META: doc_section_digest=e4b96c5106c1f3a08e7bc994b1774dbb0f39e0c83b35d7a5f72e4ed9186e3b3e
//META: purpose=The index doc names visitInfixExpr as dispatching to arithmetic opcodes like Add, Sub, Mul; mixed-operator code emits each opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc names `visitInfixExpr dispatches to arithmetic
// opcodes such as Add, Sub, Mul`. We use uniform operands per
// `_common.md` so constant folding does not collapse the
// arithmetic before IR emit, and we verify all three opcodes
// appear in one IR dump.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform int c;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (a + b) - (a * c);
}

// CHECK-DAG: add
// CHECK-DAG: sub
// CHECK-DAG: mul
