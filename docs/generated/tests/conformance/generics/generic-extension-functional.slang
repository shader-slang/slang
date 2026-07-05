//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#examples
//META: doc_section_digest=2843e3e730992c3b4a58e5dcb338c66a5db5be57ab33d9c88e766fe31406de61
//META: purpose=Two generic extensions on the same struct with different type constraints each contribute their method only for the matching specialization.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Generic extension for generic types" example: two
// `extension<T> GenericStruct<T>` blocks, one gated on `T : IFloat` and one on
// `T : IInteger`, each add an `isInt()` method returning 0 and 1 respectively.
// The float specialization dispatches to the IFloat extension and the uint
// specialization to the IInteger extension; CHECK asserts 0 then 1, proving
// constraint-selected extension resolution.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct GenericStruct<T>
{
}

extension<T> GenericStruct<T> where T : IFloat
{
    int isInt() { return 0; }
}

extension<T> GenericStruct<T> where T : IInteger
{
    int isInt() { return 1; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    GenericStruct<float> floatObj = {};
    GenericStruct<uint> uintObj = {};
    outputBuffer[0] = floatObj.isInt();
    outputBuffer[1] = uintObj.isInt();
}
// CHECK: 0
// CHECK: 1
