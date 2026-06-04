//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#interface-internals
//META: doc_section_digest=881ba3684ff6d20a2e1a262e82de634f0b857cc0f62fb483fbb5984cc6845bdf
//META: purpose=The requirementKey operand of an interface_req_entry is the same StructKey value that names the requirement on the witness_table_entry that satisfies it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `witness_table_entry vs interface_req_entry` note says
// these two opcodes are "the interface-side and implementation-side
// halves of the same key-driven dispatch table". The same
// `%IFoox5Frun` key appears on both sides.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

interface IFoo
{
    int run(int x);
};

struct A : IFoo
{
    int run(int x) { return x + 1; }
};

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = a.run(inVal);
}

// CHECK: interface_req_entry(%IFoox5Frun,
// CHECK: witness_table_entry(%IFoox5Frun,
