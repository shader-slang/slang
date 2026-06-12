//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A `where T.AssocType == int` equality constraint restricts instantiation to types whose associated type is exactly `int`; a conforming type can be used in the function body.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Type equality constraint for associated type).
// `addTwoInts` requires `T.PropertyType == int`. The function may access `.prop`
// as type `int` and add the values.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IFace
{
    associatedtype PropertyType;
    property prop : PropertyType { get; }
}

struct IntProperty : IFace
{
    typealias PropertyType = int;
    int _val;
    property prop : int { get { return _val; } }
}

int addTwoInts<T : IFace>(T a, T b) where T.PropertyType == int
{
    return a.prop + b.prop;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    IntProperty p1;
    p1._val = 3;
    IntProperty p2;
    p2._val = 7;
    outputBuffer[0] = addTwoInts(p1, p2);
}
// CHECK: 10
