//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#explicit-and-implicit-generic-arguments
//META: doc_section_digest=218ee5cba4302cd1ce6a03071af653d794cd8242eba11a62d5f8dde729a692c4
//META: purpose=Mixed binding: the leftmost generic parameter is bound explicitly and the remaining parameters are inferred from arguments.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Explicit and implicit generic arguments" example:
// `diffSingle<Ret, T, U>` lists the un-inferable return type first, and the
// call `diffSingle<int32_t>(a, b)` binds Ret explicitly while T and U are
// inferred from the int8_t/int16_t arguments. CHECK asserts 3 - 5 == -2,
// proving the leftmost-explicit / rest-inferred mixing rule.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

func diffSingle<Ret : IInteger, T : IInteger, U : IInteger>(T a, U b) -> Ret
{
    return Ret(a.toInt64() - b.toInt64());
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int8_t a = 3;
    int16_t b = 5;
    outputBuffer[0] = diffSingle<int32_t>(a, b);
}
// CHECK: -2
