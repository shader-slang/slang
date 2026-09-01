//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#generic-type-alias-for-partial-type-binding
//META: doc_section_digest=1f7e1ab2f3df268c7c4bbe2afab1e01c09e0911bdcc4fe2de793395bd3b3dc68
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
