//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=A generic taking an interface-typed parameter has its specialize call composed alongside a makeExistential argument — the dispatch site interleaves the two opcodes.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress on combined-opcode interaction: when a constrained
// generic takes an interface-typed parameter, the dispatch site
// composes `makeExistential` (from the cast) with
// `specialize(...)` (the generic call). Per `### specialize` and
// `### makeExistential` both opcodes appear in the same basic
// block, with the existential value flowing into the
// specialized call's argument list.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int processExistential<T : IFoo>(IFoo i, int x) { return i.run(x); }

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = processExistential<A>(a, inVal);
}

// CHECK: makeExistential(%{{[A-Za-z0-9_]+}}, %{{[0-9]+}})
// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, %A, %{{[0-9]+}})
