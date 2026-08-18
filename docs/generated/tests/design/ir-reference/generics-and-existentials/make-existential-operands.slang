//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=3daf40cb35b02e6a2100ffd8b47cd886d07ea61a0d83fed32606bc5b122234d2
//META: purpose=Casting a concrete value to an interface variable lowers to makeExistential(value, witness) with that two-operand shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the widening cast to an interface type packs exactly two things.
// Initialising an `IFoo` local from an `A` value is the cast; the CHECK pins
// the resulting `makeExistential` and its two operands, the value and the
// conformance witness.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

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

// CHECK: makeExistential(%{{[A-Za-z0-9_]+}}, %{{[0-9]+}})
