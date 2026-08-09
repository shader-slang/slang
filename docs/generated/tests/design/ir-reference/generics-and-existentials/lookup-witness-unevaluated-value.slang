//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=55d24b9cb1e17ce70457d4ac3ad5c2fd829e24b2244903cc228160630ca43d8f
//META: purpose=A lookupWitness is a first-class unevaluated value: it is bound in the enclosing generic's block and the function body calls that binding rather than a resolved callee.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that lowering carries the lookup itself rather than the value it
// will eventually resolve to. Only one type conforms to `IFoo` in this
// program, yet the lookup is still emitted — as a value defined in the
// generic's own block, above the `func` it parameterises. The CHECKs bind that
// value and then assert the call inside the function body uses exactly it,
// which is only possible if the lookup is an ordinary SSA value.
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

// CHECK: let  [[F:%[0-9]+]] : Func(Int, %T{{[A-Za-z0-9_]*}}, Int) = lookupWitness(
// CHECK: func %helper
// CHECK: call [[F]](
