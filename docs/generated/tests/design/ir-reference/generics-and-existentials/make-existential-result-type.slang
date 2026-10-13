//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=12fc3bdfdfd39992f73247a51ecf438875c6398134a12dce64d1e764d8adbf0f
//META: purpose=The result type of makeExistential is the target interface type, not a wrapper type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies where the packed value's type ends up. The interface local `i` is
// bound directly to the `makeExistential` result, so the CHECK reads the
// binding's declared type and asserts it is `%IFoo` itself — the concrete type
// `%A` has been erased into the operand list rather than showing up in the
// result type.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

interface IFoo { int run(int x); };
struct A : IFoo { int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    IFoo i = a;
    buf[0] = i.run(inVal);
}

// CHECK: let  %i : %IFoo = makeExistential(
