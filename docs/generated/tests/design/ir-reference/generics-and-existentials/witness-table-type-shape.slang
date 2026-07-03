//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=3383849aee881833e2f4b2d76aefa96ea7b5e5f52c85e003f63997da83c112e0
//META: purpose=A struct implementing an interface produces a witness_table whose type is witness_table_t(interface)(implementing-type).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### Witness tables and witness facts` cross-references the
// structural opcode `witness_table`. The dispatch path's
// observation is that the table's *type* carries the
// (interface, implementing-type) pair as two parenthesised
// operands: `witness_table_t(%IFoo)(%A)`.

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

// CHECK: witness_table %{{[0-9]+}}	: witness_table_t(%IFoo)(%A)
