//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry
//META: doc_section_digest=649555343c205ab0c26446093b6240518a16135126c348a497fe87b34f836ad7
//META: purpose=The function type carried in an interface_req_entry uses this_type(%I) for the receiver parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### interface_req_entry` declares an interface requirement.
// The requirement's function type names the receiver via the
// `this_type(%I)` opcode — for `interface IFoo { int run(int x); }`
// the receiver type is `this_type(%IFoo)`, not `%IFoo` directly
// (the latter would be a concrete-typed receiver, but at this
// stage the receiver type is still a self-reference placeholder).

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

// CHECK: interface_req_entry({{.*}}this_type(%IFoo)
