//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=55d24b9cb1e17ce70457d4ac3ad5c2fd829e24b2244903cc228160630ca43d8f
//META: purpose=Lowering an interface declaration synthesizes a thisTypeWitness placeholder certifying that ThisType conforms to the enclosing interface.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the abstract witness an interface needs for its own `This` type. A
// requirement returning `This` means the interface body must treat `This` as a
// conformer, and the CHECK asserts the placeholder witness that certifies it
// appears in the dump.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFrob
{
    This frob();
    int get();
};

struct A : IFrob
{
    int v;
    A frob() { A r; r.v = v + 1; return r; }
    int get() { return v; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int useFrob<T : IFrob>(T t) { return t.frob().get(); }

[numthreads(1, 1, 1)]
void main()
{
    A a;
    a.v = inVal;
    buf[0] = useFrob<A>(a);
}

// CHECK: thisTypeWitness
