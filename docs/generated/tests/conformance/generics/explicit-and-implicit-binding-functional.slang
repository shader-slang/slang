//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
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
