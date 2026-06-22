//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#interfacereqentry
//META: doc_section_digest=649555343c205ab0c26446093b6240518a16135126c348a497fe87b34f836ad7
//META: purpose=A 3-method interface emits three interface_req_entry instructions, one per method requirement.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on interface_req_entry axis: 3 requirement entries.
// `### interface_req_entry` prose says each requirement
// produces an `interface_req_entry` keyed by a `StructKey`. A
// 3-method interface therefore produces three
// `interface_req_entry(%key, FuncType)` instructions.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IMulti
{
    int methA(int x);
    int methB(int x);
    int methC(int x);
};

struct S : IMulti
{
    int methA(int x) { return x; }
    int methB(int x) { return x + 1; }
    int methC(int x) { return x + 2; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int helper<T : IMulti>(T t, int x) { return t.methA(x) + t.methB(x) + t.methC(x); }

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = helper<S>(s, inVal);
}

// CHECK: interface_req_entry(%IMultix5FmethA, Func(Int,
// CHECK: interface_req_entry(%IMultix5FmethB, Func(Int,
// CHECK: interface_req_entry(%IMultix5FmethC, Func(Int,
