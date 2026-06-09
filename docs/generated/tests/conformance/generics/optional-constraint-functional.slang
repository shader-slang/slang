//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=An optional conformance constraint `where optional T : IInteger` allows the expression `T is IInteger` to return `true` for integers and `false` for non-integer types.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Optional type constraint).
// `where optional T : IInteger` allows runtime dispatch via `if (T is IInteger)`.
// When T is int, it returns the sum; when T is float, it returns zero.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

func addTwoIfInts<T>(T a, T b) -> T
    where T : IArithmetic
    where optional T : IInteger
{
    if (T is IInteger)
        return a + b;
    else
        return a - a;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    // Integer case: returns a + b
    outputBuffer[0] = (float)addTwoIfInts(3, 4);
    // Float case: returns 0
    outputBuffer[1] = addTwoIfInts(1.5f, 2.5f);
}
// CHECK: 7
// CHECK: 0
