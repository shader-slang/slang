//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=3daf40cb35b02e6a2100ffd8b47cd886d07ea61a0d83fed32606bc5b122234d2
//META: purpose=A cast that crosses two inheritance steps feeds the transitively-derived witness table in as makeExistential's second operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies which witness a multi-step widening cast packs. `A` conforms to
// `IDerived`, and `IDerived` inherits `IBase`, so widening an `A` to `IBase`
// needs the A-to-IBase table rather than the A-to-IDerived one. That table is
// exactly the value satisfying `IDerived`'s inheritance requirement in `A`'s
// witness table; the CHECKs capture it from the `witness_table_entry` row and
// then assert it is the second operand of the `makeExistential`.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IBase { int base(int x); };
interface IDerived : IBase { int derived(int x); };
struct A : IDerived
{
    int base(int x) { return x + 1; }
    int derived(int x) { return x + 2; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    A a;
    IBase b = a;
    buf[0] = b.base(inVal);
}

// CHECK: witness_table_entry(%IDerivedx5Fx24inheritance,[[W:%[0-9]+]])
// CHECK: let  %b : %IBase = makeExistential(%a, [[W]])
