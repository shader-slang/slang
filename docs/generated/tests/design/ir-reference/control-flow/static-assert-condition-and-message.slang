//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#other-control-flow-opcodes
//META: doc_section_digest=f5793a717c7bbfcde423407642d2b04f1b04ae204499660ecf91fed86254707e
//META: purpose=A static_assert call lowers to the StaticAssert instruction carrying the condition value and the message string as its two operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the StaticAssert row: the builtin becomes a two-operand
// instruction sitting as an ordinary member of the block rather than a
// terminator, and the message survives lowering as a string literal operand.
// The condition is written over a `static const` so it is a genuine value
// operand in the dump; the CHECK binds the comparison result and requires
// StaticAssert's first operand to be exactly that value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;
static const int K = 4;

[numthreads(1, 1, 1)]
void main()
{
    static_assert(K == 4, "K must be 4");
    buf[0] = n;
}

// CHECK-LABEL: func %main
// CHECK: [[COND:%[0-9]+]]{{.*}}= cmpEQ(%K, 4 : Int)
// CHECK: StaticAssert([[COND]], "K must be 4")
