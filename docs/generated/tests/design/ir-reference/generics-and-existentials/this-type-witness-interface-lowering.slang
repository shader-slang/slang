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
// stays abstract through.
//
// `createThisTypeWitness` runs for every interface (slang-lower-to-ir.cpp:12139),
// so what this test can observe is not whether the witness is *created* but
// whether it survives to the dump. Measured on the commit this test was updated
// against, `associatedtype A : IBar` -- whose bound has to be resolved on `This`
// -- leaves it in the dump, and an interface whose only ThisType contact is the
// `This frob()` signature (kept below) does not. That signature-only form did
// surface the placeholder on a build from 2026-07-08 and no longer does, so the
// behaviour changed in between; which change is responsible has not been
// bisected, and the "needs no conformance lookup" reading of it is inferred
// rather than established. The associated type is used here because it is the
// form that is observable today.

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

// Anchored rather than a bare substring match: the result type ties the witness
// to IFrob, and end-of-line requires the inst to have no operand list, which is
// what makes it the abstract placeholder rather than a concrete table.
// CHECK: witness_table_t(%IFrob){{.*}}= thisTypeWitness{{$}}
