//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=3a7462a86fc4c15dc7bc9c9156d37eabae3d8e2c13a7f3bce50ff7cff1572439
//META: purpose=The result type of makeExistential is the target interface type, not a wrapper type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### makeExistential` says it produces "a single existential
// value of type `I`" — the dump shape is
// `let %i : %IFoo = makeExistential(...)` where `%IFoo` is the
// target interface type, not a constructed `Existential<I>`
// wrapper. The `let` line's type position pins this.

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

// CHECK: let  %i	: %IFoo	= makeExistential(
