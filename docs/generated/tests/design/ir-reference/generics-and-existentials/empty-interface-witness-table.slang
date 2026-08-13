//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=55d24b9cb1e17ce70457d4ac3ad5c2fd829e24b2244903cc228160630ca43d8f
//META: purpose=A struct conforming to an interface with zero methods emits an empty witness_table (no witness_table_entry rows).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary at zero requirements. The conformance still exists as a table so a
// constrained generic can take it as an argument, but there is nothing to key;
// the CHECK pins the semicolon-terminated form the dump uses for a childless
// witness table.
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

// CHECK: witness_table %{{[0-9]+}} : witness_table_t(%IEmpty)(%E);
