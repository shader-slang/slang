//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=55d24b9cb1e17ce70457d4ac3ad5c2fd829e24b2244903cc228160630ca43d8f
//META: purpose=A witness table is a key-to-value map, not a positional list: a conformer declaring its members in a different order than the interface still pairs each requirement key with its own satisfying value.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary that separates key-driven pairing from positional pairing. `S`
// declares `methC`, `methA`, `methB` while `IMulti` requires `methA`, `methB`,
// `methC`, so any positional association would mis-pair the rows. The
// order-independent CHECK-DAGs assert each key sits with its own
// implementation, which only holds if entries are read by key.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IMulti
{
    int methA(int x);
    int methB(int x);
    int methC(int x);
};

struct S : IMulti
{
    int methC(int x) { return x + 2; }
    int methA(int x) { return x; }
    int methB(int x) { return x + 1; }
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

// CHECK-DAG: witness_table_entry(%IMultix5FmethA,%Sx5FmethA)
// CHECK-DAG: witness_table_entry(%IMultix5FmethB,%Sx5FmethB)
// CHECK-DAG: witness_table_entry(%IMultix5FmethC,%Sx5FmethC)
