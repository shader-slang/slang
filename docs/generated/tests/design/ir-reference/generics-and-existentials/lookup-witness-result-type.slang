//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-lookup
//META: doc_section_digest=4508b8b8f3e8d47f0f2bbf47badad9a2eb49539f2f7232d58154356ce8e2226a
//META: purpose=The result type of lookupWitness is the requirement's function type, including the receiver as the second parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies what a witness lookup *evaluates to*: not the value found, but a
// value typed as the requirement's function type. The CHECK reads the `let`
// binding's declared type and asserts it is `Func(Int, %T, Int)` — return
// type, the still-abstract receiver type parameter, then the declared
// parameter.
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

// CHECK: let  %{{[0-9]+}} : Func(Int, %T{{[A-Za-z0-9_]*}}, Int) = lookupWitness(
