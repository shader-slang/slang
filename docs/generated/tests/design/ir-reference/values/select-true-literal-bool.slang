//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#select
//META: doc_section_digest=e40d6712a2443477b9dd87fd4146f5ac3852042d7d571c4f7aa5a1ef274fdbc1
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
