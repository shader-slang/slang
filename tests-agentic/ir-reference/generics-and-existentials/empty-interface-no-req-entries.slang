//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry
//META: doc_section_digest=649555343c205ab0c26446093b6240518a16135126c348a497fe87b34f836ad7
//META: purpose=An interface with zero methods lowers to let %IEmpty : Type = interface with no interface_req_entry operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on interface_req_entry axis: zero requirements. The
// `### interface_req_entry` notable-opcode prose says entries
// "declare the requirement". With no requirements the parent
// `interface(...)` opcode takes *no* `interface_req_entry`
// operands — the dump shape is `let %IEmpty : Type = interface`
// (a bare `interface` keyword on the right-hand side).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IEmpty { };
struct E : IEmpty { int payload; };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int useEmpty<T : IEmpty>(T t, int x) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    E e;
    e.payload = inVal;
    buf[0] = useEmpty<E>(e, inVal);
}

// CHECK: let  %IEmpty	: Type	= interface
// CHECK-NOT: let  %IEmpty	: Type	= interface(
