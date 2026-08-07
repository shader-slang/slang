//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=3daf40cb35b02e6a2100ffd8b47cd886d07ea61a0d83fed32606bc5b122234d2
//META: purpose=A makeExistential whose payload is a multi-field struct still emits the same two-operand let %i : %IFoo = makeExistential(value, witness) shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on payload size: packing does not decompose the value. A three-field
// struct is widened to `IFoo`, and the CHECK asserts the same single
// two-operand inst as the trivial payload case — no per-field packing appears.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };

struct Big : IFoo
{
    int a;
    int b;
    int c;
    int run(int x) { return a + b + c + x; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Big big;
    big.a = inVal;
    big.b = inVal + 1;
    big.c = inVal + 2;
    IFoo i = big;
    buf[0] = i.run(inVal);
}

// CHECK: let  %i : %IFoo = makeExistential(%{{[A-Za-z0-9_]+}}, %{{[0-9]+}})
