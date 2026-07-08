//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=4f712396b718dc4245b0d7a122fe7c024d74b3b4a70dd75948a651b3aa074c9c
//META: purpose=A `where T.PropertyType == int` equality constraint lets the body treat the associated type as int and add the property values.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Type equality constraint for associated type" example:
// `addTwoInts<T:IFace> where T.PropertyType == int` may add `a.prop + b.prop`
// because the equality constraint makes the associated PropertyType be int for
// all purposes. Two IntProperty values are summed; CHECK asserts 3, proving the
// associated-type equality constraint admitted the int addition in the body.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IFace
{
    associatedtype PropertyType;
    property prop : PropertyType;
}

struct IntProperty : IFace
{
    typealias PropertyType = int;
    PropertyType prop;
}

int addTwoInts<T : IFace>(T a, T b) where T.PropertyType == int
{
    return a.prop + b.prop;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IntProperty o1 = { 1 };
    IntProperty o2 = { 2 };
    outputBuffer[0] = addTwoInts(o1, o2);
}
// CHECK: 3
