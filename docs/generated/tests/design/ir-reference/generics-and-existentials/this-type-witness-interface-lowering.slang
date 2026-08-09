//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=55d24b9cb1e17ce70457d4ac3ad5c2fd829e24b2244903cc228160630ca43d8f
//META: purpose=A lookup that has to resolve a conformance on ThisType while the interface is still being defined goes through a thisTypeWitness, which stays abstract until specialization.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the abstract witness described in the opcode table: the witness
// that `ThisType` conforms to the enclosing interface, which a `lookupWitness`
// stays abstract through. The witness is materialized by a requirement that
// forces such a lookup during the interface's own lowering -- here
// `associatedtype A : IBar`, whose bound has to be resolved on `This`. A
// requirement that merely mentions `This` in its signature (`This frob()`,
// kept below) needs no conformance lookup and does not on its own put the
// placeholder in the dump, so the associated type is what makes the claim
// observable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IBar
{
    int bar();
};

interface IFrob
{
    associatedtype A : IBar;
    A make(int x);
    This frob();
};

struct BarImpl : IBar
{
    int v;
    int bar() { return v; }
};

struct FrobImpl : IFrob
{
    typedef BarImpl A;
    int v;
    A make(int x) { BarImpl b; b.v = x; return b; }
    FrobImpl frob() { FrobImpl r; r.v = v + 1; return r; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int useFrob<T : IFrob>(T t, int x) { return t.make(x).bar(); }

[numthreads(1, 1, 1)]
void main()
{
    FrobImpl f;
    f.v = inVal;
    buf[0] = useFrob(f, inVal);
}

// CHECK: thisTypeWitness
