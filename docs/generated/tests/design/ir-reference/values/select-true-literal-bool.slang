//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#select
//META: doc_section_digest=8d30dda9ed7d2748e6be00027e0ec0b5dd193f2f9d741727f3dc3eb6d99acea4
//META: purpose=Boundary: `select(c, 1, 0)` with literal-integer true/false arms lowers to `select(%c, 1 : Int, 0 : Int)` with the BoolLit-equivalent literal payloads on the result operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `select` opcode's value operands are full SSA values. Here
// they happen to be integer literals — the IR formatter prints
// them in the same inline `<N> : Int` form as on any other
// instruction operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool c;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(c, 1, 0);
}

// IR-LABEL: func %main
// IR: select(%c, 1 : Int, 0 : Int)
