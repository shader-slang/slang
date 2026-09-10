//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#generic-structure-with-type-and-value-parameters
//META: doc_section_digest=da081aceaa02998e40ea6cc22b6ac1d29064f4b32ae25b1a60fc6c17e56e7e73
//META: purpose=A generic struct with a type parameter and a value parameter can be specialized and its array field accessed per element.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Generic structure with type and value parameters" example:
// `struct TestStruct<T, let size : uint>` specialized to `<float,3>` holds a
// `float[3]` field. The CPU kernel writes each element to the buffer; CHECK
// asserts the three initializer values survive specialization intact.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct TestStruct<T, let size : uint>
{
    T arr[size];
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(3, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    TestStruct<float, 3> obj = { 10, 20, 30 };
    outputBuffer[id.x] = obj.arr[id.x];
}
// CHECK: 10.0
// CHECK: 20.0
// CHECK: 30.0
