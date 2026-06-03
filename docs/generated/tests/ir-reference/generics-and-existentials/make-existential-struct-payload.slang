//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#makeexistential
//META: doc_section_digest=3a7462a86fc4c15dc7bc9c9156d37eabae3d8e2c13a7f3bce50ff7cff1572439
//META: purpose=A makeExistential whose payload is a multi-field struct still emits the same two-operand let %i : %IFoo = makeExistential(value, witness) shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on makeExistential payload shape: concrete type is
// a multi-field struct. `### makeExistential` "packs a value of
// some concrete type C and a witness" — the operand shape is
// invariant in the concrete type, including when the payload is
// a multi-field struct.

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

// CHECK: let  %i	: %IFoo	= makeExistential(%{{[A-Za-z0-9_]+}}, %{{[0-9]+}})
