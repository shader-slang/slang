//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#examples
//META: doc_section_digest=2843e3e730992c3b4a58e5dcb338c66a5db5be57ab33d9c88e766fe31406de61
//META: purpose=A call violating an associated-type equality constraint (`where T.PropertyType == int` applied to a conformer whose PropertyType is float) is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the commented-out negative case in the doc's "Type equality
// constraint for associated type" example: `addTwoInts<T:IFace> where
// T.PropertyType == int` requires the associated type be int, but FloatProperty
// has PropertyType == float. Calling addTwoInts with FloatProperty values
// cannot satisfy the equality constraint; CHECK asserts E30440 "generic
// constraint not satisfied" fires, naming the 'T.PropertyType == int'
// constraint (non-exhaustive skips the constraint/function declaration notes).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

interface IFace
{
    associatedtype PropertyType;
    property prop : PropertyType;
}

struct FloatProperty : IFace
{
    typealias PropertyType = float;
    PropertyType prop;
}

int addTwoInts<T : IFace>(T a, T b) where T.PropertyType == int
{
    return a.prop + b.prop;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    FloatProperty f1 = { 1.0 };
    FloatProperty f2 = { 2.0 };
    outputBuffer[0] = addTwoInts(f1, f2);
//CHECK:                        ^ could not satisfy the generic constraint 'T.PropertyType == int'
    //CHECK: E30440
}
