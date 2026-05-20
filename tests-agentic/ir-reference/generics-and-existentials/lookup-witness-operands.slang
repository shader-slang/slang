//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#lookupwitness-lookupwitnessmethod
//META: doc_section_digest=c4815aeb9b6c67b5092b523e0e44f337c6c8104ecdaafadc93a47d08bdef573b
//META: purpose=A method call on a generic-constrained value lowers to lookupWitness(witnessTable, requirementKey) with that two-operand shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### lookupWitness / LookupWitnessMethod`: "operands are
// `witnessTable, requirementKey`". Inside the generic body of
// `int helper<T : IFoo>(T t, int x) { return t.run(x); }`, the
// `t.run(x)` call is rewritten as
// `lookupWitness(%witnessParam, %IFoox5Frun)` — first operand is
// the witness-table generic parameter, second is the key naming
// the requirement.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int helper<T : IFoo>(T t, int x) { return t.run(x); }

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = helper<A>(a, inVal);
}

// CHECK: lookupWitness(%{{[0-9]+}}, %IFoox5Frun)
