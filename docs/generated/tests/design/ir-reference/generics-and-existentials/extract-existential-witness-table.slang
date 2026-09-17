//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#existential-destructuring
//META: doc_section_digest=feec321c75d02df216fff6328f297e50cd2feb1c968e7a5a0802e7ba10ee5328
//META: purpose=Dispatching through an interface-typed value produces extractExistentialWitnessTable(%i) whose result type is witness_table_t(%I).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the witness projection: the conformance recovered from an
// existential is typed by the interface it witnesses, which is what the
// downstream lookup needs. The CHECK reads the binding's declared type and
// asserts `witness_table_t(%IFoo)`.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    IFoo i = a;
    buf[0] = i.run(inVal);
}

// CHECK: let  %{{[0-9]+}} : witness_table_t(%IFoo) = extractExistentialWitnessTable(%i)
