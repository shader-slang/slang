//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#functions-and-generics
//META: doc_section_digest=946edb590d3d1e528eec5aad3551de2e1f970420353f74e798c659afca170ea1
//META: purpose=A function call lowers to a call opcode whose first operand is the callee func and remaining operands are the arguments.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Calling `triple(n)` inside `main` should produce `call %triple(...)`
// in the caller body — the structural link between caller and callee
// runs through the `call` opcode, whose first operand is the callee
// `func` value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int triple(int x)
{
    return x + x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = triple(n);
}

// CHECK: func %triple	: Func(Int, Int)
// CHECK-LABEL: func %main
// CHECK: call %triple(%n)
