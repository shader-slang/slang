//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic constructor `__init<T : IInteger>` can be parameterized over a type and used to initialize a struct from values of type T.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Generic constructor).
// `__init<T : IInteger>` combines two values into a single int by adding them.

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
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    TestStruct obj = { 123, 345 };
    outputBuffer[0] = (float)obj.val;
}
// CHECK: 468
