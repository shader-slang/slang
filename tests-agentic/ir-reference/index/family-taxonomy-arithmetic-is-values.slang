//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:43Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ir-reference/index.md#family-taxonomy
//META: doc_section_digest=a53f1a310fda9b5efc9c45940da27ed7132b7f00cc82073c20d9fb7f689da0bc
//META: purpose=Per the index family taxonomy, arithmetic opcodes belong to the Values family; an integer add lowers to add in the IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc's family-taxonomy diagram routes arithmetic /
// conversions / aggregate constructors to values.md. A plain
// uniform-driven add should therefore emit an `add` opcode in
// the IR dump, not a control-flow or structure opcode.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + b;
}

// CHECK: add
