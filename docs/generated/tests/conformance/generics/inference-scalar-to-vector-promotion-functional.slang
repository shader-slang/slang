//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=When inference is ambiguous between a fundamental scalar and a vector argument, the scalar is promoted to the vector type.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the promotion carve-out for ambiguous inference: `second<T>(T, T)`
// is called with a `float2` and a `float`, so T is ambiguous. The rule promotes
// the fundamental type into the vector shape, making T = float2 and splatting
// the scalar across both lanes. The body returns its second argument, so the
// result is the splatted scalar; CHECK asserts 3.0 in each lane, which is only
// possible if T resolved to float2 and the scalar was promoted rather than the
// call being rejected.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

T second<T>(T a, T b)
{
    return b;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float2 v = float2(1.0, 2.0);
    float s = 3.0;
    float2 r = second(v, s);
    outputBuffer[0] = r.x;
    outputBuffer[1] = r.y;
}
// CHECK: 3.0
// CHECK: 3.0
