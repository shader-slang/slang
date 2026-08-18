//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=c07446d42292314ac54df96997ac71db597f857c5d132648cd782bcb15b2b00f
//META: purpose=A 3-method interface emits three interface_req_entry instructions, one per method requirement.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on requirement count: one entry per requirement, none merged. The
// order-independent CHECK-DAGs assert an entry exists for each of the three
// declared methods, keyed by that method's own key.
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

// CHECK-DAG: interface_req_entry(%IMultix5FmethA, Func(Int,
// CHECK-DAG: interface_req_entry(%IMultix5FmethB, Func(Int,
// CHECK-DAG: interface_req_entry(%IMultix5FmethC, Func(Int,
