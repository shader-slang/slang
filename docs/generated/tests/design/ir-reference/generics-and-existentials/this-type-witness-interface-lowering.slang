//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:08:34Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=8666fb5a8a3e866e04eed6573b9b735ade91d11b88b56b96aa10b6822be031b8
//META: purpose=Lowering an interface declaration synthesizes a thisTypeWitness placeholder certifying that ThisType conforms to the enclosing interface.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that thisTypeWitness is produced during InterfaceDecl
// lowering rather than by a later pass: a user interface whose method
// returns `This` forces the enclosing-interface self-conformance fact
// into the lowered module, and the CHECK pins the thisTypeWitness
// opcode spelling in the dump of a module containing only user code
// plus a generic dispatch that keeps the interface live.

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
