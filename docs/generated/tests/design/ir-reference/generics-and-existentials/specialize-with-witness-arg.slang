//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=A specialize on an interface-constrained generic passes the type argument followed by the conforming witness table.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `helper<A>(a, x)` for `int helper<T : IFoo>(T t, int x)` should
// produce `call specialize(%helper, %A, %witness)(%a, %x)` — the
// witness flows as an extra `specialize` argument after the type
// argument. The doc's `### specialize` row's "Applies generic
// arguments to a generic value (function, type, or witness)" claim.

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

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, %A, %{{[0-9]+}})
