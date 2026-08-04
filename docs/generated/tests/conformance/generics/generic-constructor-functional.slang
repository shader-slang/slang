//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#examples
//META: doc_section_digest=2843e3e730992c3b4a58e5dcb338c66a5db5be57ab33d9c88e766fe31406de61
//META: purpose=A generic constructor `__init<T : IInteger>` initializes a struct from two values of type T.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Generic constructor" example: `TestStruct` has a generic
// `__init<T : IInteger>(T, T)` that sums its arguments via the IInteger
// requirements and stores the result. The aggregate `{ 123, 345 }` invokes the
// constructor with T inferred as int; CHECK asserts val == 468, proving the
// generic constructor's body ran and used the IInteger constraint.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct TestStruct
{
    int val;
    __init<T : IInteger>(T initval1, T initval2)
    {
        T tmp = initval1 + initval2;
        val = tmp.toInt();
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    TestStruct obj = { 123, 345 };
    outputBuffer[0] = obj.val;
}
// CHECK: 468
