//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#lookupwitness--lookupwitnessmethod
//META: doc_section_digest=7bee6071e4a281e3b0a360470ad403730021ec831c4724b2fe0ec4cfc70d7595
//META: purpose=A method call on a generic-constrained value lowers to lookupWitness(witnessTable, requirementKey) with that two-operand shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the operand pair of a witness lookup. Inside a generic body the
// receiver's concrete type is unknown, so `t.run(x)` cannot name a callee
// directly; the CHECK asserts it becomes a two-operand `lookupWitness` whose
// second operand is the key named after the interface requirement.
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
