//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#witness-tables-and-witness-facts
//META: doc_section_digest=6b9b35ba535a49bbbb2b1c4ab4c4b3fe10721e61e528a0e174d085f837d9748a
//META: purpose=A generic body that calls a method on a constrained type parameter emits a lookupWitness opcode whose operands are (witness_table, requirementKey).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// In `int helper<T : IFoo>(T t, int x) { return t.run(x); }` the call
// to `t.run(x)` is rewritten in the generic body as a
// `lookupWitness(%witnessParam, %IFoox5Frun)` whose result is the
// requirement's function value. The structural shape is
// `(witnessTable, requirementKey)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo
{
    int run(int x);
};

struct A : IFoo { int run(int x) { return x + 1; } };

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
