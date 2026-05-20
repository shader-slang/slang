//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry
//META: doc_section_digest=649555343c205ab0c26446093b6240518a16135126c348a497fe87b34f836ad7
//META: purpose=An interface declaration appears as let %I : Type = interface(%req...) whose operands are the interface_req_entry value-ids.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### interface_req_entry` says it "sits inside an
// `InterfaceType` parent opcode" — the parent `interface(...)`
// opcode lists the requirement entries as its operands.
// For an interface with two methods, the `interface` opcode
// takes two operands, both `interface_req_entry` value-ids.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IBar
{
    int doA(int x);
    int doB(int x);
};

struct S : IBar
{
    int doA(int x) { return x + 1; }
    int doB(int x) { return x * 2; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = s.doA(inVal) + s.doB(inVal);
}

// CHECK: let  %IBar	: Type	= interface(%{{[0-9]+}}, %{{[0-9]+}})
