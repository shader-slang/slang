//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#other-control-flow-opcodes
//META: doc_section_digest=f5793a717c7bbfcde423407642d2b04f1b04ae204499660ecf91fed86254707e
//META: purpose=A printf call lowers to the Printf instruction with the format string as operand 0 and the call's remaining arguments following it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Printf row of the non-terminator table: the builtin call
// keeps its identity as a dedicated opcode instead of becoming an ordinary
// `call`, and its operand order is format-string-first. The argument is read
// from a uniform so the CHECK observes a real operand rather than a folded
// literal, and the pattern pins the literal format text in operand 0
// position with the value operand after it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    printf("v=%d\n", n);
    buf[0] = n;
}

// CHECK-LABEL: func %main
// CHECK: Printf("v=%d\n", %{{[a-zA-Z0-9_]+}})
