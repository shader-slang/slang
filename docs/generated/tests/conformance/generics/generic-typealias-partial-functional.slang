//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic type alias can partially bind a generic type, leaving remaining parameters open for the user to fill in.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Generic type alias for partial type binding).
// `typealias ArrayOfFiveElements<T> = ArrayOfElements<T, 5>` fixes the size
// at 5 while leaving the element type T open.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct ArrayOfElements<T, let size : uint>
{
    T elems[size];
}

typealias ArrayOfFiveElements<T> = ArrayOfElements<T, 5>;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(5, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    ArrayOfFiveElements<float> myArray;
    myArray.elems[0] = 1.0;
    myArray.elems[1] = 2.0;
    myArray.elems[2] = 3.0;
    myArray.elems[3] = 4.0;
    myArray.elems[4] = 5.0;
    outputBuffer[id.x] = myArray.elems[id.x];
}
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 5
