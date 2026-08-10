//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#lookupwitness--lookupwitnessmethod
//META: doc_section_digest=7bee6071e4a281e3b0a360470ad403730021ec831c4724b2fe0ec4cfc70d7595
//META: purpose=Two distinct interface methods called on the same generic-constrained value produce two lookupWitness sites keyed by different requirement keys.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on the key operand: the same witness table serves several
// requirements, so what distinguishes two dispatches is the key, not the
// table. The CHECKs pin two lookups differing only in their second operand.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IMulti
{
    int methA(int x);
    int methB(int x);
};

struct S : IMulti
{
    int methA(int x) { return x; }
    int methB(int x) { return x + 1; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int helper<T : IMulti>(T t, int x) { return t.methA(x) + t.methB(x); }

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = helper<S>(s, inVal);
}

// CHECK: lookupWitness(%{{[0-9]+}}, %IMultix5FmethA)
// CHECK: lookupWitness(%{{[0-9]+}}, %IMultix5FmethB)
