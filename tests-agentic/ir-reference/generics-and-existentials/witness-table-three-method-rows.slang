//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#witnesstable-and-witnesstableentry
//META: doc_section_digest=8793675da78b4d53076ff119e26b6c1db6781408b92a1cb0ac85f2df7ce46cf0
//META: purpose=A struct conforming to a 3-method interface emits three witness_table_entry rows — one per requirement, in declaration order.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on witness_table_entry axis: multi-method body. The
// `### witness_table and witness_table_entry` notable opcode
// prose says "Each entry maps a `requirementKey` ... one per
// interface requirement". A 3-method interface body therefore
// produces three `witness_table_entry(%key, %impl)` rows.

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

// CHECK: witness_table_entry(%IMultix5FmethA,%Sx5FmethA)
// CHECK: witness_table_entry(%IMultix5FmethB,%Sx5FmethB)
// CHECK: witness_table_entry(%IMultix5FmethC,%Sx5FmethC)
