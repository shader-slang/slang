//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:08:34Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#extractexistentialvalue-extractexistentialtype-extractexistentialwitnesstable
//META: doc_section_digest=16045cc2106f4346cc8dfb52e07857b9a6e4297569798ffd3bcd3f5258a250c1
//META: purpose=Two dispatches through the same existential share a single hoistable extractExistentialWitnessTable inst that feeds both lookupWitness sites.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the witness-table projection is hoistable: its result
// depends only on the existential operand, so dispatching two different
// interface methods on the same existential reuses one
// extractExistentialWitnessTable inst. The CHECK captures the single %w
// definition and requires both lookupWitness sites (one per method) to
// name that same captured %w as their witness-table operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); int walk(int x); };
struct A : IFoo
{
    int run(int x) { return x + 1; }
    int walk(int x) { return x + 2; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    IFoo i = a;
    buf[0] = i.run(inVal);
    buf[1] = i.walk(inVal);
}

// CHECK: let  [[W:%[0-9]+]] : witness_table_t(%IFoo) = extractExistentialWitnessTable(%i)
// CHECK: lookupWitness([[W]],
// CHECK: lookupWitness([[W]],
