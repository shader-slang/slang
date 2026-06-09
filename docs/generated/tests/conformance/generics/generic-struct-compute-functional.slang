//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Generic struct with type and value parameters can be used in a compute kernel; the specialized type stores and retrieves values correctly.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section: a generic struct with both a type param and
// a value param (`let size : uint`) stores an array and can be accessed by index.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

struct TestStruct<T, let size : uint>
{
    T arr[size];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    TestStruct<float, 3> obj;
    obj.arr[0] = 10.0;
    obj.arr[1] = 20.0;
    obj.arr[2] = 30.0;
    outputBuffer[0] = obj.arr[0] + obj.arr[1] + obj.arr[2];
}
// CHECK: 60
