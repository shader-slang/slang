//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#functions-and-generics
//META: doc_section_digest=65157f975f1cbb6740bfae10984ea872d190878fcc32a62888fced45d662037d
//META: purpose=A function call lowers to a call opcode whose first operand is the callee func and remaining operands are the arguments.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the caller-to-callee structural link. `triple(n)` is called from
// `main` with a uniform argument (so the call is not constant-folded away);
// CHECK-LABEL scopes the search to `main`'s body and CHECK then requires
// `call %triple(%n)` — callee first, argument tail after.

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
