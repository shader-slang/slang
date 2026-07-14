//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#examples
//META: doc_section_digest=2843e3e730992c3b4a58e5dcb338c66a5db5be57ab33d9c88e766fe31406de61
//META: purpose=A generic type alias partially binds a generic type, fixing one parameter and leaving the rest open.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Generic type alias for partial type binding" example:
// `typealias ArrayOfFiveElements<T> = ArrayOfElements<T, 5>` fixes the value
// parameter size=5 while leaving T open. Instantiating `<float>` yields a
// 5-element array. CHECK asserts all five initializer values are readable,
// proving the alias forwarded T and pinned size.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct ArrayOfElements<T, let size : uint>
{
    typealias ElementType = T;
    ElementType elems[size];
}

typealias ArrayOfFiveElements<T> = ArrayOfElements<T, 5>;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(5, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    ArrayOfFiveElements<float> myArray = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    outputBuffer[id.x] = myArray.elems[id.x];
}
// CHECK: 1.0
// CHECK: 2.0
// CHECK: 3.0
// CHECK: 4.0
// CHECK: 5.0
