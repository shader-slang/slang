//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=3383849aee881833e2f4b2d76aefa96ea7b5e5f52c85e003f63997da83c112e0
//META: purpose=A struct conforming to an interface with zero methods emits an empty witness_table (no witness_table_entry rows).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on witness_table_entry axis: empty interface (zero
// requirements). `### witness_table and witness_table_entry`
// says entries are "one per interface requirement" — zero
// requirements implies zero entries. The dump shape is
// `witness_table %N : witness_table_t(%IEmpty)(%E);` with no
// body, terminated by `;`.

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

// CHECK: witness_table %{{[0-9]+}}	: witness_table_t(%IEmpty)(%E);
