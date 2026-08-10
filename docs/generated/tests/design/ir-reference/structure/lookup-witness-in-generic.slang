//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witness-tables-and-witness-facts
//META: doc_section_digest=1af9e6c2e01af2b94483cbd8d90b8d30f4856731a44da18cf2e68c7379930ffc
//META: purpose=A generic body that calls a method on a constrained type parameter emits a lookupWitness opcode whose operands are (witnessTable, requirementKey).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the structural consumer side of a witness table. Inside
// `helper<T : IFoo>` the call `t.run(x)` cannot name a concrete function, so
// lowering resolves it against the generic's witness-table parameter. CHECK
// requires `lookupWitness(<numbered witness value>, %IFoox5Frun)` — the table
// first, the requirement key second.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo
{
    int run(int x);
}

struct A : IFoo { int run(int x) { return x + 1; } }

int helper<T : IFoo>(T t, int x)
{
    return t.run(x);
}

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = helper<A>(a, inVal);
}

// CHECK: lookupWitness(%{{[0-9]+}}, %IFoox5Frun)
