//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry
//META: doc_section_digest=649555343c205ab0c26446093b6240518a16135126c348a497fe87b34f836ad7
//META: purpose=An interface requirement lowers to interface_req_entry(requirementKey, requirementFuncType) — first operand is the key, second is the function type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### interface_req_entry`: "sits inside an `InterfaceType`
// parent opcode and declares the requirement. The
// `requirementKey` (a `StructKey`) is what joins the two sides".
// For `interface IFoo { int run(int x); }` the row is
// `interface_req_entry(%IFoox5Frun, Func(Int, this_type(%IFoo), Int))`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = a.run(inVal);
}

// CHECK: interface_req_entry(%IFoox5Frun, Func(Int,
