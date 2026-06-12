//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=Specializing a constrained generic with a non-conforming type is rejected before lowering — no specialize IR is produced.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative on constraint axis: `helper<T : IFoo>` expects T to
// conform to IFoo. Passing struct `A` (no interface
// implementation) must be rejected at the semantic-check stage
// before the `specialize` IR is built — verifying that the
// witness-argument operand of `specialize` (per
// `### specialize`) cannot be silently omitted.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -entry main -stage compute

interface IFoo { int run(int x); };

struct A { int x; };

uniform RWStructuredBuffer<int> buf;
uniform int inVal;

int helper<T : IFoo>(T t, int x) { return t.run(x); }

[numthreads(1, 1, 1)]
void main()
{
    A a;
    a.x = inVal;
    buf[0] = helper<A>(a, inVal);
//CHECK:     ^^^^^^ type argument doesn't conform to interface
// CHECK: E38021
// CHECK: E38029
}
