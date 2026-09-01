//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=1523536872ebc337e174666c5cf954c332dd4d3653ef98ebc19e0608a020c147
//META: purpose=Specializing a constrained generic with a non-conforming type is rejected before lowering — no specialize IR is produced.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the front end refuses to build a `specialize` whose witness
// argument cannot exist: `A` does not implement `IFoo`, so there is no witness
// table to pass. The CHECKs pin the conformance diagnostic and its codes at
// the type-argument column.
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
