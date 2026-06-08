//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Mixing explicit and implicit binding: the leftmost parameter is bound explicitly while remaining parameters are inferred from the call-site arguments.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that supplying the first generic argument explicitly while letting the rest be inferred works.
// `castTo<int32_t>(src)` fixes `Ret` to `int32_t` and infers `T` as `int16_t` from the argument;
// CHECK asserts the output value 500, proving both parameters resolved correctly.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Ret is explicit, T is inferred from argument
func castTo<Ret : IInteger, T : IInteger>(T val) -> Ret
{
    return Ret(val.toInt64());
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    int16_t src = int16_t(500);
    // Ret is int32_t (explicit); T is inferred as int16_t
    outputBuffer[0] = castTo<int32_t>(src);
}
// CHECK: 500
