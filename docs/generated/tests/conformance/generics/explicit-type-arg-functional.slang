//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Explicit generic argument binding uses angle-bracket syntax to pin the return type while letting remaining parameters be inferred from the call site.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Explicit and implicit generic arguments).
// `diffSingle<int32_t>(a, b)` — Ret is explicit, T and U are inferred.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

func diffSingle<Ret : IInteger, T : IInteger, U : IInteger>(T a, U b) -> Ret
{
    return Ret(a.toInt64() - b.toInt64());
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    int8_t  a = 10;
    int16_t b = 3;
    outputBuffer[0] = diffSingle<int32_t>(a, b);
}
// CHECK: 7
