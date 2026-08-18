//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=1523536872ebc337e174666c5cf954c332dd4d3653ef98ebc19e0608a020c147
//META: purpose=A specialize on an interface-constrained generic passes the type argument followed by the conforming witness table.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that a conformance constraint contributes an extra `specialize`
// argument. `helper<T : IFoo>` needs a witness that `T` conforms, so the call
// site's argument list is (type, witness) rather than (type) alone; the CHECK
// pins two arguments after the base, the first being the concrete type `%A`.
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
