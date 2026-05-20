//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=A generic with two interface constraints lowers to specialize(%base, T, %w1, %w2) — one witness operand per constraint.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on generic-constraints axis: multiple interface
// constraints. The doc's `### specialize` row says operands are
// `base, arg, ...` — for `helper<T> where T : IFoo where T : IBar`
// the IR carries the type arg followed by *two* witness tables,
// one per constraint.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
interface IBar { int meta(); };

struct S : IFoo, IBar
{
    int run(int x) { return x + 1; }
    int meta() { return 42; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int helper<T>(T t, int x) where T : IFoo where T : IBar
{
    return t.run(x) + t.meta();
}

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = helper<S>(s, inVal);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, %S, %{{[0-9]+}}, %{{[0-9]+}})
