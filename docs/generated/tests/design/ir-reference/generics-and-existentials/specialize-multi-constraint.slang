//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=3ad6835b3f4fbf438debd857fd413ba7cce495005327d971c499cc7510fecf17
//META: purpose=A generic with two interface constraints lowers to specialize(%base, T, %w1, %w2) — one witness operand per constraint.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on witness-operand multiplicity: a type parameter carrying two
// `where` constraints contributes one witness argument each. The CHECK pins a
// four-element argument list (base, type, witness, witness), so dropping or
// merging either conformance would fail it.
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
