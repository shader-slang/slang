//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-lookup
//META: doc_section_digest=c650b6faf3dc14ee2818d838534e3aeac6e33922a0c81c58eb0231306b87654e
//META: purpose=The result type of lookupWitness is the requirement's function type, including the receiver as the second parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `### Witness lookup` table row for `lookupWitness` says it
// "Resolves an interface requirement through a witness table".
// The result is the requirement's function value, so its type is
// the function type of that requirement: `Func(Int, %T, Int)` for
// `int run(int x)` (Int return, %T receiver, Int parameter).

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

// CHECK: let  %{{[0-9]+}}	: Func(Int, %T{{[A-Za-z0-9_]*}}, Int)	= lookupWitness(
