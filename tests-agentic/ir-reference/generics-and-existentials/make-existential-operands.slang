//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=3a7462a86fc4c15dc7bc9c9156d37eabae3d8e2c13a7f3bce50ff7cff1572439
//META: purpose=Casting a concrete value to an interface variable lowers to makeExistential(value, witness) with that two-operand shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### makeExistential`: "Packs `value` plus the witness that its
// concrete type conforms to the target interface". The natural
// surface is an `IFoo i = a;` initialization where `a : A` and
// `A : IFoo` — the IR is `makeExistential(%a, %witnessA_IFoo)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

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

// CHECK: makeExistential(%{{[A-Za-z0-9_]+}}, %{{[0-9]+}})
