//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#witnesstable-and-witnesstableentry
//META: doc_section_digest=8793675da78b4d53076ff119e26b6c1db6781408b92a1cb0ac85f2df7ce46cf0
//META: purpose=Each row inside a witness_table body is witness_table_entry(requirementKey, satisfyingVal) — first operand is the key, second is the satisfying value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### witness_table and witness_table_entry`: "Each entry maps
// a `requirementKey` (a `StructKey` declared on the interface
// type) to a `satisfyingVal` (a function, type, or value)". For
// `struct A : IFoo { int run(int x) { ... } }` the row is
// `witness_table_entry(%IFoox5Frun, %Ax5Frun)`.

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

// CHECK: witness_table_entry(%IFoox5Frun,%Ax5Frun)
